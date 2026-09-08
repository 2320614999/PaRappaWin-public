#include "pr/pr_ss0_scene0_int_spu_direct.h"

#include "pr/pr_sfx.h"
#include "pr/pr_stage1_loader_cd_hal.h"
#include "audio_engine.h"
#include "vab_player.h"

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

int g_failures = 0;

#define CHECK(expr)                                                         \
    do {                                                                    \
        if (!(expr)) {                                                      \
            std::fprintf(stderr, "check failed: %s:%d: %s\n",             \
                         __FILE__, __LINE__, #expr);                        \
            ++g_failures;                                                   \
        }                                                                   \
    } while (false)

struct SsDriverCommitProbe80032B00 {
    PrSS0Scene0IntSpuDirect::SsDriverFlushState80026ECC* state = nullptr;
    int32_t result = 0;
    uint32_t callCount = 0u;
    bool sawReentryGuardSet = false;
};

int32_t CaptureSsDriverCommit80032B00(void* userData) {
    auto* probe = static_cast<SsDriverCommitProbe80032B00*>(userData);
    if (probe == nullptr) {
        return 0;
    }
    ++probe->callCount;
    probe->sawReentryGuardSet =
        probe->state != nullptr &&
        probe->state->reentryGuard800917A4 == 1;
    return probe->result;
}

void FillCompoName(std::array<uint8_t, 16>& out) {
    constexpr char kName[] = "COMPO00.INT;1";
    for (std::size_t index = 0u; index < sizeof(kName) - 1u; ++index) {
        out[index] = static_cast<uint8_t>(kName[index]);
    }
}

void WriteU16Le(std::vector<uint8_t>& bytes,
                std::size_t offset,
                uint16_t value) {
    bytes[offset + 0u] = static_cast<uint8_t>(value & 0xFFu);
    bytes[offset + 1u] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
}

void WriteU32Le(std::vector<uint8_t>& bytes,
                std::size_t offset,
                uint32_t value) {
    bytes[offset + 0u] = static_cast<uint8_t>(value & 0xFFu);
    bytes[offset + 1u] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
    bytes[offset + 2u] = static_cast<uint8_t>((value >> 16u) & 0xFFu);
    bytes[offset + 3u] = static_cast<uint8_t>((value >> 24u) & 0xFFu);
}

void WriteArchiveEntry(std::vector<uint8_t>& bytes,
                       std::size_t blockOffset,
                       uint32_t entryIndex,
                       uint32_t size,
                       const std::string& name) {
    const std::size_t entryOffset =
        blockOffset + 16u + static_cast<std::size_t>(entryIndex) * 20u;
    WriteU32Le(bytes, entryOffset, size);
    for (std::size_t index = 0u;
         index < name.size() && index < 16u;
         ++index) {
        bytes[entryOffset + 4u + index] =
            static_cast<uint8_t>(name[index]);
    }
}

std::pair<std::vector<uint8_t>, std::vector<uint8_t>>
BuildSyntheticLayeredMidiVab() {
    constexpr std::size_t kHeaderBytes = 32u;
    constexpr std::size_t kProgramBytes = 128u * 16u;
    constexpr std::size_t kToneBytes = 2u * 16u * 32u;
    constexpr std::size_t kVagTableBytes = 256u * 2u;
    constexpr std::size_t kToneBase = kHeaderBytes + kProgramBytes;
    constexpr std::size_t kVagTable = kToneBase + kToneBytes;
    std::vector<uint8_t> vh(
        kHeaderBytes + kProgramBytes + kToneBytes + kVagTableBytes,
        0u);
    std::vector<uint8_t> vb(16u, 0u);
    vh[0] = 0x70u;
    vh[1] = 0x42u;
    vh[2] = 0x41u;
    vh[3] = 0x56u;
    WriteU32Le(vh, 4u, 7u);
    WriteU16Le(vh, 18u, 2u);
    WriteU16Le(vh, 20u, 3u);
    WriteU16Le(vh, 22u, 1u);
    vh[24u] = 127u;
    constexpr std::size_t kActiveProgram = kHeaderBytes + 16u;
    vh[kHeaderBytes + 8u] = 0xFFu;
    vh[kActiveProgram + 0u] = 3u;
    vh[kActiveProgram + 1u] = 127u;
    vh[kActiveProgram + 2u] = 6u;
    vh[kActiveProgram + 3u] = 2u;
    vh[kActiveProgram + 4u] = 64u;
    vh[kActiveProgram + 8u] = 0xFFu;

    const auto writeTone = [&](std::size_t slot,
                               uint8_t noteMin,
                               uint8_t noteMax,
                               uint8_t centerNote) {
        const std::size_t offset = kToneBase + slot * 32u;
        vh[offset + 0u] = static_cast<uint8_t>(slot + 1u);
        vh[offset + 1u] = slot == 0u ? 4u : 0u;
        vh[offset + 2u] = 127u;
        vh[offset + 3u] = 64u;
        vh[offset + 4u] = centerNote;
        vh[offset + 5u] = 16u;
        vh[offset + 6u] = noteMin;
        vh[offset + 7u] = noteMax;
        WriteU16Le(
            vh, offset + 16u,
            static_cast<uint16_t>(0x1110u + slot));
        WriteU16Le(
            vh, offset + 18u,
            static_cast<uint16_t>(0x2220u + slot));
        WriteU16Le(vh, offset + 22u, 1u);
    };
    writeTone(0u, 40u, 60u, 50u);
    writeTone(1u, 50u, 70u, 60u);
    writeTone(2u, 80u, 90u, 85u);
    WriteU16Le(vh, kVagTable + 2u, 2u);
    vb[1] = 0x01u;
    return {std::move(vh), std::move(vb)};
}

std::vector<uint8_t> BuildSyntheticYCompoSpuArchive() {
    using namespace PrSS0Scene0IntLoadDirect;

    constexpr uint32_t kVhBytes = 2592u;
    constexpr uint32_t kVbBytes = 8u;
    constexpr uint32_t kVabSectors = 2u;
    constexpr std::size_t kTimBlockBytes =
        kHeaderBytes8001A8F0 + kSectorBytes8001A8F0;
    constexpr std::size_t kVabOffset = kTimBlockBytes;
    constexpr std::size_t kVabBlockBytes =
        kHeaderBytes8001A8F0 +
        static_cast<std::size_t>(kVabSectors) * kSectorBytes8001A8F0;
    constexpr std::size_t kEofOffset = kVabOffset + kVabBlockBytes;
    std::vector<uint8_t> bytes(kEofOffset + kHeaderBytes8001A8F0, 0u);

    WriteU32Le(bytes, 0u, 1u);
    WriteU32Le(bytes, 4u, kExpectedYCompoTimEntries80015618);
    WriteU32Le(bytes, 8u, 1u);
    for (uint32_t index = 0u;
         index < kExpectedYCompoTimEntries80015618;
         ++index) {
        char name[16]{};
        std::snprintf(name, sizeof(name), "T%03u.TIM", index);
        WriteArchiveEntry(bytes, 0u, index, 1u, name);
        bytes[kHeaderBytes8001A8F0 + index] =
            static_cast<uint8_t>(0x20u + index);
    }

    WriteU32Le(bytes, kVabOffset + 0u, 2u);
    WriteU32Le(bytes, kVabOffset + 4u,
               kExpectedYCompoVabEntries80015618);
    WriteU32Le(bytes, kVabOffset + 8u, kVabSectors);
    WriteArchiveEntry(bytes, kVabOffset, 0u, kVhBytes, "PRACTICE.VH");
    WriteArchiveEntry(bytes, kVabOffset, 1u, kVbBytes, "PRACTICE.VB");
    const std::size_t vhOffset = kVabOffset + kHeaderBytes8001A8F0;
    bytes[vhOffset + 0u] = 0x70u;
    bytes[vhOffset + 1u] = 0x42u;
    bytes[vhOffset + 2u] = 0x41u;
    bytes[vhOffset + 3u] = 0x56u;
    WriteU16Le(bytes, vhOffset + 18u, 0u);
    WriteU16Le(bytes, vhOffset + 20u, 0u);
    WriteU16Le(bytes, vhOffset + 22u, 1u);
    WriteU16Le(bytes, vhOffset + 2080u, 1u);
    for (std::size_t index = 0u; index < kVbBytes; ++index) {
        bytes[vhOffset + kVhBytes + index] =
            static_cast<uint8_t>(0x60u + index);
    }

    WriteU32Le(bytes, kEofOffset, 0xFFFFFFFFu);
    return bytes;
}

std::vector<uint8_t> PackMode2RawSectors(
    const std::vector<uint8_t>& userData) {
    using namespace PrStage1LoaderCdHal;

    const std::size_t sectorCount =
        (userData.size() + 2047u) / 2048u;
    std::vector<uint8_t> raw(
        sectorCount * kIso9660LookupSectorBytes800381F8, 0u);
    for (std::size_t sector = 0u; sector < sectorCount; ++sector) {
        const std::size_t sourceOffset = sector * 2048u;
        const std::size_t count = (std::min)(
            std::size_t{2048u}, userData.size() - sourceOffset);
        const std::size_t destinationOffset =
            sector * kIso9660LookupSectorBytes800381F8 +
            kIso9660LookupUserDataOffset800381F8;
        std::copy_n(userData.data() + sourceOffset,
                    count,
                    raw.data() + destinationOffset);
    }
    return raw;
}

PrSS0Scene0IntLoadDirect::YCompoSource80015618 MakeYCompoSource(
    const std::filesystem::path& discBin,
    uint32_t requestByteCount) {
    using namespace PrSS0Scene0IntLoadDirect;

    YCompoSource80015618 source{};
    source.ingressKnown = true;
    source.ingressAccepted = true;
    source.discBinPathKnown = true;
    source.discBinPath = discBin;
    source.yCompoRowKnown = true;
    source.yCompoRow.known = true;
    source.yCompoRow.psxAddr = kYCompoRowPsxAddr80015618;
    source.yCompoRow.tableIndex = kYCompoRowTableIndex80015618;
    source.yCompoRow.pathPtrA1Plus00Known = true;
    source.yCompoRow.pathPtrA1Plus00 = kYCompoPathPtr80015618;
    source.yCompoRow.loadedStateA1Plus0CKnown = true;
    source.yCompoRow.loadedStateA1Plus0C = 1;
    source.yCompoRow.startMsfKnown = true;
    source.yCompoRow.startMsf.minute = 0x00u;
    source.yCompoRow.startMsf.second = 0x02u;
    source.yCompoRow.startMsf.frame = 0x00u;
    source.yCompoRow.lengthSourceA1Plus20Known = true;
    source.yCompoRow.lengthSourceA1Plus20 = requestByteCount;
    source.yCompoRow.cdlFileNameA1Plus18Known = true;
    constexpr char kName[] = "YCOMPO.INT;1";
    for (std::size_t index = 0u; index < sizeof(kName) - 1u; ++index) {
        source.yCompoRow.cdlFileNameA1Plus18[index] =
            static_cast<uint8_t>(kName[index]);
    }
    return source;
}

std::vector<uint8_t> ReadOriginalDiscComod0(
    const std::filesystem::path& discBin) {
    PrStage1LoaderCdHal::Iso9660LookupInput800381F8 lookupInput{};
    lookupInput.valid = true;
    lookupInput.binPath = discBin;
    lookupInput.psxPath = "\\S0\\COMOD0.BIN;1";
    const auto lookup =
        PrStage1LoaderCdHal::BuildIso9660LookupFeedback800381F8(
            lookupInput);
    CHECK(lookup.success);
    if (!lookup.success) {
        return {};
    }

    PrStage1LoaderCdHal::Iso9660UserDataReadInput8001A818 readInput{};
    readInput.valid = true;
    readInput.binPath = discBin;
    readInput.lba = static_cast<int32_t>(lookup.matchedExtentLba);
    readInput.byteCount = lookup.matchedSize;
    auto read =
        PrStage1LoaderCdHal::ReadIso9660UserDataBytes8001A818(readInput);
    CHECK(read.attempted);
    CHECK(read.success);
    CHECK(read.binPathKnown && read.binPath == discBin);
    CHECK(read.lbaKnown && read.lba == readInput.lba);
    CHECK(read.byteCountKnown && read.byteCount == lookup.matchedSize);
    CHECK(read.sectorLayoutKnown);
    CHECK(read.bytes.size() == lookup.matchedSize);
    return std::move(read.bytes);
}

PrSS0Scene0IntLoadDirect::Transaction8001AC18 BuildExactInt(
    const std::filesystem::path& discBin) {
    using namespace PrSS0Scene0IntLoadDirect;
    PrStage1LoaderCdHal::Iso9660LookupInput800381F8 lookupInput{};
    lookupInput.valid = true;
    lookupInput.binPath = discBin;
    lookupInput.psxPath = "\\S0\\COMPO00.INT;1";
    lookupInput.cdlFilePtr = kCompoRowPsxAddr8001AC18 + 0x10u;
    lookupInput.pathPtr = kCompoPathPtr8001AC18;
    const auto lookup =
        PrStage1LoaderCdHal::BuildIso9660LookupFeedback800381F8(lookupInput);
    CHECK(lookup.success);

    Source8001AC18 source{};
    source.ingressKnown = true;
    source.ingressAccepted = true;
    source.discBinPathKnown = true;
    source.discBinPath = discBin;
    source.compoRowKnown = true;
    source.compoRow.known = true;
    source.compoRow.psxAddr = kCompoRowPsxAddr8001AC18;
    source.compoRow.tableIndex = kCompoRowTableIndex8001AC18;
    source.compoRow.pathPtrA1Plus00Known = true;
    source.compoRow.pathPtrA1Plus00 = kCompoPathPtr8001AC18;
    source.compoRow.loadedStateA1Plus0CKnown = true;
    source.compoRow.loadedStateA1Plus0C = 1;
    source.compoRow.startMsfKnown = true;
    source.compoRow.startMsf.minute = lookup.feedback.cdlFilePos.minute;
    source.compoRow.startMsf.second = lookup.feedback.cdlFilePos.second;
    source.compoRow.startMsf.frame = lookup.feedback.cdlFilePos.frame;
    source.compoRow.lengthSourceA1Plus20Known = true;
    source.compoRow.lengthSourceA1Plus20 = lookup.matchedSize;
    source.compoRow.cdlFileNameA1Plus18Known = true;
    source.compoRow.cdlFileNameA1Plus18 = lookup.feedback.cdlFileName;
    return BuildTransaction8001AC18(source);
}

std::unique_ptr<PrStage1LoaderMemoryDirectState> MakeResetLoader() {
    auto state = std::make_unique<PrStage1LoaderMemoryDirectState>();
    PrStage1LoaderMemoryDirectReset(*state);
    state->gpPlus320LowWater =
        PrSS0Scene0IntSideEffectDirect::kScene0PacketArenaLowWater801C4260;
    return state;
}

bool NameEquals(const std::array<uint8_t, 16>& bytes,
                std::string_view expected) {
    if (expected.size() > bytes.size()) {
        return false;
    }
    for (std::size_t index = 0u; index < expected.size(); ++index) {
        if (bytes[index] != static_cast<uint8_t>(expected[index])) {
            return false;
        }
    }
    for (std::size_t index = expected.size(); index < bytes.size(); ++index) {
        if (bytes[index] != 0u) {
            return false;
        }
    }
    return true;
}

PrSS0Scene0IntSpuDirect::PadStartComSource80026E4C
MakePadStartComSource() {
    PrSS0Scene0IntSpuDirect::PadStartComSource80026E4C source{};
    source.staticBodyKnown = true;
    source.cuePointerKnown = true;
    source.cuePointer80094410 = 0x801C6EACu;
    return source;
}

void TestSsSequenceCallbackSurface() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsSequenceCallbackSurface8002AC20 surface =
        BuildSsSequenceCallbackSurface8002AC20();
    CHECK(IsExactSsSequenceCallbackSurface8002AC20(surface));
    CHECK(surface.known);
    CHECK(surface.completeWithinLimits);
    CHECK(surface.sourceFunction == 0x8002AC20u);
    CHECK(surface.initializerFunction == 0x8002ADE0u);
    CHECK(surface.consumerFunction == 0x8002C808u);
    CHECK(surface.consumerJalr == 0x8002C8FCu);
    CHECK(surface.callbackTableBase == 0x80095D88u);
    CHECK(surface.rowCount == 32u);
    CHECK(surface.columnCount == 16u);
    CHECK(surface.callbackTargets.size() == 512u);
    CHECK(surface.callbackTableZeroInitialized);
    CHECK(surface.dword80095C4CTickRate == 60);
    CHECK(surface.dword800928CCActiveSequenceMask == 0u);
    CHECK(surface.dword800917A4ReentryGuard == 0u);
    CHECK(!surface.psxMemoryAuthority);
    CHECK(!surface.interruptCallbackAuthority);
    CHECK(!surface.dynamicReplayAuthority);
    CHECK(!surface.oldWinS0Authority);
    CHECK(!surface.stage2PlusAuthority);
    for (const uint32_t callbackTarget : surface.callbackTargets) {
        CHECK(callbackTarget == 0u);
    }

    uint32_t callbackTarget = 0xFFFFFFFFu;
    CHECK(TryResolveSsSequenceCallback8002C808(
        surface, 0, 0, callbackTarget));
    CHECK(callbackTarget == 0u);
    CHECK(TryResolveSsSequenceCallback8002C808(
        surface, 31, 15, callbackTarget));
    CHECK(callbackTarget == 0u);
    CHECK(!TryResolveSsSequenceCallback8002C808(
        surface, -1, 0, callbackTarget));
    CHECK(!TryResolveSsSequenceCallback8002C808(
        surface, 0, -1, callbackTarget));
    CHECK(!TryResolveSsSequenceCallback8002C808(
        surface, 32, 0, callbackTarget));
    CHECK(!TryResolveSsSequenceCallback8002C808(
        surface, 0, 16, callbackTarget));

    SsSequenceCallbackSurface8002AC20 mutated = surface;
    mutated.callbackTargets[17u] = 0x8002C808u;
    mutated.callbackTableZeroInitialized = false;
    CHECK(!IsExactSsSequenceCallbackSurface8002AC20(mutated));
    CHECK(!TryResolveSsSequenceCallback8002C808(
        mutated, 1, 1, callbackTarget));
}

void TestSpuTransferCompletionSurface() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SpuTransferCompletionSurface8002AC20 surface =
        BuildSpuTransferCompletionSurface8002AC20();
    CHECK(IsExactSpuTransferCompletionSurface8002AC20(surface));
    CHECK(surface.known);
    CHECK(surface.completeWithinLimits);
    CHECK(surface.sourceFunction == 0x8002AC20u);
    CHECK(surface.lowerInitFunction == 0x8002AC70u);
    CHECK(surface.transferInitFunction == 0x8002961Cu);
    CHECK(surface.installFunction == 0x8002AD38u);
    CHECK(surface.dmaCallbackWrapperFunction == 0x8002ADBCu);
    CHECK(surface.dmaChannel == 4u);
    CHECK(surface.interruptHandlerFunction == 0x80029E6Cu);
    CHECK(surface.callbackJalr == 0x80029FE0u);
    CHECK(surface.callbackSlotAddress == 0x800555FCu);
    CHECK(surface.callbackInitialValue == 0u);
    CHECK(surface.callbackArgument == 0xF0000000u);
    CHECK(surface.fallbackEventClass == 0xF0000009u);
    CHECK(surface.fallbackEventSpec == 0x20u);
    CHECK(surface.openEventMode == 0x2000u);
    CHECK(surface.openEventArgument == 0u);
    CHECK(surface.installOnceGuardAddress == 0x80055A80u);
    CHECK(surface.installOnceGuardSetValue == 1u);
    CHECK(surface.eventHandleAddress == 0x80055678u);
    CHECK(surface.spuControlOffset == 0x1AAu);
    CHECK(surface.spuControlClearMask == 0xFFCFu);
    CHECK(surface.spuControlBusyMask == 0x0030u);
    CHECK(surface.busyPollThreshold == 0x0F01u);
    CHECK(surface.preInterruptDelayLoopCount == 3u);
    CHECK(surface.preInterruptDelayIterations == 240u);
    CHECK(surface.preInterruptDelaySeed == 13u);
    CHECK(surface.preInterruptDelayMultiplier == 3u);
    CHECK(surface.preInterruptDelayGuardAddress == 0x80055614u);
    CHECK(surface.preInterruptDelayRunsWhenGuardZero);
    CHECK(surface.callbackSlotZeroInitialized);
    CHECK(surface.fallbackEventOpenedAndEnabled);
    CHECK(surface.reverbGuardFunction == 0x80030088u);
    CHECK(surface.reverbGuardSavesClearsRestoresSlot);
    CHECK(!surface.callbackWriterClosure);
    CHECK(!surface.psxMemoryAuthority);
    CHECK(!surface.dmaCallbackAuthority);
    CHECK(!surface.dynamicReplayAuthority);
    CHECK(!surface.oldWinS0Authority);
    CHECK(!surface.stage2PlusAuthority);

    SpuTransferCompletionDispatch80029E6C dispatch{};
    CHECK(TryBuildSpuTransferCompletionDispatch80029E6C(
        surface, 0u, dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.kind ==
          SpuTransferCompletionDispatchKind80029E6C::DeliverEvent);
    CHECK(dispatch.callbackTarget == 0u);
    CHECK(dispatch.callbackArgument == 0u);
    CHECK(dispatch.eventClass == 0xF0000009u);
    CHECK(dispatch.eventSpec == 0x20u);
    CHECK(dispatch.spuControlClearMask == 0xFFCFu);
    CHECK(dispatch.spuControlBusyMask == 0x0030u);
    CHECK(dispatch.busyPollThreshold == 0x0F01u);
    CHECK(!dispatch.callbackTargetAuthority);
    CHECK(!dispatch.psxMemoryAuthority);
    CHECK(!dispatch.dmaCallbackAuthority);

    CHECK(TryBuildSpuTransferCompletionDispatch80029E6C(
        surface, 0x80030088u, dispatch));
    CHECK(dispatch.kind ==
          SpuTransferCompletionDispatchKind80029E6C::Callback);
    CHECK(dispatch.callbackTarget == 0x80030088u);
    CHECK(dispatch.callbackArgument == 0xF0000000u);
    CHECK(dispatch.eventClass == 0u);
    CHECK(dispatch.eventSpec == 0u);
    CHECK(!dispatch.callbackTargetAuthority);

    SpuTransferCompletionSurface8002AC20 mutated = surface;
    mutated.callbackWriterClosure = true;
    CHECK(!IsExactSpuTransferCompletionSurface8002AC20(mutated));
    CHECK(!TryBuildSpuTransferCompletionDispatch80029E6C(
        mutated, 0u, dispatch));
    CHECK(!dispatch.known);
}

void TestSsTickInterruptSurface() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsTickInterruptSurface8002B130 surface =
        BuildSsTickInterruptSurface8002B130();
    CHECK(IsExactSsTickInterruptSurface8002B130(surface));
    CHECK(surface.known);
    CHECK(surface.completeWithinLimits);
    CHECK(surface.sourceFunction == 0x80026E4Cu);
    CHECK(surface.startFunction == 0x8002B130u);
    CHECK(surface.installFunction == 0x8002AEC8u);
    CHECK(surface.startModeArgument == 1);
    CHECK(surface.interruptCallbackApi == 0x80035774u);
    CHECK(surface.defaultHandlerSlotAddress == 0x80055DB4u);
    CHECK(surface.defaultHandlerFunction == 0x8002B200u);
    CHECK(surface.defaultHandlerStaticInitialized);
    CHECK(surface.priorHandlerSlotAddress == 0x80055DB8u);
    CHECK(surface.priorHandlerInitialValue == 0u);
    CHECK(surface.priorHandlerWrapperFunction == 0x8002B170u);
    CHECK(surface.priorHandlerJalr == 0x8002B184u);
    CHECK(surface.defaultHandlerJalrFromPriorWrapper == 0x8002B198u);
    CHECK(surface.dividerWrapperFunction == 0x8002B1B0u);
    CHECK(surface.dividerGuardSlotAddress == 0x80055DC0u);
    CHECK(surface.dividerGuardInitialValue == 0u);
    CHECK(surface.dividerHandlerJalr == 0x8002B1E8u);
    CHECK(surface.reentryGuardAddress == 0x800917A4u);
    CHECK(surface.reentryGuardInitialValue == 0u);
    CHECK(surface.activeSequenceMaskAddress == 0x800928CCu);
    CHECK(surface.activeSequenceMaskInitialValue == 0u);
    CHECK(surface.sequenceCountAddress == 0x80096588u);
    CHECK(surface.trackCountAddress == 0x80096598u);
    CHECK(surface.sequenceStatePointerTableAddress == 0x80095D08u);
    CHECK(surface.trackStateStride == 0xACu);
    CHECK(surface.trackFlagsOffset == 0x90u);
    CHECK(surface.preTickFunction == 0x80032B00u);
    CHECK(!surface.defaultHandlerWriterClosure);
    CHECK(!surface.priorHandlerWriterClosure);
    CHECK(!surface.psxMemoryAuthority);
    CHECK(!surface.interruptCallbackAuthority);
    CHECK(!surface.dynamicReplayAuthority);
    CHECK(!surface.oldWinS0Authority);
    CHECK(!surface.stage2PlusAuthority);

    const std::array<uint32_t, kSsTickFlagRouteCount8002B200>
        expectedMasks{0x01u, 0x10u, 0x20u, 0x40u,
                      0x80u, 0x02u, 0x08u, 0x04u};
    const std::array<uint32_t, kSsTickFlagRouteCount8002B200>
        expectedFunctions{
            0x8002BA9Cu, 0x8002B474u, 0x8002B750u, 0x8002DDA4u,
            0x8002DDA4u, 0x8002B9FCu, 0x8002BAC8u, 0x8002DBE4u};
    for (std::size_t index = 0u; index < surface.flagRoutes.size();
         ++index) {
        CHECK(surface.flagRoutes[index].order == index);
        CHECK(surface.flagRoutes[index].triggerMask ==
              expectedMasks[index]);
        CHECK(surface.flagRoutes[index].lowerFunction ==
              expectedFunctions[index]);
        CHECK(surface.flagRoutes[index].requiresBitOneBranch ==
              (index >= 1u && index <= 4u));
        CHECK(surface.flagRoutes[index].clearsTrackFlags ==
              (index == 7u));
    }

    SsTickInterruptDispatch8002B170 interruptDispatch{};
    CHECK(TryBuildSsTickPriorWrapperDispatch8002B170(
        surface, 0u, interruptDispatch));
    CHECK(interruptDispatch.known);
    CHECK(!interruptDispatch.callsPriorHandler);
    CHECK(interruptDispatch.priorHandlerTarget == 0u);
    CHECK(interruptDispatch.callsDefaultHandler);
    CHECK(interruptDispatch.defaultHandlerTarget == 0x8002B200u);
    CHECK(!interruptDispatch.dividerGuardKnown);
    CHECK(!interruptDispatch.priorHandlerTargetAuthority);
    CHECK(!interruptDispatch.defaultHandlerTargetAuthority);
    CHECK(!interruptDispatch.interruptCallbackAuthority);
    CHECK(!interruptDispatch.lowerCallsCommitted);

    CHECK(TryBuildSsTickPriorWrapperDispatch8002B170(
        surface, 0x80030088u, interruptDispatch));
    CHECK(interruptDispatch.callsPriorHandler);
    CHECK(interruptDispatch.priorHandlerTarget == 0x80030088u);
    CHECK(interruptDispatch.callsDefaultHandler);
    CHECK(interruptDispatch.defaultHandlerTarget == 0x8002B200u);

    CHECK(TryBuildSsTickDividerDispatch8002B1B0(
        surface, false, interruptDispatch));
    CHECK(interruptDispatch.known);
    CHECK(!interruptDispatch.callsPriorHandler);
    CHECK(!interruptDispatch.callsDefaultHandler);
    CHECK(interruptDispatch.defaultHandlerTarget == 0u);
    CHECK(interruptDispatch.dividerGuardKnown);
    CHECK(interruptDispatch.dividerGuardAfter);

    CHECK(TryBuildSsTickDividerDispatch8002B1B0(
        surface, true, interruptDispatch));
    CHECK(interruptDispatch.callsDefaultHandler);
    CHECK(interruptDispatch.defaultHandlerTarget == 0x8002B200u);
    CHECK(interruptDispatch.dividerGuardKnown);
    CHECK(!interruptDispatch.dividerGuardAfter);

    SsTickTrackObservation8002B200 observation{};
    observation.known = true;
    observation.sequenceIndex = 3;
    observation.trackIndex = 7;
    observation.flagsAtChecks.fill(0xFFu);
    SsTickTrackDispatch8002B200 trackDispatch{};
    CHECK(TryBuildSsTickTrackDispatch8002B200(
        surface, observation, trackDispatch));
    CHECK(trackDispatch.known);
    CHECK(trackDispatch.sequenceIndex == 3);
    CHECK(trackDispatch.trackIndex == 7);
    CHECK(trackDispatch.bitOneBranchEntered);
    CHECK(trackDispatch.actions.size() == 8u);
    CHECK(trackDispatch.clearsTrackFlags);
    for (std::size_t index = 0u; index < trackDispatch.actions.size();
         ++index) {
        CHECK(trackDispatch.actions[index].order == index);
        CHECK(trackDispatch.actions[index].routeOrder == index);
        CHECK(trackDispatch.actions[index].triggerMask ==
              expectedMasks[index]);
        CHECK(trackDispatch.actions[index].lowerFunction ==
              expectedFunctions[index]);
        CHECK(trackDispatch.actions[index].sequenceIndex == 3);
        CHECK(trackDispatch.actions[index].trackIndex == 7);
        CHECK(!trackDispatch.actions[index].lowerCallCommitted);
    }
    CHECK(!trackDispatch.psxMemoryAuthority);
    CHECK(!trackDispatch.interruptCallbackAuthority);
    CHECK(!trackDispatch.lowerCallsCommitted);

    observation.flagsAtChecks.fill(0u);
    observation.flagsAtChecks[1] = 0x10u;
    observation.flagsAtChecks[5] = 0x02u;
    CHECK(TryBuildSsTickTrackDispatch8002B200(
        surface, observation, trackDispatch));
    CHECK(!trackDispatch.bitOneBranchEntered);
    CHECK(trackDispatch.actions.size() == 1u);
    CHECK(trackDispatch.actions[0].triggerMask == 0x02u);
    CHECK(!trackDispatch.clearsTrackFlags);

    SsTickInterruptSurface8002B130 mutated = surface;
    mutated.flagRoutes[7].clearsTrackFlags = false;
    CHECK(!IsExactSsTickInterruptSurface8002B130(mutated));
    CHECK(!TryBuildSsTickPriorWrapperDispatch8002B170(
        mutated, 0u, interruptDispatch));
    CHECK(!interruptDispatch.known);
    CHECK(!TryBuildSsTickTrackDispatch8002B200(
        mutated, observation, trackDispatch));
    CHECK(!trackDispatch.known);
}

void TestSsMidiParserSurface() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsMidiParserSurface8002BA9C surface =
        BuildSsMidiParserSurface8002BA9C();
    CHECK(IsExactSsMidiParserSurface8002BA9C(surface));
    CHECK(surface.known);
    CHECK(surface.headerAndDeltaCompleteWithinLimits);
    CHECK(surface.tickEntryFunction == 0x8002BA9Cu);
    CHECK(surface.schedulerFunction == 0x8002BB30u);
    CHECK(surface.parserFunction == 0x8002BC40u);
    CHECK(surface.deltaDecoderFunction == 0x8002D7D0u);
    CHECK(surface.sequenceStatePointerTableAddress == 0x80095D08u);
    CHECK(surface.trackStateStride == 0xACu);
    CHECK(surface.cursorOffset == 0x04u);
    CHECK(surface.runningStatusOffset == 0x11u);
    CHECK(surface.channelOffset == 0x12u);
    CHECK(surface.absoluteTimeOffset == 0x80u);
    CHECK(surface.deltaOffset == 0x88u);
    CHECK(!surface.crossFileExplicitReferenceClosure);
    CHECK(!surface.psxMemoryAuthority);
    CHECK(!surface.lowerEventAuthority);
    CHECK(!surface.dynamicReplayAuthority);
    CHECK(!surface.oldWinS0Authority);
    CHECK(!surface.stage2PlusAuthority);

    const std::array<uint8_t, kSsMidiStatusRouteCount8002BC40>
        expectedStatusClasses{0x90u, 0xB0u, 0xC0u, 0xE0u, 0xF0u};
    const std::array<uint32_t, kSsMidiStatusRouteCount8002BC40>
        expectedLowerFunctions{
            0x8002BEECu,
            0x8002C054u,
            0x8002BFDCu,
            0x8002D3D0u,
            0x8002D47Cu,
        };
    for (std::size_t index = 0u; index < surface.statusRoutes.size();
         ++index) {
        CHECK(surface.statusRoutes[index].statusClass ==
              expectedStatusClasses[index]);
        CHECK(surface.statusRoutes[index].lowerFunction ==
              expectedLowerFunctions[index]);
        CHECK(surface.statusRoutes[index].
                  parserDecodesDeltaBeforeLower == (index == 0u));
        CHECK(surface.statusRoutes[index].lowerOwnsAdditionalBytes ==
              (index != 0u));
    }

    std::size_t deltaCursor = 0u;
    uint32_t absoluteTime = 11u;
    uint32_t deltaTicks10 = 99u;
    CHECK(TryDecodeSsMidiDelta8002D7D0(
        {0x00u}, deltaCursor, absoluteTime, deltaTicks10));
    CHECK(deltaCursor == 1u);
    CHECK(absoluteTime == 11u);
    CHECK(deltaTicks10 == 0u);

    deltaCursor = 0u;
    absoluteTime = 5u;
    CHECK(TryDecodeSsMidiDelta8002D7D0(
        {0x7Fu}, deltaCursor, absoluteTime, deltaTicks10));
    CHECK(deltaCursor == 1u);
    CHECK(absoluteTime == 1275u);
    CHECK(deltaTicks10 == 1270u);

    deltaCursor = 0u;
    absoluteTime = 0u;
    CHECK(TryDecodeSsMidiDelta8002D7D0(
        {0x81u, 0x00u}, deltaCursor, absoluteTime, deltaTicks10));
    CHECK(deltaCursor == 2u);
    CHECK(absoluteTime == 1280u);
    CHECK(deltaTicks10 == 1280u);

    deltaCursor = 0u;
    absoluteTime = 17u;
    deltaTicks10 = 55u;
    CHECK(!TryDecodeSsMidiDelta8002D7D0(
        {0x81u}, deltaCursor, absoluteTime, deltaTicks10));
    CHECK(deltaCursor == 0u);
    CHECK(absoluteTime == 17u);
    CHECK(deltaTicks10 == 0u);

    SsMidiTrackObservation8002BC40 observation{};
    observation.known = true;
    observation.sequenceIndex = 2;
    observation.trackIndex = 4;
    observation.streamBytes = {0x93u, 0x40u, 0x64u, 0x81u, 0x00u};
    observation.runningStatusBefore = 0xB0u;
    observation.channelBefore = 7u;
    observation.absoluteTimeBefore = 20u;
    SsMidiEventDispatch8002BC40 dispatch{};
    CHECK(TryBuildSsMidiEventDispatch8002BC40(
        surface, observation, dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.sequenceIndex == 2);
    CHECK(dispatch.trackIndex == 4);
    CHECK(dispatch.explicitStatusByte);
    CHECK(dispatch.eventByte == 0x93u);
    CHECK(dispatch.statusClass == 0x90u);
    CHECK(dispatch.runningStatusAfter == 0x90u);
    CHECK(dispatch.channelAfter == 3u);
    CHECK(dispatch.kind == SsMidiEventKind8002BC40::NoteOn);
    CHECK(dispatch.parserDataByteCount == 2u);
    CHECK(dispatch.parserDataBytes[0] == 0x40u);
    CHECK(dispatch.parserDataBytes[1] == 0x64u);
    CHECK(dispatch.cursorAfterParser == 5u);
    CHECK(dispatch.deltaDecoded);
    CHECK(dispatch.deltaTicks10 == 1280u);
    CHECK(dispatch.absoluteTimeAfter == 1300u);
    CHECK(dispatch.lowerFunction == 0x8002BEECu);
    CHECK(!dispatch.lowerOwnsAdditionalBytes);
    CHECK(!dispatch.lowerCallCommitted);
    CHECK(!dispatch.psxMemoryAuthority);
    CHECK(!dispatch.lowerEventAuthority);

    observation.streamBytes = {0x41u, 0x20u, 0x00u};
    observation.runningStatusBefore = 0x90u;
    observation.channelBefore = 5u;
    observation.absoluteTimeBefore = 40u;
    CHECK(TryBuildSsMidiEventDispatch8002BC40(
        surface, observation, dispatch));
    CHECK(!dispatch.explicitStatusByte);
    CHECK(dispatch.statusClass == 0x90u);
    CHECK(dispatch.runningStatusAfter == 0x90u);
    CHECK(dispatch.channelAfter == 5u);
    CHECK(dispatch.parserDataBytes[0] == 0x41u);
    CHECK(dispatch.parserDataBytes[1] == 0x20u);
    CHECK(dispatch.cursorAfterParser == 3u);
    CHECK(dispatch.deltaDecoded);
    CHECK(dispatch.deltaTicks10 == 0u);
    CHECK(dispatch.absoluteTimeAfter == 40u);

    observation.streamBytes = {0xC2u, 0x05u, 0x7Fu};
    observation.runningStatusBefore = 0x90u;
    observation.channelBefore = 1u;
    CHECK(TryBuildSsMidiEventDispatch8002BC40(
        surface, observation, dispatch));
    CHECK(dispatch.kind == SsMidiEventKind8002BC40::ProgramChange);
    CHECK(dispatch.runningStatusAfter == 0xC0u);
    CHECK(dispatch.channelAfter == 2u);
    CHECK(dispatch.parserDataByteCount == 1u);
    CHECK(dispatch.parserDataBytes[0] == 0x05u);
    CHECK(dispatch.cursorAfterParser == 2u);
    CHECK(!dispatch.deltaDecoded);
    CHECK(dispatch.lowerFunction == 0x8002BFDCu);
    CHECK(dispatch.lowerOwnsAdditionalBytes);

    observation.streamBytes = {0x07u, 0x64u};
    observation.runningStatusBefore = 0xB0u;
    observation.channelBefore = 6u;
    CHECK(TryBuildSsMidiEventDispatch8002BC40(
        surface, observation, dispatch));
    CHECK(dispatch.kind == SsMidiEventKind8002BC40::ControlChange);
    CHECK(dispatch.parserDataBytes[0] == 0x07u);
    CHECK(dispatch.cursorAfterParser == 1u);
    CHECK(dispatch.lowerFunction == 0x8002C054u);
    CHECK(dispatch.lowerOwnsAdditionalBytes);

    observation.streamBytes = {0xE1u, 0x20u, 0x40u};
    CHECK(TryBuildSsMidiEventDispatch8002BC40(
        surface, observation, dispatch));
    CHECK(dispatch.kind == SsMidiEventKind8002BC40::PitchBend);
    CHECK(dispatch.parserDataBytes[0] == 0x20u);
    CHECK(dispatch.cursorAfterParser == 2u);
    CHECK(dispatch.lowerFunction == 0x8002D3D0u);
    CHECK(dispatch.lowerOwnsAdditionalBytes);

    observation.streamBytes = {0xFFu, 0x2Fu};
    CHECK(TryBuildSsMidiEventDispatch8002BC40(
        surface, observation, dispatch));
    CHECK(dispatch.kind == SsMidiEventKind8002BC40::Meta);
    CHECK(dispatch.statusClass == 0xF0u);
    CHECK(dispatch.runningStatusAfter == 0xFFu);
    CHECK(dispatch.channelAfter == 0x0Fu);
    CHECK(dispatch.parserDataBytes[0] == 0x2Fu);
    CHECK(dispatch.cursorAfterParser == 2u);
    CHECK(dispatch.lowerFunction == 0x8002D47Cu);
    CHECK(dispatch.lowerOwnsAdditionalBytes);

    observation.streamBytes = {0xA4u, 0x01u};
    observation.runningStatusBefore = 0x90u;
    observation.channelBefore = 2u;
    CHECK(TryBuildSsMidiEventDispatch8002BC40(
        surface, observation, dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.kind == SsMidiEventKind8002BC40::Unsupported);
    CHECK(dispatch.statusClass == 0xA0u);
    CHECK(dispatch.runningStatusAfter == 0x90u);
    CHECK(dispatch.channelAfter == 4u);
    CHECK(dispatch.cursorAfterParser == 1u);
    CHECK(dispatch.lowerFunction == 0u);

    observation.streamBytes = {0x90u, 0x40u};
    CHECK(!TryBuildSsMidiEventDispatch8002BC40(
        surface, observation, dispatch));
    CHECK(!dispatch.known);

    SsMidiParserSurface8002BA9C mutated = surface;
    mutated.deltaOffset = 0x84u;
    CHECK(!IsExactSsMidiParserSurface8002BA9C(mutated));
    observation.streamBytes = {0x90u, 0x40u, 0x7Fu, 0x00u};
    CHECK(!TryBuildSsMidiEventDispatch8002BC40(
        mutated, observation, dispatch));
    CHECK(!dispatch.known);
}

void TestSsMidiScheduler8002BB30() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsMidiParserSurface8002BA9C surface =
        BuildSsMidiParserSurface8002BA9C();
    SsMidiSchedulerObservation8002BB30 observation{};
    observation.known = true;
    observation.sequenceIndex = 2;
    observation.trackIndex = 5;
    observation.wordTrackOffset6E = 0;
    observation.wordTrackOffset70 = 10;
    observation.dwordTrackOffset88 = 5u;
    observation.parserDeltaAfterCalls = {0u, 3u, 2u};

    SsMidiSchedulerDispatch8002BB30 dispatch{};
    CHECK(TryBuildSsMidiSchedulerDispatch8002BB30(
        surface, observation, dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.branch ==
          SsMidiSchedulerBranch8002BB30::DueEventParserLoop);
    CHECK(dispatch.sequenceIndex == 2);
    CHECK(dispatch.trackIndex == 5);
    CHECK(dispatch.wordTrackOffset6EBefore == 0);
    CHECK(dispatch.wordTrackOffset6EAfter == 0);
    CHECK(dispatch.wordTrackOffset70 == 10);
    CHECK(dispatch.dwordTrackOffset88Before == 5u);
    CHECK(dispatch.dwordTrackOffset88After == 0u);
    CHECK(dispatch.result == 0);
    CHECK(dispatch.parserCallCount == 3u);
    CHECK(dispatch.parserDeltaAfterCalls.size() == 3u);
    CHECK(dispatch.parserCallsBounded);
    CHECK(!dispatch.parserInputExhausted);
    CHECK(!dispatch.psxMemoryAuthority);
    CHECK(!dispatch.lowerEventAuthority);
    CHECK(!dispatch.oldWinS0Authority);
    CHECK(!dispatch.stage2PlusAuthority);

    observation.wordTrackOffset6E = 3;
    observation.wordTrackOffset70 = 10;
    observation.dwordTrackOffset88 = 20u;
    observation.parserDeltaAfterCalls.clear();
    CHECK(TryBuildSsMidiSchedulerDispatch8002BB30(
        surface, observation, dispatch));
    CHECK(dispatch.branch ==
          SsMidiSchedulerBranch8002BB30::PositiveCountdownDecrement);
    CHECK(dispatch.wordTrackOffset6EAfter == 2);
    CHECK(dispatch.dwordTrackOffset88After == 20u);
    CHECK(dispatch.result == 2);
    CHECK(dispatch.parserCallCount == 0u);

    observation.wordTrackOffset6E = -1;
    CHECK(TryBuildSsMidiSchedulerDispatch8002BB30(
        surface, observation, dispatch));
    CHECK(dispatch.branch ==
          SsMidiSchedulerBranch8002BB30::NegativeCountdownRebase);
    CHECK(dispatch.wordTrackOffset6EAfter == -1);
    CHECK(dispatch.dwordTrackOffset88After == 10u);
    CHECK(dispatch.result == -1);

    observation.wordTrackOffset6E = 0;
    CHECK(TryBuildSsMidiSchedulerDispatch8002BB30(
        surface, observation, dispatch));
    CHECK(dispatch.branch ==
          SsMidiSchedulerBranch8002BB30::CountdownArmAndDeltaDecrement);
    CHECK(dispatch.wordTrackOffset6EAfter == 10);
    CHECK(dispatch.dwordTrackOffset88After == 19u);
    CHECK(dispatch.result == 19);

    observation.wordTrackOffset70 = 10;
    observation.dwordTrackOffset88 = 5u;
    observation.parserDeltaAfterCalls = {0u};
    CHECK(!TryBuildSsMidiSchedulerDispatch8002BB30(
        surface, observation, dispatch));
    CHECK(!dispatch.known);
    CHECK(dispatch.parserInputExhausted);
    CHECK(!dispatch.parserCallsBounded);

    SsMidiParserSurface8002BA9C mutated = surface;
    mutated.schedulerFunction = 0x8002BC40u;
    observation.parserDeltaAfterCalls = {1u};
    CHECK(!TryBuildSsMidiSchedulerDispatch8002BB30(
        mutated, observation, dispatch));
    CHECK(!dispatch.known);
}

void TestSsMidiProgramChange8002BFDC() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsMidiParserSurface8002BA9C surface =
        BuildSsMidiParserSurface8002BA9C();
    SsMidiProgramChangeObservation8002BFDC observation{};
    observation.known = true;
    observation.sequenceIndex = 1;
    observation.trackIndex = 4;
    observation.channel = 3u;
    observation.program = 12u;
    observation.channelProgramsBefore.fill(7u);
    observation.deltaStreamBytes = {0x81u, 0x00u};
    observation.deltaCursorBefore = 0u;
    observation.absoluteTimeBefore = 20u;

    SsMidiProgramChangeDispatch8002BFDC dispatch{};
    CHECK(TryBuildSsMidiProgramChangeDispatch8002BFDC(
        surface, observation, dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.sequenceIndex == 1);
    CHECK(dispatch.trackIndex == 4);
    CHECK(dispatch.channel == 3u);
    CHECK(dispatch.program == 12u);
    CHECK(dispatch.channelProgramsAfter[3] == 12u);
    CHECK(dispatch.channelProgramsAfter[2] == 7u);
    CHECK(dispatch.programTableOffset == 0x2Fu);
    CHECK(dispatch.deltaCursorAfter == 2u);
    CHECK(dispatch.deltaTicks10 == 1280u);
    CHECK(dispatch.absoluteTimeAfter == 1300u);
    CHECK(dispatch.dwordTrackOffset88After == 1280u);
    CHECK(dispatch.lowerFunction == 0x8002BFDCu);
    CHECK(dispatch.deltaDecoderFunction == 0x8002D7D0u);
    CHECK(!dispatch.lowerCallCommitted);
    CHECK(!dispatch.psxMemoryAuthority);
    CHECK(!dispatch.lowerEventAuthority);
    CHECK(!dispatch.oldWinS0Authority);
    CHECK(!dispatch.stage2PlusAuthority);

    observation.deltaStreamBytes = {0x81u};
    CHECK(!TryBuildSsMidiProgramChangeDispatch8002BFDC(
        surface, observation, dispatch));
    CHECK(!dispatch.known);

    observation.deltaStreamBytes = {0x00u};
    observation.channel = 16u;
    CHECK(!TryBuildSsMidiProgramChangeDispatch8002BFDC(
        surface, observation, dispatch));
    CHECK(!dispatch.known);
}

void TestSsMidiControlChange8002C054() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsMidiParserSurface8002BA9C surface =
        BuildSsMidiParserSurface8002BA9C();
    SsMidiControlChangeObservation8002C054 observation{};
    observation.known = true;
    observation.sequenceIndex = 2;
    observation.trackIndex = 3;
    observation.channel = 4u;
    observation.controller = 7u;
    observation.value = 99u;
    observation.vabId = 1;
    observation.channelPrograms.fill(5u);
    observation.channelVolumes.fill(80u);
    observation.channelPans.fill(64u);
    observation.deltaStreamBytes = {0x00u};
    observation.absoluteTimeBefore = 30u;

    SsMidiControlChangeDispatch8002C054 dispatch{};
    CHECK(TryBuildSsMidiControlChangeDispatch8002C054(
        surface, observation, dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.branch ==
          SsMidiControlChangeBranch8002C054::ChannelVolumeStore);
    CHECK(dispatch.channel == 4u);
    CHECK(dispatch.controller == 7u);
    CHECK(dispatch.value == 99u);
    CHECK(dispatch.channelVolumesAfter[4] == 99u);
    CHECK(dispatch.channelPansAfter[4] == 64u);
    CHECK(dispatch.callsGsVoiceUpdate80033D08);
    CHECK(dispatch.gsVoiceUpdateFunction == 0x80033D08u);
    CHECK(dispatch.packedSequenceTrack == 0x0302u);
    CHECK(dispatch.gsVoiceUpdateVabId == 1);
    CHECK(dispatch.gsVoiceUpdateProgram == 5u);
    CHECK(dispatch.gsVoiceUpdateVolume == 99u);
    CHECK(dispatch.gsVoiceUpdatePan == 64u);
    CHECK(dispatch.deltaDecoded);
    CHECK(dispatch.deltaTicks10 == 0u);
    CHECK(dispatch.absoluteTimeAfter == 30u);
    CHECK(dispatch.dwordTrackOffset88After == 0u);
    CHECK(!dispatch.lowerCallCommitted);
    CHECK(!dispatch.psxMemoryAuthority);
    CHECK(!dispatch.lowerEventAuthority);
    CHECK(!dispatch.oldWinS0Authority);
    CHECK(!dispatch.stage2PlusAuthority);

    observation.controller = 10u;
    observation.value = 33u;
    observation.channelVolumes[4] = 99u;
    observation.deltaStreamBytes = {0x7Fu};
    CHECK(TryBuildSsMidiControlChangeDispatch8002C054(
        surface, observation, dispatch));
    CHECK(dispatch.branch ==
          SsMidiControlChangeBranch8002C054::ChannelPanStore);
    CHECK(dispatch.channelPansAfter[4] == 33u);
    CHECK(dispatch.gsVoiceUpdateVolume == 99u);
    CHECK(dispatch.gsVoiceUpdatePan == 33u);
    CHECK(dispatch.deltaTicks10 == 1270u);
    CHECK(dispatch.absoluteTimeAfter == 1300u);

    observation.controller = 11u;
    observation.value = 64u;
    observation.deltaStreamBytes = {0x00u};
    CHECK(TryBuildSsMidiControlChangeDispatch8002C054(
        surface, observation, dispatch));
    CHECK(dispatch.branch ==
          SsMidiControlChangeBranch8002C054::ProgramVolumeUpdate);
    CHECK(dispatch.callsSsVmSetProgVol);
    CHECK(dispatch.ssVmSetProgVolFunction == 0u);
    CHECK(dispatch.callsGsVoiceUpdate80033D08);

    observation.controller = 64u;
    observation.value = 0x3Fu;
    CHECK(TryBuildSsMidiControlChangeDispatch8002C054(
        surface, observation, dispatch));
    CHECK(dispatch.branch ==
          SsMidiControlChangeBranch8002C054::Threshold64Dispatch);
    CHECK(dispatch.thresholdFunction == 0x8002EFD0u);
    observation.value = 0x40u;
    CHECK(TryBuildSsMidiControlChangeDispatch8002C054(
        surface, observation, dispatch));
    CHECK(dispatch.thresholdFunction == 0x8002EFE0u);

    observation.controller = 91u;
    observation.value = 17u;
    CHECK(TryBuildSsMidiControlChangeDispatch8002C054(
        surface, observation, dispatch));
    CHECK(dispatch.branch ==
          SsMidiControlChangeBranch8002C054::Timer91Dispatch);
    CHECK(dispatch.timerFunction == 0x800302A4u);
    CHECK(dispatch.timerArg0 == 17u && dispatch.timerArg1 == 17u);

    observation.controller = 6u;
    observation.value = 2u;
    observation.deltaStreamBytes.clear();
    CHECK(TryBuildSsMidiControlChangeDispatch8002C054(
        surface, observation, dispatch));
    CHECK(dispatch.branch ==
          SsMidiControlChangeBranch8002C054::LowerOwnsAdditionalBytes);
    CHECK(dispatch.lowerFunction == 0x8002CB6Cu);
    CHECK(dispatch.lowerOwnsAdditionalBytes);
    CHECK(!dispatch.deltaDecoded);

    observation.controller = 0u;
    observation.value = 3u;
    observation.deltaStreamBytes = {0x81u};
    CHECK(!TryBuildSsMidiControlChangeDispatch8002C054(
        surface, observation, dispatch));
    CHECK(!dispatch.known);
}

void TestSsMidiPitchBend8002D3D0() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsMidiParserSurface8002BA9C surface =
        BuildSsMidiParserSurface8002BA9C();
    SsMidiPitchBendObservation8002D3D0 observation{};
    observation.known = true;
    observation.sequenceIndex = 4;
    observation.trackIndex = 2;
    observation.channel = 6u;
    observation.bendValue = 0x55u;
    observation.vabId = 1;
    observation.channelPrograms.fill(9u);
    observation.deltaStreamBytes = {0x81u, 0x00u};
    observation.absoluteTimeBefore = 7u;

    SsMidiPitchBendDispatch8002D3D0 dispatch{};
    CHECK(TryBuildSsMidiPitchBendDispatch8002D3D0(
        surface, observation, dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.sequenceIndex == 4);
    CHECK(dispatch.trackIndex == 2);
    CHECK(dispatch.channel == 6u);
    CHECK(dispatch.bendValue == 0x55u);
    CHECK(dispatch.packedSequenceTrack == 0x0204u);
    CHECK(dispatch.voiceUpdateVabId == 1);
    CHECK(dispatch.voiceUpdateProgram == 9u);
    CHECK(dispatch.voiceUpdateValue == 0x55u);
    CHECK(dispatch.voiceUpdateFunction == 0x80032A10u);
    CHECK(dispatch.lowerFunction == 0x8002D3D0u);
    CHECK(dispatch.deltaDecoderFunction == 0x8002D7D0u);
    CHECK(dispatch.deltaCursorAfter == 2u);
    CHECK(dispatch.deltaTicks10 == 1280u);
    CHECK(dispatch.absoluteTimeAfter == 1287u);
    CHECK(dispatch.dwordTrackOffset88After == 1280u);
    CHECK(!dispatch.lowerCallCommitted);
    CHECK(!dispatch.psxMemoryAuthority);
    CHECK(!dispatch.lowerEventAuthority);
    CHECK(!dispatch.oldWinS0Authority);
    CHECK(!dispatch.stage2PlusAuthority);

    observation.deltaStreamBytes = {0x81u};
    CHECK(!TryBuildSsMidiPitchBendDispatch8002D3D0(
        surface, observation, dispatch));
    CHECK(!dispatch.known);

    observation.channel = 16u;
    observation.deltaStreamBytes = {0x00u};
    CHECK(!TryBuildSsMidiPitchBendDispatch8002D3D0(
        surface, observation, dispatch));
    CHECK(!dispatch.known);
}

void TestSsMidiMeta8002D47C() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsMidiParserSurface8002BA9C surface =
        BuildSsMidiParserSurface8002BA9C();
    SsMidiMetaObservation8002D47C observation{};
    observation.known = true;
    observation.sequenceIndex = 1;
    observation.trackIndex = 2;
    observation.metaType = 0x2Fu;
    observation.wordTrackOffset70 = 4;
    observation.wordTrackOffset72 = 3u;
    observation.dwordTrackOffset08StreamStart = 0x1000u;
    observation.dwordTrackOffset04Cursor = 0x2000u;
    observation.dwordTrackOffset0C = 0x3000u;
    observation.dwordTrackOffset80AbsoluteTime = 77u;
    observation.dwordTrackOffset88Delta = 5u;
    observation.dwordTrackOffset90Flags = 0x20Fu;
    observation.byteTrackOffset00 = 7u;
    observation.byteTrackOffset27 = 9u;
    observation.byteTrackOffset2B = 8u;
    observation.byteTrackOffset3C = 2u;

    SsMidiMetaDispatch8002D47C dispatch{};
    CHECK(TryBuildSsMidiMetaDispatch8002D47C(
        surface, observation, dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.branch == SsMidiMetaBranch8002D47C::EndOfTrackComplete);
    CHECK(dispatch.wordTrackOffset72After == 4u);
    CHECK(dispatch.wordTrackOffset6EAfter == 0u);
    CHECK(dispatch.wordTrackOffset70After == 4u);
    CHECK(dispatch.dwordTrackOffset04After == 0x2000u);
    CHECK(dispatch.dwordTrackOffset0CAfter == 0x1000u);
    CHECK(dispatch.dwordTrackOffset80After == 77u);
    CHECK(dispatch.dwordTrackOffset88After == 4u);
    CHECK(dispatch.dwordTrackOffset90After == 0x204u);
    CHECK(dispatch.byteTrackOffset27After == 9u);
    CHECK(dispatch.byteTrackOffset2BAfter == 0u);
    CHECK(dispatch.callsTrackEndCleanup);
    CHECK(dispatch.trackEndCleanupFunction == 0x8002D970u);
    CHECK(dispatch.trackEndCleanupArg == 2u);
    CHECK(dispatch.trackEndCleanupArg1 == 7u);
    CHECK(dispatch.callsSequenceTrackEnd == 2u);
    CHECK(dispatch.sequenceTrackEndFunction == 0x80033A34u);
    CHECK(dispatch.result == 4u);
    CHECK(!dispatch.deltaDecoded);
    CHECK(!dispatch.lowerCallCommitted);
    CHECK(!dispatch.psxMemoryAuthority);
    CHECK(!dispatch.lowerEventAuthority);
    CHECK(!dispatch.oldWinS0Authority);
    CHECK(!dispatch.stage2PlusAuthority);

    observation.wordTrackOffset72 = 1u;
    observation.byteTrackOffset3C = 0xFFu;
    CHECK(TryBuildSsMidiMetaDispatch8002D47C(
        surface, observation, dispatch));
    CHECK(dispatch.branch == SsMidiMetaBranch8002D47C::EndOfTrackReset);
    CHECK(dispatch.wordTrackOffset72After == 2u);
    CHECK(dispatch.dwordTrackOffset04After == 0x1000u);
    CHECK(dispatch.dwordTrackOffset0CAfter == 0x1000u);
    CHECK(dispatch.dwordTrackOffset80After == 0u);
    CHECK(dispatch.dwordTrackOffset88After == 0u);
    CHECK(dispatch.byteTrackOffset27After == 0u);
    CHECK(dispatch.byteTrackOffset2BAfter == 8u);

    observation.metaType = 0x01u;
    CHECK(TryBuildSsMidiMetaDispatch8002D47C(
        surface, observation, dispatch));
    CHECK(dispatch.branch == SsMidiMetaBranch8002D47C::UnsupportedResult81);
    CHECK(dispatch.result == 0x51u);
    CHECK(dispatch.dwordTrackOffset80After == 77u);

    observation.metaType = 0x51u;
    observation.wordTrackOffset74 = 24u;
    observation.tickRate80095C4C = 60u;
    observation.tempoBytes = {0x07u, 0xA1u, 0x20u};
    observation.deltaStreamBytes = {0x00u};
    observation.deltaCursorBefore = 0u;
    observation.absoluteTimeBefore = 20u;
    CHECK(TryBuildSsMidiMetaDispatch8002D47C(
        surface, observation, dispatch));
    CHECK(dispatch.branch == SsMidiMetaBranch8002D47C::TempoUpdate);
    CHECK(dispatch.tempoMicrosecondsPerQuarter == 500000u);
    CHECK(dispatch.tempoBpm == 120u);
    CHECK(dispatch.dwordTrackOffset8CAfter == 120u);
    CHECK(dispatch.wordTrackOffset6EAfter == 0xFFFFu);
    CHECK(dispatch.wordTrackOffset70After == 8u);
    CHECK(dispatch.deltaDecoded);
    CHECK(dispatch.deltaTicks10 == 0u);
    CHECK(dispatch.absoluteTimeAfter == 20u);

    observation.tempoBytes = {0x00u, 0x00u, 0x00u};
    CHECK(!TryBuildSsMidiMetaDispatch8002D47C(
        surface, observation, dispatch));
    CHECK(!dispatch.known);
}

void TestSsMidiCallback8002C808() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsMidiParserSurface8002BA9C surface =
        BuildSsMidiParserSurface8002BA9C();
    SsMidiCallbackObservation8002C808 observation{};
    observation.known = true;
    observation.sequenceIndex = 2;
    observation.trackIndex = 3;
    observation.callbackValue = 7u;
    observation.byteTrackOffset16 = 0u;
    observation.byteTrackOffset21 = 1u;
    observation.byteTrackOffset22 = 40u;
    observation.byteTrackOffset39 = 1u;
    observation.byteTrackOffset40 = 0u;
    observation.byteTrackOffset41 = 0u;
    observation.byteTrackOffset42 = 2u;
    observation.callbackTarget = 0x80123456u;
    observation.deltaStreamBytes = {0x02u};
    observation.absoluteTimeBefore = 100u;

    SsMidiCallbackDispatch8002C808 dispatch{};
    CHECK(TryBuildSsMidiCallbackDispatch8002C808(
        surface, observation, dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.branch == SsMidiCallbackBranch8002C808::ArmAndDecode);
    CHECK(dispatch.sequenceIndex == 2);
    CHECK(dispatch.trackIndex == 3);
    CHECK(dispatch.byteTrackOffset16After == 1u);
    CHECK(dispatch.byteTrackOffset21After == 7u);
    CHECK(dispatch.byteTrackOffset40After == 7u);
    CHECK(dispatch.byteTrackOffset42After == 3u);
    CHECK(dispatch.callbackTarget == 0x80123456u);
    CHECK(dispatch.callbackTargetPresent);
    CHECK(dispatch.callbackDispatchObserved);
    CHECK(!dispatch.callbackCallCommitted);
    CHECK(dispatch.deltaDecoded);
    CHECK(dispatch.deltaTicks10 == 20u);
    CHECK(dispatch.deltaCursorAfter == 1u);
    CHECK(dispatch.absoluteTimeAfter == 120u);
    CHECK(dispatch.dwordTrackOffset88After == 20u);
    CHECK(dispatch.lowerFunction == 0x8002C808u);
    CHECK(dispatch.deltaDecoderFunction == 0x8002D7D0u);
    CHECK(!dispatch.lowerCallCommitted);
    CHECK(!dispatch.psxMemoryAuthority);
    CHECK(!dispatch.lowerEventAuthority);
    CHECK(!dispatch.oldWinS0Authority);
    CHECK(!dispatch.stage2PlusAuthority);

    observation.byteTrackOffset39 = 0u;
    observation.byteTrackOffset16 = 1u;
    observation.byteTrackOffset22 = 30u;
    observation.byteTrackOffset21 = 9u;
    observation.byteTrackOffset40 = 8u;
    observation.byteTrackOffset42 = 0xFFu;
    observation.deltaStreamBytes = {0x00u};
    observation.absoluteTimeBefore = 55u;
    CHECK(TryBuildSsMidiCallbackDispatch8002C808(
        surface, observation, dispatch));
    CHECK(dispatch.branch ==
          SsMidiCallbackBranch8002C808::Mode20Or30Decode);
    CHECK(dispatch.byteTrackOffset16After == 1u);
    CHECK(dispatch.byteTrackOffset21After == 9u);
    CHECK(dispatch.byteTrackOffset40After == 8u);
    CHECK(dispatch.byteTrackOffset42After == 0xFFu);
    CHECK(!dispatch.callbackDispatchObserved);
    CHECK(dispatch.deltaTicks10 == 0u);
    CHECK(dispatch.absoluteTimeAfter == 55u);

    observation.byteTrackOffset22 = 40u;
    observation.byteTrackOffset42 = 0xFFu;
    observation.deltaStreamBytes = {0x01u};
    CHECK(TryBuildSsMidiCallbackDispatch8002C808(
        surface, observation, dispatch));
    CHECK(dispatch.branch ==
          SsMidiCallbackBranch8002C808::AccumulateAndCallback);
    CHECK(dispatch.byteTrackOffset42After == 0u);
    CHECK(dispatch.callbackDispatchObserved);

    observation.deltaStreamBytes.clear();
    CHECK(!TryBuildSsMidiCallbackDispatch8002C808(
        surface, observation, dispatch));
    CHECK(!dispatch.known);
}

void TestSsMidiCallbackGate8002C938() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsMidiParserSurface8002BA9C surface =
        BuildSsMidiParserSurface8002BA9C();
    SsMidiCallbackGateObservation8002C938 observation{};
    observation.known = true;
    observation.sequenceIndex = 1;
    observation.trackIndex = 4;
    observation.inputValue = 20u;
    observation.byteTrackOffset16 = 0u;
    observation.byteTrackOffset22 = 0u;
    observation.byteTrackOffset39 = 0u;
    observation.byteTrackOffset40 = 0u;
    observation.byteTrackOffset42 = 8u;
    observation.deltaStreamBytes = {0x01u};

    SsMidiCallbackGateDispatch8002C938 dispatch{};
    CHECK(TryBuildSsMidiCallbackGateDispatch8002C938(
        surface, observation, dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.branch ==
          SsMidiCallbackGateBranch8002C938::Mode20Arm);
    CHECK(dispatch.byteTrackOffset22After == 20u);
    CHECK(dispatch.byteTrackOffset39After == 1u);
    CHECK(dispatch.byteTrackOffset40After == 0u);
    CHECK(dispatch.deltaTicks10 == 10u);
    CHECK(dispatch.dwordTrackOffset88After == 10u);
    CHECK(dispatch.result == 10u);
    CHECK(dispatch.lowerFunction == 0x8002C938u);

    observation.inputValue = 30u;
    observation.byteTrackOffset16 = 1u;
    observation.byteTrackOffset40 = 0u;
    observation.byteTrackOffset22 = 20u;
    observation.deltaStreamBytes = {0x00u};
    CHECK(TryBuildSsMidiCallbackGateDispatch8002C938(
        surface, observation, dispatch));
    CHECK(dispatch.branch ==
          SsMidiCallbackGateBranch8002C938::Mode30ClearGate);
    CHECK(dispatch.byteTrackOffset22After == 30u);
    CHECK(dispatch.byteTrackOffset16After == 0u);
    CHECK(dispatch.deltaTicks10 == 0u);

    observation.byteTrackOffset40 = 2u;
    observation.dwordTrackOffset0CStoredCursor = 0x1234u;
    observation.dwordTrackOffset04Cursor = 0x5678u;
    observation.deltaStreamBytes = {0x02u};
    CHECK(TryBuildSsMidiCallbackGateDispatch8002C938(
        surface, observation, dispatch));
    CHECK(dispatch.branch ==
          SsMidiCallbackGateBranch8002C938::Mode30RetainPending);
    CHECK(dispatch.byteTrackOffset40After == 1u);
    CHECK(dispatch.dwordTrackOffset04After == 0x1234u);
    CHECK(dispatch.dwordTrackOffset88After == 20u);

    observation.byteTrackOffset40 = 0x7Fu;
    observation.deltaStreamBytes = {0x03u};
    CHECK(TryBuildSsMidiCallbackGateDispatch8002C938(
        surface, observation, dispatch));
    CHECK(dispatch.branch ==
          SsMidiCallbackGateBranch8002C938::Mode30FinishPending);
    CHECK(dispatch.dwordTrackOffset04After == 0x1234u);
    CHECK(dispatch.dwordTrackOffset88After == 0u);
    CHECK(dispatch.result == 0u);

    observation.inputValue = 77u;
    observation.byteTrackOffset40 = 9u;
    observation.byteTrackOffset42 = 0xFFu;
    observation.deltaStreamBytes = {0x01u};
    CHECK(TryBuildSsMidiCallbackGateDispatch8002C938(
        surface, observation, dispatch));
    CHECK(dispatch.branch ==
          SsMidiCallbackGateBranch8002C938::ModeOtherAccumulate);
    CHECK(dispatch.byteTrackOffset22After == 77u);
    CHECK(dispatch.byteTrackOffset42After == 0u);

    observation.deltaStreamBytes.clear();
    CHECK(!TryBuildSsMidiCallbackGateDispatch8002C938(
        surface, observation, dispatch));
    CHECK(!dispatch.known);
}

void TestSsMidiTrackByteCounters8002CA7C8002CAF4() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsMidiParserSurface8002BA9C surface =
        BuildSsMidiParserSurface8002BA9C();
    SsMidiTrackByteCounterObservation8002CA7C ca7c{};
    ca7c.known = true;
    ca7c.sequenceIndex = 3;
    ca7c.trackIndex = 1;
    ca7c.inputValue = 0x2Au;
    ca7c.byteTrackOffset19 = 9u;
    ca7c.byteTrackOffset41 = 0xFFu;
    ca7c.deltaStreamBytes = {0x02u};
    ca7c.absoluteTimeBefore = 40u;
    SsMidiTrackByteCounterDispatch8002CA7C ca7cDispatch{};
    CHECK(TryBuildSsMidiTrackByteCounterDispatch8002CA7C(
        surface, ca7c, ca7cDispatch));
    CHECK(ca7cDispatch.known);
    CHECK(ca7cDispatch.sequenceIndex == 3);
    CHECK(ca7cDispatch.trackIndex == 1);
    CHECK(ca7cDispatch.inputValue == 0x2Au);
    CHECK(ca7cDispatch.byteTrackOffset19After == 0x2Au);
    CHECK(ca7cDispatch.byteTrackOffset41After == 0u);
    CHECK(ca7cDispatch.deltaDecoded);
    CHECK(ca7cDispatch.deltaTicks10 == 20u);
    CHECK(ca7cDispatch.absoluteTimeAfter == 60u);
    CHECK(ca7cDispatch.dwordTrackOffset88After == 20u);
    CHECK(ca7cDispatch.lowerFunction == 0x8002CA7Cu);
    CHECK(ca7cDispatch.deltaDecoderFunction == 0x8002D7D0u);

    SsMidiTrackByteCounterObservation8002CAF4 caf4{};
    caf4.known = true;
    caf4.sequenceIndex = 3;
    caf4.trackIndex = 1;
    caf4.inputValue = 0x19u;
    caf4.byteTrackOffset20 = 3u;
    caf4.byteTrackOffset41 = 7u;
    caf4.deltaStreamBytes = {0x00u};
    caf4.absoluteTimeBefore = 99u;
    SsMidiTrackByteCounterDispatch8002CAF4 caf4Dispatch{};
    CHECK(TryBuildSsMidiTrackByteCounterDispatch8002CAF4(
        surface, caf4, caf4Dispatch));
    CHECK(caf4Dispatch.known);
    CHECK(caf4Dispatch.byteTrackOffset20After == 0x19u);
    CHECK(caf4Dispatch.byteTrackOffset41After == 8u);
    CHECK(caf4Dispatch.deltaDecoded);
    CHECK(caf4Dispatch.deltaTicks10 == 0u);
    CHECK(caf4Dispatch.absoluteTimeAfter == 99u);
    CHECK(caf4Dispatch.dwordTrackOffset88After == 0u);
    CHECK(caf4Dispatch.lowerFunction == 0x8002CAF4u);

    caf4.deltaStreamBytes.clear();
    CHECK(!TryBuildSsMidiTrackByteCounterDispatch8002CAF4(
        surface, caf4, caf4Dispatch));
    CHECK(!caf4Dispatch.known);
}

void TestSsMidiControllerReset8002C740() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsMidiParserSurface8002BA9C surface =
        BuildSsMidiParserSurface8002BA9C();
    SsMidiControllerResetObservation8002C740 observation{};
    observation.known = true;
    observation.sequenceIndex = 1;
    observation.trackIndex = 5;
    observation.channel = 4u;
    observation.byteTrackOffset19 = 9u;
    observation.byteTrackOffset20 = 8u;
    observation.channelProgramScratch.fill(3u);
    observation.channelVolumeScratch.fill(12u);
    observation.channelPanScratch.fill(13u);
    observation.deltaStreamBytes = {0x02u};
    observation.absoluteTimeBefore = 200u;

    SsMidiControllerResetDispatch8002C740 dispatch{};
    CHECK(TryBuildSsMidiControllerResetDispatch8002C740(
        surface, observation, dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.sequenceIndex == 1);
    CHECK(dispatch.trackIndex == 5);
    CHECK(dispatch.channel == 4u);
    CHECK(dispatch.byteTrackOffset19After == 0u);
    CHECK(dispatch.byteTrackOffset20After == 0u);
    CHECK(dispatch.selectedProgramIndex == 4u);
    CHECK(dispatch.selectedProgramScratchAfter == 4u);
    CHECK(dispatch.selectedVolumeAfter == 127u);
    CHECK(dispatch.selectedPanAfter == 64u);
    CHECK(dispatch.channelProgramScratchAfter[4] == 4u);
    CHECK(dispatch.channelVolumeScratchAfter[4] == 127u);
    CHECK(dispatch.channelPanScratchAfter[4] == 64u);
    CHECK(dispatch.callsTimerReset);
    CHECK(dispatch.timerResetFunction == 0x80030244u);
    CHECK(dispatch.callsThresholdReset);
    CHECK(dispatch.thresholdResetFunction == 0x8002EFD0u);
    CHECK(dispatch.deltaDecoded);
    CHECK(dispatch.deltaTicks10 == 20u);
    CHECK(dispatch.absoluteTimeAfter == 220u);
    CHECK(dispatch.dwordTrackOffset88After == 20u);
    CHECK(dispatch.lowerFunction == 0x8002C740u);

    observation.channel = 16u;
    CHECK(!TryBuildSsMidiControllerResetDispatch8002C740(
        surface, observation, dispatch));
    CHECK(!dispatch.known);
}

void TestSsMidiController65_8002C5DC() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsMidiParserSurface8002BA9C surface =
        BuildSsMidiParserSurface8002BA9C();
    SsMidiController65Observation8002C5DC observation{};
    observation.known = true;
    observation.sequenceIndex = 2;
    observation.trackIndex = 1;
    observation.vabId = 0;
    observation.program = 3u;
    observation.inputValue = 0x40u;
    observation.toneCount = 4u;
    observation.deltaStreamBytes = {0x01u};
    observation.absoluteTimeBefore = 12u;

    SsMidiController65Dispatch8002C5DC dispatch{};
    CHECK(TryBuildSsMidiController65Dispatch8002C5DC(
        surface, observation, dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.sequenceIndex == 2);
    CHECK(dispatch.trackIndex == 1);
    CHECK(dispatch.vabId == 0);
    CHECK(dispatch.program == 3u);
    CHECK(dispatch.inputValue == 0x40u);
    CHECK(dispatch.toneCount == 4u);
    CHECK(dispatch.band ==
          SsMidiController65Band8002C5DC::Inclusive64To127);
    CHECK(dispatch.toneReadCallCount == 4u);
    CHECK(dispatch.toneWriteCallCount == 4u);
    CHECK(dispatch.toneReadFunction == 0x8002F200u);
    CHECK(dispatch.toneWriteFunction == 0x8003037Cu);
    CHECK(dispatch.deltaDecoded);
    CHECK(dispatch.deltaTicks10 == 10u);
    CHECK(dispatch.absoluteTimeAfter == 22u);
    CHECK(dispatch.dwordTrackOffset88After == 10u);
    CHECK(dispatch.lowerFunction == 0x8002C5DCu);
    CHECK(dispatch.deltaDecoderFunction == 0x8002D7D0u);

    observation.inputValue = 0x20u;
    observation.toneCount = 0u;
    observation.deltaStreamBytes = {0x00u};
    CHECK(TryBuildSsMidiController65Dispatch8002C5DC(
        surface, observation, dispatch));
    CHECK(dispatch.band == SsMidiController65Band8002C5DC::NoTones);
    CHECK(dispatch.toneReadCallCount == 0u);
    CHECK(dispatch.toneWriteCallCount == 0u);

    observation.inputValue = 0x80u;
    observation.toneCount = 1u;
    observation.deltaStreamBytes = {0x02u};
    CHECK(TryBuildSsMidiController65Dispatch8002C5DC(
        surface, observation, dispatch));
    CHECK(dispatch.band == SsMidiController65Band8002C5DC::Above127);

    observation.toneCount = 17u;
    CHECK(!TryBuildSsMidiController65Dispatch8002C5DC(
        surface, observation, dispatch));
    CHECK(!dispatch.known);
}

void TestSsMidiController6_8002CB6C() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsMidiParserSurface8002BA9C surface =
        BuildSsMidiParserSurface8002BA9C();
    SsMidiController6Observation8002CB6C observation{};
    observation.known = true;
    observation.sequenceIndex = 2;
    observation.trackIndex = 6;
    observation.vabId = 0;
    observation.channel = 3u;
    observation.program = 4u;
    observation.inputValue = 0x55u;
    observation.toneCount = 3u;
    observation.byteTrackOffset39 = 1u;
    observation.byteTrackOffset16 = 0u;
    observation.byteTrackOffset41 = 0u;
    observation.byteTrackOffset42 = 0u;
    observation.deltaStreamBytes = {0x01u};
    observation.absoluteTimeBefore = 10u;

    SsMidiController6Dispatch8002CB6C dispatch{};
    CHECK(TryBuildSsMidiController6Dispatch8002CB6C(
        surface, observation, dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.branch == SsMidiController6Branch8002CB6C::ArmPending);
    CHECK(dispatch.pendingCallbackArmed);
    CHECK(dispatch.byteTrackOffset16After == 1u);
    CHECK(dispatch.byteTrackOffset40After == 0x55u);
    CHECK(dispatch.toneReadCallCount == 0u);
    CHECK(dispatch.toneUpdateCallCount == 0u);
    CHECK(dispatch.deltaTicks10 == 10u);
    CHECK(dispatch.dwordTrackOffset88After == 10u);

    observation.byteTrackOffset39 = 0u;
    observation.byteTrackOffset16 = 1u;
    observation.byteTrackOffset41 = 2u;
    observation.byteTrackOffset19 = 0u;
    observation.byteTrackOffset20 = 0u;
    observation.deltaStreamBytes = {0x02u};
    CHECK(TryBuildSsMidiController6Dispatch8002CB6C(
        surface, observation, dispatch));
    CHECK(dispatch.branch ==
          SsMidiController6Branch8002CB6C::ToneRewriteBase);
    CHECK(dispatch.toneReadCallCount == 3u);
    CHECK(dispatch.toneWriteCallCount == 3u);
    CHECK(dispatch.baseToneRewriteValue == (0x55u & 0x7Fu));
    CHECK(dispatch.counter41Reset);
    CHECK(dispatch.byteTrackOffset41After == 0u);

    observation.byteTrackOffset41 = 0u;
    observation.byteTrackOffset42 = 2u;
    observation.byteTrackOffset22 = 16u;
    observation.deltaStreamBytes = {0x00u};
    CHECK(TryBuildSsMidiController6Dispatch8002CB6C(
        surface, observation, dispatch));
    CHECK(dispatch.branch ==
          SsMidiController6Branch8002CB6C::DynamicToneUpdateAll);
    CHECK(dispatch.toneUpdateCallCount == 3u);
    CHECK(dispatch.counter42Reset);
    CHECK(dispatch.byteTrackOffset42After == 0u);

    observation.byteTrackOffset22 = 7u;
    observation.toneCount = 0u;
    observation.deltaStreamBytes = {0x01u};
    CHECK(TryBuildSsMidiController6Dispatch8002CB6C(
        surface, observation, dispatch));
    CHECK(dispatch.branch ==
          SsMidiController6Branch8002CB6C::DynamicToneUpdateSingle);
    CHECK(dispatch.toneUpdateCallCount == 1u);

    observation.toneCount = 17u;
    CHECK(!TryBuildSsMidiController6Dispatch8002CB6C(
        surface, observation, dispatch));
    CHECK(!dispatch.known);
}

void TestSsMidiEnvelopeLowerHandlers8002B4748002B750() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsMidiParserSurface8002BA9C surface =
        BuildSsMidiParserSurface8002BA9C();
    SsMidiEnvelopeObservation8002B474 observation{};
    observation.known = true;
    observation.sequenceIndex = 1;
    observation.trackIndex = 2;
    observation.wordTrackOffset3E = 100;
    observation.wordTrackOffset40 = 3;
    observation.wordTrackOffset42 = 1;
    observation.dwordTrackOffset94 = 10u;
    observation.dwordTrackOffset98 = 10u;
    observation.dwordTrackOffset90Flags = 0x30u;
    observation.wordTrackOffset74 = 40u;
    observation.wordTrackOffset76 = 50u;
    observation.wordTrackOffset78 = 1u;
    observation.wordTrackOffset7A = 2u;

    SsMidiEnvelopeDispatch8002B474 b474{};
    CHECK(TryBuildSsMidiEnvelopeDispatch8002B474(
        surface, observation, b474));
    CHECK(b474.known);
    CHECK(b474.branch == SsMidiEnvelopeBranch8002B474::PositiveRamp);
    CHECK(b474.packedSequenceTrack == 0x0201u);
    CHECK(b474.dwordTrackOffset98After == 9u);
    CHECK(b474.wordTrackOffset40After == 2);
    CHECK(b474.helperReadFunction == 0x80033928u);
    CHECK(b474.helperWriteFunction == 0x800337B4u);
    CHECK(b474.helperReadCallCount == 2u);
    CHECK(b474.helperWriteCallCount == 1u);
    CHECK(b474.helperWriteLeftRequested == 41);
    CHECK(b474.helperWriteRightRequested == 51);
    CHECK(b474.wordTrackOffset74After == 41u);
    CHECK(b474.wordTrackOffset76After == 51u);
    CHECK(b474.wordTrackOffset78After == 41u);
    CHECK(b474.wordTrackOffset7AAfter == 51u);
    CHECK(!b474.flag10Cleared);
    CHECK(!b474.lowerCallCommitted);

    observation.wordTrackOffset42 = 2;
    observation.dwordTrackOffset98 = 12u;
    CHECK(TryBuildSsMidiEnvelopeDispatch8002B474(
        surface, observation, b474));
    CHECK(b474.branch ==
          SsMidiEnvelopeBranch8002B474::PositiveModuloSkip);
    CHECK(b474.helperReadCallCount == 1u);
    CHECK(b474.helperWriteCallCount == 0u);
    CHECK(b474.wordTrackOffset40After == 3);
    CHECK(b474.result == 1u);

    observation.wordTrackOffset42 = -2;
    observation.wordTrackOffset40 = 1;
    observation.dwordTrackOffset98 = 10u;
    observation.dwordTrackOffset90Flags = 0x10u;
    CHECK(TryBuildSsMidiEnvelopeDispatch8002B474(
        surface, observation, b474));
    CHECK(b474.branch == SsMidiEnvelopeBranch8002B474::NegativeReset);
    CHECK(b474.wordTrackOffset40After == -1);
    CHECK(b474.helperReadCallCount == 1u);
    CHECK(b474.helperWriteCallCount == 1u);
    CHECK(b474.helperWriteLeftApplied == 127u);
    CHECK(b474.helperWriteRightApplied == 127u);
    CHECK(b474.flag10Cleared);
    CHECK((b474.dwordTrackOffset90FlagsAfter & 0x10u) == 0u);

    observation.wordTrackOffset42 = -1;
    observation.wordTrackOffset40 = 5;
    observation.wordTrackOffset3E = 100;
    observation.wordTrackOffset74 = 20u;
    observation.wordTrackOffset76 = 30u;
    CHECK(TryBuildSsMidiEnvelopeDispatch8002B474(
        surface, observation, b474));
    CHECK(b474.branch == SsMidiEnvelopeBranch8002B474::NegativeRamp);
    CHECK(b474.helperReadCallCount == 2u);
    CHECK(b474.helperWriteCallCount == 1u);
    CHECK(b474.helperWriteLeftRequested == 21);
    CHECK(b474.helperWriteRightRequested == 31);

    observation.wordTrackOffset42 = 0;
    observation.wordTrackOffset40 = 7;
    CHECK(TryBuildSsMidiEnvelopeDispatch8002B474(
        surface, observation, b474));
    CHECK(b474.branch == SsMidiEnvelopeBranch8002B474::ZeroStepCopy);
    CHECK(b474.helperReadCallCount == 1u);
    CHECK(b474.helperWriteCallCount == 0u);
    CHECK(b474.wordTrackOffset78After == observation.wordTrackOffset74);
    CHECK(b474.wordTrackOffset7AAfter == observation.wordTrackOffset76);

    SsMidiEnvelopeDispatch8002B750 b750{};
    observation.wordTrackOffset42 = -1;
    observation.wordTrackOffset40 = 5;
    observation.wordTrackOffset3E = 100;
    observation.wordTrackOffset74 = 20u;
    observation.wordTrackOffset76 = 30u;
    observation.dwordTrackOffset98 = 10u;
    CHECK(TryBuildSsMidiEnvelopeDispatch8002B750(
        surface, observation, b750));
    CHECK(b750.branch == SsMidiEnvelopeBranch8002B750::NonPositiveRamp);
    CHECK(b750.helperReadCallCount == 2u);
    CHECK(b750.helperWriteCallCount == 1u);
    CHECK(b750.helperWriteLeftRequested == 19);
    CHECK(b750.helperWriteRightRequested == 29);

    observation.wordTrackOffset40 = 0;
    observation.dwordTrackOffset90Flags = 0x20u;
    CHECK(TryBuildSsMidiEnvelopeDispatch8002B750(
        surface, observation, b750));
    CHECK(b750.branch == SsMidiEnvelopeBranch8002B750::NonPositiveReset);
    CHECK(b750.helperReadCallCount == 1u);
    CHECK(b750.helperWriteCallCount == 0u);
    CHECK(b750.flag20Cleared);
    CHECK((b750.dwordTrackOffset90FlagsAfter & 0x20u) == 0u);

    observation.wordTrackOffset42 = 1;
    observation.wordTrackOffset40 = 3;
    observation.wordTrackOffset74 = 50u;
    observation.wordTrackOffset76 = 60u;
    observation.dwordTrackOffset90Flags = 0x20u;
    CHECK(TryBuildSsMidiEnvelopeDispatch8002B750(
        surface, observation, b750));
    CHECK(b750.branch == SsMidiEnvelopeBranch8002B750::PositiveRamp);
    CHECK(b750.wordTrackOffset40After == 2);
    CHECK(b750.helperReadCallCount == 2u);
    CHECK(b750.helperWriteCallCount == 1u);
    CHECK(b750.helperWriteLeftRequested == 49);
    CHECK(b750.helperWriteRightRequested == 59);

    observation.wordTrackOffset40 = 1;
    CHECK(TryBuildSsMidiEnvelopeDispatch8002B750(
        surface, observation, b750));
    CHECK(b750.branch == SsMidiEnvelopeBranch8002B750::PositiveReset);
    CHECK(b750.helperReadCallCount == 1u);
    CHECK(b750.helperWriteCallCount == 0u);
    CHECK(b750.flag20Cleared);

    observation.wordTrackOffset42 = 2;
    observation.wordTrackOffset40 = 3;
    observation.dwordTrackOffset98 = 12u;
    CHECK(TryBuildSsMidiEnvelopeDispatch8002B750(
        surface, observation, b750));
    CHECK(b750.branch == SsMidiEnvelopeBranch8002B750::PositiveModuloSkip);
    CHECK(b750.helperReadCallCount == 1u);
    CHECK(b750.helperWriteCallCount == 0u);
    CHECK(b750.result == 1u);

    observation.sequenceIndex = 0x100;
    CHECK(!TryBuildSsMidiEnvelopeDispatch8002B474(
        surface, observation, b474));
    CHECK(!b474.known);
}

void TestSsMidiTempoTick8002DDA4() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsMidiParserSurface8002BA9C surface =
        BuildSsMidiParserSurface8002BA9C();
    SsMidiTempoTickObservation8002DDA4 observation{};
    observation.known = true;
    observation.sequenceIndex = 2;
    observation.trackIndex = 3;
    observation.wordTrackOffset44 = 2;
    observation.wordTrackOffset74 = 60;
    observation.dwordTrackOffset8C = 10u;
    observation.dwordTrackOffsetA0 = 5u;
    observation.dwordTrackOffsetA4 = 2u;
    observation.dwordTrackOffset90Flags = 0xC0u;
    observation.tickRate80095C4C = 60u;

    SsMidiTempoTickDispatch8002DDA4 dispatch{};
    CHECK(TryBuildSsMidiTempoTickDispatch8002DDA4(
        surface, observation, dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.branch == SsMidiTempoTickBranch8002DDA4::
              PositiveStepDown);
    CHECK(dispatch.dwordTrackOffsetA0After == 4u);
    CHECK(dispatch.dwordTrackOffset8CAfter == 9u);
    CHECK(dispatch.numerator == 5400u);
    CHECK(dispatch.denominator == 3600u);
    CHECK(dispatch.wordTrackOffset70After == 1u);
    CHECK(!dispatch.flags40And80Cleared);
    CHECK(dispatch.divisorKnown);
    CHECK(!dispatch.lowerCallCommitted);

    observation.dwordTrackOffsetA0 = 6u;
    CHECK(TryBuildSsMidiTempoTickDispatch8002DDA4(
        surface, observation, dispatch));
    CHECK(dispatch.branch == SsMidiTempoTickBranch8002DDA4::
              PositiveModuloSkip);
    CHECK(dispatch.positiveModulo == 1u);
    CHECK(!dispatch.divisorKnown);
    CHECK(dispatch.dwordTrackOffset8CAfter == observation.dwordTrackOffset8C);

    observation.wordTrackOffset44 = -2;
    observation.dwordTrackOffsetA0 = 5u;
    observation.dwordTrackOffset8C = 1u;
    observation.dwordTrackOffsetA4 = 5u;
    CHECK(TryBuildSsMidiTempoTickDispatch8002DDA4(
        surface, observation, dispatch));
    CHECK(dispatch.branch == SsMidiTempoTickBranch8002DDA4::
              NonPositiveRebase);
    CHECK(dispatch.dwordTrackOffset8CAfter == 3u);

    observation.wordTrackOffset44 = -10;
    CHECK(TryBuildSsMidiTempoTickDispatch8002DDA4(
        surface, observation, dispatch));
    CHECK(dispatch.branch == SsMidiTempoTickBranch8002DDA4::
              NonPositiveClamp);
    CHECK(dispatch.dwordTrackOffset8CAfter == 5u);

    observation.wordTrackOffset44 = -2;
    observation.dwordTrackOffset8C = 5u;
    observation.dwordTrackOffsetA4 = 5u;
    CHECK(TryBuildSsMidiTempoTickDispatch8002DDA4(
        surface, observation, dispatch));
    CHECK(dispatch.branch == SsMidiTempoTickBranch8002DDA4::NonPositiveHold);
    CHECK(dispatch.dwordTrackOffset8CAfter == 5u);

    observation.dwordTrackOffsetA0 = 1u;
    observation.dwordTrackOffset90Flags = 0xC0u;
    CHECK(TryBuildSsMidiTempoTickDispatch8002DDA4(
        surface, observation, dispatch));
    CHECK(dispatch.flags40And80Cleared);
    CHECK((dispatch.dwordTrackOffset90FlagsAfter & 0xC0u) == 0u);

    observation.wordTrackOffset44 = 2;
    observation.dwordTrackOffsetA0 = 3u;
    observation.dwordTrackOffset8C = 10u;
    observation.dwordTrackOffsetA4 = 2u;
    observation.tickRate80095C4C = 0u;
    CHECK(!TryBuildSsMidiTempoTickDispatch8002DDA4(
        surface, observation, dispatch));
    CHECK(!dispatch.known);
}

void TestSsMidiTrackFlagRoutes8002B9FC8002BAC88002DBE4() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsMidiParserSurface8002BA9C surface =
        BuildSsMidiParserSurface8002BA9C();
    SsMidiTrackFlagToggleObservation8002B9FC toggle{};
    toggle.known = true;
    toggle.sequenceIndex = 1;
    toggle.trackIndex = 2;
    toggle.byteTrackOffset2B = 7u;
    toggle.dwordTrackOffset90Flags = 0xFFu;

    SsMidiTrackFlagToggleDispatch8002B9FC toggleDispatch{};
    CHECK(TryBuildSsMidiTrackFlagToggleDispatch8002B9FC(
        surface, toggle, toggleDispatch));
    CHECK(toggleDispatch.known);
    CHECK(toggleDispatch.byteTrackOffset2BAfter == 0u);
    CHECK(toggleDispatch.dwordTrackOffset90FlagsAfter == 0xFDu);
    CHECK(toggleDispatch.packedSequenceTrack == 0x0201u);
    CHECK(toggleDispatch.cleanupFunction == 0x80033A34u);
    CHECK(toggleDispatch.cleanupObserved);
    CHECK(toggleDispatch.result == 0xFDu);
    CHECK(!toggleDispatch.lowerCallCommitted);

    CHECK(TryBuildSsMidiTrackFlagToggleDispatch8002BAC8(
        surface, toggle, toggleDispatch));
    CHECK(toggleDispatch.byteTrackOffset2BAfter == 1u);
    CHECK(toggleDispatch.dwordTrackOffset90FlagsAfter == 0xF7u);
    CHECK(toggleDispatch.cleanupObserved);

    SsMidiTrackResetObservation8002DBE4 reset{};
    reset.known = true;
    reset.sequenceIndex = 3;
    reset.trackIndex = 4;
    reset.dwordTrackOffset90Flags = 0x0Fu;
    reset.dwordTrackOffset7C = 0x12345678u;
    reset.dwordTrackOffset84 = 0x87654321u;
    reset.wordTrackOffset72 = 0x009Au;
    reset.dwordTrackOffset08 = 0x00ABCDEFu;
    reset.byteTrackOffset2B = 1u;
    reset.byteTrackOffset16 = 1u;
    reset.byteTrackOffset17 = 2u;
    reset.byteTrackOffset18 = 3u;
    reset.byteTrackOffset19 = 4u;
    reset.byteTrackOffset20 = 5u;
    reset.byteTrackOffset21 = 6u;
    reset.byteTrackOffset22 = 7u;
    reset.byteTrackOffset39 = 8u;
    reset.byteTrackOffset40 = 9u;
    reset.byteTrackOffset41 = 10u;
    reset.byteTrackOffset42 = 11u;
    reset.byteTrackOffset43 = 12u;
    reset.channelProgramMarkers[0] = 9u;
    reset.channelPans[0] = 10u;
    reset.channelVolumes[0] = 11u;

    SsMidiTrackResetDispatch8002DBE4 resetDispatch{};
    CHECK(TryBuildSsMidiTrackResetDispatch8002DBE4(
        surface, reset, resetDispatch));
    CHECK(resetDispatch.known);
    CHECK(resetDispatch.dwordTrackOffset90FlagsAfter == 0x05u);
    CHECK(resetDispatch.dwordTrackOffset80After == 0u);
    CHECK(resetDispatch.dwordTrackOffset88After == 0x12345678u);
    CHECK(resetDispatch.dwordTrackOffset8CAfter == 0x87654321u);
    CHECK(resetDispatch.dwordTrackOffset04After == 0x00ABCDEFu);
    CHECK(resetDispatch.dwordTrackOffset0CAfter == 0x00ABCDEFu);
    CHECK(resetDispatch.wordTrackOffset72After == 0u);
    CHECK(resetDispatch.wordTrackOffset70After == 0x009Au);
    CHECK(resetDispatch.byteTrackOffset16After == 0u);
    CHECK(resetDispatch.byteTrackOffset17After == 0u);
    CHECK(resetDispatch.byteTrackOffset18After == 0u);
    CHECK(resetDispatch.byteTrackOffset19After == 0u);
    CHECK(resetDispatch.byteTrackOffset20After == 0u);
    CHECK(resetDispatch.byteTrackOffset21After == 0u);
    CHECK(resetDispatch.byteTrackOffset22After == 0u);
    CHECK(resetDispatch.byteTrackOffset2BAfter == 0u);
    CHECK(resetDispatch.byteTrackOffset39After == 0u);
    CHECK(resetDispatch.byteTrackOffset40After == 0u);
    CHECK(resetDispatch.byteTrackOffset41After == 0u);
    CHECK(resetDispatch.byteTrackOffset42After == 0u);
    CHECK(resetDispatch.byteTrackOffset43After == 0u);
    CHECK(resetDispatch.channelLoopCount == 16u);
    CHECK(resetDispatch.channelProgramMarkersAfter[0] == 0u);
    CHECK(resetDispatch.channelProgramMarkersAfter[15] == 15u);
    CHECK(resetDispatch.channelPansAfter[3] == 64u);
    CHECK(resetDispatch.channelVolumesAfter[3] == 127u);
    CHECK(resetDispatch.wordTrackOffset78After == 127u);
    CHECK(resetDispatch.wordTrackOffset7AAfter == 127u);
    CHECK(resetDispatch.packedSequenceTrack == 0x0403u);
    CHECK(resetDispatch.cleanupFunction == 0x80033A34u);
    CHECK(resetDispatch.cleanupObserved);
    CHECK(resetDispatch.result == 127u);

    reset.sequenceIndex = 0x100;
    CHECK(!TryBuildSsMidiTrackResetDispatch8002DBE4(
        surface, reset, resetDispatch));
    CHECK(!resetDispatch.known);
}

void TestSsMidiNoteExecutionSurface() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsMidiNoteSurface8002BEEC surface =
        BuildSsMidiNoteSurface8002BEEC();
    CHECK(IsExactSsMidiNoteSurface8002BEEC(surface));
    CHECK(surface.known);
    CHECK(surface.noteDecisionCompleteWithinLimits);
    CHECK(surface.hostToneLayerSelectionCompleteWithinLimits);
    CHECK(surface.hostNoteOffOwnershipCompleteWithinLimits);
    CHECK(surface.noteDecisionFunction == 0x8002BEECu);
    CHECK(surface.noteOnFunction == 0x80032EACu);
    CHECK(surface.noteOffFunction == 0x8003349Cu);
    CHECK(surface.sequenceStatePointerTableAddress == 0x80095D08u);
    CHECK(surface.trackStateStride == 0xACu);
    CHECK(surface.channelOffset == 0x12u);
    CHECK(surface.channelPanBaseOffset == 0x17u);
    CHECK(surface.channelProgramBaseOffset == 0x2Cu);
    CHECK(surface.vabIdOffset == 0x4Cu);
    CHECK(surface.channelVolumeBaseOffset == 0x4Eu);
    CHECK(surface.sequenceEnabledOffset == 0x74u);
    CHECK(surface.sequenceVolumeLeftOffset == 0x74u);
    CHECK(surface.sequenceVolumeRightOffset == 0x76u);
    CHECK(surface.lastVelocityOffset == 0xA8u);
    CHECK(surface.muteMaskOffset == 0xAAu);
    CHECK(surface.programAttributeStride == 0x10u);
    CHECK(surface.toneAttributeStride == 0x20u);
    CHECK(surface.toneNoteMinOffset == 0x06u);
    CHECK(surface.toneNoteMaxOffset == 0x07u);
    CHECK(surface.maxTonesPerProgram == 16u);
    CHECK(surface.crossFileExplicitReferenceScanCompleteWithinScope);
    CHECK(!surface.psxMemoryAuthority);
    CHECK(!surface.psxVoiceIdentityAuthority);
    CHECK(!surface.psxSpuTimingAuthority);
    CHECK(!surface.dynamicReplayAuthority);
    CHECK(!surface.oldWinS0Authority);
    CHECK(!surface.stage2PlusAuthority);

    SsMidiNoteTrackState8002BEEC state{};
    state.known = true;
    state.sequenceIndex = 2;
    state.trackIndex = 4;
    state.channel = 3u;
    state.vabId = 5;
    state.sequenceEnabled = 100u;
    state.sequenceVolumeRight = 80u;
    state.channelPan[3] = 60u;
    state.channelProgram[3] = 7u;
    state.channelVolume[3] = 100u;
    state.lastVelocity = 11;

    SsMidiNoteDispatch8002BEEC dispatch{};
    CHECK(TryBuildSsMidiNoteDispatch8002BEEC(
        surface, state, 64u, 80u, dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.action == SsMidiNoteAction8002BEEC::NoteOn);
    CHECK(dispatch.sequenceIndex == 2);
    CHECK(dispatch.trackIndex == 4);
    CHECK(dispatch.packedSequenceTrack == 0x0402u);
    CHECK(dispatch.channel == 3u);
    CHECK(dispatch.vabId == 5);
    CHECK(dispatch.program == 7u);
    CHECK(dispatch.note == 64u);
    CHECK(dispatch.velocity == 80u);
    CHECK(dispatch.pan == 60u);
    CHECK(dispatch.channelVolume == 100u);
    CHECK(dispatch.sequenceVolumeLeft == 100u);
    CHECK(dispatch.sequenceVolumeRight == 80u);
    CHECK(dispatch.scaledVelocity == 62u);
    CHECK(dispatch.lowerFunction == 0x80032EACu);
    CHECK(!dispatch.muted);
    CHECK(dispatch.sequenceEnabled);
    CHECK(dispatch.lastVelocityUpdated);
    CHECK(dispatch.lastVelocityAfter == 80);
    CHECK(state.lastVelocity == 80);
    CHECK(!dispatch.lowerCallCommitted);
    CHECK(dispatch.hostProjectionReady);
    CHECK(!dispatch.psxMemoryAuthority);
    CHECK(!dispatch.psxVoiceIdentityAuthority);
    CHECK(!dispatch.psxSpuTimingAuthority);

    CHECK(TryBuildSsMidiNoteDispatch8002BEEC(
        surface, state, 64u, 0u, dispatch));
    CHECK(dispatch.action == SsMidiNoteAction8002BEEC::NoteOff);
    CHECK(dispatch.lowerFunction == 0x8003349Cu);
    CHECK(dispatch.scaledVelocity == 0u);
    CHECK(!dispatch.lastVelocityUpdated);
    CHECK(dispatch.lastVelocityAfter == 80);
    CHECK(state.lastVelocity == 80);
    CHECK(dispatch.hostProjectionReady);

    state.muteMask = 1u << state.channel;
    CHECK(TryBuildSsMidiNoteDispatch8002BEEC(
        surface, state, 65u, 90u, dispatch));
    CHECK(dispatch.action == SsMidiNoteAction8002BEEC::Suppressed);
    CHECK(dispatch.muted);
    CHECK(dispatch.lowerFunction == 0u);
    CHECK(!dispatch.lastVelocityUpdated);
    CHECK(!dispatch.hostProjectionReady);
    CHECK(state.lastVelocity == 80);

    state.muteMask = 0u;
    state.sequenceEnabled = 0u;
    CHECK(TryBuildSsMidiNoteDispatch8002BEEC(
        surface, state, 66u, 90u, dispatch));
    CHECK(dispatch.action == SsMidiNoteAction8002BEEC::Suppressed);
    CHECK(!dispatch.muted);
    CHECK(!dispatch.sequenceEnabled);
    CHECK(!dispatch.hostProjectionReady);

    state.sequenceEnabled = 100u;
    state.channel = 16u;
    CHECK(!TryBuildSsMidiNoteDispatch8002BEEC(
        surface, state, 64u, 80u, dispatch));
    CHECK(!dispatch.known);

    state.channel = 3u;
    SsMidiNoteSurface8002BEEC mutated = surface;
    mutated.toneNoteMaxOffset = 0x08u;
    CHECK(!IsExactSsMidiNoteSurface8002BEEC(mutated));
    CHECK(!TryBuildSsMidiNoteDispatch8002BEEC(
        mutated, state, 64u, 80u, dispatch));
    CHECK(!dispatch.known);

    auto [vh, vb] = BuildSyntheticLayeredMidiVab();
    VabPlayer layeredVab{};
    CHECK(layeredVab.LoadFromMemory(
        vh.data(), vh.size(), vb.data(), vb.size()));
    CHECK(layeredVab.GetSpuAllocationBytes8002E474() == 16u);
    CHECK(layeredVab.ApplySpuAllocationBase8002E474(0x1010u));
    CHECK(layeredVab.GetMasterVolume80032EAC() == 127u);
    VabPlayer::MidiProgramAttributes80032EAC programAttributes{};
    std::vector<VabPlayer::MidiToneAttributes80032EAC> toneAttributes;
    CHECK(layeredVab.ResolveMidiNoteAttributes80032EAC(
        1u, 55u, programAttributes, toneAttributes));
    CHECK(programAttributes.valid);
    CHECK(programAttributes.toneCount == 3u);
    CHECK(programAttributes.volume == 127u);
    CHECK(programAttributes.priority == 6u);
    CHECK(programAttributes.mode == 2u);
    CHECK(programAttributes.pan == 64u);
    CHECK(programAttributes.toneTableProgram == 0u);
    CHECK(toneAttributes.size() == 2u);
    CHECK(toneAttributes[0].priority == 1u);
    CHECK(toneAttributes[0].mode == 4u);
    CHECK(toneAttributes[0].centerFine == 16u);
    CHECK(toneAttributes[0].adsr1 == 0x1110u);
    CHECK(toneAttributes[0].adsr2 == 0x2220u);
    CHECK(toneAttributes[0].sampleId == 1u);
    CHECK(toneAttributes[0].sampleStartAddressKnown);
    CHECK(toneAttributes[0].sampleStartAddress == 0x0202u);
    CHECK(toneAttributes[1].priority == 2u);
    CHECK(toneAttributes[1].adsr1 == 0x1111u);
    CHECK(toneAttributes[1].adsr2 == 0x2221u);
    const std::vector<int> key55 =
        layeredVab.ResolveMidiNoteToneLayers80032EAC(1u, 55u);
    CHECK(key55.size() == 2u);
    CHECK(key55[0] == 0);
    CHECK(key55[1] == 1);
    const std::vector<int> key75 =
        layeredVab.ResolveMidiNoteToneLayers80032EAC(1u, 75u);
    CHECK(key75.empty());
    const std::vector<int> key85 =
        layeredVab.ResolveMidiNoteToneLayers80032EAC(1u, 85u);
    CHECK(key85.size() == 1u);
    CHECK(key85[0] == 2);
    CHECK(layeredVab.ResolveMidiNoteToneLayers80032EAC(2u, 55u).empty());
}

void CheckRejected(
    const PrSS0Scene0IntSpuDirect::Transaction8001A8F0& transaction,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18& intLoad,
    const PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0&
        loaderCandidate,
    const PrSS0Scene0IntSpuDirect::PadStartComSource80026E4C&
        padStartComSource) {
    CHECK(!transaction.accepted);
    CHECK(!transaction.hostVabCandidatePrepared);
    CHECK(!transaction.hostVabBankCommitted);
    CHECK(!transaction.padStartComGlobalWritesCommitted);
    CHECK(!transaction.psxVabIdAuthority);
    CHECK(!transaction.psxSpuRamAuthority);
    CHECK(!PrSS0Scene0IntSpuDirect::IsExactAcceptedTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));
    CHECK(!PrSS0Scene0IntSpuDirect::
              IsExactAcceptedYCompoTransaction8001A8F0(
                  transaction,
                  intLoad,
                  loaderCandidate,
                  padStartComSource));
}

void TestFailClosed() {
    PrSS0Scene0IntLoadDirect::Transaction8001AC18 badLoad{};
    PrSS0Scene0IntSideEffectDirect::Transaction8001A8F0 badCandidate{};
    PrSS0Scene0IntSpuDirect::PadStartComSource80026E4C badPadSource{};
    CheckRejected(
        PrSS0Scene0IntSpuDirect::BuildTransaction8001A8F0(
            badLoad, badCandidate, badPadSource),
        badLoad,
        badCandidate,
        badPadSource);
}

void TestSsDriverFlushSurface() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsDriverFlushSurface80026ECC surface =
        BuildSsDriverFlushSurface80026ECC();
    CHECK(IsExactSsDriverFlushSurface80026ECC(surface));
    CHECK(surface.wrapperFunction == 0x80026ECCu);
    CHECK(surface.wrapperBusyFlagAddress == 0x800943B4u);
    CHECK(surface.lowerFlushFunction == 0x8002EFF4u);
    CHECK(surface.reentryGuardAddress == 0x800917A4u);
    CHECK(surface.driverCommitFunction == 0x80032B00u);
    CHECK(surface.lowerFlushExplicitCodeXrefCount == 1u);
    CHECK(surface.comod0DirectWrapperCallCount == 3u);
    CHECK(surface.comod1DirectWrapperCallCount == 2u);
    CHECK(!surface.comod0PathEntersSequenceScheduler);
    CHECK(!surface.psxSpuRegisterAuthority);
    CHECK(!surface.dynamicReplayAuthority);
    CHECK(!surface.oldWinS0Authority);
    CHECK(!surface.stage2PlusAuthority);

    SsDriverFlushState80026ECC state{};
    SsDriverCommitProbe80032B00 probe{};
    probe.state = &state;
    probe.result = 0x1357;
    SsDriverFlushDispatch80026ECC dispatch{};
    CHECK(TryExecuteSsDriverFlush80026ECC(
        surface,
        state,
        CaptureSsDriverCommit80032B00,
        &probe,
        dispatch));
    CHECK(dispatch.known);
    CHECK(!dispatch.wrapperSkippedByBusyFlag);
    CHECK(dispatch.lowerFlushEntered);
    CHECK(!dispatch.lowerSkippedByReentryGuard);
    CHECK(dispatch.reentryGuardSetBeforeCommit);
    CHECK(dispatch.driverCommitExecuted);
    CHECK(dispatch.reentryGuardClearedAfterCommit);
    CHECK(dispatch.result == 0x1357);
    CHECK(dispatch.hostProjection);
    CHECK(!dispatch.psxSpuRegisterAuthority);
    CHECK(probe.callCount == 1u);
    CHECK(probe.sawReentryGuardSet);
    CHECK(state.reentryGuard800917A4 == 0);

    state.wrapperBusyFlag800943B4 = 7;
    probe.callCount = 0u;
    probe.sawReentryGuardSet = false;
    CHECK(TryExecuteSsDriverFlush80026ECC(
        surface,
        state,
        CaptureSsDriverCommit80032B00,
        &probe,
        dispatch));
    CHECK(dispatch.wrapperSkippedByBusyFlag);
    CHECK(!dispatch.lowerFlushEntered);
    CHECK(!dispatch.driverCommitExecuted);
    CHECK(dispatch.result == 7);
    CHECK(probe.callCount == 0u);

    state.wrapperBusyFlag800943B4 = 0;
    state.reentryGuard800917A4 = 1;
    CHECK(TryExecuteSsDriverFlush80026ECC(
        surface,
        state,
        CaptureSsDriverCommit80032B00,
        &probe,
        dispatch));
    CHECK(dispatch.lowerFlushEntered);
    CHECK(dispatch.lowerSkippedByReentryGuard);
    CHECK(!dispatch.driverCommitExecuted);
    CHECK(dispatch.result == 1);
    CHECK(state.reentryGuard800917A4 == 1);
    CHECK(probe.callCount == 0u);

    state.reentryGuard800917A4 = 2;
    probe.result = -9;
    CHECK(TryExecuteSsDriverFlush80026ECC(
        surface,
        state,
        CaptureSsDriverCommit80032B00,
        &probe,
        dispatch));
    CHECK(!dispatch.lowerSkippedByReentryGuard);
    CHECK(dispatch.driverCommitExecuted);
    CHECK(dispatch.result == -9);
    CHECK(state.reentryGuard800917A4 == 0);
    CHECK(probe.callCount == 1u);

    auto invalidSurface = surface;
    invalidSurface.driverCommitFunction ^= 4u;
    CHECK(!TryExecuteSsDriverFlush80026ECC(
        invalidSurface,
        state,
        CaptureSsDriverCommit80032B00,
        &probe,
        dispatch));
    CHECK(!dispatch.known);
    CHECK(!TryExecuteSsDriverFlush80026ECC(
        surface, state, nullptr, &probe, dispatch));
}

void TestSsDriverVoiceCompletionSurface() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsDriverVoiceCompletionSurface80032B00 surface =
        BuildSsDriverVoiceCompletionSurface80032B00();
    CHECK(IsExactSsDriverVoiceCompletionSurface80032B00(surface));
    CHECK(surface.known);
    CHECK(surface.completionHistoryControlCompleteWithinLimits);
    CHECK(surface.function == 0x80032B00u);
    CHECK(surface.voiceCountAddress == 0x800928A0u);
    CHECK(surface.completionRingIndexAddress == 0x80088220u);
    CHECK(surface.completionRingBaseAddress == 0x80088224u);
    CHECK(surface.completionScanDisableAddress == 0x8009290Cu);
    CHECK(surface.spuVoiceBasePointerAddress == 0x80055DD8u);
    CHECK(surface.voiceEnvelopeRegisterOffset == 0x0Cu);
    CHECK(surface.voiceStateBaseAddress == 0x80087D5Bu);
    CHECK(surface.voiceStateStride == 0x34u);
    CHECK(surface.noiseMaskClearFunction == 0x800353E8u);
    CHECK(surface.noiseMaskClearValue == 0x00FFFFFFu);
    CHECK(surface.maxVoiceCount == 24u);
    CHECK(surface.completionRingEntryCount == 16u);
    CHECK(surface.stableCompletionEntryCount == 15u);
    CHECK(!surface.psxEnvelopeAuthority);
    CHECK(!surface.psxNoiseRegisterAuthority);
    CHECK(!surface.psxSpuRegisterAuthority);
    CHECK(!surface.dynamicReplayAuthority);
    CHECK(!surface.oldWinS0Authority);
    CHECK(!surface.stage2PlusAuthority);

    SsDriverVoiceCompletionObservation80032B00 observation{};
    observation.voiceCount = 3u;
    observation.envelopeObservations[0] = 0u;
    observation.envelopeObservations[1] = 7u;
    observation.envelopeObservations[2] = 0u;
    std::array<SsDriverVoiceOwnerState8003226C, 24> completionOwners{};
    completionOwners[0].field80087D46 = 99u;
    completionOwners[1].field80087D46 = 99u;
    completionOwners[2].field80087D46 = 99u;
    CHECK(TryProjectSsDriverVoiceEnvelope80032B00(
        surface, observation, completionOwners));
    CHECK(completionOwners[0].field80087D46 == 0u);
    CHECK(completionOwners[1].field80087D46 == 7u);
    CHECK(completionOwners[2].field80087D46 == 0u);
    CHECK(completionOwners[3].field80087D46 == 0u);
    SsDriverVoiceCompletionState80032B00 state{};
    state.voiceStatus[0] = 2u;
    state.voiceStatus[2] = 1u;
    SsDriverVoiceCompletionDispatch80032B00 dispatch{};

    CHECK(TryExecuteSsDriverVoiceCompletion80032B00(
        surface, observation, state, dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.completionRingIndexBefore == 0u);
    CHECK(dispatch.completionRingIndexAfter == 1u);
    CHECK(dispatch.currentZeroEnvelopeMask == 0x5u);
    CHECK(!dispatch.completionScanSkipped);
    CHECK(dispatch.stableZeroEnvelopeMask == 0u);
    CHECK(dispatch.clearedVoiceStatusMask == 0u);
    CHECK(dispatch.noiseMaskClearCallCount == 0u);
    CHECK(dispatch.hostEnvelopeProjection);
    CHECK(!dispatch.psxEnvelopeAuthority);
    CHECK(!dispatch.psxNoiseRegisterAuthority);
    CHECK(!dispatch.psxSpuRegisterAuthority);

    for (uint32_t call = 1u; call < 16u; ++call) {
        CHECK(TryExecuteSsDriverVoiceCompletion80032B00(
            surface, observation, state, dispatch));
    }
    CHECK(dispatch.completionRingIndexAfter == 0u);
    CHECK(dispatch.stableZeroEnvelopeMask == 0x5u);
    CHECK(dispatch.clearedVoiceStatusMask == 0x5u);
    CHECK(dispatch.noiseMaskClearCallCount == 1u);
    CHECK(state.voiceStatus[0] == 0u);
    CHECK(state.voiceStatus[2] == 0u);

    SsDriverVoiceCompletionState80032B00 entry15Excluded{};
    entry15Excluded.completionRing.fill(1u);
    entry15Excluded.completionRing[15] = 0u;
    entry15Excluded.voiceStatus[0] = 2u;
    SsDriverVoiceCompletionObservation80032B00 oneVoice{};
    oneVoice.voiceCount = 1u;
    oneVoice.envelopeObservations[0] = 0u;
    CHECK(TryExecuteSsDriverVoiceCompletion80032B00(
        surface, oneVoice, entry15Excluded, dispatch));
    CHECK(dispatch.stableZeroEnvelopeMask == 1u);
    CHECK(dispatch.noiseMaskClearCallCount == 1u);

    SsDriverVoiceCompletionState80032B00 disabled{};
    disabled.voiceStatus[0] = 2u;
    oneVoice.completionScanDisabled = true;
    CHECK(TryExecuteSsDriverVoiceCompletion80032B00(
        surface, oneVoice, disabled, dispatch));
    CHECK(dispatch.completionScanSkipped);
    CHECK(dispatch.stableZeroEnvelopeMask == 0u);
    CHECK(dispatch.clearedVoiceStatusMask == 0u);
    CHECK(disabled.voiceStatus[0] == 2u);

    auto mutated = surface;
    mutated.stableCompletionEntryCount = 16u;
    CHECK(!IsExactSsDriverVoiceCompletionSurface80032B00(mutated));
    CHECK(!TryProjectSsDriverVoiceEnvelope80032B00(
        mutated, oneVoice, completionOwners));
    CHECK(!TryExecuteSsDriverVoiceCompletion80032B00(
        mutated, oneVoice, disabled, dispatch));
    oneVoice.voiceCount = 25u;
    CHECK(!TryProjectSsDriverVoiceEnvelope80032B00(
        surface, oneVoice, completionOwners));
    CHECK(!TryExecuteSsDriverVoiceCompletion80032B00(
        surface, oneVoice, disabled, dispatch));
}

void TestHostAudioVoiceGenerationLease() {
    auto& engine = AudioEngine::Get();
    engine.ResetAllVoices();
    const int first = engine.AllocVoice(1, 44100u, 1.0f, true);
    CHECK(first >= 0);
    const uint64_t firstGeneration =
        engine.GetVoiceGeneration(first);
    CHECK(firstGeneration != 0u);
    CHECK(engine.IsVoiceLeaseActive(first, firstGeneration));
    CHECK(!engine.FreeVoiceIfGeneration(
        first, firstGeneration + 1u));
    CHECK(engine.IsVoiceLeaseActive(first, firstGeneration));
    CHECK(engine.FreeVoiceIfGeneration(first, firstGeneration));
    CHECK(!engine.IsVoiceLeaseActive(first, firstGeneration));

    const int second = engine.AllocVoice(1, 44100u, 1.0f, true);
    CHECK(second == first);
    const uint64_t secondGeneration =
        engine.GetVoiceGeneration(second);
    CHECK(secondGeneration != firstGeneration);
    CHECK(!engine.IsVoiceLeaseActive(second, firstGeneration));
    CHECK(engine.IsVoiceLeaseActive(second, secondGeneration));
    CHECK(!engine.FreeVoiceIfGeneration(second, firstGeneration));
    CHECK(engine.IsVoiceLeaseActive(second, secondGeneration));
    const int fixed = engine.AllocVoiceAt(
        second, 1, 22050u, 0.5f, true);
    CHECK(fixed == second);
    const uint64_t fixedGeneration =
        engine.GetVoiceGeneration(fixed);
    CHECK(fixedGeneration != secondGeneration);
    CHECK(!engine.IsVoiceLeaseActive(fixed, secondGeneration));
    CHECK(engine.IsVoiceLeaseActive(fixed, fixedGeneration));
    CHECK(engine.GetVoiceSampleRate(fixed) == 22050u);
    CHECK(!engine.FreeVoiceIfGeneration(second, secondGeneration));
    CHECK(engine.FreeVoiceIfGeneration(fixed, fixedGeneration));
}

void TestSsDriverInitializationOwner8003226C() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsDriverInitializeSurface8003226C surface =
        BuildSsDriverInitializeSurface8003226C();
    CHECK(IsExactSsDriverInitializeSurface8003226C(surface));
    CHECK(surface.known);
    CHECK(surface.driverStateProducerCompleteWithinLimits);
    CHECK(surface.function == 0x8003226Cu);
    CHECK(surface.callerFunction == 0x8002ADE0u);
    CHECK(surface.callerCall == 0x8002AE60u);
    CHECK(surface.callerRequestedVoiceCount == 24u);
    CHECK(surface.driverControlFunction == 0x8002EB44u);
    CHECK(surface.driverControlCall == 0x8003227Cu);
    CHECK(surface.driverControlAddress == 0x800555F8u);
    CHECK(surface.spuInitMallocFunction == 0x80035394u);
    CHECK(surface.spuInitMallocCall == 0x8003229Cu);
    CHECK(surface.spuInitMallocRecordCount == 32u);
    CHECK(surface.spuInitMallocTableAddress == 0x80088268u);
    CHECK(surface.voiceCountAddress == 0x800928A0u);
    CHECK(surface.voiceStateBaseAddress == 0x80087D40u);
    CHECK(surface.voiceStateStride == 0x34u);
    CHECK(surface.pendingVoiceRegisterBaseAddress == 0x80087BA8u);
    CHECK(surface.pendingVoiceRegisterStride == 0x10u);
    CHECK(surface.dirtyFlagsAddress == 0x80087D28u);
    CHECK(surface.completionScratchBaseAddress == 0x800928F8u);
    CHECK(surface.completionScratchByteCount == 16u);
    CHECK(surface.lastVoiceIndexAddress == 0x800928F2u);
    CHECK(surface.masterVolumeLeftAddress == 0x8008ECC8u);
    CHECK(surface.masterVolumeRightAddress == 0x8008ECCAu);
    CHECK(surface.completionScanDisableAddress == 0x8009290Cu);
    CHECK(surface.monoFlagAddress == 0x80091728u);
    CHECK(surface.driverScalarAddress == 0x800917A8u);
    CHECK(surface.driverCommitFunction == 0x80032B00u);
    CHECK(surface.driverCommitCall == 0x80032620u);
    CHECK(surface.maxVoiceCount == 24u);
    CHECK(surface.defaultPitch == 0x1000u);
    CHECK(surface.defaultStartAddress == 0x0200u);
    CHECK(surface.defaultAdsr1 == 0x80FFu);
    CHECK(surface.defaultAdsr2 == 0x4000u);
    CHECK(surface.defaultMasterVolume == 0x3FFFu);
    CHECK(surface.defaultDriverScalar == 0x0080u);
    CHECK(!surface.psxSpuAllocatorAuthority);
    CHECK(!surface.psxSpuRegisterAuthority);
    CHECK(!surface.dynamicReplayAuthority);
    CHECK(!surface.oldWinS0Authority);
    CHECK(!surface.stage2PlusAuthority);

    std::array<SsDriverVoiceOwnerState8003226C, 24> owners{};
    SsDriverVoiceCompletionState80032B00 completion{};
    std::array<SsDriverVoiceRegisterState80032B00, 24> voices{};
    SsDriverVolumeMix80032B00 mix{};
    SsDriverGlobalMaskState80032B00 masks{};
    SsDriverInitializeState8003226C state{};
    for (uint32_t voice = 0u; voice < 24u; ++voice) {
        owners[voice].field80087D42 = 99u;
        completion.voiceStatus[voice] = 3u;
        voices[voice].dirtyFlags = 0x1Fu;
        voices[voice].pendingVolumeLeft = 1u;
        voices[voice].pendingVolumeRight = 2u;
        voices[voice].pendingPitch = 3u;
        voices[voice].pendingStartAddress = 4u;
        voices[voice].pendingAdsr1 = 5u;
        voices[voice].pendingAdsr2 = 6u;
        voices[voice].firstRamp.active = 7;
        voices[voice].secondRamp.active = 8;
    }
    mix.masterVolume = 91u;
    mix.mix800928E2 = 92u;
    mix.pan800928E3 = 93u;
    mix.mix800928E5 = 94u;
    mix.pan800928E6 = 95u;
    mix.monoFlag80091728 = 1;
    masks = {1u, 2u, 3u, 4u, 5u, 6u};
    state.word800928C8 = 9u;
    state.word80091688 = 10u;
    state.word801C35F0 = 11u;
    state.completionScratch800928F8.fill(12u);
    state.lastVoiceIndex800928F2 = 13u;
    state.completionScanDisabled8009290C = true;

    SsDriverCommitProbe80032B00 probe{};
    probe.result = 17;
    SsDriverInitializeDispatch8003226C dispatch{};
    CHECK(TryInitializeSsDriverState8003226C(
        surface,
        2u,
        owners,
        completion,
        voices,
        mix,
        masks,
        state,
        CaptureSsDriverCommit80032B00,
        &probe,
        dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.requestedVoiceCount == 2u);
    CHECK(dispatch.configuredVoiceCount == 2u);
    CHECK(dispatch.clearedPendingRegisterVoiceMask == 0x00FFFFFFu);
    CHECK(dispatch.clearedDirtyVoiceMask == 0x00FFFFFFu);
    CHECK(dispatch.initializedVoiceMask == 0x3u);
    CHECK(dispatch.driverControlExecuted);
    CHECK(dispatch.spuInitMallocProjected);
    CHECK(dispatch.driverCommitExecuted);
    CHECK(dispatch.driverCommitResult == 17);
    CHECK(dispatch.hostProjection);
    CHECK(!dispatch.psxSpuAllocatorAuthority);
    CHECK(!dispatch.psxSpuRegisterAuthority);
    CHECK(probe.callCount == 1u);
    CHECK(state.driverControl800555F8 == 1);
    CHECK(state.spuMallocRecordCount == 32u);
    CHECK(state.spuMallocTableAddress == 0x80088268u);
    CHECK(state.word800928C8 == 0u);
    CHECK(state.word80091688 == 0u);
    CHECK(state.word801C35F0 == 0u);
    for (const uint8_t value : state.completionScratch800928F8) {
        CHECK(value == 0u);
    }
    CHECK(state.configuredVoiceCount800928A0 == 2u);
    CHECK(state.lastVoiceIndex800928F2 == 1u);
    CHECK(state.masterVolumeLeft8008ECC8 == 0x3FFFu);
    CHECK(state.masterVolumeRight8008ECCA == 0x3FFFu);
    CHECK(!state.completionScanDisabled8009290C);
    CHECK(state.driverScalar800917A8 == 0x0080u);
    CHECK(owners[0].field80087D40 == 0u);
    CHECK(owners[0].field80087D42 == 24u);
    CHECK(owners[0].pan80087D4A == 64u);
    CHECK(owners[0].packedSequenceTrack80087D4E == -1);
    CHECK(owners[0].toneIndex80087D54 == 255u);
    CHECK(completion.voiceStatus[0] == 0u);
    CHECK(completion.voiceStatus[1] == 0u);
    CHECK(completion.voiceStatus[2] == 3u);
    CHECK(voices[0].firstRamp.active == 0);
    CHECK(voices[1].secondRamp.active == 0);
    CHECK(voices[2].firstRamp.active == 7);
    CHECK(voices[2].secondRamp.active == 8);
    for (const auto& voice : voices) {
        CHECK(voice.dirtyFlags == 0u);
        CHECK(voice.pendingVolumeLeft == 0u);
        CHECK(voice.pendingVolumeRight == 0u);
    }
    CHECK(voices[0].pendingPitch == 0x1000u);
    CHECK(voices[0].pendingStartAddress == 0x0200u);
    CHECK(voices[0].pendingAdsr1 == 0x80FFu);
    CHECK(voices[0].pendingAdsr2 == 0x4000u);
    CHECK(voices[2].pendingPitch == 0u);
    CHECK(mix.masterVolume == 91u);
    CHECK(mix.mix800928E2 == 92u);
    CHECK(mix.pan800928E3 == 93u);
    CHECK(mix.mix800928E5 == 94u);
    CHECK(mix.pan800928E6 == 95u);
    CHECK(mix.monoFlag80091728 == 0);
    CHECK(masks.keyOffLow == 0u);
    CHECK(masks.keyOffHigh == 0u);
    CHECK(masks.keyOnLow == 0u);
    CHECK(masks.keyOnHigh == 0u);
    CHECK(masks.reverbLow == 0u);
    CHECK(masks.reverbHigh == 0u);

    CHECK(TryInitializeSsDriverState8003226C(
        surface,
        25u,
        owners,
        completion,
        voices,
        mix,
        masks,
        state,
        CaptureSsDriverCommit80032B00,
        &probe,
        dispatch));
    CHECK(dispatch.configuredVoiceCount == 24u);
    CHECK(dispatch.initializedVoiceMask == 0x00FFFFFFu);
    CHECK(state.lastVoiceIndex800928F2 == 23u);
    auto mutated = surface;
    mutated.defaultPitch ^= 1u;
    CHECK(!TryInitializeSsDriverState8003226C(
        mutated,
        24u,
        owners,
        completion,
        voices,
        mix,
        masks,
        state,
        CaptureSsDriverCommit80032B00,
        &probe,
        dispatch));
    CHECK(!TryInitializeSsDriverState8003226C(
        surface,
        24u,
        owners,
        completion,
        voices,
        mix,
        masks,
        state,
        nullptr,
        &probe,
        dispatch));
}

void TestSsDriverResetStateProducer800351B8() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsDriverResetSurface800351B8 surface =
        BuildSsDriverResetSurface800351B8();
    CHECK(IsExactSsDriverResetSurface800351B8(surface));
    CHECK(surface.known);
    CHECK(surface.resetStateProducerCompleteWithinLimits);
    CHECK(surface.wrapperFunction == 0x80026FA4u);
    CHECK(surface.wrapperCall == 0x80026FACu);
    CHECK(surface.resetFunction == 0x800351B8u);
    CHECK(surface.voiceCountAddress == 0x800928A0u);
    CHECK(surface.voiceStateBaseAddress == 0x80087D40u);
    CHECK(surface.voiceStateStride == 0x34u);
    CHECK(surface.completionStatusOffset == 0x1Bu);
    CHECK(surface.pendingVoiceRegisterBaseAddress == 0x80087BA8u);
    CHECK(surface.pendingVoiceRegisterStride == 0x10u);
    CHECK(surface.lastVoiceIndexAddress == 0x800928F2u);
    CHECK(surface.keyOffLowAddress == 0x801C386Cu);
    CHECK(surface.keyOffHighAddress == 0x801C386Eu);
    CHECK(surface.keyOnLowAddress == 0x8008EC98u);
    CHECK(surface.keyOnHighAddress == 0x8008EC9Au);
    CHECK(surface.maxVoiceCount == 24u);
    CHECK(surface.defaultOwnerKind == 24u);
    CHECK(surface.defaultOwnerSentinel == 255u);
    CHECK(surface.defaultPitch == 0x1000u);
    CHECK(surface.defaultStartAddress == 0x0200u);
    CHECK(surface.defaultAdsr1 == 0x80FFu);
    CHECK(surface.defaultAdsr2 == 0x4000u);
    CHECK(!surface.callsDriverCommit);
    CHECK(!surface.psxSpuRegisterAuthority);
    CHECK(!surface.psxInterruptTimingAuthority);
    CHECK(!surface.dynamicReplayAuthority);
    CHECK(!surface.oldWinS0Authority);
    CHECK(!surface.stage2PlusAuthority);

    std::array<SsDriverVoiceOwnerState8003226C, 24> owners{};
    SsDriverVoiceCompletionState80032B00 completion{};
    std::array<SsDriverVoiceRegisterState80032B00, 24> voices{};
    SsDriverGlobalMaskState80032B00 masks{};
    SsDriverInitializeState8003226C state{};
    state.configuredVoiceCount800928A0 = 2u;
    state.lastVoiceIndex800928F2 = 99u;
    for (uint32_t voice = 0u; voice < 3u; ++voice) {
        owners[voice].field80087D40 =
            static_cast<uint16_t>(100u + voice);
        owners[voice].field80087D42 =
            static_cast<uint16_t>(110u + voice);
        owners[voice].field80087D44 =
            static_cast<uint16_t>(120u + voice);
        owners[voice].field80087D46 =
            static_cast<uint16_t>(130u + voice);
        owners[voice].velocity80087D48 =
            static_cast<uint16_t>(140u + voice);
        owners[voice].pan80087D4A =
            static_cast<uint8_t>(50u + voice);
        owners[voice].note80087D4C =
            static_cast<uint16_t>(150u + voice);
        owners[voice].packedSequenceTrack80087D4E =
            static_cast<int16_t>(160 + voice);
        owners[voice].toneTableProgram80087D50 =
            static_cast<uint16_t>(170u + voice);
        owners[voice].program80087D52 =
            static_cast<uint16_t>(180u + voice);
        owners[voice].toneIndex80087D54 =
            static_cast<uint16_t>(190u + voice);
        owners[voice].sourceVabId80087D56 =
            static_cast<uint16_t>(200u + voice);
        owners[voice].priority80087D58 =
            static_cast<uint16_t>(210u + voice);
        completion.voiceStatus[voice] =
            static_cast<uint8_t>(3u + voice);
        voices[voice].dirtyFlags =
            static_cast<uint8_t>(0x10u + voice);
        voices[voice].pendingVolumeLeft = 1u;
        voices[voice].pendingVolumeRight = 2u;
        voices[voice].pendingPitch = 3u;
        voices[voice].pendingStartAddress = 4u;
        voices[voice].pendingAdsr1 = 5u;
        voices[voice].pendingAdsr2 = 6u;
        voices[voice].firstRamp.active =
            static_cast<int16_t>(7 + voice);
        voices[voice].secondRamp.active =
            static_cast<int16_t>(10 + voice);
    }
    masks.keyOffLow = 0x0004u;
    masks.keyOffHigh = 0x0002u;
    masks.keyOnLow = 0xFFFFu;
    masks.keyOnHigh = 0xAAAAu;
    masks.reverbLow = 0x1234u;
    masks.reverbHigh = 0x0056u;

    SsDriverResetDispatch800351B8 dispatch{};
    CHECK(TryExecuteSsDriverReset800351B8(
        surface,
        owners,
        completion,
        voices,
        masks,
        state,
        dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.returnedVoiceCount == 2u);
    CHECK(!dispatch.zeroVoiceCountNoOp);
    CHECK(dispatch.resetVoiceMask == 0x3u);
    CHECK(dispatch.keyOffLowAfter == 0x0007u);
    CHECK(dispatch.keyOffHighAfter == 0x0002u);
    CHECK(dispatch.keyOnLowAfter == 0xFFF8u);
    CHECK(dispatch.keyOnHighAfter == 0xAAAAu);
    CHECK(dispatch.pendingRegistersReinitialized);
    CHECK(dispatch.dirtyFlagsPreserved);
    CHECK(dispatch.rampStatePreserved);
    CHECK(dispatch.untouchedOwnerFieldsPreserved);
    CHECK(dispatch.reverbMasksPreserved);
    CHECK(!dispatch.driverCommitExecuted);
    CHECK(dispatch.hostVoiceResetProjectionReady);
    CHECK(!dispatch.psxSpuRegisterAuthority);
    CHECK(!dispatch.psxInterruptTimingAuthority);
    CHECK(state.lastVoiceIndex800928F2 == 1u);
    for (uint32_t voice = 0u; voice < 2u; ++voice) {
        CHECK(owners[voice].field80087D40 == 0u);
        CHECK(owners[voice].field80087D42 == 24u);
        CHECK(owners[voice].field80087D44 == 0u);
        CHECK(owners[voice].field80087D46 == 0u);
        CHECK(owners[voice].packedSequenceTrack80087D4E == 255);
        CHECK(owners[voice].toneTableProgram80087D50 == 0u);
        CHECK(owners[voice].program80087D52 == 0u);
        CHECK(owners[voice].toneIndex80087D54 == 255u);
        CHECK(owners[voice].velocity80087D48 == 140u + voice);
        CHECK(owners[voice].pan80087D4A == 50u + voice);
        CHECK(owners[voice].note80087D4C == 150u + voice);
        CHECK(owners[voice].sourceVabId80087D56 == 200u + voice);
        CHECK(owners[voice].priority80087D58 == 210u + voice);
        CHECK(completion.voiceStatus[voice] == 0u);
        CHECK(voices[voice].dirtyFlags == 0x10u + voice);
        CHECK(voices[voice].pendingVolumeLeft == 0u);
        CHECK(voices[voice].pendingVolumeRight == 0u);
        CHECK(voices[voice].pendingPitch == 0x1000u);
        CHECK(voices[voice].pendingStartAddress == 0x0200u);
        CHECK(voices[voice].pendingAdsr1 == 0x80FFu);
        CHECK(voices[voice].pendingAdsr2 == 0x4000u);
        CHECK(voices[voice].firstRamp.active == 7 + voice);
        CHECK(voices[voice].secondRamp.active == 10 + voice);
    }
    CHECK(owners[2].field80087D40 == 102u);
    CHECK(owners[2].packedSequenceTrack80087D4E == 162);
    CHECK(completion.voiceStatus[2] == 5u);
    CHECK(voices[2].pendingPitch == 3u);
    CHECK(voices[2].dirtyFlags == 0x12u);
    CHECK(masks.reverbLow == 0x1234u);
    CHECK(masks.reverbHigh == 0x0056u);

    state.configuredVoiceCount800928A0 = 0u;
    state.lastVoiceIndex800928F2 = 77u;
    owners[0].field80087D42 = 88u;
    voices[0].pendingPitch = 99u;
    masks.keyOffLow = 0x4321u;
    CHECK(TryExecuteSsDriverReset800351B8(
        surface,
        owners,
        completion,
        voices,
        masks,
        state,
        dispatch));
    CHECK(dispatch.zeroVoiceCountNoOp);
    CHECK(dispatch.returnedVoiceCount == 0u);
    CHECK(dispatch.resetVoiceMask == 0u);
    CHECK(!dispatch.hostVoiceResetProjectionReady);
    CHECK(state.lastVoiceIndex800928F2 == 77u);
    CHECK(owners[0].field80087D42 == 88u);
    CHECK(voices[0].pendingPitch == 99u);
    CHECK(masks.keyOffLow == 0x4321u);

    state.configuredVoiceCount800928A0 = 25u;
    CHECK(!TryExecuteSsDriverReset800351B8(
        surface,
        owners,
        completion,
        voices,
        masks,
        state,
        dispatch));
    auto mutated = surface;
    mutated.callsDriverCommit = true;
    state.configuredVoiceCount800928A0 = 2u;
    CHECK(!TryExecuteSsDriverReset800351B8(
        mutated,
        owners,
        completion,
        voices,
        masks,
        state,
        dispatch));
}

void TestSsSpuFirstAllocation8002E87C() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsSpuFirstAllocationSurface8002E87C surface =
        BuildSsSpuFirstAllocationSurface8002E87C();
    CHECK(surface.known);
    CHECK(surface.currentScene0FirstAllocationCompleteWithinLimits);
    CHECK(surface.scene0CallerFunction == 0x80027078u);
    CHECK(surface.vabOpenWrapperFunction == 0x8002E3D8u);
    CHECK(surface.vabLoaderFunction == 0x8002E474u);
    CHECK(surface.spuMallocFunction == 0x8002E87Cu);
    CHECK(surface.allocatorNormalizeFunction == 0x8002E0D8u);
    CHECK(surface.spuInitMallocFunction == 0x80035394u);
    CHECK(surface.recordTableAddress == 0x80088268u);
    CHECK(surface.recordCount == 32u);
    CHECK(surface.initialFreeAddress == 0x1010u);
    CHECK(surface.spuRamEndAddress == 0x80000u);
    CHECK(surface.initialFreeFlag == 0x40000000u);
    CHECK(surface.addressMask == 0x0FFFFFFFu);
    CHECK(surface.alignmentMask == 7u);
    CHECK(surface.alignmentShift == 3u);
    CHECK(surface.activeReserveBytes == 0u);
    CHECK(!surface.fullSpuAllocatorAuthority);
    CHECK(!surface.psxSpuRamTransferAuthority);
    CHECK(!surface.dynamicReplayAuthority);
    CHECK(!surface.oldWinS0Authority);
    CHECK(!surface.stage2PlusAuthority);

    SsSpuFirstAllocationState8002E87C state{};
    SsSpuFirstAllocationDispatch8002E87C dispatch{};
    CHECK(TryInitializeSsSpuAllocator80035394(
        surface, state, dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.initialized);
    CHECK(state.initialized);
    CHECK(state.recordCount == 32u);
    CHECK(state.recordTableAddress == 0x80088268u);
    CHECK(state.records[0].addressAndFlags == 0x40001010u);
    CHECK(state.records[0].size == 0x7EFF0u);
    CHECK(dispatch.remainingFreeAddress == 0x1010u);
    CHECK(dispatch.remainingFreeBytes == 0x7EFF0u);

    constexpr uint32_t kScene0VabBytes = 55600u;
    CHECK(TryAllocateFirstSsSpuBlock8002E87C(
        surface, kScene0VabBytes, state, dispatch));
    CHECK(dispatch.allocationAttempted);
    CHECK(dispatch.allocationSucceeded);
    CHECK(dispatch.requestedBytes == kScene0VabBytes);
    CHECK(dispatch.alignedBytes == kScene0VabBytes);
    CHECK(dispatch.allocatedAddress == 0x1010u);
    CHECK(dispatch.remainingFreeAddress ==
          0x1010u + kScene0VabBytes);
    CHECK(dispatch.remainingFreeBytes ==
          0x7EFF0u - kScene0VabBytes);
    CHECK(dispatch.sampleStartProductionReady);
    CHECK(!dispatch.fullSpuAllocatorAuthority);
    CHECK(!dispatch.psxSpuRamTransferAuthority);
    CHECK(state.allocationCount == 1u);
    CHECK(state.highWaterRecordIndex == 1u);
    CHECK(state.records[0].addressAndFlags == 0x1010u);
    CHECK(state.records[0].size == kScene0VabBytes);
    CHECK(state.records[1].addressAndFlags ==
          (0x40000000u | (0x1010u + kScene0VabBytes)));
    CHECK(state.records[1].size ==
          0x7EFF0u - kScene0VabBytes);

    CHECK(TryAllocateFirstSsSpuBlock8002E87C(
        surface, 8u, state, dispatch));
    CHECK(!dispatch.allocationSucceeded);

    SsSpuFirstAllocationState8002E87C alignedState{};
    CHECK(TryInitializeSsSpuAllocator80035394(
        surface, alignedState, dispatch));
    CHECK(TryAllocateFirstSsSpuBlock8002E87C(
        surface, 17u, alignedState, dispatch));
    CHECK(dispatch.allocationSucceeded);
    CHECK(dispatch.alignedBytes == 24u);

    SsSpuFirstAllocationState8002E87C oversizedState{};
    CHECK(TryInitializeSsSpuAllocator80035394(
        surface, oversizedState, dispatch));
    CHECK(TryAllocateFirstSsSpuBlock8002E87C(
        surface, 0x7EFF1u, oversizedState, dispatch));
    CHECK(!dispatch.allocationSucceeded);

    auto mutated = surface;
    mutated.alignmentMask = 15u;
    CHECK(!TryInitializeSsSpuAllocator80035394(
        mutated, state, dispatch));
    CHECK(!TryAllocateFirstSsSpuBlock8002E87C(
        mutated, 8u, state, dispatch));
}

void TestSsVabBodyTransfer8002EB80() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsVabBodyTransferSurface8002EB80 surface =
        BuildSsVabBodyTransferSurface8002EB80();
    CHECK(IsExactSsVabBodyTransferSurface8002EB80(surface));
    CHECK(surface.known);
    CHECK(surface.firstScene0VabTransferCompleteWithinLimits);
    CHECK(surface.completionControlCompleteWithinLimits);
    CHECK(surface.initializeFunction == 0x8003226Cu);
    CHECK(surface.openFunction == 0x8002E474u);
    CHECK(surface.transferWrapperFunction == 0x800270D4u);
    CHECK(surface.transferFunction == 0x8002EB80u);
    CHECK(surface.setTransferModeFunction == 0x8002ECDCu);
    CHECK(surface.setTransferStartFunction == 0x8002ECA0u);
    CHECK(surface.writeFunction == 0x8002EC40u);
    CHECK(surface.transferReadyFunction == 0x8002EB44u);
    CHECK(surface.completionWrapperFunction == 0x800270FCu);
    CHECK(surface.completionFunction == 0x8002EEFCu);
    CHECK(surface.testEventFunction == 0x8002EF28u);
    CHECK(surface.vabStatusAddress == 0x800928F8u);
    CHECK(surface.vabSpuBaseAddress == 0x801C35F8u);
    CHECK(surface.vabTransferBytesAddress == 0x801C35B0u);
    CHECK(surface.transferStartAddress == 0x800555C4u);
    CHECK(surface.transferReadyAddress == 0x800555F8u);
    CHECK(surface.transferModeAddress == 0x80055624u);
    CHECK(surface.transferPathAddress == 0x800555E0u);
    CHECK(surface.initializedVabSlotCount == 16u);
    CHECK(surface.transferAcceptedStatus == 2u);
    CHECK(surface.transferPendingStatus == 1u);
    CHECK(surface.transferAlignmentBytes == 8u);
    CHECK(surface.transferAddressShift == 3u);
    CHECK(surface.maximumTransferBytes == 0x7F000u);
    CHECK(surface.spuRamEndAddress == 0x80000u);
    CHECK(!surface.psxDmaAuthority);
    CHECK(!surface.psxEventAuthority);
    CHECK(!surface.psxSpuRamAuthority);
    CHECK(!surface.dynamicReplayAuthority);
    CHECK(!surface.oldWinS0Authority);
    CHECK(!surface.stage2PlusAuthority);

    constexpr uint32_t kScene0VabBytes = 55600u;
    SsVabBodyTransferState8002EB80 state{};
    CHECK(TryPrepareFirstSsVabBodyTransferState8002E474(
        surface, 0x1010u, kScene0VabBytes, state));
    CHECK(state.vabStatus800928F8[0] == 2u);
    CHECK(state.vabSpuBase801C35F8[0] == 0x1010u);
    CHECK(state.vabTransferBytes801C35B0[0] ==
          kScene0VabBytes);
    CHECK(state.transferReady800555F8);

    SsVabBodyTransferDispatch8002EB80 transfer{};
    CHECK(TryBeginSsVabBodyTransfer8002EB80(
        surface, 0u, kScene0VabBytes, state, transfer));
    CHECK(transfer.known);
    CHECK(transfer.accepted);
    CHECK(transfer.result == 0);
    CHECK(transfer.vabId == 0u);
    CHECK(transfer.statusBefore == 2u);
    CHECK(transfer.statusAfter == 1u);
    CHECK(transfer.requestedTransferBytes == kScene0VabBytes);
    CHECK(transfer.committedTransferBytes == kScene0VabBytes);
    CHECK(transfer.spuBaseBeforeAlignment == 0x1010u);
    CHECK(transfer.spuBaseAfterAlignment == 0x1010u);
    CHECK(transfer.transferStartUnits == 0x0202u);
    CHECK(transfer.sourceRangeAvailable);
    CHECK(transfer.hostPayloadProjectionReady);
    CHECK(!transfer.psxDmaCommitted);
    CHECK(!transfer.psxDmaAuthority);
    CHECK(!transfer.psxSpuRamAuthority);
    CHECK(state.vabStatus800928F8[0] == 1u);
    CHECK(state.transferMode80055624 == 0u);
    CHECK(!state.alternateTransferPath800555E0);
    CHECK(!state.transferReady800555F8);

    SsVabTransferCompletionDispatch8002EF28 completion{};
    CHECK(TryCompleteSsVabBodyTransfer8002EEFC(
        surface, true, true, state, completion));
    CHECK(completion.known);
    CHECK(completion.waitRequested);
    CHECK(completion.eventObserved);
    CHECK(completion.testEventRequested);
    CHECK(!completion.wouldBlock);
    CHECK(completion.completionLatched);
    CHECK(completion.result == 1);
    CHECK(state.transferReady800555F8);
    CHECK(!completion.psxEventAuthority);
    CHECK(!completion.psxDmaAuthority);

    CHECK(TryBeginSsVabBodyTransfer8002EB80(
        surface, 0u, kScene0VabBytes, state, transfer));
    CHECK(!transfer.accepted);
    CHECK(transfer.result == -1);
    CHECK(state.transferReady800555F8);

    SsVabBodyTransferState8002EB80 alignedState{};
    CHECK(TryPrepareFirstSsVabBodyTransferState8002E474(
        surface, 0x1011u, 24u, alignedState));
    CHECK(TryBeginSsVabBodyTransfer8002EB80(
        surface, 0u, 24u, alignedState, transfer));
    CHECK(transfer.accepted);
    CHECK(transfer.spuBaseAfterAlignment == 0x1018u);
    CHECK(transfer.transferStartUnits == 0x0203u);
    CHECK(TryCompleteSsVabBodyTransfer8002EEFC(
        surface, false, false, alignedState, completion));
    CHECK(completion.testEventRequested);
    CHECK(!completion.wouldBlock);
    CHECK(!completion.completionLatched);
    CHECK(completion.result == 0);
    CHECK(!alignedState.transferReady800555F8);
    CHECK(TryCompleteSsVabBodyTransfer8002EEFC(
        surface, true, false, alignedState, completion));
    CHECK(completion.wouldBlock);
    CHECK(completion.result == 0);

    SsVabBodyTransferState8002EB80 shortSourceState{};
    CHECK(TryPrepareFirstSsVabBodyTransferState8002E474(
        surface, 0x1010u, kScene0VabBytes, shortSourceState));
    CHECK(TryBeginSsVabBodyTransfer8002EB80(
        surface, 0u, kScene0VabBytes - 1u,
        shortSourceState, transfer));
    CHECK(transfer.accepted);
    CHECK(!transfer.sourceRangeAvailable);
    CHECK(!transfer.hostPayloadProjectionReady);

    SsVabBodyTransferState8002EB80 invalidState{};
    CHECK(!TryPrepareFirstSsVabBodyTransferState8002E474(
        surface, 0u, kScene0VabBytes, invalidState));
    CHECK(TryBeginSsVabBodyTransfer8002EB80(
        surface, 17u, kScene0VabBytes, invalidState, transfer));
    CHECK(!transfer.accepted);

    auto mutated = surface;
    mutated.maximumTransferBytes = 0x7E000u;
    CHECK(!TryPrepareFirstSsVabBodyTransferState8002E474(
        mutated, 0x1010u, kScene0VabBytes, invalidState));
    CHECK(!TryBeginSsVabBodyTransfer8002EB80(
        mutated, 0u, kScene0VabBytes, invalidState, transfer));
    CHECK(!TryCompleteSsVabBodyTransfer8002EEFC(
        mutated, true, true, invalidState, completion));
}

void TestSsCompactSfxCommand80026EF8() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsCompactSfxSurface80034240 surface =
        BuildSsCompactSfxSurface80034240();
    CHECK(IsExactSsCompactSfxSurface80034240(surface));
    CHECK(surface.known);
    CHECK(surface.current80026EF8RouteCompleteWithinLimits);
    CHECK(surface.wrapperFunction == 0x80026EF8u);
    CHECK(surface.function == 0x80034240u);
    CHECK(surface.validationFunction == 0x8002F13Cu);
    CHECK(surface.allocatorFunction == 0x80030544u);
    CHECK(surface.allocatorNoiseMaskClearFunction == 0x800353E8u);
    CHECK(surface.allocatorNoiseMaskClearValue == 0x00FFFFFFu);
    CHECK(surface.prepareFunction == 0x80030C90u);
    CHECK(surface.noiseStartFunction == 0x80030EA4u);
    CHECK(surface.pitchFunction == 0x800315C8u);
    CHECK(surface.regularStartFunction == 0x800307ACu);
    CHECK(surface.sourceVabIdAddress == 0x800943A8u);
    CHECK(surface.resultVoiceAddress == 0x800943ACu);
    CHECK(surface.reentryGuardAddress == 0x800917A4u);
    CHECK(surface.programCountAddress == 0x800917A8u);
    CHECK(surface.vabStatusAddress == 0x800928F8u);
    CHECK(surface.programAttributeTableAddress == 0x800917D0u);
    CHECK(surface.toneAttributeTableAddress == 0x800917DCu);
    CHECK(surface.voiceStateBaseAddress == 0x80087D40u);
    CHECK(surface.voiceStateStride == 0x34u);
    CHECK(surface.cuePitchAdd == 0x18u);
    CHECK(surface.acceptedVabSlotCount == 16u);
    CHECK(surface.maximumProgramCount == 128u);
    CHECK(surface.maximumToneSlotCount == 16u);
    CHECK(surface.compactOwnerTag == 33u);
    CHECK(surface.equalVolumeCurrentWrapperOnly);
    CHECK(surface.fineAdjustZeroCurrentWrapperOnly);
    CHECK(!surface.psxVoiceIdentityAuthority);
    CHECK(!surface.psxNoiseRegisterAuthority);
    CHECK(!surface.psxSpuRegisterAuthority);
    CHECK(!surface.psxSpuTimingAuthority);
    CHECK(!surface.dynamicReplayAuthority);
    CHECK(!surface.oldWinS0Authority);
    CHECK(!surface.stage2PlusAuthority);

    SsCompactSfxRequest80034240 request{};
    request.bankReady = true;
    request.sourceVabId = 0;
    request.command = {0u, 1u, 0u, 90u};
    request.vabMasterVolume = 127u;
    request.programAttributes.valid = true;
    request.programAttributes.toneCount = 16u;
    request.programAttributes.volume = 127u;
    request.programAttributes.priority = 6u;
    request.programAttributes.pan = 64u;
    request.programAttributes.toneTableProgram = 0u;
    request.tone.valid = true;
    request.tone.toneSlot = 1u;
    request.tone.toneIndex = 1u;
    request.tone.priority = 6u;
    request.tone.volume = 127u;
    request.tone.pan = 64u;
    request.tone.centerNote = 60u;
    request.tone.centerFine = 0u;
    request.tone.noteMin = 0u;
    request.tone.noteMax = 127u;
    request.tone.adsr1 = 0x1234u;
    request.tone.adsr2 = 0x5678u;
    request.tone.sampleId = 1u;
    request.tone.sampleStartAddressKnown = true;
    request.tone.sampleStartAddress = 0x0222u;

    SsDriverFlushState80026ECC reentry{};
    SsDriverInitializeState8003226C initialize{};
    initialize.configuredVoiceCount800928A0 = 24u;
    std::array<SsDriverVoiceOwnerState8003226C, 24> owners{};
    SsDriverVoiceCompletionState80032B00 completion{};
    std::array<SsDriverVoiceRegisterState80032B00, 24> voices{};
    SsDriverVolumeMix80032B00 mix{};
    SsDriverVolumeScratch80032B00 scratch{};
    SsDriverGlobalMaskState80032B00 masks{};
    owners[0].pan80087D4A = 0x33u;
    masks.keyOnLow = 0x0002u;
    masks.keyOffLow = 0xFFFFu;
    SsCompactSfxDispatch80034240 dispatch{};
    CHECK(TryExecuteSsCompactSfxCommand80026EF8(
        surface,
        request,
        reentry,
        initialize,
        owners,
        completion,
        voices,
        mix,
        scratch,
        masks,
        dispatch));
    CHECK(dispatch.known);
    CHECK(dispatch.commandMutated80026EF8);
    CHECK(dispatch.commandBefore[2] == 0u);
    CHECK(dispatch.commandAfter[2] == 25u);
    CHECK(dispatch.reentryGuardSet);
    CHECK(dispatch.reentryGuardCleared);
    CHECK(!dispatch.reentrySkipped);
    CHECK(!dispatch.validationRejected);
    CHECK(!dispatch.allocationFailed);
    CHECK(dispatch.allocatorNoiseMaskClearCallCount == 0u);
    CHECK(dispatch.allocatorNoiseMaskClearValue == 0u);
    CHECK(dispatch.selectedVoice == 0u);
    CHECK(dispatch.word800928F4 == 0u);
    CHECK(dispatch.word800928F6 == 1u);
    CHECK(dispatch.transientScratchCommitted);
    CHECK(dispatch.byte800928DA == 25u);
    CHECK(dispatch.byte800928DB == 0u);
    CHECK(dispatch.byte800928D8 == request.programAttributes.toneCount);
    CHECK(dispatch.byte800928DF == request.programAttributes.toneTableProgram);
    CHECK(dispatch.byte800928E4 == 1u);
    CHECK(dispatch.byte800928E5 == 127u);
    CHECK(dispatch.byte800928E6 == 64u);
    CHECK(dispatch.byte800928E7 == 6u);
    CHECK(dispatch.byte800928E8 == 60u);
    CHECK(dispatch.byte800928E9 == 0u);
    CHECK(dispatch.byte800928EA == 0u);
    CHECK(dispatch.byte800928EB == 127u);
    CHECK(dispatch.byte800928EC == request.tone.mode);
    CHECK(dispatch.word800928F0 == 1u);
    CHECK(dispatch.sampleId == 1u);
    CHECK(!dispatch.noiseSample);
    CHECK(dispatch.pitch == 542u);
    CHECK(dispatch.pendingVolumeLeft > 0u);
    CHECK(dispatch.pendingVolumeLeft == dispatch.pendingVolumeRight);
    CHECK(dispatch.dirtyFlagsAfter == 0x3Fu);
    CHECK(dispatch.sampleStartAddressKnown);
    CHECK(dispatch.result == 0);
    CHECK(dispatch.hostPlaybackReady);
    CHECK(!dispatch.psxVoiceIdentityAuthority);
    CHECK(!dispatch.psxNoiseRegisterAuthority);
    CHECK(!dispatch.psxSpuRegisterAuthority);
    CHECK(!dispatch.psxSpuTimingAuthority);
    CHECK(reentry.reentryGuard800917A4 == 0);
    CHECK(initialize.lastVoiceIndex800928F2 == 0u);
    CHECK(initialize.word800928F4 == 0u);
    CHECK(initialize.word800928F6 == 1u);
    CHECK(owners[0].packedSequenceTrack80087D4E == 33);
    CHECK(owners[0].sourceVabId80087D56 == 0u);
    CHECK(owners[0].program80087D52 == 0u);
    CHECK(owners[0].toneTableProgram80087D50 == 0u);
    CHECK(owners[0].toneIndex80087D54 == 1u);
    CHECK(owners[0].note80087D4C == 25u);
    CHECK(owners[0].field80087D40 == 1u);
    CHECK(owners[0].field80087D44 == 542u);
    CHECK(owners[0].field80087D42 == 0u);
    CHECK(owners[0].priority80087D58 == 6u);
    CHECK(owners[0].pan80087D4A == 0x33u);
    CHECK(completion.voiceStatus[0] == 1u);
    CHECK(voices[0].pendingStartAddress == 0x0222u);
    CHECK(voices[0].pendingAdsr1 == 0x1234u);
    CHECK(voices[0].pendingAdsr2 == 0x5678u);
    CHECK(voices[0].pendingPitch == 542u);
    CHECK(voices[0].dirtyFlags == 0x3Fu);
    CHECK(masks.keyOnLow == 0x0003u);
    CHECK(masks.keyOffLow == 0xFFFCu);
    CHECK(scratch.firstCurrent800928DC == 90u);
    CHECK(scratch.secondCurrent800928DD == 64u);

    reentry.reentryGuard800917A4 = 1;
    CHECK(TryExecuteSsCompactSfxCommand80026EF8(
        surface,
        request,
        reentry,
        initialize,
        owners,
        completion,
        voices,
        mix,
        scratch,
        masks,
        dispatch));
    CHECK(dispatch.reentrySkipped);
    CHECK(dispatch.result == -1);
    CHECK(reentry.reentryGuard800917A4 == 1);
    reentry.reentryGuard800917A4 = 0;

    auto rejected = request;
    rejected.tone.sampleId = 0u;
    CHECK(TryExecuteSsCompactSfxCommand80026EF8(
        surface,
        rejected,
        reentry,
        initialize,
        owners,
        completion,
        voices,
        mix,
        scratch,
        masks,
        dispatch));
    CHECK(dispatch.validationRejected);
    CHECK(dispatch.result == -1);
    CHECK(dispatch.reentryGuardCleared);
    CHECK(reentry.reentryGuard800917A4 == 0);

    auto noise = request;
    noise.tone.sampleId = 255u;
    owners = {};
    completion = {};
    voices = {};
    masks = {};
    CHECK(TryExecuteSsCompactSfxCommand80026EF8(
        surface,
        noise,
        reentry,
        initialize,
        owners,
        completion,
        voices,
        mix,
        scratch,
        masks,
        dispatch));
    CHECK(dispatch.noiseSample);
    CHECK(!dispatch.hostPlaybackReady);
    CHECK(completion.voiceStatus[0] == 2u);

    // With only one configured voice, the next regular compact command must
    // reuse the active noise voice.  80030544 requests one exact 800353E8
    // all-noise-mask clear before 80034240 republishes regular state.
    initialize.configuredVoiceCount800928A0 = 1u;
    CHECK(TryExecuteSsCompactSfxCommand80026EF8(
        surface,
        request,
        reentry,
        initialize,
        owners,
        completion,
        voices,
        mix,
        scratch,
        masks,
        dispatch));
    CHECK(dispatch.replacedNoiseVoice);
    CHECK(dispatch.allocatorNoiseMaskClearCallCount == 1u);
    CHECK(dispatch.allocatorNoiseMaskClearValue == 0x00FFFFFFu);
    CHECK(!dispatch.psxNoiseRegisterAuthority);
    CHECK(completion.voiceStatus[0] == 1u);

    auto mutated = surface;
    mutated.pitchFunction = 0x80031510u;
    CHECK(!TryExecuteSsCompactSfxCommand80026EF8(
        mutated,
        request,
        reentry,
        initialize,
        owners,
        completion,
        voices,
        mix,
        scratch,
        masks,
        dispatch));
}

void TestSsDriverVoiceAllocationStateProducer80032EAC() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsDriverVoiceAllocationSurface80032EAC surface =
        BuildSsDriverVoiceAllocationSurface80032EAC();
    CHECK(IsExactSsDriverVoiceAllocationSurface80032EAC(surface));
    CHECK(surface.known);
    CHECK(surface.allocationAndRegisterProducerCompleteWithinKnownInputs);
    CHECK(surface.noteOffStateProducerCompleteWithinLimits);
    CHECK(surface.function == 0x80032EACu);
    CHECK(surface.validationFunction == 0x8002F13Cu);
    CHECK(surface.allocatorFunction == 0x80030544u);
    CHECK(surface.prepareFunction == 0x80030C90u);
    CHECK(surface.noiseStartFunction == 0x80030EA4u);
    CHECK(surface.pitchFunction == 0x80031510u);
    CHECK(surface.regularStartFunction == 0x800307ACu);
    CHECK(surface.noteOffFunction == 0x8003349Cu);
    CHECK(surface.pitchTableAddress == 0x80055DDCu);
    CHECK(surface.pitchTableEntryCount == 192u);
    CHECK(surface.maxVoiceCount == 24u);
    CHECK(surface.maxToneLayers == 16u);
    CHECK(surface.voiceStateBaseAddress == 0x80087D40u);
    CHECK(surface.voiceStateStride == 0x34u);
    CHECK(surface.pendingVoiceRegisterBaseAddress == 0x80087BA8u);
    CHECK(surface.pendingVoiceRegisterStride == 0x10u);
    CHECK(surface.completionRingBaseAddress == 0x80088224u);
    CHECK(surface.completionRingEntryCount == 16u);
    CHECK(!surface.psxSampleStartAddressAuthority);
    CHECK(!surface.psxNoiseRegisterAuthority);
    CHECK(!surface.psxSpuRegisterAuthority);
    CHECK(!surface.psxSpuTimingAuthority);
    CHECK(!surface.dynamicReplayAuthority);
    CHECK(!surface.oldWinS0Authority);
    CHECK(!surface.stage2PlusAuthority);

    SsDriverInitializeState8003226C initializeState{};
    initializeState.configuredVoiceCount800928A0 = 3u;
    initializeState.word80091688 = 2u;
    std::array<SsDriverVoiceOwnerState8003226C, 24> owners{};
    SsDriverVoiceCompletionState80032B00 completion{};
    std::array<SsDriverVoiceRegisterState80032B00, 24> voices{};
    SsDriverVolumeMix80032B00 mix{};
    SsDriverVolumeScratch80032B00 scratch{};
    SsDriverGlobalMaskState80032B00 masks{};
    completion.completionRing.fill(0xFFFFFFFFu);
    completion.voiceStatus[0] = 1u;
    completion.voiceStatus[1] = 1u;
    owners[0].priority80087D58 = 5u;
    owners[0].field80087D46 = 100u;
    owners[0].field80087D42 = 10u;
    owners[1].priority80087D58 = 10u;
    owners[1].field80087D46 = 50u;
    owners[1].field80087D42 = 20u;

    SsDriverNoteOnRequest80032EAC request{};
    request.bankReady = true;
    request.packedSequenceTrack = 0x0402u;
    request.sourceVabId = 5;
    request.program = 7u;
    request.note = 64u;
    request.velocity = 80u;
    request.channelVolume = 100u;
    request.channelPan = 60u;
    request.trackVolumeLeft = 100u;
    request.trackVolumeRight = 80u;
    request.vabMasterVolume = 127u;
    request.programAttributes.valid = true;
    request.programAttributes.toneCount = 2u;
    request.programAttributes.volume = 127u;
    request.programAttributes.priority = 6u;
    request.programAttributes.pan = 64u;
    request.programAttributes.toneTableProgram = 3u;
    request.matchedToneCount = 2u;

    auto& regularTone = request.tones[0];
    regularTone.valid = true;
    regularTone.toneSlot = 0u;
    regularTone.toneIndex = 48u;
    regularTone.priority = 10u;
    regularTone.mode = 4u;
    regularTone.volume = 127u;
    regularTone.pan = 64u;
    regularTone.centerNote = 64u;
    regularTone.centerFine = 0u;
    regularTone.noteMin = 40u;
    regularTone.noteMax = 80u;
    regularTone.adsr1 = 0x1234u;
    regularTone.adsr2 = 0x5678u;
    regularTone.sampleId = 1u;
    regularTone.sampleStartAddressKnown = true;
    regularTone.sampleStartAddress = 0x0400u;

    auto& noiseTone = request.tones[1];
    noiseTone.valid = true;
    noiseTone.toneSlot = 1u;
    noiseTone.toneIndex = 49u;
    noiseTone.priority = 20u;
    noiseTone.mode = 0u;
    noiseTone.volume = 127u;
    noiseTone.pan = 32u;
    noiseTone.centerNote = 60u;
    noiseTone.centerFine = 0u;
    noiseTone.noteMin = 40u;
    noiseTone.noteMax = 80u;
    noiseTone.adsr1 = 0x1111u;
    noiseTone.adsr2 = 0x2222u;
    noiseTone.sampleId = 255u;

    SsDriverNoteOnDispatch80032EAC dispatch{};
    CHECK(TryExecuteSsDriverNoteOn80032EAC(
        surface,
        request,
        initializeState,
        owners,
        completion,
        voices,
        mix,
        scratch,
        masks,
        dispatch));
    CHECK(dispatch.known);
    CHECK(!dispatch.validationRejected);
    CHECK(dispatch.configuredVoiceCount == 3u);
    CHECK(dispatch.matchedToneCount == 2u);
    CHECK(dispatch.allocatedToneCount == 2u);
    CHECK(dispatch.allocatedVoiceMask == 0x5u);
    CHECK(dispatch.replacedVoiceMask == 0x1u);
    CHECK(dispatch.noiseMaskClearRequestCount == 0u);
    CHECK(dispatch.result == 0x5);
    CHECK(dispatch.hostProjectionReady);
    CHECK(!dispatch.psxSampleStartAddressAuthority);
    CHECK(!dispatch.psxNoiseRegisterAuthority);
    CHECK(!dispatch.psxSpuRegisterAuthority);
    CHECK(!dispatch.psxSpuTimingAuthority);

    CHECK(dispatch.layers[0].selectedVoice == 2u);
    CHECK(!dispatch.layers[0].allocationFailed);
    CHECK(!dispatch.layers[0].replacedActiveVoice);
    CHECK(!dispatch.layers[0].noiseSample);
    CHECK(dispatch.layers[0].effectiveVelocity == 62u);
    CHECK(dispatch.layers[0].pitch == 0x1000u);
    CHECK(dispatch.layers[0].pendingVolumeLeft == 2420u);
    CHECK(dispatch.layers[0].pendingVolumeRight == 1405u);
    CHECK(dispatch.layers[0].dirtyFlagsAfter == 0x3Fu);
    CHECK(dispatch.layers[0].sampleStartAddressKnown);
    CHECK(dispatch.layers[0].hostPlaybackReady);
    CHECK(dispatch.layers[1].selectedVoice == 0u);
    CHECK(dispatch.layers[1].replacedActiveVoice);
    CHECK(dispatch.layers[1].noiseSample);
    CHECK(dispatch.layers[1].pitch == 0u);
    CHECK(dispatch.layers[1].pendingVolumeLeft == 12900u);
    CHECK(dispatch.layers[1].pendingVolumeRight == 4991u);
    CHECK(dispatch.layers[1].dirtyFlagsAfter == 0x3Bu);
    CHECK(!dispatch.layers[1].hostPlaybackReady);

    CHECK(owners[2].field80087D40 == 1u);
    CHECK(owners[2].field80087D42 == 1u);
    CHECK(owners[2].field80087D44 == 0x1000u);
    CHECK(owners[2].field80087D46 == 0x7FFFu);
    CHECK(owners[2].velocity80087D48 == 80u);
    CHECK(owners[2].pan80087D4A == 60u);
    CHECK(owners[2].note80087D4C == 64u);
    CHECK(owners[2].packedSequenceTrack80087D4E ==
          static_cast<int16_t>(0x0402u));
    CHECK(owners[2].toneTableProgram80087D50 == 3u);
    CHECK(owners[2].program80087D52 == 7u);
    CHECK(owners[2].toneIndex80087D54 == 0u);
    CHECK(owners[2].sourceVabId80087D56 == 5u);
    CHECK(owners[2].priority80087D58 == 10u);
    CHECK(owners[0].field80087D40 == 255u);
    CHECK(owners[0].field80087D42 == 0u);
    CHECK(owners[0].field80087D44 == 10u);
    CHECK(owners[0].toneIndex80087D54 == 1u);
    CHECK(owners[0].priority80087D58 == 20u);
    CHECK(owners[1].field80087D42 == 22u);
    CHECK(initializeState.lastVoiceIndex800928F2 == 0u);
    CHECK(initializeState.word800928F4 == 0u);
    CHECK(initializeState.word800928F6 == 49u);

    CHECK(completion.voiceStatus[0] == 2u);
    CHECK(completion.voiceStatus[1] == 1u);
    CHECK(completion.voiceStatus[2] == 1u);
    for (const uint32_t entry : completion.completionRing) {
        CHECK(entry == 0xFFFFFFFAu);
    }
    CHECK(voices[2].pendingPitch == 0x1000u);
    CHECK(voices[2].pendingStartAddress == 0x0400u);
    CHECK(voices[2].pendingAdsr1 == 0x1234u);
    CHECK(voices[2].pendingAdsr2 == 0x567Au);
    CHECK(voices[2].dirtyFlags == 0x3Fu);
    CHECK(voices[0].pendingAdsr1 == 0x1111u);
    CHECK(voices[0].pendingAdsr2 == 0x2224u);
    CHECK(voices[0].dirtyFlags == 0x3Bu);
    CHECK(mix.masterVolume == 127u);
    CHECK(mix.mix800928E2 == 127u);
    CHECK(mix.pan800928E3 == 64u);
    CHECK(mix.mix800928E5 == 127u);
    CHECK(mix.pan800928E6 == 32u);
    CHECK(scratch.firstCurrent800928DC == 62u);
    CHECK(scratch.secondCurrent800928DD == 60u);
    CHECK(masks.keyOnLow == 0x0005u);
    CHECK(masks.keyOffLow == 0u);
    CHECK(masks.reverbLow == 0x0004u);

    SsDriverNoteOffDispatch8003349C noteOff{};
    CHECK(TryExecuteSsDriverNoteOff8003349C(
        surface,
        request.packedSequenceTrack,
        request.sourceVabId,
        request.program,
        request.note,
        initializeState,
        owners,
        completion,
        masks,
        noteOff));
    CHECK(noteOff.known);
    CHECK(noteOff.matchedVoiceCount == 2u);
    CHECK(noteOff.matchedVoiceMask == 0x5u);
    CHECK(noteOff.regularKeyOffVoiceMask == 0x4u);
    CHECK(noteOff.noiseRegisterClearRequestCount == 1u);
    CHECK(noteOff.hostProjectionReady);
    CHECK(!noteOff.psxNoiseRegisterAuthority);
    CHECK(!noteOff.psxSpuRegisterAuthority);
    CHECK(completion.voiceStatus[0] == 0u);
    CHECK(completion.voiceStatus[2] == 0u);
    CHECK(owners[0].field80087D40 == 255u);
    CHECK(owners[2].field80087D40 == 0u);
    CHECK(owners[0].field80087D44 == 0u);
    CHECK(owners[2].field80087D44 == 0u);
    CHECK(initializeState.lastVoiceIndex800928F2 == 2u);
    CHECK(masks.keyOnLow == 0x0001u);
    CHECK(masks.keyOffLow == 0x0004u);

    SsDriverInitializeState8003226C oneVoiceState{};
    oneVoiceState.configuredVoiceCount800928A0 = 1u;
    std::array<SsDriverVoiceOwnerState8003226C, 24> blockedOwners{};
    SsDriverVoiceCompletionState80032B00 blockedCompletion{};
    std::array<SsDriverVoiceRegisterState80032B00, 24> blockedVoices{};
    SsDriverVolumeMix80032B00 blockedMix{};
    SsDriverVolumeScratch80032B00 blockedScratch{};
    SsDriverGlobalMaskState80032B00 blockedMasks{};
    blockedOwners[0].priority80087D58 = 20u;
    blockedOwners[0].field80087D46 = 1u;
    blockedCompletion.voiceStatus[0] = 1u;
    SsDriverNoteOnRequest80032EAC blockedRequest = request;
    blockedRequest.matchedToneCount = 1u;
    blockedRequest.programAttributes.toneCount = 1u;
    blockedRequest.tones[0].priority = 10u;
    CHECK(TryExecuteSsDriverNoteOn80032EAC(
        surface,
        blockedRequest,
        oneVoiceState,
        blockedOwners,
        blockedCompletion,
        blockedVoices,
        blockedMix,
        blockedScratch,
        blockedMasks,
        dispatch));
    CHECK(dispatch.result == -1);
    CHECK(dispatch.allocatedToneCount == 0u);
    CHECK(dispatch.layers[0].allocationFailed);

    auto mutated = surface;
    mutated.pitchTableEntryCount = 191u;
    CHECK(!TryExecuteSsDriverNoteOn80032EAC(
        mutated,
        request,
        initializeState,
        owners,
        completion,
        voices,
        mix,
        scratch,
        masks,
        dispatch));
}

void TestSsDriverVolumeRampSetup80035110() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsDriverVolumeRampSetupSurface80035110 surface =
        BuildSsDriverVolumeRampSetupSurface80035110();
    CHECK(IsExactSsDriverVolumeRampSetupSurface80035110(surface));
    CHECK(surface.known);
    CHECK(surface.setupFunctionCompleteWithinLimits);
    CHECK(surface.wrapperFunction == 0x80035110u);
    CHECK(surface.setupFunction == 0x80031878u);
    CHECK(surface.voiceStateBaseAddress == 0x80087D40u);
    CHECK(surface.voiceStateStride == 0x34u);
    CHECK(surface.activeOffset == 0x1Cu);
    CHECK(surface.stepOffset == 0x1Eu);
    CHECK(surface.periodOffset == 0x20u);
    CHECK(surface.countdownOffset == 0x22u);
    CHECK(surface.currentOffset == 0x24u);
    CHECK(surface.targetOffset == 0x26u);
    CHECK(surface.maxVoiceCount == 24u);
    CHECK(!surface.psxMemoryAuthority);
    CHECK(!surface.psxSpuTimingAuthority);

    SsDriverVoiceRegisterState80032B00 voice{};
    voice.firstRamp.active = 7;
    voice.firstRamp.step = -3;
    voice.firstRamp.period = 4;
    voice.firstRamp.countdown = 5u;
    voice.firstRamp.current = 9u;
    voice.firstRamp.target = 11;
    SsDriverVolumeRampSetupDispatch80035110 dispatch{};

    // Equal endpoints are an early return and preserve every pre-existing
    // ramp field.
    CHECK(TryExecuteSsDriverVolumeRampSetup80035110(
        surface, 0u, 11, 11, 1, voice, dispatch));
    CHECK(dispatch.branch ==
          SsDriverVolumeRampSetupBranch80035110::EqualNoOp);
    CHECK(dispatch.equalStartTargetNoOp);
    CHECK(!dispatch.lowerCallCommitted);
    CHECK(voice.firstRamp.active == 7);
    CHECK(voice.firstRamp.step == -3);
    CHECK(voice.firstRamp.period == 4);
    CHECK(voice.firstRamp.countdown == 5u);
    CHECK(voice.firstRamp.current == 9u);
    CHECK(voice.firstRamp.target == 11);

    // A difference at least as large as the rate takes the period=0 coarse
    // path and preserves the prior countdown field (D62 is not written).
    CHECK(TryExecuteSsDriverVolumeRampSetup80035110(
        surface, 0u, 10, 0, 2, voice, dispatch));
    CHECK(dispatch.branch ==
          SsDriverVolumeRampSetupBranch80035110::CoarseStep);
    CHECK(dispatch.lowerCallCommitted);
    CHECK(voice.firstRamp.active == 1);
    CHECK(voice.firstRamp.current == 10u);
    CHECK(voice.firstRamp.target == 0);
    CHECK(voice.firstRamp.step == 5);
    CHECK(voice.firstRamp.period == 0);
    CHECK(voice.firstRamp.countdown == 5u);

    // A smaller difference takes the fine path: step=1 and rate/difference
    // is written to both period and countdown with PSX 16-bit wrapping.
    CHECK(TryExecuteSsDriverVolumeRampSetup80035110(
        surface, 0u, 0, 10, 100, voice, dispatch));
    CHECK(dispatch.branch ==
          SsDriverVolumeRampSetupBranch80035110::FineStep);
    CHECK(dispatch.lowerCallCommitted);
    CHECK(voice.firstRamp.active == 1);
    CHECK(voice.firstRamp.current == 0u);
    CHECK(voice.firstRamp.target == 10);
    CHECK(voice.firstRamp.step == 1);
    CHECK(voice.firstRamp.period == -10);
    CHECK(voice.firstRamp.countdown == 0xFFF6u);

    // The wrapper rejects voice 24 and above without touching state.
    const SsDriverRampState80032B00 beforeReject = voice.firstRamp;
    CHECK(TryExecuteSsDriverVolumeRampSetup80035110(
        surface, 24u, 0, 10, 1, voice, dispatch));
    CHECK(dispatch.branch ==
          SsDriverVolumeRampSetupBranch80035110::WrapperVoiceReject);
    CHECK(dispatch.wrapperVoiceRejected);
    CHECK(!dispatch.lowerCallCommitted);
    CHECK(voice.firstRamp.active == beforeReject.active);
    CHECK(voice.firstRamp.step == beforeReject.step);
    CHECK(voice.firstRamp.period == beforeReject.period);
    CHECK(voice.firstRamp.countdown == beforeReject.countdown);
    CHECK(voice.firstRamp.current == beforeReject.current);
    CHECK(voice.firstRamp.target == beforeReject.target);

    // A zero rate reaches the original divide-by-zero break after publishing
    // active/current/target; the host owner records the trap and stops there.
    voice.firstRamp.step = 9;
    voice.firstRamp.period = 8;
    voice.firstRamp.countdown = 7u;
    CHECK(TryExecuteSsDriverVolumeRampSetup80035110(
        surface, 0u, 0, 10, 0, voice, dispatch));
    CHECK(dispatch.branch ==
          SsDriverVolumeRampSetupBranch80035110::DivisionGuarded);
    CHECK(dispatch.divisionGuarded);
    CHECK(!dispatch.lowerCallCommitted);
    CHECK(voice.firstRamp.active == 1);
    CHECK(voice.firstRamp.current == 0u);
    CHECK(voice.firstRamp.target == 10);
    CHECK(voice.firstRamp.step == 9);
    CHECK(voice.firstRamp.period == 8);
    CHECK(voice.firstRamp.countdown == 7u);

    auto badSurface = surface;
    badSurface.setupFunction = 0x80000000u;
    CHECK(!IsExactSsDriverVolumeRampSetupSurface80035110(badSurface));
    CHECK(!TryExecuteSsDriverVolumeRampSetup80035110(
        badSurface, 0u, 0, 1, 1, voice, dispatch));
}

void TestSsDriverDirectVolumeWrites80034E5C80034F8C80035084() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsDriverDirectVolumeWriteSurface80034E5C surface =
        BuildSsDriverDirectVolumeWriteSurface80034E5C();
    CHECK(IsExactSsDriverDirectVolumeWriteSurface80034E5C(surface));
    CHECK(surface.known);
    CHECK(surface.threeWriteFunctionsCompleteWithinLimits);
    CHECK(surface.ownerIdentityFunction == 0x80034E5Cu);
    CHECK(surface.directFunction == 0x80034F8Cu);
    CHECK(surface.scaledFunction == 0x80035084u);
    CHECK(surface.voiceStateBaseAddress == 0x80087D40u);
    CHECK(surface.voiceStateStride == 0x34u);
    CHECK(surface.sourceVabIdOffset == 0x16u);
    CHECK(surface.programOffset == 0x12u);
    CHECK(surface.noteOffset == 0x0Cu);
    CHECK(surface.pendingVolumeLeftAddress == 0x80087BA8u);
    CHECK(surface.pendingVolumeRightAddress == 0x80087BAAu);
    CHECK(surface.dirtyFlagsAddress == 0x80087D28u);
    CHECK(surface.identityDirtyMask == 0x30u);
    CHECK(surface.directDirtyMask == 0x03u);
    CHECK(surface.scaledFactor == 129u);
    CHECK(!surface.psxMemoryAuthority);
    CHECK(!surface.psxSpuRegisterAuthority);

    SsDriverVoiceOwnerState8003226C owner{};
    owner.sourceVabId80087D56 = 2u;
    owner.program80087D52 = 3u;
    owner.note80087D4C = 60u;
    SsDriverVoiceRegisterState80032B00 voice{};
    voice.dirtyFlags = 0x04u;
    SsDriverDirectVolumeWriteDispatch80034E5C dispatch{};

    CHECK(TryExecuteSsDriverDirectVolumeWrite80034E5C(
        surface,
        SsDriverDirectVolumeWriteKind80034E5C::OwnerIdentity80034E5C,
        0u,
        2,
        3,
        60,
        100,
        200,
        owner,
        voice,
        dispatch));
    CHECK(dispatch.ownerIdentityMatched);
    CHECK(dispatch.writeCommitted);
    CHECK(dispatch.result == 0);
    CHECK(voice.pendingVolumeLeft == 100u);
    CHECK(voice.pendingVolumeRight == 200u);
    CHECK(voice.dirtyFlags == 0x34u);

    voice.dirtyFlags = 0u;
    CHECK(TryExecuteSsDriverDirectVolumeWrite80034E5C(
        surface,
        SsDriverDirectVolumeWriteKind80034E5C::OwnerIdentity80034E5C,
        0u,
        2,
        3,
        61,
        7,
        8,
        owner,
        voice,
        dispatch));
    CHECK(!dispatch.ownerIdentityMatched);
    CHECK(!dispatch.writeCommitted);
    CHECK(dispatch.result == -1);
    CHECK(voice.pendingVolumeLeft == 100u);
    CHECK(voice.pendingVolumeRight == 200u);
    CHECK(voice.dirtyFlags == 0u);

    CHECK(TryExecuteSsDriverDirectVolumeWrite80034E5C(
        surface,
        SsDriverDirectVolumeWriteKind80034E5C::Direct80034F8C,
        0u,
        -1,
        0,
        0,
        -10,
        20,
        owner,
        voice,
        dispatch));
    CHECK(dispatch.writeCommitted);
    CHECK(dispatch.function == 0x80034F8Cu);
    CHECK(voice.pendingVolumeLeft == 0xFFF6u);
    CHECK(voice.pendingVolumeRight == 20u);
    CHECK(voice.dirtyFlags == 0x03u);

    voice.dirtyFlags = 0u;
    CHECK(TryExecuteSsDriverDirectVolumeWrite80034E5C(
        surface,
        SsDriverDirectVolumeWriteKind80034E5C::Scaled12980035084,
        0u,
        -1,
        0,
        0,
        100,
        -2,
        owner,
        voice,
        dispatch));
    CHECK(dispatch.writeCommitted);
    CHECK(dispatch.function == 0x80035084u);
    CHECK(dispatch.scaledLeft == 12900);
    CHECK(dispatch.scaledRight == -258);
    CHECK(voice.pendingVolumeLeft == 12900u);
    CHECK(voice.pendingVolumeRight == 0xFEFEu);
    CHECK(voice.dirtyFlags == 0x03u);

    const uint16_t leftBeforeReject = voice.pendingVolumeLeft;
    const uint16_t rightBeforeReject = voice.pendingVolumeRight;
    const uint8_t dirtyBeforeReject = voice.dirtyFlags;
    CHECK(TryExecuteSsDriverDirectVolumeWrite80034E5C(
        surface,
        SsDriverDirectVolumeWriteKind80034E5C::Direct80034F8C,
        24u,
        0,
        0,
        0,
        1,
        2,
        owner,
        voice,
        dispatch));
    CHECK(dispatch.wrapperVoiceRejected);
    CHECK(!dispatch.writeCommitted);
    CHECK(dispatch.result == -1);
    CHECK(voice.pendingVolumeLeft == leftBeforeReject);
    CHECK(voice.pendingVolumeRight == rightBeforeReject);
    CHECK(voice.dirtyFlags == dirtyBeforeReject);

    auto badSurface = surface;
    badSurface.scaledFactor = 128u;
    CHECK(!IsExactSsDriverDirectVolumeWriteSurface80034E5C(badSurface));
    CHECK(!TryExecuteSsDriverDirectVolumeWrite80034E5C(
        badSurface,
        SsDriverDirectVolumeWriteKind80034E5C::Direct80034F8C,
        0u,
        0,
        0,
        0,
        1,
        2,
        owner,
        voice,
        dispatch));
}

void TestSsDriverVolumeRampsAndRegisterCommit() {
    using namespace PrSS0Scene0IntSpuDirect;

    const SsDriverVolumeRampSurface80032B00 rampSurface =
        BuildSsDriverVolumeRampSurface80032B00();
    CHECK(IsExactSsDriverVolumeRampSurface80032B00(rampSurface));
    CHECK(rampSurface.known);
    CHECK(rampSurface.twoRampFunctionsCompleteWithinLimits);
    CHECK(rampSurface.firstFunction == 0x80031A28u);
    CHECK(rampSurface.secondFunction == 0x80031F28u);
    CHECK(rampSurface.voiceStateBaseAddress == 0x80087D40u);
    CHECK(rampSurface.voiceStateStride == 0x34u);
    CHECK(rampSurface.firstActiveOffset == 0x1Cu);
    CHECK(rampSurface.firstStepOffset == 0x1Eu);
    CHECK(rampSurface.firstPeriodOffset == 0x20u);
    CHECK(rampSurface.firstCountdownOffset == 0x22u);
    CHECK(rampSurface.firstCurrentOffset == 0x24u);
    CHECK(rampSurface.firstTargetOffset == 0x26u);
    CHECK(rampSurface.secondActiveOffset == 0x28u);
    CHECK(rampSurface.secondStepOffset == 0x2Au);
    CHECK(rampSurface.secondPeriodOffset == 0x2Cu);
    CHECK(rampSurface.secondCountdownOffset == 0x2Eu);
    CHECK(rampSurface.secondCurrentOffset == 0x30u);
    CHECK(rampSurface.secondTargetOffset == 0x32u);
    CHECK(rampSurface.pendingVolumeLeftAddress == 0x80087BA8u);
    CHECK(rampSurface.pendingVolumeRightAddress == 0x80087BAAu);
    CHECK(rampSurface.dirtyFlagsAddress == 0x80087D28u);
    CHECK(!rampSurface.psxVolumeAuthority);
    CHECK(!rampSurface.psxSpuRegisterAuthority);

    SsDriverVolumeMix80032B00 mix{};
    mix.masterVolume = 127u;
    mix.mix800928E2 = 127u;
    mix.pan800928E3 = 64u;
    mix.mix800928E5 = 127u;
    mix.pan800928E6 = 64u;
    SsDriverVolumeScratch80032B00 scratch{};
    scratch.secondCurrent800928DD = 32u;
    SsDriverVoiceRegisterState80032B00 voice{};
    voice.firstRamp.active = 1;
    voice.firstRamp.step = 1;
    voice.firstRamp.current = 63u;
    voice.firstRamp.target = 64;
    SsDriverVolumeRampDispatch80032B00 rampDispatch{};
    CHECK(TryExecuteSsDriverVolumeRamp80032B00(
        rampSurface,
        SsDriverVolumeRampKind80032B00::First80031A28,
        mix,
        scratch,
        voice,
        rampDispatch));
    CHECK(rampDispatch.known);
    CHECK(rampDispatch.currentAdvanced);
    CHECK(rampDispatch.targetReached);
    CHECK(rampDispatch.currentAfter == 64u);
    CHECK(voice.firstRamp.active == 0);
    CHECK(scratch.firstCurrent800928DC == 64u);
    CHECK(rampDispatch.pendingVolumeLeft == 8000u);
    CHECK(rampDispatch.pendingVolumeRight == 4128u);
    CHECK(voice.pendingVolumeLeft == 8000u);
    CHECK(voice.pendingVolumeRight == 4128u);
    CHECK(voice.dirtyFlags == 0x03u);
    CHECK(rampDispatch.hostVolumeProjectionReady);
    CHECK(!rampDispatch.psxVolumeAuthority);
    CHECK(!rampDispatch.psxSpuRegisterAuthority);

    voice.dirtyFlags = 0u;
    voice.secondRamp.active = 1;
    voice.secondRamp.step = -1;
    voice.secondRamp.current = 33u;
    voice.secondRamp.target = 32;
    CHECK(TryExecuteSsDriverVolumeRamp80032B00(
        rampSurface,
        SsDriverVolumeRampKind80032B00::Second80031F28,
        mix,
        scratch,
        voice,
        rampDispatch));
    CHECK(rampDispatch.targetReached);
    CHECK(voice.secondRamp.active == 0);
    CHECK(scratch.secondCurrent800928DD == 32u);
    CHECK(rampDispatch.pendingVolumeLeft == 8000u);
    CHECK(rampDispatch.pendingVolumeRight == 4128u);
    CHECK(voice.dirtyFlags == 0x03u);

    SsDriverVoiceRegisterState80032B00 countdownVoice{};
    countdownVoice.firstRamp.active = 1;
    countdownVoice.firstRamp.step = 1;
    countdownVoice.firstRamp.period = 3;
    countdownVoice.firstRamp.countdown = 1u;
    countdownVoice.firstRamp.current = 10u;
    countdownVoice.firstRamp.target = 20;
    CHECK(TryExecuteSsDriverVolumeRamp80032B00(
        rampSurface,
        SsDriverVolumeRampKind80032B00::First80031A28,
        mix,
        scratch,
        countdownVoice,
        rampDispatch));
    CHECK(rampDispatch.countdownSkippedUpdate);
    CHECK(countdownVoice.firstRamp.countdown == 0u);
    CHECK(countdownVoice.firstRamp.current == 10u);
    CHECK(countdownVoice.dirtyFlags == 0u);
    CHECK(TryExecuteSsDriverVolumeRamp80032B00(
        rampSurface,
        SsDriverVolumeRampKind80032B00::First80031A28,
        mix,
        scratch,
        countdownVoice,
        rampDispatch));
    CHECK(!rampDispatch.countdownSkippedUpdate);
    CHECK(countdownVoice.firstRamp.countdown == 3u);
    CHECK(countdownVoice.firstRamp.current == 11u);

    mix.monoFlag80091728 = 1;
    voice.firstRamp.active = 1;
    voice.firstRamp.step = 1;
    voice.firstRamp.current = 63u;
    voice.firstRamp.target = 64;
    voice.dirtyFlags = 0u;
    CHECK(TryExecuteSsDriverVolumeRamp80032B00(
        rampSurface,
        SsDriverVolumeRampKind80032B00::First80031A28,
        mix,
        scratch,
        voice,
        rampDispatch));
    CHECK(rampDispatch.pendingVolumeLeft == 8000u);
    CHECK(rampDispatch.pendingVolumeRight == 8000u);

    auto badRampSurface = rampSurface;
    badRampSurface.secondCurrentOffset = 0x31u;
    CHECK(!IsExactSsDriverVolumeRampSurface80032B00(
        badRampSurface));
    CHECK(!TryExecuteSsDriverVolumeRamp80032B00(
        badRampSurface,
        SsDriverVolumeRampKind80032B00::First80031A28,
        mix,
        scratch,
        voice,
        rampDispatch));

    const SsDriverRegisterCommitSurface80032B00 commitSurface =
        BuildSsDriverRegisterCommitSurface80032B00();
    CHECK(IsExactSsDriverRegisterCommitSurface80032B00(
        commitSurface));
    CHECK(commitSurface.function == 0x80032B00u);
    CHECK(commitSurface.dirtyLoopStart == 0x80032D28u);
    CHECK(commitSurface.dirtyLoopEnd == 0x80032E18u);
    CHECK(commitSurface.voiceCount == 24u);
    CHECK(commitSurface.voiceRegisterStride == 0x10u);
    CHECK(commitSurface.volumeDirtyMask == 0x01u);
    CHECK(commitSurface.pitchDirtyMask == 0x04u);
    CHECK(commitSurface.startAddressDirtyMask == 0x08u);
    CHECK(commitSurface.adsrDirtyMask == 0x10u);
    CHECK(commitSurface.spuKeyOffLowOffset == 0x18Cu);
    CHECK(commitSurface.spuKeyOnLowOffset == 0x188u);
    CHECK(commitSurface.spuReverbLowOffset == 0x198u);
    CHECK(!commitSurface.psxSpuRegisterAuthority);

    std::array<SsDriverVoiceRegisterState80032B00, 24> voices{};
    voices[0].dirtyFlags = 0x1Du;
    voices[0].pendingVolumeLeft = 10u;
    voices[0].pendingVolumeRight = 20u;
    voices[0].pendingPitch = 30u;
    voices[0].pendingStartAddress = 40u;
    voices[0].pendingAdsr1 = 50u;
    voices[0].pendingAdsr2 = 60u;
    voices[1].dirtyFlags = 0x02u;
    SsDriverGlobalMaskState80032B00 masks{};
    masks.keyOffLow = 0x0003u;
    masks.keyOffHigh = 0x0001u;
    masks.keyOnLow = 0x0007u;
    masks.keyOnHigh = 0x0003u;
    masks.reverbLow = 0x1234u;
    masks.reverbHigh = 0x0056u;
    SsDriverRegisterCommitDispatch80032B00 commitDispatch{};
    CHECK(TryExecuteSsDriverRegisterCommit80032B00(
        commitSurface, voices, masks, commitDispatch));
    CHECK(commitDispatch.known);
    CHECK(commitDispatch.pendingKeyOnMaskedByKeyOff);
    CHECK(commitDispatch.pendingKeyOnAndKeyOffCleared);
    CHECK(commitDispatch.reverbMasksRetained);
    CHECK(commitDispatch.dirtyVoiceMask == 0x3u);
    CHECK(commitDispatch.voiceRequests[0].writeVolume);
    CHECK(commitDispatch.voiceRequests[0].writePitch);
    CHECK(commitDispatch.voiceRequests[0].writeStartAddress);
    CHECK(commitDispatch.voiceRequests[0].writeAdsr);
    CHECK(commitDispatch.voiceRequests[0].volumeLeft == 10u);
    CHECK(commitDispatch.voiceRequests[0].volumeRight == 20u);
    CHECK(commitDispatch.voiceRequests[0].pitch == 30u);
    CHECK(commitDispatch.voiceRequests[0].startAddress == 40u);
    CHECK(commitDispatch.voiceRequests[0].adsr1 == 50u);
    CHECK(commitDispatch.voiceRequests[0].adsr2 == 60u);
    CHECK(!commitDispatch.voiceRequests[1].writeVolume);
    CHECK(voices[0].dirtyFlags == 0u);
    CHECK(voices[1].dirtyFlags == 0u);
    CHECK(commitDispatch.keyOffLow == 0x0003u);
    CHECK(commitDispatch.keyOffHigh == 0x0001u);
    CHECK(commitDispatch.keyOnLow == 0x0004u);
    CHECK(commitDispatch.keyOnHigh == 0x0002u);
    CHECK(commitDispatch.reverbLow == 0x1234u);
    CHECK(commitDispatch.reverbHigh == 0x0056u);
    CHECK(masks.keyOffLow == 0u);
    CHECK(masks.keyOffHigh == 0u);
    CHECK(masks.keyOnLow == 0u);
    CHECK(masks.keyOnHigh == 0u);
    CHECK(masks.reverbLow == 0x1234u);
    CHECK(masks.reverbHigh == 0x0056u);
    CHECK(commitDispatch.hostVolumeProjectionReady);
    CHECK(!commitDispatch.psxSpuRegisterAuthority);

    auto badCommitSurface = commitSurface;
    badCommitSurface.adsrDirtyMask = 0x20u;
    CHECK(!IsExactSsDriverRegisterCommitSurface80032B00(
        badCommitSurface));
    CHECK(!TryExecuteSsDriverRegisterCommit80032B00(
        badCommitSurface, voices, masks, commitDispatch));
}

void TestSyntheticYCompoSpuCandidate() {
    using namespace PrSS0Scene0IntLoadDirect;
    using namespace PrSS0Scene0IntSpuDirect;

    const std::vector<uint8_t> archive = BuildSyntheticYCompoSpuArchive();
    const std::vector<uint8_t> raw = PackMode2RawSectors(archive);
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() /
        ("test_ss0_scene0_ycompo_spu_mode2_" +
         std::to_string(reinterpret_cast<std::uintptr_t>(&g_failures)) +
         ".bin");
    {
        std::ofstream stream(path, std::ios::binary | std::ios::trunc);
        CHECK(static_cast<bool>(stream));
        if (!stream) {
            return;
        }
        stream.write(reinterpret_cast<const char*>(raw.data()),
                     static_cast<std::streamsize>(raw.size()));
        CHECK(static_cast<bool>(stream));
    }

    const auto intLoad = BuildYCompoTransaction80015618(
        MakeYCompoSource(path, static_cast<uint32_t>(archive.size())));
    auto loaderCandidate =
        PrSS0Scene0IntSideEffectDirect::BuildYCompoTransaction8001A8F0(
            intLoad, MakeResetLoader());
    const auto padStartComSource = MakePadStartComSource();
    CHECK(IsExactAcceptedYCompoTransaction80015618(intLoad));
    CHECK(PrSS0Scene0IntSideEffectDirect::
              IsExactAcceptedYCompoTransaction8001A8F0(
                  loaderCandidate, intLoad));

    Transaction8001A8F0 transaction = BuildYCompoTransaction8001A8F0(
        intLoad, loaderCandidate, padStartComSource);
    CHECK(IsExactAcceptedYCompoTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));
    CHECK(!IsExactAcceptedTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));
    CHECK(transaction.sourceArchiveKind ==
          ArchiveKind8001AC18::PracticeYCompo);
    CHECK(transaction.requestBound);
    CHECK(transaction.originalDiscPayloadAuthority);
    CHECK(NameEquals(transaction.vhName, "PRACTICE.VH"));
    CHECK(NameEquals(transaction.vbName, "PRACTICE.VB"));
    CHECK(transaction.vhBytes.size() == 2592u);
    CHECK(transaction.vbBytes.size() == 8u);
    CHECK(transaction.decoderPreflightKnown);
    CHECK(transaction.decoderPreflightReady);
    CHECK(transaction.decoderProgramCount == 0u);
    CHECK(transaction.decoderDeclaredToneCount == 0u);
    CHECK(transaction.decoderVagCount == 1u);
    CHECK(transaction.decoderEffectiveVagCount == 1u);
    CHECK(transaction.decoderToneSlotCount == 0u);
    CHECK(transaction.decoderVagTableOffset == 2080u);
    CHECK(transaction.actionSequenceKnown);
    CHECK(transaction.padStartComRequired);
    CHECK(transaction.padStartComStaticBodyKnown);
    CHECK(transaction.padStartComSsSequenceCallbackSurfaceKnown);
    CHECK(IsExactSsSequenceCallbackSurface8002AC20(
        transaction.padStartComSsSequenceCallbackSurface));
    CHECK(transaction.padStartComSpuTransferCompletionSurfaceKnown);
    CHECK(IsExactSpuTransferCompletionSurface8002AC20(
        transaction.padStartComSpuTransferCompletionSurface));
    CHECK(transaction.padStartComSsTickInterruptSurfaceKnown);
    CHECK(IsExactSsTickInterruptSurface8002B130(
        transaction.padStartComSsTickInterruptSurface));
    CHECK(transaction.padStartComGlobalWritesKnown);
    CHECK(transaction.padStartComCuePointerPreserved);
    CHECK(!transaction.padStartComGlobalWritesCommitted);
    CHECK(!transaction.padStartComLowerCallsCommitted);
    CHECK(!transaction.hostVabCandidatePrepared);
    CHECK(!transaction.hostVabBankCommitted);
    CHECK(!transaction.psxVabIdAuthority);
    CHECK(!transaction.psxSpuRamAuthority);
    CHECK(!transaction.padStartComCommitted);
    CHECK(!transaction.audibleOutputAuthority);
    CHECK(!transaction.replayValueAuthority);
    CHECK(!transaction.hostFilesystemAuthority);
    CHECK(!transaction.consumerReadAuthority);
    CHECK(!transaction.oldWinS0Authority);
    CHECK(!transaction.stage2PlusAuthority);
    CHECK(!CommitPadStartComGlobalWrites80026E4C(
        transaction, intLoad, loaderCandidate, padStartComSource));
    CHECK(!transaction.padStartComGlobalWritesCommitted);
    CHECK(!CommitHostVabBank8001A8F0(transaction));

    transaction.sourceArchiveKind = ArchiveKind8001AC18::Scene0Compo00;
    CHECK(!IsExactAcceptedYCompoTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));
    transaction.sourceArchiveKind = ArchiveKind8001AC18::PracticeYCompo;
    transaction.vhBytes[0] ^= 1u;
    CHECK(!IsExactAcceptedYCompoTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));
    transaction.vhBytes[0] ^= 1u;
    CHECK(IsExactAcceptedYCompoTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));

    Transaction8001A8F0 commitMutation = BuildYCompoTransaction8001A8F0(
        intLoad, loaderCandidate, padStartComSource);
    commitMutation.vhBytes[0] ^= 1u;
    CHECK(!CommitYCompoPadStartComGlobalWrites80026E4C(
        commitMutation,
        intLoad,
        loaderCandidate,
        padStartComSource));
    CHECK(!commitMutation.padStartComGlobalWritesCommitted);
    CHECK(!commitMutation.hostVabBankCommitted);

    CHECK(CommitYCompoPadStartComGlobalWrites80026E4C(
        transaction,
        intLoad,
        loaderCandidate,
        padStartComSource));
    CHECK(IsExactYCompoPadStartComGlobalWritesCommittedTransaction80026E4C(
        transaction,
        intLoad,
        loaderCandidate,
        padStartComSource));
    CHECK(!IsExactAcceptedYCompoTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));
    CHECK(!IsExactPadStartComGlobalWritesCommittedTransaction80026E4C(
        transaction,
        intLoad,
        loaderCandidate,
        padStartComSource));
    CHECK(transaction.padStartComGlobalWritesCommitted);
    CHECK(!transaction.padStartComLowerCallsCommitted);
    CHECK(!transaction.hostVabCandidatePrepared);
    CHECK(!transaction.hostVabBankCommitted);
    CHECK(!transaction.psxVabIdAuthority);
    CHECK(!transaction.psxSpuRamAuthority);
    CHECK(!transaction.padStartComCommitted);
    CHECK(!transaction.audibleOutputAuthority);
    CHECK(!CommitYCompoPadStartComGlobalWrites80026E4C(
        transaction,
        intLoad,
        loaderCandidate,
        padStartComSource));

    transaction.hostVabCandidatePrepared = true;
    CHECK(CommitHostVabBank8001A8F0(transaction));
    CHECK(IsExactCommittedYCompoTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));
    CHECK(!IsExactCommittedTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));
    CHECK(transaction.hostVabBankCommitted);
    CHECK(!transaction.padStartComLowerCallsCommitted);
    CHECK(!transaction.psxVabIdAuthority);
    CHECK(!transaction.psxSpuRamAuthority);
    CHECK(!transaction.padStartComCommitted);
    CHECK(!transaction.audibleOutputAuthority);

    transaction.padStartComLowerCallsCommitted = true;
    CHECK(!IsExactCommittedYCompoTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));
    transaction.padStartComLowerCallsCommitted = false;
    transaction.psxSpuRamAuthority = true;
    CHECK(!IsExactCommittedYCompoTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));
    transaction.psxSpuRamAuthority = false;
    CHECK(IsExactCommittedYCompoTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));
    CheckRejected(
        BuildTransaction8001A8F0(
            intLoad, loaderCandidate, padStartComSource),
        intLoad,
        loaderCandidate,
        padStartComSource);

    std::error_code removeError;
    std::filesystem::remove(path, removeError);
    CHECK(!removeError);
}

void TestExactDisc(const std::filesystem::path& discBin) {
    using namespace PrSS0Scene0IntSpuDirect;
    constexpr uint32_t kComod0LoadBase801C3870 = 0x801C3870u;
    struct ExpectedDiscCue {
        uint32_t psxAddress;
        std::array<uint8_t, 4> bytes;
    };
    constexpr ExpectedDiscCue kExpectedDiscCues[] = {
        {0x801C6ED0u, {0x00u, 0x0Cu, 0x24u, 0x7Fu}},
        {0x801C6ED8u, {0x00u, 0x0Du, 0x25u, 0x7Fu}},
        {0x801C6EE0u, {0x00u, 0x0Eu, 0x26u, 0x7Fu}},
        {0x801C6EE8u, {0x00u, 0x0Fu, 0x27u, 0x7Fu}},
    };
    const std::vector<uint8_t> comod0Bytes =
        ReadOriginalDiscComod0(discBin);
    for (const auto& expected : kExpectedDiscCues) {
        const std::size_t offset = static_cast<std::size_t>(
            expected.psxAddress - kComod0LoadBase801C3870);
        CHECK(offset + expected.bytes.size() <= comod0Bytes.size());
        if (offset + expected.bytes.size() > comod0Bytes.size()) {
            continue;
        }
        for (std::size_t index = 0u;
             index < expected.bytes.size();
             ++index) {
            CHECK(comod0Bytes[offset + index] == expected.bytes[index]);
        }
    }

    const auto intLoad = BuildExactInt(discBin);
    auto loaderCandidate =
        PrSS0Scene0IntSideEffectDirect::BuildTransaction8001A8F0(
            intLoad, MakeResetLoader());
    CHECK(PrSS0Scene0IntSideEffectDirect::
              IsExactAcceptedTransaction8001A8F0(
                  loaderCandidate, intLoad));
    const auto padStartComSource = MakePadStartComSource();

    PrStage1LoaderSpuHal::State resetProbe{};
    resetProbe.word_800943A8 = 7;
    resetProbe.word_800943AA = 8;
    resetProbe.word_800943AC = 9;
    resetProbe.dword_800943B4 = 10;
    resetProbe.dword_80094410 = padStartComSource.cuePointer80094410;
    PrStage1LoaderSpuHal::ApplyPadStartComAudioGlobalResetContract(resetProbe);
    CHECK(resetProbe.word_800943A8 == 0);
    CHECK(resetProbe.word_800943AA == -1);
    CHECK(resetProbe.word_800943AC == -1);
    CHECK(resetProbe.dword_800943B4 == 0);
    CHECK(resetProbe.dword_80094410 ==
          padStartComSource.cuePointer80094410);

    Transaction8001A8F0 transaction =
        BuildTransaction8001A8F0(
            intLoad, loaderCandidate, padStartComSource);
    CHECK(IsExactAcceptedTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));
    CHECK(transaction.requestBound);
    CHECK(transaction.originalDiscPayloadAuthority);
    CHECK(NameEquals(transaction.vhName, "MINIMUM.VH"));
    CHECK(NameEquals(transaction.vbName, "MINIMUM.VB"));
    CHECK(!transaction.vhBytes.empty());
    CHECK(!transaction.vbBytes.empty());
    CHECK(transaction.vhBytes[0] == 0x70u);
    CHECK(transaction.vhBytes[1] == 0x42u);
    CHECK(transaction.vhBytes[2] == 0x41u);
    CHECK(transaction.vhBytes[3] == 0x56u);
    CHECK(transaction.vhHash != 0u && transaction.vbHash != 0u);
    CHECK(transaction.decoderPreflightKnown);
    CHECK(transaction.decoderPreflightReady);
    CHECK(transaction.decoderToneSlotCount ==
          static_cast<uint32_t>(transaction.decoderProgramCount) * 16u);
    CHECK(transaction.decoderEffectiveVagCount >=
          transaction.decoderVagCount);
    VabPlayer hostDecoderCandidate{};
    CHECK(hostDecoderCandidate.LoadFromMemory(
        transaction.vhBytes.data(),
        transaction.vhBytes.size(),
        transaction.vbBytes.data(),
        transaction.vbBytes.size()));
    CHECK(hostDecoderCandidate.IsLoaded());
    CHECK(static_cast<uint32_t>(hostDecoderCandidate.GetToneCount()) ==
          transaction.decoderToneSlotCount);
    CHECK(static_cast<uint32_t>(hostDecoderCandidate.GetVagCount()) ==
          transaction.decoderEffectiveVagCount);

    CHECK(transaction.actionSequenceKnown);
    CHECK(transaction.padStartComRequired);
    CHECK(transaction.actionRequests[0].wrapperFunction == 0x80027120u);
    CHECK(transaction.actionRequests[0].lowerFunction == 0x8002DF80u);
    CHECK(transaction.actionRequests[0].lowerCallConditional);
    CHECK(transaction.actionRequests[1].wrapperFunction == 0x80027078u);
    CHECK(transaction.actionRequests[1].lowerFunction == 0x8002E3D8u);
    CHECK(transaction.actionRequests[1].pointerArg ==
          transaction.vhPsxAddress);
    CHECK(transaction.actionRequests[1].scalarArg == -1);
    CHECK(transaction.actionRequests[2].wrapperFunction == 0x800270D4u);
    CHECK(transaction.actionRequests[2].lowerFunction == 0x8002EB80u);
    CHECK(transaction.actionRequests[2].pointerArg ==
          transaction.vbPsxAddress);
    CHECK(transaction.actionRequests[3].wrapperFunction == 0x800270FCu);
    CHECK(transaction.actionRequests[3].lowerFunction == 0x8002EEFCu);
    CHECK(transaction.actionRequests[3].scalarArg == 1);
    for (const auto& action : transaction.actionRequests) {
        CHECK(!action.lowerResultAuthority);
    }
    CHECK(transaction.padStartComStaticBodyKnown);
    CHECK(transaction.padStartComGlobalWritesKnown);
    CHECK(transaction.padStartComCalls[0].lowerFunction == 0x8002AC20u);
    CHECK(transaction.padStartComCalls[1].lowerFunction == 0x8002DA78u);
    CHECK(transaction.padStartComCalls[1].args[0] == 0x1000);
    CHECK(transaction.padStartComCalls[2].lowerFunction == 0x8002B130u);
    CHECK(transaction.padStartComCalls[3].lowerFunction == 0x8002A6ACu);
    CHECK(transaction.padStartComCalls[3].args[0] == 0x5A);
    CHECK(transaction.padStartComCalls[3].args[1] == 0x5A);
    CHECK(transaction.padStartComCalls[4].lowerFunction == 0x8002AA90u);
    CHECK(transaction.padStartComCalls[4].args[2] == 1);
    CHECK(transaction.padStartComCalls[5].lowerFunction == 0x8002AB24u);
    CHECK(transaction.padStartComCalls[5].args[1] == 0x7F);
    CHECK(transaction.padStartComCalls[5].args[2] == 0x7F);
    for (const auto& call : transaction.padStartComCalls) {
        CHECK(!call.lowerSideEffectCommitted);
    }
    CHECK(transaction.padStartComGlobalState.word_800943A8 == 0);
    CHECK(transaction.padStartComGlobalState.word_800943AA == -1);
    CHECK(transaction.padStartComGlobalState.word_800943AC == -1);
    CHECK(transaction.padStartComGlobalState.dword_800943B4 == 0);
    CHECK(transaction.padStartComCuePointerPreserved);
    CHECK(transaction.padStartComCuePointerBefore == 0x801C6EACu);
    CHECK(transaction.padStartComCuePointerAfter == 0x801C6EACu);

    CHECK(!transaction.hostVabCandidatePrepared);
    CHECK(!transaction.hostVabBankCommitted);
    CHECK(!transaction.padStartComGlobalWritesCommitted);
    CHECK(!transaction.padStartComLowerCallsCommitted);
    CHECK(!transaction.psxVabIdAuthority);
    CHECK(!transaction.psxSpuRamAuthority);
    CHECK(!transaction.padStartComCommitted);
    CHECK(!transaction.audibleOutputAuthority);
    CHECK(!transaction.replayValueAuthority);
    CHECK(!transaction.hostFilesystemAuthority);
    CHECK(!transaction.consumerReadAuthority);
    CHECK(!transaction.oldWinS0Authority);
    CHECK(!transaction.stage2PlusAuthority);
    CHECK(!CommitHostVabBank8001A8F0(transaction));
    CHECK(CommitPadStartComGlobalWrites80026E4C(
        transaction,
        intLoad,
        loaderCandidate,
        padStartComSource));
    CHECK(IsExactPadStartComGlobalWritesCommittedTransaction80026E4C(
        transaction,
        intLoad,
        loaderCandidate,
        padStartComSource));
    CHECK(!transaction.padStartComCommitted);
    CHECK(!transaction.padStartComLowerCallsCommitted);

    const auto unknownCue = PrSfx::ResolveScene0UiCue80025C8C(0x0001u);
    CHECK(!unknownCue.accepted);
    size_t unavailableCueSamples = 1u;
    uint32_t unavailableCueRate = 1u;
    CHECK(!PrSfx::GetScene0IntVabCuePcmInfo80025C8C(
        0x20u, unavailableCueSamples, unavailableCueRate));
    CHECK(unavailableCueSamples == 0u);
    CHECK(unavailableCueRate == 0u);

    const auto optionsLanguageCue =
        PrSfx::ResolveScene0OptionsLanguageCue80025DBC();
    CHECK(optionsLanguageCue.accepted);
    CHECK(optionsLanguageCue.pointerSlotAddress ==
          PrSfx::kScene0OptionsLanguageCuePointerSlot80094400);
    CHECK(optionsLanguageCue.pointerSlotAddress == 0x80094400u);
    CHECK(optionsLanguageCue.sourceAddress ==
          PrSfx::kScene0OptionsLanguageCueSource801C6E7C);
    CHECK(optionsLanguageCue.sourceAddress == 0x801C6E7Cu);
    CHECK(optionsLanguageCue.program == 0u);
    CHECK(optionsLanguageCue.note == 1u);
    CHECK(optionsLanguageCue.key == 0x19u);
    CHECK(optionsLanguageCue.volume == 0x5Au);
    size_t unavailableOptionsLanguageCueSamples = 1u;
    uint32_t unavailableOptionsLanguageCueRate = 1u;
    CHECK(!PrSfx::GetScene0IntVabOptionsLanguageCuePcmInfo80025DBC(
        unavailableOptionsLanguageCueSamples,
        unavailableOptionsLanguageCueRate));
    CHECK(unavailableOptionsLanguageCueSamples == 0u);
    CHECK(unavailableOptionsLanguageCueRate == 0u);

    auto scene0HostCandidate = PrSfx::BuildScene0IntVabCandidate8001A8F0(
        transaction.vhBytes.data(),
        transaction.vhBytes.size(),
        transaction.vbBytes.data(),
        transaction.vbBytes.size());
    CHECK(PrSfx::IsExactScene0IntVabCandidate8001A8F0(
        scene0HostCandidate,
        transaction.vhBytes.size(),
        transaction.vhHash,
        transaction.vbBytes.size(),
        transaction.vbHash,
        transaction.decoderToneSlotCount,
        transaction.decoderEffectiveVagCount));
    CHECK(scene0HostCandidate.player.GetSpuAllocationBytes8002E474() ==
          transaction.vbBytes.size());
    transaction.hostVabCandidatePrepared = true;
    CHECK(CommitHostVabBank8001A8F0(transaction));
    CHECK(IsExactCommittedTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));
    PrSfx::CommitScene0IntVabCandidate8001A8F0(
        std::move(scene0HostCandidate));
    CHECK(!scene0HostCandidate.prepared);
    uint8_t projectedProgram = 0u;
    uint8_t projectedNote = 0u;
    bool projectedToneFound = false;
    std::vector<int> projectedToneLayers;
    for (uint16_t program = 0u;
         program < 128u && !projectedToneFound;
         ++program) {
        for (uint16_t note = 0u; note < 128u; ++note) {
            CHECK(PrSfx::ResolveScene0SsMidiNoteToneLayers80032EAC(
                static_cast<uint8_t>(program),
                static_cast<uint8_t>(note),
                projectedToneLayers));
            if (!projectedToneLayers.empty()) {
                projectedProgram = static_cast<uint8_t>(program);
                projectedNote = static_cast<uint8_t>(note);
                projectedToneFound = true;
                break;
            }
        }
    }
    CHECK(projectedToneFound);
    const auto projectedMidiNote =
        PrSfx::ProjectScene0SsMidiNote8002BEEC(
            0x0402u,
            0,
            projectedProgram,
            projectedNote,
            80u,
            127u,
            64u,
            127u,
            127u,
            true);
    CHECK(projectedMidiNote.directDriverStateTranslated);
    CHECK(projectedMidiNote.directVabBodyTransferTranslated);
    CHECK(projectedMidiNote.directAllocatedVoiceCount > 0u);
    CHECK(projectedMidiNote.directSampleStartKnownVoiceMask != 0u);
    const auto projectedMidiNoteOff =
        PrSfx::ProjectScene0SsMidiNote8002BEEC(
            0x0402u,
            0,
            projectedProgram,
            projectedNote,
            0u,
            127u,
            64u,
            127u,
            127u,
            false);
    CHECK(projectedMidiNoteOff.directDriverStateTranslated);
    CHECK(projectedMidiNoteOff.directVabBodyTransferTranslated);
    CHECK(projectedMidiNoteOff.stoppedVoiceCount > 0u);
    std::vector<int> invalidProgramToneLayers{1, 2, 3};
    CHECK(PrSfx::ResolveScene0SsMidiNoteToneLayers80032EAC(
        0xFFu, 60u, invalidProgramToneLayers));
    CHECK(invalidProgramToneLayers.empty());
    size_t bgmSamples = 0u;
    uint32_t bgmRate = 0u;
    CHECK(PrSfx::GetBgmPcmInfo(bgmSamples, bgmRate));
    CHECK(bgmSamples > 0u);
    CHECK(bgmRate > 0u);
    // The title/menu BGM is a loop-flagged VAG.  Keep the decoded loop
    // bounds observable in the source-backed test so a future VAB change
    // cannot silently regress to a one-shot host voice.
    size_t bgmLoopStart = 0u;
    size_t bgmLoopEnd = 0u;
    CHECK(hostDecoderCandidate.GetTonePcmInfo(
        0u,
        7u,
        0x1Fu,
        bgmSamples,
        bgmRate,
        &bgmLoopStart,
        &bgmLoopEnd));
    CHECK(bgmLoopStart < bgmLoopEnd);
    CHECK(bgmLoopEnd <= bgmSamples);
    size_t uiSamples = 0u;
    uint32_t uiRate = 0u;
    size_t uiLoopStart = 0u;
    size_t uiLoopEnd = 0u;
    CHECK(hostDecoderCandidate.GetTonePcmInfo(
        0u,
        12u,
        0x24u,
        uiSamples,
        uiRate,
        &uiLoopStart,
        &uiLoopEnd));
    CHECK(uiSamples > 0u);
    CHECK(uiRate > 0u);
    CHECK(uiLoopStart == 0u);
    CHECK(uiLoopEnd == uiSamples);

    struct ExpectedUiCue {
        uint16_t inputMask;
        uint8_t note;
        uint8_t key;
    };
    constexpr ExpectedUiCue kExpectedUiCues[] = {
        {0x20u, 14u, 0x26u},
        {0x40u, 13u, 0x25u},
        {0x100u, 15u, 0x27u},
        {0x1000u, 12u, 0x24u},
        {0x2000u, 12u, 0x24u},
        {0x4000u, 12u, 0x24u},
        {0x8000u, 12u, 0x24u},
    };
    for (const auto& expected : kExpectedUiCues) {
        const auto cue =
            PrSfx::ResolveScene0UiCue80025C8C(expected.inputMask);
        CHECK(cue.accepted);
        CHECK(cue.inputMask == expected.inputMask);
        CHECK(cue.program == 0u);
        CHECK(cue.note == expected.note);
        CHECK(cue.key == expected.key);
        CHECK(cue.volume == 0x7Fu);
        size_t cueSamples = 0u;
        uint32_t cueRate = 0u;
        CHECK(PrSfx::GetScene0IntVabCuePcmInfo80025C8C(
            expected.inputMask, cueSamples, cueRate));
        CHECK(cueSamples > 0u);
        CHECK(cueRate > 0u);
    }
    CHECK(PrSfx::PlayScene0UiCue80025C8CRaw(0x40u));

    size_t optionsLanguageCueSamples = 0u;
    uint32_t optionsLanguageCueRate = 0u;
    const auto optionsLanguageCuePlayback =
        PrSfx::ResolveScene0OptionsLanguageCue80025DBC();
    CHECK(optionsLanguageCuePlayback.accepted);
    CHECK(optionsLanguageCuePlayback.pointerSlotAddress == 0x80094400u);
    CHECK(optionsLanguageCuePlayback.sourceAddress == 0x801C6E7Cu);
    CHECK(optionsLanguageCuePlayback.program == 0u);
    CHECK(optionsLanguageCuePlayback.note == 1u);
    CHECK(optionsLanguageCuePlayback.key == 0x19u);
    CHECK(optionsLanguageCuePlayback.volume == 0x5Au);
    CHECK(PrSfx::GetScene0IntVabOptionsLanguageCuePcmInfo80025DBC(
        optionsLanguageCueSamples,
        optionsLanguageCueRate));
    CHECK(optionsLanguageCueSamples > 0u);
    CHECK(optionsLanguageCueRate > 0u);
    CHECK(PrSfx::PlayScene0OptionsLanguageCue80025DBCRaw());

    transaction.actionRequests[2].pointerArg ^= 8u;
    CHECK(!IsExactCommittedTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));
    transaction.actionRequests[2].pointerArg ^= 8u;
    CHECK(IsExactCommittedTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));

    transaction.padStartComGlobalState.word_800943AA = 0;
    CHECK(!IsExactCommittedTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));
    transaction.padStartComGlobalState.word_800943AA = -1;
    CHECK(IsExactCommittedTransaction8001A8F0(
        transaction, intLoad, loaderCandidate, padStartComSource));

    auto badLoad = intLoad;
    badLoad.accepted = false;
    CheckRejected(
        BuildTransaction8001A8F0(
            badLoad, loaderCandidate, padStartComSource),
        badLoad,
        loaderCandidate,
        padStartComSource);

    auto replayPadSource = padStartComSource;
    replayPadSource.replayValueAuthority = true;
    CheckRejected(
        BuildTransaction8001A8F0(
            intLoad, loaderCandidate, replayPadSource),
        intLoad,
        loaderCandidate,
        replayPadSource);

    std::printf(
        "test_ss0_scene0_int_spu_direct: source vh=%zu vb=%zu tones=%u vags=%u\n",
        transaction.vhBytes.size(),
        transaction.vbBytes.size(),
        transaction.decoderToneSlotCount,
        transaction.decoderEffectiveVagCount);
}

}  // namespace

int main(int argc, char** argv) {
    TestFailClosed();
    TestSsSequenceCallbackSurface();
    TestSpuTransferCompletionSurface();
    TestSsTickInterruptSurface();
    TestSsMidiParserSurface();
    TestSsMidiScheduler8002BB30();
    TestSsMidiProgramChange8002BFDC();
    TestSsMidiControlChange8002C054();
    TestSsMidiPitchBend8002D3D0();
    TestSsMidiMeta8002D47C();
    TestSsMidiCallback8002C808();
    TestSsMidiCallbackGate8002C938();
    TestSsMidiTrackByteCounters8002CA7C8002CAF4();
    TestSsMidiControllerReset8002C740();
    TestSsMidiController65_8002C5DC();
    TestSsMidiController6_8002CB6C();
    TestSsMidiEnvelopeLowerHandlers8002B4748002B750();
    TestSsMidiTempoTick8002DDA4();
    TestSsMidiTrackFlagRoutes8002B9FC8002BAC88002DBE4();
    TestSsMidiNoteExecutionSurface();
    TestSsDriverFlushSurface();
    TestSsDriverVoiceCompletionSurface();
    TestHostAudioVoiceGenerationLease();
    TestSsDriverInitializationOwner8003226C();
    TestSsDriverResetStateProducer800351B8();
    TestSsSpuFirstAllocation8002E87C();
    TestSsVabBodyTransfer8002EB80();
    TestSsCompactSfxCommand80026EF8();
    TestSsDriverVoiceAllocationStateProducer80032EAC();
    TestSsDriverVolumeRampSetup80035110();
    TestSsDriverDirectVolumeWrites80034E5C80034F8C80035084();
    TestSsDriverVolumeRampsAndRegisterCommit();
    TestSyntheticYCompoSpuCandidate();
    const bool exactDisc = argc >= 2;
    if (exactDisc) {
        TestExactDisc(std::filesystem::path(argv[1]));
    }
    if (g_failures != 0) {
        return 1;
    }
    std::printf("test_ss0_scene0_int_spu_direct: ok exactDisc=%d\n",
                exactDisc ? 1 : 0);
    return 0;
}
