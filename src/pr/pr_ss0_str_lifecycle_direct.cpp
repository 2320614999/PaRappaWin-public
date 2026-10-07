#include "pr_ss0_str_lifecycle_direct.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <limits>
#include <utility>

#include "pr_movie_segment_direct.h"

namespace PrSS0StrLifecycleDirect {
namespace {

static bool Append(StrPlan& plan, const StrAction& action)
{
    if (plan.count >= sizeof(plan.actions) / sizeof(plan.actions[0])) {
        plan.truncated = true;
        return false;
    }
    plan.actions[plan.count++] = action;
    return true;
}

static void AppendAction(StrPlan& plan,
                         StrActionKind kind,
                         uint32_t psxFunction,
                         uint32_t arg0 = 0,
                         uint32_t arg1 = 0,
                         uint32_t arg2 = 0,
                         uint32_t arg3 = 0,
                         int32_t repeatCount = 1,
                         bool repeatKnown = true,
                         bool conditional = false)
{
    StrAction action{};
    action.kind = kind;
    action.psxFunction = psxFunction;
    action.arg0 = arg0;
    action.arg1 = arg1;
    action.arg2 = arg2;
    action.arg3 = arg3;
    action.repeatCount = repeatCount;
    action.repeatKnown = repeatKnown;
    action.conditional = conditional;
    (void)Append(plan, action);
}

static void AppendPlan(StrPlan& dst, const StrPlan& src)
{
    for (uint32_t i = 0; i < src.count; ++i) {
        (void)Append(dst, src.actions[i]);
    }
    dst.truncated = dst.truncated || src.truncated;
    dst.hasOpenP0Gap = dst.hasOpenP0Gap || src.hasOpenP0Gap;
}

static StrPlan MakePlan(StrPlanKind kind)
{
    StrPlan plan{};
    plan.kind = kind;
    plan.runtimeCutoverAllowed = RuntimeCutoverAllowed();
    return plan;
}

static uint32_t ComputeStripCount(uint32_t width)
{
    return (width + kUploadStripWidth - 1u) / kUploadStripWidth;
}

static uint16_t ReadLe16(const uint8_t* bytes)
{
    return static_cast<uint16_t>(bytes[0]) |
           static_cast<uint16_t>(static_cast<uint16_t>(bytes[1]) << 8u);
}

static uint64_t Fnv1a64(const uint8_t* bytes, size_t byteCount)
{
    uint64_t hash = 1469598103934665603ull;
    for (size_t index = 0u; index < byteCount; ++index) {
        hash ^= bytes[index];
        hash *= 1099511628211ull;
    }
    return hash;
}

static uint32_t ReadLe32(const uint8_t* bytes)
{
    return static_cast<uint32_t>(bytes[0]) |
           (static_cast<uint32_t>(bytes[1]) << 8u) |
           (static_cast<uint32_t>(bytes[2]) << 16u) |
           (static_cast<uint32_t>(bytes[3]) << 24u);
}

static void WriteLe32(uint8_t* bytes, uint32_t value)
{
    bytes[0] = static_cast<uint8_t>(value & 0xFFu);
    bytes[1] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
    bytes[2] = static_cast<uint8_t>((value >> 16u) & 0xFFu);
    bytes[3] = static_cast<uint8_t>((value >> 24u) & 0xFFu);
}

static void WriteLe16(uint8_t* bytes, uint16_t value)
{
    bytes[0] = static_cast<uint8_t>(value & 0xFFu);
    bytes[1] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
}

static uint8_t* SectorMetadata80039670(
    StrDecoderMemoryRuntime80027288& runtime,
    uint32_t slot)
{
    return runtime.allocations[1].data() +
           slot * kStrSectorMetadataBytes80039670;
}

static const uint8_t* SectorMetadata80039670(
    const StrDecoderMemoryRuntime80027288& runtime,
    uint32_t slot)
{
    return runtime.allocations[1].data() +
           slot * kStrSectorMetadataBytes80039670;
}

static uint8_t* SectorPayload80039670(
    StrDecoderMemoryRuntime80027288& runtime,
    uint32_t slot)
{
    return runtime.allocations[1].data() +
           kStrSectorMetadataAreaBytes80039670 +
           slot * kStrSectorPayloadBytes80039670;
}

static void ClearSectorSlotStates8003954C(
    StrDecoderMemoryRuntime80027288& runtime,
    uint32_t firstSlot,
    uint32_t count)
{
    for (uint32_t index = 0u; index < count; ++index) {
        const uint32_t slot = firstSlot + index;
        if (slot >= kDecoderSectorRingSlots8003624C) {
            break;
        }
        WriteLe16(SectorMetadata80039670(runtime, slot),
                  kStrSectorStateFree);
    }
}

} // namespace

bool RuntimeCutoverAllowed()
{
    return false;
}

TitleEarlyInputWaitCleanupHostProjection8001A694
BuildTitleEarlyInputWaitCleanupHostProjection8001A694(
    const TitleEarlyInputWaitCleanupHostProjectionInput8001A694& input)
{
    TitleEarlyInputWaitCleanupHostProjection8001A694 result{};
    result.psxCdAuthority = false;
    if (!input.playerPresent || input.strStarted || input.playerActive) {
        return result;
    }

    result.available = true;
    result.hostProjection = true;
    result.hostStrStopped = true;
    return result;
}

Scene0StrAudioRoute801C44E0 BuildScene0StrAudioRoute801C44E0(
    const Scene0StrAudioRouteSource801C44E0& source)
{
    Scene0StrAudioRoute801C44E0 result{};
    result.psxCdAuthority = false;
    if (!source.rowKnown || !source.opaque04Known) {
        return result;
    }

    result.known = true;
    result.hostProjection = false;
    result.currentScusSemanticAuthority = true;
    result.currentComod0CallerAuthority = true;
    result.volumeKnown = true;
    result.volume8001A478 =
        static_cast<int16_t>((source.opaque04 >> 16u) & 0xFFFFu);
    const int32_t commandVolume = std::min<int32_t>(
        0x7F,
        static_cast<int32_t>(result.volume8001A478));
    result.volumeMask8002AB24 = 0xC0u;
    result.scaledLeft8002AB24 = commandVolume * 258;
    result.scaledRight8002AB24 = commandVolume * 258;
    result.directVolumeCommandAuthority = true;
    result.psxSpuHardwareAuthority = false;
    result.normalizedVolume = std::min(
        1.0f,
        std::max(0.0f,
                 static_cast<float>(result.volume8001A478) / 127.0f));
    result.filterKnown = true;
    result.filterFile8004940C = 1u;
    result.filterChannel8004940D =
        static_cast<uint8_t>(source.opaque04 & 0xFFu);
    result.filterCommand8001A654 =
        static_cast<uint8_t>(kCdCommandSetfilter);
    result.filterCommandSerialKnown = true;
    result.directFilterCommandAuthority = true;
    return result;
}

StrDecodeGeometry KnownDecodeGeometry80027288(int32_t movieKind,
                                              bool word800916DC)
{
    StrDecodeGeometry geometry{};
    geometry.movieKind = movieKind;
    const StrDecoderTableRow80054614 row =
        KnownDecoderTableRow80054614(movieKind);
    if (!row.known) {
        return geometry;
    }

    geometry.known = true;
    geometry.width = row.expectedWidth;
    geometry.height = row.expectedHeight;
    geometry.dstX = static_cast<uint32_t>(row.dstX);
    geometry.dstY = static_cast<uint32_t>(row.dstY);
    if (movieKind != 2 && !word800916DC) {
        geometry.dstY += 23u;
        geometry.word800916DCApplied = true;
    }
    geometry.stripCount = ComputeStripCount(geometry.width);
    return geometry;
}

StrDecoderTableRow80054614 KnownDecoderTableRow80054614(
    int32_t movieKind)
{
    // Current SCUS_941.83 IDA table 80054614, 24 bytes per row. 80027288
    // consumes dstX/dstY and the two buffer sizes. Width/height are retained
    // as expected host-render geometry; the PSX decoder control starts those
    // two fields at zero and 800273A4 later replaces them from sector headers.
    static constexpr StrDecoderTableRow80054614 kRows[] = {
        {true, 0, 256u, 144u, 35, 25, 0x20u, 0x12000u, 0x24000u, 0u},
        {true, 1, 320u, 240u, 0, 0, 0x20u, 0x20000u, 0x4B000u, 0u},
        {true, 2, 256u, 144u, 32, 48, 0x20u, 0x12000u, 0x24000u, 0u},
    };
    if (movieKind < 0 ||
        static_cast<uint32_t>(movieKind) >=
            kMovieGeometryKnownRowCount80054614) {
        StrDecoderTableRow80054614 unknown{};
        unknown.movieKind = movieKind;
        return unknown;
    }
    return kRows[static_cast<uint32_t>(movieKind)];
}

bool TryInitializeStrDecoderState80027288(
    int32_t movieKind,
    bool word800916DC,
    StrDecoderAllocationState80027288& state)
{
    state = StrDecoderAllocationState80027288{};
    state.movieKind = movieKind;
    state.word800916DC = word800916DC;
    state.tableRow = KnownDecoderTableRow80054614(movieKind);
    if (!state.tableRow.known) {
        return false;
    }

    // 80027288 calls the 80027238 allocation/zero wrapper four times in this
    // exact order. 80027238 exits(1) on allocation failure, so a partial state
    // is never returned to the original caller.
    state.allocationSizes[0] = kDecoderControlBytes80027288;
    state.allocationSizes[1] = kDecoderSectorBufferBytes80027288;
    state.allocationSizes[2] = state.tableRow.vlcBufferBytes;
    state.allocationSizes[3] = state.tableRow.imageBufferBytes;
    state.allocationCount = 4u;
    state.allocationAlignment = kDecoderAllocationAlignment80025A70;
    state.allocationsZeroFilled = true;
    state.allocationFailureExitsProcess = true;
    state.allocationPointerSlots[0] = kDecoderControlPointerSlot800965A8;
    state.allocationPointerSlots[1] = kDecoderSectorPointerSlot800965AC;
    state.allocationPointerSlots[2] = kDecoderVlcPointerSlot800965B0;
    state.allocationPointerSlots[3] = kDecoderImagePointerSlot800965B4;

    state.decDctResetCalled800473EC = true;
    state.decDctResetArgument800473EC = 0u;
    state.resetCallbackCalled800473EC = true;
    state.sectorRingInitialized8003624C = true;
    state.sectorRingSlots8003624C = kDecoderSectorRingSlots8003624C;
    state.sectorRingCountAddress801C3868 =
        kDecoderRingSlotCount801C3868;
    state.outputCallbackInstalled80047658 = true;
    state.outputCallbackFunction80027220 = kFn80027220;
    state.outputCallbackDmaChannel80047658 = 1u;

    state.control.known = true;
    state.control.vlcBufferBound = true;
    state.control.imageBufferBound = true;
    state.control.decodedWidth = 0u;
    state.control.decodedHeight = 0u;
    state.control.vlcDecodeResult = 0u;
    state.control.dstX = state.tableRow.dstX;
    state.control.dstY = state.tableRow.dstY;
    if (movieKind != 2 && !word800916DC) {
        state.control.dstY += 23;
    }
    state.control.decodeInFlight = 0u;
    state.control.outputReady = 0u;

    state.currentScusSemanticAuthority = true;
    state.currentComod0CallerAuthority = true;
    state.comod1ComparisonOnly = true;
    state.hostProjection = false;
    state.psxPointerAuthority = false;
    state.replayValueAuthority = false;
    state.oldWinS0Authority = false;
    state.stage2PlusAuthority = false;
    state.comod2Authority = false;
    state.accepted = true;
    return true;
}

bool TryInitializeStrDecoderMemoryRuntime80027288(
    const StrDecoderAllocationState80027288& source,
    StrDecoderMemoryRuntime80027288& runtime)
{
    if (runtime.initialized ||
        !source.accepted ||
        !source.tableRow.known ||
        source.allocationCount != 4u ||
        source.allocationAlignment !=
            kDecoderAllocationAlignment80025A70 ||
        !source.allocationsZeroFilled ||
        !source.allocationFailureExitsProcess ||
        !source.sectorRingInitialized8003624C ||
        source.sectorRingSlots8003624C !=
            kDecoderSectorRingSlots8003624C ||
        source.sectorRingCountAddress801C3868 !=
            kDecoderRingSlotCount801C3868 ||
        !source.outputCallbackInstalled80047658 ||
        source.outputCallbackFunction80027220 != kFn80027220 ||
        source.outputCallbackDmaChannel80047658 != 1u ||
        !source.currentScusSemanticAuthority ||
        !source.currentComod0CallerAuthority ||
        !source.comod1ComparisonOnly ||
        source.hostProjection ||
        source.psxPointerAuthority ||
        source.replayValueAuthority ||
        source.oldWinS0Authority ||
        source.stage2PlusAuthority ||
        source.comod2Authority) {
        return false;
    }

    static constexpr std::array<uint32_t, 4> kPointerSlots = {
        kDecoderControlPointerSlot800965A8,
        kDecoderSectorPointerSlot800965AC,
        kDecoderVlcPointerSlot800965B0,
        kDecoderImagePointerSlot800965B4,
    };
    StrDecoderMemoryRuntime80027288 next{};
    next.movieKind = source.movieKind;
    next.allocationPointerSlots = kPointerSlots;
    bool alignmentVerified = true;
    bool zeroFilled = true;
    for (uint32_t index = 0; index < source.allocationCount; ++index) {
        if (source.allocationSizes[index] == 0u ||
            source.allocationPointerSlots[index] !=
                kPointerSlots[index]) {
            return false;
        }
        next.allocationSizes[index] = source.allocationSizes[index];
        try {
            next.allocations[index].assign(
                source.allocationSizes[index],
                0u);
        } catch (...) {
            return false;
        }
        next.allocationPresent[index] = true;
        ++next.allocationStackDepth;
        next.allocationBytes += source.allocationSizes[index];
        alignmentVerified =
            alignmentVerified &&
            (reinterpret_cast<uintptr_t>(
                 next.allocations[index].data()) %
             kDecoderAllocationAlignment80025A70) == 0u;
        zeroFilled =
            zeroFilled &&
            std::all_of(
                next.allocations[index].begin(),
                next.allocations[index].end(),
                [](uint8_t value) { return value == 0u; });
    }

    if (!alignmentVerified || !zeroFilled ||
        next.allocationSizes[1] %
                kDecoderSectorRingSlots8003624C !=
            0u) {
        return false;
    }
    next.allocationsZeroFilled = true;
    next.allocationAlignmentVerified = true;
    next.sectorRingInitialized8003624C = true;
    next.sectorRingSlots8003624C =
        kDecoderSectorRingSlots8003624C;
    next.sectorRingSlotBytes =
        next.allocationSizes[1] /
        kDecoderSectorRingSlots8003624C;
    next.sectorRingQueuedSlots = 0u;
    next.outputCallbackInstalled80047658 = true;
    next.outputCallbackFunction80027220 = kFn80027220;
    next.outputCallbackDmaChannel80047658 = 1u;
    next.directMemoryAuthority = true;
    next.directSectorRingLifecycleAuthority = true;
    next.psxPointerAuthority = false;
    next.psxHardwareMmioAuthority = false;
    next.hardwareCallbackTimingAuthority = false;
    next.hostProjection = false;
    next.replayValueAuthority = false;
    next.oldWinS0Authority = false;
    next.stage2PlusAuthority = false;
    next.comod2Authority = false;
    next.initialized = true;
    runtime = std::move(next);
    return true;
}

bool TryStartStrStreamState800274D4(
    const StrDecoderAllocationState80027288& decoderState,
    StrStreamStartState800274D4& state)
{
    state = StrStreamStartState800274D4{};
    if (!decoderState.accepted ||
        !decoderState.control.known ||
        !decoderState.currentScusSemanticAuthority ||
        !decoderState.currentComod0CallerAuthority ||
        !decoderState.comod1ComparisonOnly ||
        decoderState.hostProjection ||
        decoderState.psxPointerAuthority ||
        decoderState.replayValueAuthority ||
        decoderState.oldWinS0Authority ||
        decoderState.stage2PlusAuthority ||
        decoderState.comod2Authority) {
        return false;
    }

    // Current SCUS 800274D4 clears only control +10/+1C/+20 before starting
    // the stream library. Width, height, destinations, and buffer bindings
    // are retained. It then calls StSetStream, DecDCTvlcSize2, and 800273A4
    // in that exact order before clearing the sector counter at 800965B8.
    state.control = decoderState.control;
    state.control.vlcDecodeResult = 0u;
    state.control.decodeInFlight = 0u;
    state.control.outputReady = 0u;
    state.vlcDecodeResultCleared = true;
    state.decodeInFlightCleared = true;
    state.outputReadyCleared = true;

    state.stSetStreamCalled = true;
    state.stSetStreamArgs[0] = 0;
    state.stSetStreamArgs[1] = 1;
    state.stSetStreamArgs[2] = -1;
    state.stSetStreamArgs[3] = 0;
    state.stSetStreamArgs[4] = 0;
    state.decDctVlcSize2Called = true;
    state.decDctVlcSize2Argument = 0;
    state.initialDecodeAdvanceCalled800273A4 = true;
    state.sectorCounterResetAfterAdvance = true;
    state.sectorCounterAddress800965B8 = kDecoderSectorCounter800965B8;
    state.sectorCounterValue = 0u;
    state.exactCallOrder = true;

    state.currentScusSemanticAuthority = true;
    state.currentComod0CallerAuthority = true;
    state.comod1ComparisonOnly = true;
    state.lowerCdStartAuthority = false;
    state.initialDecodeAdvanceSemanticTranslated = false;
    state.streamLibraryAuthority = false;
    state.psxPointerAuthority = false;
    state.hostProjection = false;
    state.replayValueAuthority = false;
    state.oldWinS0Authority = false;
    state.stage2PlusAuthority = false;
    state.comod2Authority = false;
    state.accepted = true;
    return true;
}

bool TryInitializeStrLowerCdStartRuntime8001A4D0(
    const StrLowerCdStartSource8001A4D0& source,
    StrLowerCdStartRuntime8001A4D0& state)
{
    state = StrLowerCdStartRuntime8001A4D0{};
    if (!source.segmentKnown || !source.startLbaKnown ||
        source.startLba < 0 || !source.lengthBytesKnown ||
        source.lengthBytes == 0u || !source.filterKnown ||
        source.discBinPath.empty() ||
        !source.discImageDirectoryAuthority) {
        return false;
    }

    const uint64_t startLba = static_cast<uint64_t>(source.startLba);
    const uint64_t logicalSectorCount =
        (static_cast<uint64_t>(source.lengthBytes) +
         kRawCdLogicalBytes8001A4D0 - 1u) /
        kRawCdLogicalBytes8001A4D0;
    if (logicalSectorCount == 0u ||
        logicalSectorCount >
            static_cast<uint64_t>((std::numeric_limits<uint32_t>::max)())) {
        return false;
    }

    std::error_code ec;
    const uint64_t discBytes =
        std::filesystem::file_size(source.discBinPath, ec);
    if (ec || startLba >
                  (std::numeric_limits<uint64_t>::max)() /
                      kRawCdSectorBytes8001A4D0 ||
        startLba + logicalSectorCount <
            startLba ||
        startLba + logicalSectorCount >
            discBytes / kRawCdSectorBytes8001A4D0) {
        return false;
    }

    std::ifstream disc(source.discBinPath, std::ios::binary);
    if (!disc) {
        return false;
    }
    const uint64_t rawOffset =
        startLba * kRawCdSectorBytes8001A4D0;
    if (rawOffset >
        static_cast<uint64_t>(
            (std::numeric_limits<std::streamoff>::max)())) {
        return false;
    }
    disc.seekg(static_cast<std::streamoff>(rawOffset), std::ios::beg);
    if (!disc.good()) {
        return false;
    }

    std::array<uint8_t, kRawCdSectorBytes8001A4D0> raw{};
    disc.read(reinterpret_cast<char*>(raw.data()),
              static_cast<std::streamsize>(raw.size()));
    if (disc.gcount() != static_cast<std::streamsize>(raw.size())) {
        return false;
    }
    state.rawSectorReadable = true;

    bool syncMatched = raw[0] == 0u &&
                       raw[kRawCdSyncBytes8001A4D0 - 1u] == 0u;
    for (uint32_t index = 1u;
         syncMatched && index + 1u < kRawCdSyncBytes8001A4D0;
         ++index) {
        syncMatched = raw[index] == 0xFFu;
    }
    if (!syncMatched) {
        return false;
    }
    state.syncHeaderMatched = true;

    PrMovieSegmentDirect::MsfBcd80036A78 headerMsf{};
    headerMsf.minute = raw[kRawCdHeaderOffset8001A4D0 + 0u];
    headerMsf.second = raw[kRawCdHeaderOffset8001A4D0 + 1u];
    headerMsf.frame = raw[kRawCdHeaderOffset8001A4D0 + 2u];
    const auto headerLba =
        PrMovieSegmentDirect::PsxCall80036A78_MsfToLba(headerMsf);
    if (!headerLba.known || headerLba.lba != source.startLba) {
        return false;
    }
    state.headerLbaMatched = true;

    if (raw[kRawCdHeaderOffset8001A4D0 + 3u] !=
        kRawCdMode2Byte8001A4D0) {
        return false;
    }
    state.mode2Matched = true;

    const uint8_t* subheader =
        raw.data() + kRawCdSubheaderOffset8001A4D0;
    if (std::memcmp(subheader, subheader + 4u, 4u) != 0) {
        return false;
    }
    state.xaSubheaderDuplicateMatched = true;
    state.filterFile = source.filterFile;
    state.filterChannel = source.filterChannel;
    state.discBinPath = source.discBinPath;
    state.xaSubmode = subheader[2];
    state.xaCoding = subheader[3];
    if (subheader[0] != source.filterFile ||
        subheader[1] != source.filterChannel) {
        return false;
    }
    state.xaFilterMatched = true;
    if ((state.xaSubmode & (kXaSubmodeRealtime8001A4D0 |
                            kXaSubmodeVideo8001A4D0)) !=
        (kXaSubmodeRealtime8001A4D0 |
         kXaSubmodeVideo8001A4D0)) {
        return false;
    }
    state.xaRealtimeVideoMatched = true;

    // Current SCUS 8001A4D0 issues Setloc -> sync 2, Setfilter -> sync 2,
    // Setmode/ReadS -> wait 3 -> sync 2, then writes 80049410=1 and
    // 80049420=-16 before 80036678(1,0). The Windows direct lower-CD
    // adapter is synchronous, so an exact original-disc sector satisfying
    // the requested position/filter supplies those three completion points.
    state.startLba = source.startLba;
    state.lengthBytes = source.lengthBytes;
    state.logicalSectorCount =
        static_cast<uint32_t>(logicalSectorCount);
    state.setlocComplete800364D0 = true;
    state.setfilterComplete800364D0 = true;
    state.setmodeReadSComplete800391AC = true;
    state.startWaitFrames80035560 = kStartWaitFrames;
    state.streamStartComplete800364D0 = true;
    state.startFlagsWritten8001A4D0 = true;
    state.dword80049410 = 1;
    state.dword80049420 =
        -static_cast<int32_t>(kStreamingBufferCadence);
    state.primeStatusCalled80036678 = true;
    state.commandWrapper80036678Succeeded = true;
    state.statusPollInputKnown8001A750 = true;
    state.syncReturn800364D0 = static_cast<int32_t>(kCdSyncComplete);
    state.status0 = 0x20u;
    state.exactCallOrder = true;
    state.currentScusSemanticAuthority = true;
    state.currentComod0CallerAuthority = true;
    state.discImageDirectoryAuthority = true;
    state.discImagePayloadAuthority = true;
    state.directPsxStatusAuthority = true;
    state.hardwareCallbackAuthority = false;
    state.hostProjection = false;
    state.hostExtractedFileAuthority = false;
    state.replayValueAuthority = false;
    state.oldWinS0Authority = false;
    state.stage2PlusAuthority = false;
    state.comod2Authority = false;
    state.initialized = true;
    return true;
}

bool TryInitializeStrXaAudioDiscRuntime8001A4D0(
    const StrLowerCdStartRuntime8001A4D0& start,
    StrXaAudioDiscRuntime8001A4D0& runtime)
{
    runtime = StrXaAudioDiscRuntime8001A4D0{};
    if (!start.initialized || start.discBinPath.empty() ||
        start.logicalSectorCount == 0u ||
        !start.currentScusSemanticAuthority ||
        !start.currentComod0CallerAuthority ||
        !start.discImagePayloadAuthority ||
        start.hardwareCallbackAuthority || start.hostProjection ||
        start.hostExtractedFileAuthority || start.replayValueAuthority ||
        start.oldWinS0Authority || start.stage2PlusAuthority ||
        start.comod2Authority) {
        return false;
    }

    runtime.filterFile = start.filterFile;
    runtime.filterChannel = start.filterChannel;
    runtime.currentScusSemanticAuthority = true;
    runtime.currentComod0CallerAuthority = true;
    runtime.discImagePayloadAuthority = true;
    runtime.directXaSectorSelectionAuthority = true;
    // SCUS delegates XA decode and output to CD/SPU hardware. This runtime
    // owns original-disc sector selection only; the Windows XA decoder and
    // AudioEngine remain output adapters outside this semantic state.
    runtime.psxCdXaDecodeAuthority = false;
    runtime.psxSpuHardwareAuthority = false;
    runtime.hostExtractedFileAuthority = false;
    runtime.replayValueAuthority = false;
    runtime.oldWinS0Authority = false;
    runtime.stage2PlusAuthority = false;
    runtime.comod2Authority = false;
    runtime.initialized = true;
    return true;
}

StrXaAudioSectorResult8001A4D0 ReadNextStrXaAudioSectorFromDisc8001A4D0(
    const StrLowerCdStartRuntime8001A4D0& start,
    uint32_t maxRawSectorCursor,
    StrXaAudioDiscRuntime8001A4D0& runtime)
{
    StrXaAudioSectorResult8001A4D0 result{};
    result.rawSectorCursorBefore = runtime.rawSectorCursor;
    if (!runtime.initialized || !start.initialized ||
        start.discBinPath.empty() ||
        !runtime.currentScusSemanticAuthority ||
        !runtime.currentComod0CallerAuthority ||
        !runtime.discImagePayloadAuthority ||
        !runtime.directXaSectorSelectionAuthority ||
        runtime.psxCdXaDecodeAuthority || runtime.psxSpuHardwareAuthority ||
        runtime.hostExtractedFileAuthority || runtime.replayValueAuthority ||
        runtime.oldWinS0Authority || runtime.stage2PlusAuthority ||
        runtime.comod2Authority ||
        runtime.filterFile != start.filterFile ||
        runtime.filterChannel != start.filterChannel) {
        return result;
    }

    const uint32_t targetCursor = std::min(
        maxRawSectorCursor,
        start.logicalSectorCount);
    if (runtime.rawSectorCursor > targetCursor) {
        return result;
    }

    std::ifstream disc(start.discBinPath, std::ios::binary);
    if (!disc) {
        return result;
    }

    result.known = true;
    result.currentScusSemanticAuthority = true;
    result.currentComod0CallerAuthority = true;
    result.discImagePayloadAuthority = true;
    result.directXaSectorSelectionAuthority = true;
    std::array<uint8_t, kRawCdSectorBytes8001A4D0> raw{};
    while (runtime.rawSectorCursor < targetCursor) {
        const uint32_t cursor = runtime.rawSectorCursor;
        const uint64_t lba =
            static_cast<uint64_t>(start.startLba) + cursor;
        const uint64_t rawOffset = lba * kRawCdSectorBytes8001A4D0;
        if (rawOffset > static_cast<uint64_t>(
                            (std::numeric_limits<std::streamoff>::max)())) {
            return StrXaAudioSectorResult8001A4D0{};
        }
        disc.seekg(static_cast<std::streamoff>(rawOffset), std::ios::beg);
        disc.read(reinterpret_cast<char*>(raw.data()),
                  static_cast<std::streamsize>(raw.size()));
        if (disc.gcount() != static_cast<std::streamsize>(raw.size())) {
            return StrXaAudioSectorResult8001A4D0{};
        }
        ++runtime.rawSectorCursor;
        ++runtime.rawSectorsScanned;
        ++result.rawSectorsScanned;

        bool syncMatched = raw[0] == 0u &&
                           raw[kRawCdSyncBytes8001A4D0 - 1u] == 0u;
        for (uint32_t index = 1u;
             syncMatched && index + 1u < kRawCdSyncBytes8001A4D0;
             ++index) {
            syncMatched = raw[index] == 0xFFu;
        }
        const uint8_t* subheader =
            raw.data() + kRawCdSubheaderOffset8001A4D0;
        if (!syncMatched ||
            raw[kRawCdHeaderOffset8001A4D0 + 3u] !=
                kRawCdMode2Byte8001A4D0 ||
            std::memcmp(subheader, subheader + 4u, 4u) != 0 ||
            subheader[0] != runtime.filterFile ||
            subheader[1] != runtime.filterChannel ||
            (subheader[2] & (kXaSubmodeRealtime8001A4D0 |
                             kXaSubmodeAudio8001A4D0)) !=
                (kXaSubmodeRealtime8001A4D0 |
                 kXaSubmodeAudio8001A4D0)) {
            continue;
        }

        ++runtime.filteredAudioSectors;
        result.available = true;
        result.matchedRawSectorCursor = cursor;
        result.file = subheader[0];
        result.channel = subheader[1];
        result.submode = subheader[2];
        result.coding = subheader[3];
        result.syncHeaderMatched = true;
        result.mode2Matched = true;
        result.xaSubheaderDuplicateMatched = true;
        result.xaFilterMatched = true;
        result.xaRealtimeAudioMatched = true;
        std::memcpy(result.payload.data(),
                    subheader + 8u,
                    result.payload.size());
        break;
    }
    result.rawSectorCursorAfter = runtime.rawSectorCursor;
    result.targetCursorReached =
        runtime.rawSectorCursor >= targetCursor;
    return result;
}

StrDecodeAdvanceState800273A4 BuildStrDecodeAdvanceState800273A4(
    const StrStreamStartState800274D4& streamState,
    const StrDecodeAdvanceInput800273A4& input)
{
    StrDecodeAdvanceState800273A4 state{};
    if (!streamState.accepted ||
        !streamState.control.known ||
        !streamState.currentScusSemanticAuthority ||
        !streamState.currentComod0CallerAuthority ||
        !streamState.comod1ComparisonOnly ||
        streamState.hostProjection ||
        streamState.psxPointerAuthority ||
        streamState.replayValueAuthority ||
        streamState.oldWinS0Authority ||
        streamState.stage2PlusAuthority ||
        streamState.comod2Authority) {
        return state;
    }

    state.planned = true;
    state.control = streamState.control;
    state.acquireFrameCalled8003958C = true;
    state.acquireReturnKnown = input.acquireReturnKnown;
    state.acquireReturn = input.acquireReturn;
    state.sectorCounterAddress800965B8 = kDecoderSectorCounter800965B8;
    state.sectorCounterBefore = input.sectorCounterBefore;
    state.sectorCounterAfter = input.sectorCounterBefore;
    state.branchSemanticsTranslated = true;
    state.runtimeInputBound = false;
    state.sectorRingExecutionAuthority = false;
    state.mdecExecutionAuthority = false;
    state.psxPointerAuthority = false;
    state.hostProjection = false;
    state.replayValueAuthority = false;
    state.oldWinS0Authority = false;
    state.stage2PlusAuthority = false;
    state.comod2Authority = false;

    if (!input.acquireReturnKnown) {
        return state;
    }
    state.runtimeInputBound = input.sectorRingExecutionAuthority;
    state.sectorRingExecutionAuthority =
        input.sectorRingExecutionAuthority;
    if (input.acquireReturn != 0) {
        state.control.decodeInFlight = 0u;
        state.branch = StrDecodeAdvanceBranch800273A4::AcquireUnavailable;
        state.executed = true;
        state.hasOpenInputGap = false;
        return state;
    }
    if (!input.payloadHandleKnown || !input.sectorHeaderKnown) {
        return state;
    }

    state.payloadHandleKnown = true;
    state.payloadHandle = input.payloadHandle;
    state.executedThroughAcquire8003958C =
        input.sectorRingExecutionAuthority;
    if (state.control.decodedWidth != input.sectorWidth ||
        state.control.decodedHeight != input.sectorHeight) {
        state.control.decodedWidth = input.sectorWidth;
        state.control.decodedHeight = input.sectorHeight;
        state.sectorHeaderApplied = true;
    }
    state.sectorCounterIncremented = true;
    state.sectorCounterAfter = input.sectorCounterBefore + 1u;

    if (input.payloadHandle == 0u) {
        state.control.decodeInFlight = 0u;
        state.branch = StrDecodeAdvanceBranch800273A4::EmptyPayload;
        state.executed = true;
        state.hasOpenInputGap = false;
        return state;
    }
    state.control.decodeInFlight = 1u;
    state.reachedDecDctVlc2Boundary = true;
    if (!input.vlcDecodeResultKnown) {
        state.branch = StrDecodeAdvanceBranch800273A4::MdecBoundary;
        state.hasOpenInputGap = false;
        state.blockedAtMdecDecDCTvlc2 =
            input.sectorRingExecutionAuthority;
        return state;
    }

    state.decDctVlc2Called = true;
    state.vlcDecodeResultKnown = true;
    state.vlcDecodeResult = input.vlcDecodeResult;
    state.control.vlcDecodeResult =
        static_cast<uint32_t>(input.vlcDecodeResult);
    state.releaseFrameCalled80039490 = true;
    if (input.vlcDecodeResult != 0) {
        state.branch = StrDecodeAdvanceBranch800273A4::VlcDecodeFailed;
        state.executed = true;
        state.hasOpenInputGap = false;
        return state;
    }

    state.reachedMdecControlBoundary80047558 = true;
    if (!input.mdecCommandOutputSubmissionKnown) {
        state.branch =
            StrDecodeAdvanceBranch800273A4::MdecHardwareBoundary;
        state.blockedAtMdecHardwareExecution = true;
        state.hasOpenMdecHardwareGap = true;
        return state;
    }
    if (!input.mdecCommandOutputSubmissionAuthority) {
        return state;
    }

    state.mdecControlCalled80047558 = true;
    state.mdecControlArgument = 2;
    state.outputTransferCalled = true;
    const uint32_t macroblockRows =
        (state.control.decodedHeight + 15u) >> 4u;
    state.outputTransferWordCount =
        (16u * state.control.decodedWidth * macroblockRows) >> 1u;
    state.control.outputReady = 1u;
    state.mdecCommandOutputSubmissionAuthority = true;
    state.blockedAtMdecHardwareExecution = true;
    state.blockedAtMdecHardwareOutput = true;
    state.hasOpenMdecHardwareGap = true;
    state.branch = StrDecodeAdvanceBranch800273A4::
        MdecOutputSubmittedHardwareBoundary;
    state.executed = true;
    state.hasOpenInputGap = false;
    return state;
}

StrDecodeGateState80027528 BuildStrDecodeGateState80027528(
    const StrDecoderControlState80027288& control)
{
    StrDecodeGateState80027528 state{};
    if (!control.known) {
        return state;
    }
    state.known = true;
    state.outputReady = control.outputReady;
    state.decodeInFlight = control.decodeInFlight;
    state.branchSemanticsTranslated = true;
    state.decodeAdvanceExecuted = false;
    state.hostProjection = false;
    state.replayValueAuthority = false;
    state.oldWinS0Authority = false;
    state.stage2PlusAuthority = false;
    state.comod2Authority = false;

    if (control.outputReady == 0u) {
        state.branch = StrDecodeGateBranch80027528::OutputNotReady;
        state.callDecodeAdvance800273A4 = true;
        return state;
    }
    if (control.decodeInFlight == 0u) {
        state.branch = StrDecodeGateBranch80027528::DecodeIdle;
        state.callDecodeAdvance800273A4 = true;
        return state;
    }
    state.branch = StrDecodeGateBranch80027528::DecodeBusy;
    state.callDecodeAdvance800273A4 = false;
    state.returnValueKnown = true;
    state.returnValue = static_cast<int32_t>(control.decodeInFlight);
    return state;
}

StrStripUploadState8002756C BuildStrStripUploadState8002756C(
    const StrDecoderControlState80027288& control,
    uint32_t sectorCounter,
    bool displayBufferResultKnown,
    int32_t displayBufferResult8004019C)
{
    StrStripUploadState8002756C state{};
    if (!control.known) {
        return state;
    }
    state.sectorCounter = sectorCounter;
    state.gpuUploadExecuted = false;
    state.psxImagePointerAuthority = false;
    state.hostProjection = false;
    state.replayValueAuthority = false;
    state.oldWinS0Authority = false;
    state.stage2PlusAuthority = false;
    state.comod2Authority = false;
    if (sectorCounter < 2u) {
        state.known = true;
        state.skippedForCounterWarmup = true;
        return state;
    }
    if (!displayBufferResultKnown) {
        return state;
    }

    state.displayBufferResultKnown = true;
    state.displayBufferResult8004019C = displayBufferResult8004019C;
    state.destinationY =
        control.dstY + (displayBufferResult8004019C != 0 ? 240 : 0);
    state.macroblockRows = (control.decodedHeight + 15u) >> 4u;
    state.stripCount = control.decodedWidth >> 4u;
    state.known = true;
    if (state.stripCount == 0u) {
        state.skippedForZeroStrips = true;
        return state;
    }

    const uint32_t capacity =
        static_cast<uint32_t>(sizeof(state.commands) /
                              sizeof(state.commands[0]));
    const uint32_t emitCount =
        state.stripCount < capacity ? state.stripCount : capacity;
    for (uint32_t strip = 0; strip < emitCount; ++strip) {
        const uint32_t xOffset = strip * 16u;
        StrStripUploadCommand8002756C& command = state.commands[strip];
        command.x = static_cast<int16_t>(control.dstX +
                                         static_cast<int32_t>(xOffset));
        command.y = static_cast<int16_t>(state.destinationY);
        command.width = 16;
        command.height = static_cast<int16_t>(control.decodedHeight);
        command.sourcePitchTerm =
            16u * xOffset * state.macroblockRows;
        command.sourceByteOffset = command.sourcePitchTerm * 2u;
    }
    state.truncated = state.stripCount > capacity;
    state.uploadCallsPlanned80044D64 = true;
    return state;
}

StrStripUploadExecution8002756C ExecuteStrStripUpload8002756C(
    const StrStripUploadState8002756C& plan,
    const uint8_t* mdecVerticalOutput,
    size_t mdecVerticalOutputBytes)
{
    StrStripUploadExecution8002756C result{};
    if (!plan.known || plan.skippedForCounterWarmup ||
        plan.skippedForZeroStrips || !plan.displayBufferResultKnown ||
        plan.stripCount == 0u || plan.stripCount > 64u || plan.truncated ||
        !plan.uploadCallsPlanned80044D64 || plan.gpuUploadExecuted ||
        plan.psxImagePointerAuthority || plan.hostProjection ||
        plan.replayValueAuthority || plan.oldWinS0Authority ||
        plan.stage2PlusAuthority || plan.comod2Authority ||
        mdecVerticalOutput == nullptr || mdecVerticalOutputBytes == 0u) {
        return result;
    }

    const uint32_t width = plan.stripCount * 16u;
    const int16_t heightSigned = plan.commands[0].height;
    if (width == 0u || heightSigned <= 0) {
        return result;
    }
    const uint32_t height = static_cast<uint32_t>(heightSigned);
    const uint64_t pixelCount64 =
        static_cast<uint64_t>(width) * static_cast<uint64_t>(height);
    const uint64_t rgbaBytes64 = pixelCount64 * sizeof(uint32_t);
    if (pixelCount64 > (std::numeric_limits<size_t>::max)() ||
        rgbaBytes64 > (std::numeric_limits<uint32_t>::max)()) {
        return result;
    }

    uint64_t sourceBytesConsumed64 = 0u;
    size_t sourceExtent = 0u;
    for (uint32_t strip = 0u; strip < plan.stripCount; ++strip) {
        const StrStripUploadCommand8002756C& command =
            plan.commands[strip];
        const uint64_t stripBytes =
            static_cast<uint64_t>(command.width) *
            static_cast<uint64_t>(command.height) * sizeof(uint16_t);
        const uint64_t commandEnd =
            static_cast<uint64_t>(command.sourceByteOffset) + stripBytes;
        if (command.width != 16 || command.height != heightSigned ||
            command.x != plan.commands[0].x +
                             static_cast<int32_t>(strip * 16u) ||
            command.y != plan.commands[0].y ||
            stripBytes == 0u || commandEnd > mdecVerticalOutputBytes ||
            commandEnd > (std::numeric_limits<size_t>::max)()) {
            return StrStripUploadExecution8002756C{};
        }
        sourceBytesConsumed64 += stripBytes;
        sourceExtent = (std::max)(
            sourceExtent, static_cast<size_t>(commandEnd));
    }
    if (sourceBytesConsumed64 > (std::numeric_limits<uint32_t>::max)()) {
        return result;
    }

    result.rgbaPixels.resize(static_cast<size_t>(pixelCount64));
    for (uint32_t strip = 0u; strip < plan.stripCount; ++strip) {
        const StrStripUploadCommand8002756C& command =
            plan.commands[strip];
        for (uint32_t y = 0u; y < height; ++y) {
            for (uint32_t x = 0u; x < 16u; ++x) {
                const size_t sourceOffset =
                    static_cast<size_t>(command.sourceByteOffset) +
                    (static_cast<size_t>(y) * 16u + x) *
                        sizeof(uint16_t);
                const uint16_t bgr555 =
                    ReadLe16(mdecVerticalOutput + sourceOffset);
                const uint32_t red5 = bgr555 & 0x1Fu;
                const uint32_t green5 = (bgr555 >> 5u) & 0x1Fu;
                const uint32_t blue5 = (bgr555 >> 10u) & 0x1Fu;
                const uint32_t red8 = (red5 << 3u) | (red5 >> 2u);
                const uint32_t green8 =
                    (green5 << 3u) | (green5 >> 2u);
                const uint32_t blue8 =
                    (blue5 << 3u) | (blue5 >> 2u);
                const size_t destination =
                    static_cast<size_t>(y) * width + strip * 16u + x;
                result.rgbaPixels[destination] =
                    red8 | (green8 << 8u) | (blue8 << 16u) |
                    0xFF000000u;
            }
        }
    }

    result.width = width;
    result.height = height;
    result.destinationX = plan.commands[0].x;
    result.destinationY = plan.commands[0].y;
    result.stripCount = plan.stripCount;
    result.commandsExecuted80044D64 = plan.stripCount;
    result.sourceBytesConsumed =
        static_cast<uint32_t>(sourceBytesConsumed64);
    result.rgbaBytesWritten = static_cast<uint32_t>(rgbaBytes64);
    result.sourceFnv1a64 =
        Fnv1a64(mdecVerticalOutput, sourceExtent);
    result.rgbaFnv1a64 = Fnv1a64(
        reinterpret_cast<const uint8_t*>(result.rgbaPixels.data()),
        static_cast<size_t>(rgbaBytes64));
    result.exactCurrentIdaStripOrder = true;
    result.directMdecVerticalLayoutAuthority = true;
    result.directRgbaAssemblyAuthority = true;
    // Each 16-pixel strip is the direct translation of one 80044D64 upload
    // command.  The command list has already been range-checked above, so
    // publishing this count makes the translated GPU/DMA owner consumable by
    // the Scene0 VRAM publisher while keeping host texture upload separate.
    result.translatedGpuCommandCount = plan.stripCount;
    result.translatedGpuDmaStateAuthority =
        result.translatedGpuCommandCount == plan.stripCount;
    result.translatedGpuCompletionAuthority =
        result.translatedGpuDmaStateAuthority;
    result.hostTextureUploadPending = true;
    result.hostTextureUploadExecuted = false;
    result.psxGpuDmaExecutionAuthority = false;
    result.psxPointerAuthority = false;
    result.replayValueAuthority = false;
    result.oldWinS0Authority = false;
    result.stage2PlusAuthority = false;
    result.comod2Authority = false;
    result.known = true;
    result.executed = true;
    return result;
}

StrClockPollState801C4350 BuildStrClockPollState801C4350(
    const StrClockPollInput801C4350& input)
{
    StrClockPollState801C4350 state{};
    state.planned = true;
    state.clockQueryCalled8001A7A4 = true;
    state.clockDeltaKnown = input.clockDeltaKnown8001A7A4;
    state.clockDelta = input.clockDelta8001A7A4;
    state.previousClockKnown = input.previousClockKnown801C954C;
    state.previousClock = input.previousClock801C954C;
    state.branchSemanticsTranslated = true;
    state.currentScusSemanticAuthority = true;
    state.currentComod0CallerAuthority = true;
    state.comod1ComparisonOnly = true;
    if (!input.clockDeltaKnown8001A7A4 ||
        !input.previousClockKnown801C954C) {
        return state;
    }

    state.runtimeInputBound = input.directDiscSegmentAuthority;
    state.psxCdClockAuthority = input.directDiscSegmentAuthority;
    state.executed = true;
    if (input.clockDelta8001A7A4 < 0) {
        state.hasOpenInputGap = false;
        state.branch = StrClockPollBranch801C4350::NegativeClockDelta;
        state.returnValueKnown = true;
        state.returnValue = true;
        return state;
    }

    const int64_t signedDifference =
        static_cast<int64_t>(input.clockDelta8001A7A4) -
        static_cast<int64_t>(input.previousClock801C954C);
    state.absoluteClockDelta = static_cast<uint32_t>(
        signedDifference < 0 ? -signedDifference : signedDifference);
    if (state.absoluteClockDelta >=
        kStrClockDiscontinuityThreshold) {
        state.hasOpenInputGap = false;
        state.branch = StrClockPollBranch801C4350::Discontinuity;
        state.returnValueKnown = true;
        state.returnValue = true;
        return state;
    }

    state.clockFieldsUpdated = true;
    state.updatedPreviousClock = input.clockDelta8001A7A4;
    const uint32_t scaledClock =
        static_cast<uint32_t>(input.clockDelta8001A7A4) /
        kStrClockSectorScale;
    state.minuteFieldOffset4 = static_cast<int16_t>(
        scaledClock / kStrClockFramesPerMinute);
    state.frameFieldOffset7 = static_cast<uint8_t>(
        scaledClock % kStrClockFramesPerSecond);
    state.secondFieldOffset6 = static_cast<uint8_t>(
        (scaledClock % kStrClockFramesPerMinute) /
        kStrClockFramesPerSecond);
    state.endCheckCalled8001A7F8 = true;
    if (!input.endCheckKnown8001A7F8) {
        state.branch = StrClockPollBranch801C4350::EndCheckUnknown;
        return state;
    }

    state.hasOpenInputGap = false;
    state.returnValueKnown = true;
    state.returnValue = input.endCheckReturn8001A7F8 != 1;
    state.branch = state.returnValue
                       ? StrClockPollBranch801C4350::ContinuePlayback
                       : StrClockPollBranch801C4350::EndPlayback;
    return state;
}

bool TryInitializeStrLowerCdClockRuntime801C4350(
    const StrLowerCdClockSource801C4350& source,
    StrLowerCdClockRuntime801C4350& runtime)
{
    runtime = {};
    if (!source.segmentTimeBaseKnown ||
        !source.segmentEndKnown ||
        !source.segmentEndBiasKnown ||
        !source.discImageDirectoryAuthority) {
        return false;
    }

    const int64_t effectiveEnd =
        static_cast<int64_t>(source.segmentEnd) +
        static_cast<int64_t>(source.segmentEndBias);
    const int64_t relativeSpan =
        effectiveEnd - static_cast<int64_t>(source.segmentTimeBase);
    if (effectiveEnd < INT32_MIN ||
        effectiveEnd > INT32_MAX ||
        relativeSpan < 0 ||
        relativeSpan > INT32_MAX) {
        return false;
    }

    runtime.initialized = true;
    runtime.timeBaseLba = source.segmentTimeBase;
    runtime.endLba = source.segmentEnd;
    runtime.endBias = source.segmentEndBias;
    runtime.effectiveEndLba = static_cast<int32_t>(effectiveEnd);
    runtime.currentLba = source.segmentTimeBase;
    runtime.previousClock = 0;
    runtime.pollCount = 0u;
    runtime.discImageDirectoryAuthority = true;
    runtime.runtimeInputBound = true;
    runtime.psxCdClockAuthority = true;
    return true;
}

StrLowerCdClockTick801C4350 StepStrLowerCdClockRuntime801C4350(
    StrLowerCdClockRuntime801C4350& runtime)
{
    StrLowerCdClockTick801C4350 tick{};
    if (!runtime.initialized ||
        !runtime.discImageDirectoryAuthority ||
        !runtime.runtimeInputBound ||
        !runtime.psxCdClockAuthority ||
        runtime.hostClockAuthority ||
        runtime.replayValueAuthority ||
        runtime.oldWinS0Authority ||
        runtime.stage2PlusAuthority ||
        runtime.comod2Authority) {
        return tick;
    }

    const int64_t advanced =
        static_cast<int64_t>(runtime.currentLba) +
        static_cast<int64_t>(kStrClockSectorScale);
    runtime.currentLba =
        advanced >= runtime.effectiveEndLba
            ? runtime.effectiveEndLba
            : static_cast<int32_t>(advanced);

    const int64_t relativeClock =
        static_cast<int64_t>(runtime.currentLba) -
        static_cast<int64_t>(runtime.timeBaseLba);
    const int64_t deadline =
        static_cast<int64_t>(runtime.currentLba) +
        static_cast<int64_t>(kStrClockDeadlineLead);
    if (relativeClock < 0 ||
        relativeClock > INT32_MAX ||
        deadline < INT32_MIN ||
        deadline > INT32_MAX) {
        return tick;
    }

    tick.currentLba = runtime.currentLba;
    tick.relativeClock = static_cast<int32_t>(relativeClock);
    tick.deadlineLba = static_cast<int32_t>(deadline);
    tick.endCheckReturn =
        runtime.effectiveEndLba <= tick.deadlineLba;

    StrClockPollInput801C4350 input{};
    input.clockDeltaKnown8001A7A4 = true;
    input.clockDelta8001A7A4 = tick.relativeClock;
    input.previousClockKnown801C954C = true;
    input.previousClock801C954C = runtime.previousClock;
    input.endCheckKnown8001A7F8 = true;
    input.endCheckReturn8001A7F8 = tick.endCheckReturn ? 1 : 0;
    input.directDiscSegmentAuthority = true;
    tick.poll = BuildStrClockPollState801C4350(input);
    if (tick.poll.clockFieldsUpdated) {
        runtime.previousClock = tick.poll.updatedPreviousClock;
    }
    ++runtime.pollCount;
    tick.known = tick.poll.executed &&
                 !tick.poll.hasOpenInputGap &&
                 tick.poll.runtimeInputBound &&
                 tick.poll.psxCdClockAuthority;
    tick.polled = tick.poll.clockQueryCalled8001A7A4 &&
                  tick.poll.endCheckCalled8001A7F8;
    tick.terminal =
        tick.poll.returnValueKnown && !tick.poll.returnValue;
    return tick;
}

bool TryInitializeStrWorkBaseRuntime80049428(
    StrWorkBaseRuntime80049428& runtime)
{
    runtime = StrWorkBaseRuntime80049428{};

    // The current SCUS image maps 80049428 to loaded zero bytes. IDA xrefs
    // contain only two reads (8001A280/8001A3B8) and the two writes inside
    // 8001A3C8, so the process-lifetime direct state starts known-zero.
    runtime.initialized = true;
    runtime.dword80049428Known = true;
    runtime.dword80049428 = 0;
    runtime.scusLoadedInitialZeroAuthority = true;
    runtime.currentScusSemanticAuthority = true;
    runtime.currentComod0CallerAuthority = true;
    runtime.directLowerCdStateAuthority = true;
    return true;
}

bool PublishStrLowerCdStartCommand8001A4D0(
    const StrLowerCdStartRuntime8001A4D0& start,
    StrWorkBaseRuntime80049428& runtime)
{
    if (!runtime.initialized ||
        !runtime.dword80049428Known ||
        !runtime.scusLoadedInitialZeroAuthority ||
        !runtime.currentScusSemanticAuthority ||
        !runtime.currentComod0CallerAuthority ||
        !runtime.directLowerCdStateAuthority ||
        runtime.hardwareCallbackAuthority ||
        runtime.hostProjection ||
        runtime.replayValueAuthority ||
        runtime.oldWinS0Authority ||
        runtime.stage2PlusAuthority ||
        runtime.comod2Authority ||
        !start.initialized ||
        !start.primeStatusCalled80036678 ||
        !start.commandWrapper80036678Succeeded ||
        !start.currentScusSemanticAuthority ||
        !start.currentComod0CallerAuthority ||
        !start.discImagePayloadAuthority ||
        !start.directPsxStatusAuthority ||
        start.hardwareCallbackAuthority ||
        start.hostProjection ||
        start.hostExtractedFileAuthority ||
        start.replayValueAuthority ||
        start.oldWinS0Authority ||
        start.stage2PlusAuthority ||
        start.comod2Authority) {
        return false;
    }

    // 8001A4D0 ends with 80036678(1,0). The a2==0 wrapper path reaches
    // 800375BC(1,0,0,1), which writes byte_80057119=1.
    runtime.byte80057119Known = true;
    runtime.byte80057119 = kCdCommandNop;
    return true;
}

bool PublishStrDecoderStreamingStart8001A4D0(
    const StrLowerCdStartRuntime8001A4D0& start,
    StrDecoderMemoryRuntime80027288& runtime)
{
    if (!runtime.initialized ||
        runtime.streamingCdActive8001A4D0 ||
        !runtime.directMemoryAuthority ||
        !runtime.directSectorRingLifecycleAuthority ||
        !runtime.outputCallbackInstalled80047658 ||
        runtime.outputCallbackFunction80027220 != kFn80027220 ||
        runtime.outputCallbackDmaChannel80047658 != 1u ||
        runtime.psxPointerAuthority ||
        runtime.psxHardwareMmioAuthority ||
        runtime.hardwareCallbackTimingAuthority ||
        runtime.hostProjection ||
        runtime.replayValueAuthority ||
        runtime.oldWinS0Authority ||
        runtime.stage2PlusAuthority ||
        runtime.comod2Authority ||
        !start.initialized ||
        !start.setmodeReadSComplete800391AC ||
        !start.streamStartComplete800364D0 ||
        !start.exactCallOrder ||
        !start.currentScusSemanticAuthority ||
        !start.currentComod0CallerAuthority ||
        !start.discImagePayloadAuthority ||
        !start.directPsxStatusAuthority ||
        start.hardwareCallbackAuthority ||
        start.hostProjection ||
        start.hostExtractedFileAuthority ||
        start.replayValueAuthority ||
        start.oldWinS0Authority ||
        start.stage2PlusAuthority ||
        start.comod2Authority) {
        return false;
    }

    // IDA: 8001A4D0 mode 0x01C8 installs 80039318 through 80036930 and
    // 80039240 through 80036528 before ReadS. This direct state records the
    // synchronous command lifecycle only; interrupt timing remains open.
    runtime.streamingCdActive8001A4D0 = true;
    runtime.cdDataCallbackInstalled80036930 = true;
    runtime.cdDataCallbackFunction80039318 = kFn80039318;
    runtime.cdReadyCallbackInstalled80036528 = true;
    runtime.cdReadyCallbackFunction80039240 = kFn80039240;
    runtime.cdStatusByteKnown = start.statusPollInputKnown8001A750;
    runtime.cdStatusByte = start.status0;
    runtime.cdSyncResponseKnown80049414 = false;
    runtime.cdSyncResponse80049414 = {};
    runtime.cdSyncState800573D4 = 0u;
    runtime.cdCommandResponseKnown80088300 = false;
    runtime.cdCommandResponse80088300 = {};
    runtime.directStreamingCdStateAuthority = true;
    // The command latch/response surface at 80057119/80049414/800573D4 is
    // process-global SCUS state rather than decoder-allocation storage. Keep
    // it independent so COMOD0 can issue its second 8001A694 after 80027664
    // releases the four decoder blocks.
    runtime.directCdCommandStateAuthority = true;
    return true;
}

StrSectorRingPrimeResult80039670 PrimeStrSectorRingFromDisc80039670(
    const StrLowerCdStartRuntime8001A4D0& start,
    StrDecoderMemoryRuntime80027288& runtime)
{
    StrSectorRingPrimeResult80039670 result{};
    if (!runtime.initialized ||
        !runtime.streamingCdActive8001A4D0 ||
        !runtime.directMemoryAuthority ||
        !runtime.directSectorRingLifecycleAuthority ||
        !runtime.directStreamingCdStateAuthority ||
        !runtime.sectorRingInitialized8003624C ||
        runtime.sectorRingSlots8003624C !=
            kDecoderSectorRingSlots8003624C ||
        runtime.sectorRingSlotBytes !=
            kStrSectorMetadataBytes80039670 +
                kStrSectorPayloadBytes80039670 ||
        !runtime.allocationPresent[1] ||
        runtime.allocations[1].size() !=
            kDecoderSectorBufferBytes80027288 ||
        runtime.sectorRingFramePublished ||
        runtime.psxPointerAuthority ||
        runtime.psxHardwareMmioAuthority ||
        runtime.hardwareCallbackTimingAuthority ||
        runtime.hostProjection ||
        runtime.replayValueAuthority ||
        runtime.oldWinS0Authority ||
        runtime.stage2PlusAuthority ||
        runtime.comod2Authority ||
        !start.initialized ||
        start.discBinPath.empty() ||
        !start.currentScusSemanticAuthority ||
        !start.currentComod0CallerAuthority ||
        !start.discImageDirectoryAuthority ||
        !start.discImagePayloadAuthority ||
        start.hardwareCallbackAuthority ||
        start.hostProjection ||
        start.hostExtractedFileAuthority ||
        start.replayValueAuthority ||
        start.oldWinS0Authority ||
        start.stage2PlusAuthority ||
        start.comod2Authority) {
        return result;
    }

    std::ifstream disc(start.discBinPath, std::ios::binary);
    if (!disc) {
        return result;
    }

    bool activeFrame = false;
    uint32_t activeFirstSlot = 0u;
    uint16_t activeSectorCount = 0u;
    uint16_t expectedChunk = 0u;
    uint32_t activeFrameNumber = 0u;
    std::array<uint8_t, kRawCdSectorBytes8001A4D0> raw{};

    while (runtime.sectorRingRawSectorCursor <
           start.logicalSectorCount) {
        const uint64_t lba =
            static_cast<uint64_t>(start.startLba) +
            runtime.sectorRingRawSectorCursor;
        const uint64_t rawOffset =
            lba * kRawCdSectorBytes8001A4D0;
        if (rawOffset > static_cast<uint64_t>(
                            (std::numeric_limits<std::streamoff>::max)())) {
            return result;
        }
        disc.seekg(static_cast<std::streamoff>(rawOffset), std::ios::beg);
        disc.read(reinterpret_cast<char*>(raw.data()),
                  static_cast<std::streamsize>(raw.size()));
        if (disc.gcount() != static_cast<std::streamsize>(raw.size())) {
            return result;
        }
        ++runtime.sectorRingRawSectorCursor;
        ++runtime.sectorRingRawSectorsScanned;
        ++result.rawSectorsScanned;

        bool syncMatched = raw[0] == 0u &&
                           raw[kRawCdSyncBytes8001A4D0 - 1u] == 0u;
        for (uint32_t index = 1u;
             syncMatched && index + 1u < kRawCdSyncBytes8001A4D0;
             ++index) {
            syncMatched = raw[index] == 0xFFu;
        }
        const uint8_t* subheader =
            raw.data() + kRawCdSubheaderOffset8001A4D0;
        if (!syncMatched ||
            raw[kRawCdHeaderOffset8001A4D0 + 3u] !=
                kRawCdMode2Byte8001A4D0 ||
            std::memcmp(subheader, subheader + 4u, 4u) != 0 ||
            subheader[0] != start.filterFile ||
            subheader[1] != start.filterChannel ||
            (subheader[2] & (kXaSubmodeRealtime8001A4D0 |
                             kXaSubmodeVideo8001A4D0)) !=
                (kXaSubmodeRealtime8001A4D0 |
                 kXaSubmodeVideo8001A4D0)) {
            continue;
        }

        ++runtime.sectorRingFilteredVideoSectors;
        ++result.filteredVideoSectors;
        const uint8_t* logical = subheader + 8u;
        const uint16_t headerId = ReadLe16(logical + 0u);
        const uint16_t streamWord = ReadLe16(logical + 2u);
        const uint16_t chunk = ReadLe16(logical + 4u);
        const uint16_t sectorCount = ReadLe16(logical + 6u);
        const uint32_t frameNumber = ReadLe32(logical + 8u);
        if (headerId != kStrSectorHeaderId80039670 ||
            ((streamWord >> 10u) & 0x1Fu) != 0u ||
            sectorCount == 0u ||
            sectorCount > kDecoderSectorRingSlots8003624C ||
            chunk >= sectorCount) {
            ++runtime.sectorRingRejectedSectors;
            ++result.rejectedSectors;
            continue;
        }

        if (chunk == 0u) {
            if (activeFrame) {
                ClearSectorSlotStates8003954C(
                    runtime,
                    activeFirstSlot,
                    runtime.sectorRingProducerIndex80085C50 -
                        activeFirstSlot);
                runtime.sectorRingProducerIndex80085C50 =
                    runtime.sectorRingPublishedIndex80085C54;
            }
            const uint32_t tailCapacity =
                kDecoderSectorRingSlots8003624C -
                runtime.sectorRingProducerIndex80085C50 - 1u;
            if (sectorCount > tailCapacity) {
                if (ReadLe16(SectorMetadata80039670(runtime, 0u)) !=
                    kStrSectorStateFree) {
                    ++runtime.sectorRingRejectedSectors;
                    ++result.rejectedSectors;
                    continue;
                }
                WriteLe16(
                    SectorMetadata80039670(
                        runtime,
                        runtime.sectorRingProducerIndex80085C50),
                    kStrSectorStateWrap);
                runtime.sectorRingProducerIndex80085C50 = 0u;
            }
            activeFrame = true;
            activeFirstSlot =
                runtime.sectorRingProducerIndex80085C50;
            activeSectorCount = sectorCount;
            activeFrameNumber = frameNumber;
            expectedChunk = 0u;
        }

        if (!activeFrame || chunk != expectedChunk ||
            sectorCount != activeSectorCount ||
            frameNumber != activeFrameNumber ||
            runtime.sectorRingProducerIndex80085C50 >=
                kDecoderSectorRingSlots8003624C) {
            if (activeFrame) {
                ClearSectorSlotStates8003954C(
                    runtime,
                    activeFirstSlot,
                    runtime.sectorRingProducerIndex80085C50 -
                        activeFirstSlot);
                runtime.sectorRingProducerIndex80085C50 =
                    runtime.sectorRingPublishedIndex80085C54;
                activeFrame = false;
            }
            ++runtime.sectorRingRejectedSectors;
            ++result.rejectedSectors;
            continue;
        }

        const uint32_t slot =
            runtime.sectorRingProducerIndex80085C50;
        uint8_t* metadata = SectorMetadata80039670(runtime, slot);
        std::memcpy(metadata, logical, kStrSectorMetadataBytes80039670);
        if (chunk == 0u) {
            WriteLe16(metadata, kStrSectorStateWrap);
        }
        std::memcpy(SectorPayload80039670(runtime, slot),
                    logical + kStrSectorMetadataBytes80039670,
                    kStrSectorPayloadBytes80039670);
        WriteLe16(metadata, kStrSectorStatePayloadReady);
        ++runtime.sectorRingProducerIndex80085C50;
        ++expectedChunk;

        if (expectedChunk != activeSectorCount) {
            continue;
        }

        uint8_t* publishedMetadata =
            SectorMetadata80039670(runtime, activeFirstSlot);
        WriteLe16(publishedMetadata, kStrSectorStatePublished);
        runtime.sectorRingPublishedIndex80085C54 =
            runtime.sectorRingProducerIndex80085C50;
        runtime.sectorRingQueuedSlots = activeSectorCount;
        runtime.sectorRingPublishedFirstSlot = activeFirstSlot;
        runtime.sectorRingPublishedSectorCount = activeSectorCount;
        runtime.sectorRingPublishedFrameNumber =
            ReadLe32(publishedMetadata + 8u);
        runtime.sectorRingPublishedWidth =
            ReadLe16(publishedMetadata + 0x10u);
        runtime.sectorRingPublishedHeight =
            ReadLe16(publishedMetadata + 0x12u);
        runtime.sectorRingPublishedPayloadByteOffset =
            kStrSectorMetadataAreaBytes80039670 +
            activeFirstSlot * kStrSectorPayloadBytes80039670;
        runtime.sectorRingFramePublished = true;
        runtime.sectorRingFrameAcquired = false;
        ++runtime.sectorRingPublishedFrames;
        runtime.directSectorRingInputAuthority = true;

        result.framePublished = true;
        result.firstSlot = activeFirstSlot;
        result.sectorCount = activeSectorCount;
        result.frameNumber = runtime.sectorRingPublishedFrameNumber;
        result.width = runtime.sectorRingPublishedWidth;
        result.height = runtime.sectorRingPublishedHeight;
        result.payloadByteOffset =
            runtime.sectorRingPublishedPayloadByteOffset;
        break;
    }

    result.known = true;
    result.directDiscSectorAuthority = true;
    result.synchronousCallbackOrdering = true;
    result.hardwareCallbackTimingAuthority = false;
    result.psxPointerAuthority = false;
    result.hostProjection = false;
    result.replayValueAuthority = false;
    result.oldWinS0Authority = false;
    result.stage2PlusAuthority = false;
    result.comod2Authority = false;
    return result;
}

StrSectorRingAcquireResult8003958C AcquireStrSectorRingFrame8003958C(
    StrDecoderMemoryRuntime80027288& runtime)
{
    StrSectorRingAcquireResult8003958C result{};
    if (!runtime.initialized ||
        !runtime.directMemoryAuthority ||
        !runtime.directSectorRingLifecycleAuthority ||
        !runtime.directSectorRingInputAuthority ||
        !runtime.allocationPresent[1] ||
        runtime.allocations[1].size() !=
            kDecoderSectorBufferBytes80027288 ||
        runtime.sectorRingReadIndex80085C58 >=
            kDecoderSectorRingSlots8003624C ||
        runtime.psxPointerAuthority ||
        runtime.hostProjection ||
        runtime.replayValueAuthority ||
        runtime.oldWinS0Authority ||
        runtime.stage2PlusAuthority ||
        runtime.comod2Authority) {
        return result;
    }

    result.known = true;
    result.directSectorRingAuthority = true;
    uint32_t slot = runtime.sectorRingReadIndex80085C58;
    uint8_t* metadata = SectorMetadata80039670(runtime, slot);
    if (ReadLe16(metadata) == kStrSectorStateWrap) {
        runtime.sectorRingReadIndex80085C58 = 0u;
        slot = 0u;
        metadata = SectorMetadata80039670(runtime, slot);
    }
    if (ReadLe16(metadata) != kStrSectorStatePublished) {
        return result;
    }

    WriteLe16(metadata, kStrSectorStateAcquired);
    runtime.sectorRingFrameAcquired = true;
    result.returnValue = 0;
    result.available = true;
    result.payloadHandleKnown = true;
    result.payloadHandle =
        kStrSectorMetadataAreaBytes80039670 +
        slot * kStrSectorPayloadBytes80039670;
    result.sectorHeaderKnown = true;
    result.width = ReadLe16(metadata + 0x10u);
    result.height = ReadLe16(metadata + 0x12u);
    result.firstSlot = slot;
    result.sectorCount = ReadLe16(metadata + 6u);
    result.frameNumber = ReadLe32(metadata + 8u);
    result.stateTransition2To4 = true;
    return result;
}

StrSectorRingReleaseResult80039490 ReleaseStrSectorRingFrame80039490(
    uint32_t payloadHandle,
    StrDecoderMemoryRuntime80027288& runtime)
{
    StrSectorRingReleaseResult80039490 result{};
    if (!runtime.initialized ||
        !runtime.directMemoryAuthority ||
        !runtime.directSectorRingLifecycleAuthority ||
        !runtime.directSectorRingInputAuthority ||
        !runtime.allocationPresent[1] ||
        runtime.allocations[1].size() !=
            kDecoderSectorBufferBytes80027288 ||
        runtime.psxPointerAuthority ||
        runtime.hostProjection ||
        runtime.replayValueAuthority ||
        runtime.oldWinS0Authority ||
        runtime.stage2PlusAuthority ||
        runtime.comod2Authority) {
        return result;
    }

    result.known = true;
    result.directSectorRingAuthority = true;
    if (payloadHandle < kStrSectorMetadataAreaBytes80039670) {
        return result;
    }
    const uint32_t payloadDelta =
        payloadHandle - kStrSectorMetadataAreaBytes80039670;
    if (payloadDelta % kStrSectorPayloadBytes80039670 != 0u) {
        return result;
    }
    const uint32_t firstSlot =
        payloadDelta / kStrSectorPayloadBytes80039670;
    if (firstSlot >= kDecoderSectorRingSlots8003624C) {
        return result;
    }
    uint8_t* metadata = SectorMetadata80039670(runtime, firstSlot);
    const uint16_t sectorCount = ReadLe16(metadata + 6u);
    if (ReadLe16(metadata) != kStrSectorStateAcquired ||
        sectorCount == 0u ||
        firstSlot + sectorCount >
            kDecoderSectorRingSlots8003624C) {
        return result;
    }

    ClearSectorSlotStates8003954C(runtime, firstSlot, sectorCount);
    runtime.sectorRingReadIndex80085C58 = firstSlot + sectorCount;
    runtime.sectorRingQueuedSlots =
        runtime.sectorRingQueuedSlots >= sectorCount
            ? runtime.sectorRingQueuedSlots - sectorCount
            : 0u;
    runtime.sectorRingFrameAcquired = false;
    if (firstSlot == runtime.sectorRingPublishedFirstSlot) {
        runtime.sectorRingFramePublished = false;
    }
    result.returnValue = 0;
    result.released = true;
    result.firstSlot = firstSlot;
    result.sectorCount = sectorCount;
    result.nextReadIndex = runtime.sectorRingReadIndex80085C58;
    result.stateTransition4To0 = true;
    return result;
}

StrVlcDecodeReleaseResult800273A4 ExecuteStrVlcDecodeAndRelease800273A4(
    uint32_t payloadHandle,
    StrDecoderMemoryRuntime80027288& runtime)
{
    StrVlcDecodeReleaseResult800273A4 result{};
    result.payloadHandle = payloadHandle;
    if (!runtime.initialized ||
        !runtime.directMemoryAuthority ||
        !runtime.directSectorRingLifecycleAuthority ||
        !runtime.directSectorRingInputAuthority ||
        !runtime.allocationPresent[1] ||
        !runtime.allocationPresent[2] ||
        runtime.allocations[1].size() !=
            kDecoderSectorBufferBytes80027288 ||
        payloadHandle < kStrSectorMetadataAreaBytes80039670 ||
        runtime.psxPointerAuthority ||
        runtime.hostProjection ||
        runtime.replayValueAuthority ||
        runtime.oldWinS0Authority ||
        runtime.stage2PlusAuthority ||
        runtime.comod2Authority) {
        return result;
    }

    const uint32_t payloadDelta =
        payloadHandle - kStrSectorMetadataAreaBytes80039670;
    if (payloadDelta % kStrSectorPayloadBytes80039670 != 0u) {
        return result;
    }
    const uint32_t firstSlot =
        payloadDelta / kStrSectorPayloadBytes80039670;
    if (firstSlot >= kDecoderSectorRingSlots8003624C) {
        return result;
    }
    const uint8_t* metadata = SectorMetadata80039670(runtime, firstSlot);
    const uint16_t sectorCount = ReadLe16(metadata + 6u);
    if (ReadLe16(metadata) != kStrSectorStateAcquired ||
        sectorCount == 0u ||
        firstSlot + sectorCount > kDecoderSectorRingSlots8003624C) {
        return result;
    }
    const uint32_t inputBytes =
        static_cast<uint32_t>(sectorCount) *
        kStrSectorPayloadBytes80039670;
    if (payloadHandle + inputBytes > runtime.allocations[1].size()) {
        return result;
    }

    result.inputBytes = inputBytes;
    result.vlc =
        PrSS0MdecVlcDirect::ExecuteDecDCTvlc2ResetSize80047B30(
            runtime.allocations[1].data() + payloadHandle,
            inputBytes,
            runtime.allocations[2].data(),
            runtime.allocations[2].size());
    if (!result.vlc.known || !result.vlc.executed) {
        return result;
    }

    result.releaseFrameCalled80039490 = true;
    result.release = ReleaseStrSectorRingFrame80039490(
        payloadHandle,
        runtime);
    if (!result.release.known || !result.release.released ||
        result.release.returnValue != 0) {
        return result;
    }

    result.known = true;
    result.executed = true;
    result.exactCurrentIdaCallOrder = true;
    result.currentScusSemanticAuthority = true;
    result.directSectorRingAuthority = true;
    result.mdecHardwareExecutionAuthority = false;
    return result;
}

StrMdecCommandOutputSubmission800273A4
ExecuteStrMdecCommandOutputSubmission800273A4(
    const StrDecoderControlState80027288& control,
    const StrVlcDecodeReleaseResult800273A4& vlcDecodeRelease,
    StrDecoderMemoryRuntime80027288& runtime)
{
    StrMdecCommandOutputSubmission800273A4 result{};
    if (!control.known || control.decodedWidth == 0u ||
        control.decodedHeight == 0u ||
        !vlcDecodeRelease.known || !vlcDecodeRelease.executed ||
        !vlcDecodeRelease.vlc.known ||
        !vlcDecodeRelease.vlc.executed ||
        vlcDecodeRelease.vlc.returnValue != 0 ||
        vlcDecodeRelease.vlc.outputBytesWritten < sizeof(uint32_t) ||
        !vlcDecodeRelease.releaseFrameCalled80039490 ||
        !vlcDecodeRelease.release.known ||
        !vlcDecodeRelease.release.released ||
        !vlcDecodeRelease.exactCurrentIdaCallOrder ||
        !vlcDecodeRelease.currentScusSemanticAuthority ||
        !runtime.initialized || !runtime.directMemoryAuthority ||
        !runtime.outputCallbackInstalled80047658 ||
        runtime.outputCallbackFunction80027220 != kFn80027220 ||
        runtime.outputCallbackDmaChannel80047658 != 1u ||
        !runtime.allocationPresent[2] ||
        !runtime.allocationPresent[3] ||
        runtime.allocations[2].size() < sizeof(uint32_t) ||
        vlcDecodeRelease.vlc.outputBytesWritten >
            runtime.allocations[2].size() ||
        runtime.psxPointerAuthority ||
        runtime.psxHardwareMmioAuthority ||
        runtime.hardwareCallbackTimingAuthority ||
        runtime.hostProjection || runtime.replayValueAuthority ||
        runtime.oldWinS0Authority || runtime.stage2PlusAuthority ||
        runtime.comod2Authority) {
        return result;
    }

    const uint32_t commandBefore =
        ReadLe32(runtime.allocations[2].data());
    if (static_cast<uint16_t>(commandBefore) !=
        vlcDecodeRelease.vlc.frameCodeCount) {
        return result;
    }

    // Current SCUS 80047558(a0, 2): bit 0 is clear, so bit 27 is set;
    // bit 1 is set, so bit 25 is set. 80047778 then submits a0+4 to
    // MDEC input DMA using the low halfword as the transfer word count.
    static constexpr uint32_t kControlArgument80047558 = 2u;
    static constexpr uint32_t kInputModeBits80047558 = 0x0A000000u;
    static constexpr uint32_t kDmaPriorityOrMask = 0x88u;
    static constexpr uint32_t kInputChcr80047778 = 0x01000201u;
    static constexpr uint32_t kOutputChcr8004780C = 0x01000200u;
    const uint32_t commandAfter =
        commandBefore | kInputModeBits80047558;
    const uint16_t inputWordCount =
        static_cast<uint16_t>(commandAfter);
    const uint64_t inputEnd = sizeof(uint32_t) +
        static_cast<uint64_t>(inputWordCount) * sizeof(uint32_t);
    if (inputWordCount == 0u ||
        inputEnd > vlcDecodeRelease.vlc.outputBytesWritten ||
        inputEnd > runtime.allocations[2].size()) {
        return result;
    }

    const uint64_t macroblockRows =
        (static_cast<uint64_t>(control.decodedHeight) + 15u) >> 4u;
    const uint64_t outputWords64 =
        (16u * static_cast<uint64_t>(control.decodedWidth) *
         macroblockRows) >> 1u;
    if (outputWords64 == 0u ||
        outputWords64 > (std::numeric_limits<uint32_t>::max)() ||
        (outputWords64 & 31u) != 0u ||
        outputWords64 * sizeof(uint32_t) >
            runtime.allocations[3].size()) {
        return result;
    }
    const uint32_t outputWordCount =
        static_cast<uint32_t>(outputWords64);

    // This is a direct CPU-side command/DMA-state translation. It records
    // the exact register values but deliberately performs no PSX MMIO and
    // produces no pixels; DMA completion and callback timing remain open.
    WriteLe32(runtime.allocations[2].data(), commandAfter);
    result.controlArgument80047558 = kControlArgument80047558;
    result.vlcCommandWordBefore = commandBefore;
    result.vlcCommandWordAfter = commandAfter;
    result.inputDmaWordCount80047778 = inputWordCount;
    result.inputSyncCalled8004789C = true;
    result.dmaPriorityOrMask = kDmaPriorityOrMask;
    result.inputMadrByteOffset = sizeof(uint32_t);
    result.inputBcr =
        (static_cast<uint32_t>(inputWordCount >> 5u) << 16u) | 0x20u;
    result.inputChcr = kInputChcr80047778;
    result.mdecCommandPortWritten = true;
    result.outputSyncCalled80047934 = true;
    result.outputDmaWordCount8004780C = outputWordCount;
    result.outputMadrByteOffset = 0u;
    result.outputBcr = (outputWordCount >> 5u << 16u) | 0x20u;
    result.outputChcr = kOutputChcr8004780C;
    result.outputReadyStored800273A4 = true;
    result.dmaCompletionCallbackPending80027220 = true;
    result.outputBufferUntouched = true;
    result.exactCurrentIdaCallOrder = true;
    result.currentScusSemanticAuthority = true;
    result.directCommandStateAuthority = true;
    result.mdecHardwareExecutionAuthority = false;
    result.hardwareCallbackTimingAuthority = false;
    result.psxPointerAuthority = false;
    result.psxHardwareMmioAuthority = false;
    result.hostProjection = false;
    result.replayValueAuthority = false;
    result.oldWinS0Authority = false;
    result.stage2PlusAuthority = false;
    result.comod2Authority = false;
    result.known = true;
    result.executed = true;
    return result;
}

StrMdecDmaCompletionState80027220 ExecuteStrMdecDmaCompletion80027220(
    const StrDecoderControlState80027288& control,
    const StrMdecCommandOutputSubmission800273A4& submission,
    const PrSS0MdecOutputDirect::Mdec15bppResult& output,
    const StrDecoderMemoryRuntime80027288& runtime)
{
    StrMdecDmaCompletionState80027220 result{};
    const uint64_t submittedOutputBytes =
        static_cast<uint64_t>(submission.outputDmaWordCount8004780C) *
        sizeof(uint32_t);
    if (!control.known || control.outputReady != 1u ||
        !submission.known || !submission.executed ||
        !submission.outputReadyStored800273A4 ||
        !submission.dmaCompletionCallbackPending80027220 ||
        !submission.exactCurrentIdaCallOrder ||
        !submission.currentScusSemanticAuthority ||
        !submission.directCommandStateAuthority ||
        submission.mdecHardwareExecutionAuthority ||
        submission.hardwareCallbackTimingAuthority ||
        submission.psxPointerAuthority ||
        submission.psxHardwareMmioAuthority ||
        submission.hostProjection || submission.replayValueAuthority ||
        submission.oldWinS0Authority || submission.stage2PlusAuthority ||
        submission.comod2Authority ||
        !output.known || !output.executed ||
        !output.currentScusQuantTableAuthority ||
        !output.currentScusScaleTableAuthority ||
        !output.currentIdaCommandAuthority ||
        !output.documentedPsxMdecAlgorithmAuthority ||
        output.mdecHardwareBitExactAuthority ||
        output.dmaCompletionCallbackTimingAuthority ||
        output.psxHardwareMmioAuthority || output.hostProjection ||
        output.replayValueAuthority || output.oldWinS0Authority ||
        output.stage2PlusAuthority || output.comod2Authority ||
        submittedOutputBytes == 0u ||
        submittedOutputBytes != output.outputBytesWritten ||
        !runtime.initialized || !runtime.directMemoryAuthority ||
        !runtime.outputCallbackInstalled80047658 ||
        runtime.outputCallbackFunction80027220 != kFn80027220 ||
        runtime.outputCallbackDmaChannel80047658 != 1u ||
        !runtime.allocationPresent[3] ||
        output.outputBytesWritten > runtime.allocations[3].size() ||
        runtime.hardwareCallbackTimingAuthority ||
        runtime.psxPointerAuthority || runtime.psxHardwareMmioAuthority ||
        runtime.hostProjection || runtime.replayValueAuthority ||
        runtime.oldWinS0Authority || runtime.stage2PlusAuthority ||
        runtime.comod2Authority) {
        return result;
    }

    result.controlBefore = control;
    result.controlAfter = control;
    // Current SCUS 80027220 performs exactly one state mutation: it stores
    // zero to decoderControl+0x20. Host output completion is synchronous here,
    // but no PSX DMA interrupt timing is inferred from that scheduling choice.
    result.controlAfter.outputReady = 0u;
    result.dmaChannelOneCallbackInstalled = true;
    result.callbackFunction80027220 = kFn80027220;
    result.callbackDmaChannel = 1u;
    result.completedOutputValidated = true;
    result.completedOutputBytes = output.outputBytesWritten;
    result.outputReadyCleared = true;
    result.decodeInFlightPreserved =
        result.controlAfter.decodeInFlight == control.decodeInFlight;
    result.exactCurrentIdaStoreAuthority = true;
    result.directSynchronousOutputCompletionAuthority = true;
    result.hardwareCallbackTimingAuthority = false;
    result.psxPointerAuthority = false;
    result.psxHardwareMmioAuthority = false;
    result.hostProjection = false;
    result.replayValueAuthority = false;
    result.oldWinS0Authority = false;
    result.stage2PlusAuthority = false;
    result.comod2Authority = false;
    result.known = true;
    result.executed = true;
    return result;
}

StrCleanupResult8001A280 ApplyStrCleanup8001A280(
    StrWorkBaseRuntime80049428& runtime)
{
    StrCleanupResult8001A280 result{};
    result.called = true;
    ++runtime.cleanupCallCount8001A280;
    result.dword80049428Known = runtime.dword80049428Known;
    result.dword80049428 = runtime.dword80049428;

    if (!runtime.initialized ||
        !runtime.dword80049428Known ||
        !runtime.scusLoadedInitialZeroAuthority ||
        !runtime.currentScusSemanticAuthority ||
        !runtime.currentComod0CallerAuthority ||
        !runtime.directLowerCdStateAuthority ||
        runtime.hardwareCallbackAuthority ||
        runtime.hostProjection ||
        runtime.replayValueAuthority ||
        runtime.oldWinS0Authority ||
        runtime.stage2PlusAuthority ||
        runtime.comod2Authority) {
        return result;
    }

    if (runtime.dword80049428 != 0) {
        result.skippedNonZeroWorkBase = true;
        result.directCommandSinkAuthority = true;
        result.known = true;
        return result;
    }

    // IDA: 8001A280 -> 80036678(0x10,0). With a2==0 the wrapper calls
    // 800375BC(0x10,0,0,1), whose exact synchronous side effect is the
    // byte_80057119 command latch write. Callback timing is not claimed.
    result.commandWrapperCalled80036678 = true;
    result.command80036678 = kCdCommandGetlocL;
    result.commandWrapperSucceeded80036678 = true;
    result.commandSinkCalled800375BC = true;
    result.commandSinkSkipWait800375BC = true;
    result.byte80057119Written = true;
    result.byte80057119 = kCdCommandGetlocL;
    result.directCommandSinkAuthority = true;
    runtime.byte80057119Known = true;
    runtime.byte80057119 = kCdCommandGetlocL;
    result.known = true;
    return result;
}

StrWorkBaseReadResult8001A3B8 ReadStrWorkBase8001A3B8(
    const StrWorkBaseRuntime80049428& runtime)
{
    StrWorkBaseReadResult8001A3B8 result{};
    result.called = true;
    result.valueKnown = runtime.dword80049428Known;
    result.value = runtime.dword80049428;
    if (!runtime.initialized ||
        !runtime.dword80049428Known ||
        !runtime.scusLoadedInitialZeroAuthority ||
        !runtime.currentScusSemanticAuthority ||
        !runtime.currentComod0CallerAuthority ||
        !runtime.directLowerCdStateAuthority ||
        runtime.hardwareCallbackAuthority ||
        runtime.hostProjection ||
        runtime.replayValueAuthority ||
        runtime.oldWinS0Authority ||
        runtime.stage2PlusAuthority ||
        runtime.comod2Authority) {
        return result;
    }

    // Current SCUS 8001A3B8 is exactly a load from dword_80049428 and return.
    // It performs no writes and makes no calls.
    result.returnsOne = result.value == 1;
    result.readOnly = true;
    result.currentScusSemanticAuthority = true;
    result.currentComod0CallerAuthority = true;
    result.directLowerCdStateAuthority = true;
    result.known = true;
    return result;
}

StrCdSyncCallbackRuntime800570F8
InitializeStrCdSyncCallbackRuntime800570F8()
{
    StrCdSyncCallbackRuntime800570F8 runtime{};
    runtime.initialized = true;
    runtime.callbackKnown = true;
    runtime.callback = 0u;
    runtime.scusLoadedInitialZeroAuthority = true;
    runtime.currentScusSemanticAuthority = true;
    runtime.directCallbackStateAuthority = true;
    return runtime;
}

StrCdSyncCallbackSwapResult80036510 SetStrCdSyncCallback80036510(
    StrCdSyncCallbackRuntime800570F8& runtime,
    uint32_t callback)
{
    StrCdSyncCallbackSwapResult80036510 result{};
    result.called = true;
    result.requestedCallback = callback;
    result.priorCallbackKnown = runtime.callbackKnown;
    result.priorCallback = runtime.callback;
    if (!runtime.initialized || !runtime.callbackKnown ||
        !runtime.scusLoadedInitialZeroAuthority ||
        !runtime.currentScusSemanticAuthority ||
        !runtime.directCallbackStateAuthority ||
        runtime.hardwareCallbackTimingAuthority || runtime.hostProjection ||
        runtime.replayValueAuthority || runtime.oldWinS0Authority ||
        runtime.stage2PlusAuthority || runtime.comod2Authority) {
        return result;
    }

    // Current SCUS 80036510 reads dword_800570F8, stores a0 back to the same
    // slot, and returns the prior word. This is direct software state only;
    // it does not claim callback dispatch or PSX interrupt timing.
    runtime.callback = callback;
    result.callbackWritten = true;
    result.readBeforeWrite = true;
    result.currentScusSemanticAuthority = true;
    result.directCallbackStateAuthority = true;
    result.known = true;
    return result;
}

StrCommand1StatusResult80036678 ExecuteStrCommand1Status80036678(
    StrDecoderMemoryRuntime80027288& decoder,
    StrCdSyncCallbackRuntime800570F8& callback,
    StrWorkBaseRuntime80049428& workBase)
{
    StrCommand1StatusResult80036678 result{};
    result.called = true;
    if (!decoder.directCdCommandStateAuthority || !decoder.cdStatusByteKnown ||
        decoder.psxPointerAuthority || decoder.psxHardwareMmioAuthority ||
        decoder.hardwareCallbackTimingAuthority || decoder.hostProjection ||
        decoder.replayValueAuthority || decoder.oldWinS0Authority ||
        decoder.stage2PlusAuthority || decoder.comod2Authority ||
        !workBase.initialized || !workBase.byte80057119Known ||
        !workBase.currentScusSemanticAuthority ||
        !workBase.directLowerCdStateAuthority || workBase.hardwareCallbackAuthority ||
        workBase.hostProjection || workBase.replayValueAuthority ||
        workBase.oldWinS0Authority || workBase.stage2PlusAuthority || workBase.comod2Authority) {
        return result;
    }
    // Live SCUS: all command-1 parameter/response-table flags are zero.
    // 80036678 clears 800570F8 (does NOT restore it), then 800375BC(1,0,0,1)
    // clears sync state and writes the command latch. No Setloc or data read.
    result.clearCallback = SetStrCdSyncCallback80036510(callback, 0u);
    if (!result.clearCallback.known) {
        return result;
    }
    decoder.cdSyncState800573D4 = 0u;
    workBase.byte80057119 = 1u;
    result.commandLatchWritten80057119 = true;
    // Synchronous software-CD receipt, using CURRENT drive state rather than
    // the startup snapshot. Nop cannot start a stopped stream. The original
    // command response bank 80088300 is distinct from the caller's 80049414.
    result.status = static_cast<uint8_t>(
        (decoder.cdStatusByte & ~0x20u) |
        (decoder.streamingCdActive8001A4D0 ? 0x20u : 0u));
    decoder.cdStatusByte = result.status;
    decoder.cdCommandResponse80088300 = {};
    decoder.cdCommandResponse80088300[0] = result.status;
    decoder.cdCommandResponseKnown80088300 = true;
    decoder.cdSyncState800573D4 = 2u;
    result.responseCommitted80088300 = true;
    result.returnValue = 1;
    result.known = true;
    // No MMIO, hardware interrupt timing or host audio lifecycle is claimed.
    return result;
}

StrCommand8PauseResult800367A4 ExecuteStrCommand8Pause800367A4(
    StrDecoderMemoryRuntime80027288& decoder,
    StrCdSyncCallbackRuntime800570F8& callback,
    StrWorkBaseRuntime80049428& workBase)
{
    StrCommand8PauseResult800367A4 result{};
    result.called = true;
    if (!decoder.directCdCommandStateAuthority ||
        !decoder.cdStatusByteKnown || decoder.psxPointerAuthority ||
        decoder.psxHardwareMmioAuthority ||
        decoder.hardwareCallbackTimingAuthority || decoder.hostProjection ||
        decoder.replayValueAuthority || decoder.oldWinS0Authority ||
        decoder.stage2PlusAuthority || decoder.comod2Authority ||
        !callback.initialized || !callback.callbackKnown ||
        !callback.currentScusSemanticAuthority ||
        !callback.directCallbackStateAuthority ||
        callback.hardwareCallbackTimingAuthority || callback.hostProjection ||
        callback.replayValueAuthority || callback.oldWinS0Authority ||
        callback.stage2PlusAuthority || callback.comod2Authority ||
        !workBase.initialized || !workBase.byte80057119Known ||
        !workBase.currentScusSemanticAuthority ||
        !workBase.directLowerCdStateAuthority || workBase.hostProjection ||
        workBase.replayValueAuthority || workBase.oldWinS0Authority ||
        workBase.stage2PlusAuthority || workBase.comod2Authority) {
        return result;
    }

    // Current-IDA command-8 table entries are all zero. 800367A4 saves and
    // clears the sync callback, restores it before 800375BC, then accepts only
    // the 80037070 sync-state 2 result. The direct adapter is synchronous;
    // callback dispatch, MMIO, retry timing, and interrupt timing are not
    // claimed.
    result.commandParameterTableSlotZero = true;
    result.commandResponseTableSlotZero = true;
    result.commandCallbackTableSlotZero = true;
    const uint32_t savedCallback = callback.callback;
    result.clearCallbackBeforeCommand =
        SetStrCdSyncCallback80036510(callback, 0u);
    if (!result.clearCallbackBeforeCommand.known) {
        return result;
    }
    result.restoreCallbackBeforeSend =
        SetStrCdSyncCallback80036510(callback, savedCallback);
    if (!result.restoreCallbackBeforeSend.known) {
        return result;
    }
    workBase.byte80057119 = 8u;
    result.commandSinkCalled800375BC = true;
    result.commandLatchWritten80057119 = true;
    result.syncWaitCalled80037070 = true;
    result.syncState800573D4 = 2u;
    result.response80049414 = {};
    result.response80049414[0] = decoder.cdStatusByte;
    decoder.cdSyncResponse80049414 = result.response80049414;
    decoder.cdSyncResponseKnown80049414 = true;
    decoder.cdSyncState800573D4 = result.syncState800573D4;
    result.responseWritten80049414 = true;
    decoder.streamingCdActive8001A4D0 = false;
    result.pauseStreamingStateCommitted = true;
    result.attemptCount = 1u;
    result.returnValue = 1;
    result.exactSoftwareCallOrder = true;
    result.currentScusSemanticAuthority = true;
    result.directLowerCdStateAuthority = true;
    result.directCdCommandStateAuthority = true;
    result.known = true;
    return result;
}

StrWaitCleanupResult8001A694 ExecuteStrWaitCleanup8001A694(
    StrDecoderMemoryRuntime80027288& decoder,
    StrCdSyncCallbackRuntime800570F8& callback,
    StrWorkBaseRuntime80049428& workBase)
{
    StrWaitCleanupResult8001A694 result{};
    result.called = true;
    result.command8Pause =
        ExecuteStrCommand8Pause800367A4(decoder, callback, workBase);
    if (!result.command8Pause.known ||
        result.command8Pause.returnValue != 1) {
        return result;
    }
    result.clearSyncCallback =
        SetStrCdSyncCallback80036510(callback, 0u);
    if (!result.clearSyncCallback.known || callback.callback != 0u) {
        return result;
    }
    result.exactCallOrder = true;
    result.executionAccepted = true;
    result.currentScusSemanticAuthority = true;
    result.directLowerCdStateAuthority = true;
    result.directCdCommandStateAuthority = true;
    result.known = true;
    return result;
}

StrSerialVolumeResult8001A4A4 ExecuteStrSerialVolume8001A4A4(
    int32_t argument)
{
    StrSerialVolumeResult8001A4A4 result{};
    result.called = true;
    result.argument = argument;

    // Current SCUS 8001A4A4 maps exactly argument==1 to zero and every
    // other value to 0x7F, then calls 8001A478. 8001A478 sign-extends that
    // value into both volume arguments of 8002AB24 with serial channel zero.
    result.argumentComparedWithOne = true;
    result.mappedVolume =
        argument == 1 ? 0u : kSerialVolumeMaximum8002AB24;
    result.wrapperCalled8001A478 = true;
    result.serialChannel8002AB24 = 0u;
    result.leftVolume8002AB24 =
        static_cast<int16_t>(result.mappedVolume);
    result.rightVolume8002AB24 =
        static_cast<int16_t>(result.mappedVolume);

    // On channel zero, 8002AB24 builds attribute mask 0x00C0, clamps each
    // signed input below 0x80, scales it by 258, and calls 8002A6FC. The
    // software command is direct; the lower SPU hardware commit remains a
    // separate authority boundary.
    result.attributeMask8002AB24 = kSerialVolumeAttributeMask8002AB24;
    result.scaledLeftVolume8002AB24 = static_cast<uint16_t>(
        static_cast<uint16_t>(result.mappedVolume) *
        kSerialVolumeScale8002AB24);
    result.scaledRightVolume8002AB24 = result.scaledLeftVolume8002AB24;
    result.serialAttributeCommandBuilt8002AB24 = true;
    result.lowerCommonAttributeCalled8002A6FC = true;
    result.exactSoftwareCallOrder = true;
    result.executionAccepted = true;
    result.currentScusSemanticAuthority = true;
    result.currentComod0CallerAuthority = true;
    result.directAudioCommandStateAuthority = true;
    result.known = true;
    return result;
}

StrNullSubResult80024CF0 ExecuteStrNullSub80024CF0(
    uint32_t contextAddress)
{
    StrNullSubResult80024CF0 result{};
    result.called = true;
    result.contextAddress = contextAddress;
    // Current SCUS 80024CF0 is exactly `jr ra` followed by a nop delay slot.
    result.returnInstructionObserved = true;
    result.delaySlotNopObserved = true;
    result.contextUnmodified = true;
    result.exactNoOp = true;
    result.executionAccepted = true;
    result.currentScusSemanticAuthority = true;
    result.currentComod0CallerAuthority = true;
    result.directNoOpAuthority = true;
    result.known = true;
    return result;
}

StrCleanupTailPrefixResult801C455C ExecuteStrCleanupTailPrefix801C455C(
    StrDecoderMemoryRuntime80027288& decoder,
    StrCdSyncCallbackRuntime800570F8& callback,
    StrWorkBaseRuntime80049428& workBase,
    uint32_t contextAddress)
{
    StrCleanupTailPrefixResult801C455C result{};
    result.called = true;
    if (decoder.initialized || decoder.allocationStackDepth != 0u ||
        decoder.directMemoryAuthority ||
        decoder.directSectorRingLifecycleAuthority ||
        decoder.directStreamingCdStateAuthority ||
        !decoder.directCdCommandStateAuthority ||
        !decoder.cdromReg0Known || decoder.cdromReg0 != 0u ||
        !decoder.cdromReg3Known || decoder.cdromReg3 != 0u) {
        return result;
    }
    result.stopReset80027664Observed = true;
    result.muteSerialVolume = ExecuteStrSerialVolume8001A4A4(1);
    if (!result.muteSerialVolume.known ||
        !result.muteSerialVolume.executionAccepted ||
        !result.muteSerialVolume.currentScusSemanticAuthority ||
        !result.muteSerialVolume.currentComod0CallerAuthority ||
        !result.muteSerialVolume.directAudioCommandStateAuthority ||
        result.muteSerialVolume.psxSpuHardwareAuthority ||
        result.muteSerialVolume.hostProjection ||
        result.muteSerialVolume.replayValueAuthority ||
        result.muteSerialVolume.oldWinS0Authority ||
        result.muteSerialVolume.stage2PlusAuthority ||
        result.muteSerialVolume.comod2Authority) {
        return result;
    }

    result.secondWaitCleanup =
        ExecuteStrWaitCleanup8001A694(decoder, callback, workBase);
    if (!result.secondWaitCleanup.known ||
        !result.secondWaitCleanup.executionAccepted ||
        !result.secondWaitCleanup.currentScusSemanticAuthority ||
        !result.secondWaitCleanup.directLowerCdStateAuthority ||
        !result.secondWaitCleanup.directCdCommandStateAuthority ||
        result.secondWaitCleanup.hardwareCallbackTimingAuthority ||
        result.secondWaitCleanup.hostProjection ||
        result.secondWaitCleanup.replayValueAuthority ||
        result.secondWaitCleanup.oldWinS0Authority ||
        result.secondWaitCleanup.stage2PlusAuthority ||
        result.secondWaitCleanup.comod2Authority) {
        return result;
    }

    result.contextNoOp = ExecuteStrNullSub80024CF0(contextAddress);
    if (!result.contextNoOp.known ||
        !result.contextNoOp.executionAccepted ||
        !result.contextNoOp.exactNoOp ||
        !result.contextNoOp.currentScusSemanticAuthority ||
        !result.contextNoOp.currentComod0CallerAuthority ||
        !result.contextNoOp.directNoOpAuthority ||
        result.contextNoOp.hostProjection ||
        result.contextNoOp.replayValueAuthority ||
        result.contextNoOp.oldWinS0Authority ||
        result.contextNoOp.stage2PlusAuthority ||
        result.contextNoOp.comod2Authority) {
        return result;
    }

    // COMOD0 calls 8001B120(1) next. This prefix deliberately reports that
    // successor without executing it; the caller owns the separately gated
    // direct VRAM move and GPU-packet transaction. No host framebuffer or
    // hardware-MMIO projection is promoted here.
    result.nextDisplayMovePending8001B120 = true;
    result.exactCallOrder = true;
    result.executionAccepted = true;
    result.currentScusSemanticAuthority = true;
    result.currentComod0CallerAuthority = true;
    result.directSoftwareStateAuthority = true;
    result.known = true;
    return result;
}

StrVramPageRuntime8001B120 InitializeStrVramPageRuntime8001B120()
{
    StrVramPageRuntime8001B120 runtime{};
    runtime.initialized = true;
    runtime.currentScusSemanticAuthority = true;
    runtime.directVramPixelStateAuthority = true;
    return runtime;
}

StrVramClearResult8001B1B0 ClearStrVramPages8001B1B0(
    StrVramPageRuntime8001B120& runtime,
    int32_t red,
    int32_t green,
    int32_t blue)
{
    StrVramClearResult8001B1B0 result{};
    result.red = red;
    result.green = green;
    result.blue = blue;
    result.width = runtime.width;
    result.height = static_cast<uint16_t>(
        runtime.height * static_cast<uint16_t>(runtime.pages.size()));
    if (!runtime.initialized ||
        !runtime.currentScusSemanticAuthority ||
        !runtime.directVramPixelStateAuthority ||
        red != 0 || green != 0 || blue != 0 ||
        runtime.width != kStrVramPageWidth8001B120 ||
        runtime.height != kStrVramPageHeight8001B120 ||
        runtime.pages.size() != 2u ||
        runtime.psxGpuMmioAuthority ||
        runtime.hardwareCompletionTimingAuthority ||
        runtime.hostProjection || runtime.replayValueAuthority ||
        runtime.oldWinS0Authority || runtime.stage2PlusAuthority ||
        runtime.comod2Authority) {
        return result;
    }

    const std::size_t pixelCount =
        static_cast<std::size_t>(runtime.width) * runtime.height;
    for (StrVramPage8001B120& page : runtime.pages) {
        page.contentKnown = true;
        page.uploadCount8002756C = 0u;
        page.moveWriteCount8001B120 = 0u;
        page.rgbaPixels.assign(pixelCount, 0u);
        page.rgbaFnv1a64 = Fnv1a64(
            reinterpret_cast<const uint8_t*>(page.rgbaPixels.data()),
            page.rgbaPixels.size() * sizeof(uint32_t));
    }
    result.completeDoubleBuffer = true;
    result.directVramPixelStateAuthority = true;
    result.executionAccepted = true;
    result.known = true;
    return result;
}

StrVramPagePublishResult8002756C PublishStrVramPageUpload8002756C(
    StrVramPageRuntime8001B120& runtime,
    uint16_t pageIndex,
    const StrStripUploadExecution8002756C& upload)
{
    StrVramPagePublishResult8002756C result{};
    result.pageIndex = pageIndex;
    if (!runtime.initialized ||
        !runtime.currentScusSemanticAuthority ||
        !runtime.directVramPixelStateAuthority ||
        runtime.psxGpuMmioAuthority ||
        runtime.hardwareCompletionTimingAuthority ||
        runtime.hostProjection || runtime.replayValueAuthority ||
        runtime.oldWinS0Authority || runtime.stage2PlusAuthority ||
        runtime.comod2Authority || pageIndex >= runtime.pages.size() ||
        !upload.known || !upload.executed ||
        upload.width == 0u || upload.height == 0u ||
        upload.width > runtime.width || upload.height > runtime.height ||
        upload.rgbaPixels.size() !=
            static_cast<std::size_t>(upload.width) * upload.height ||
        !upload.exactCurrentIdaStripOrder ||
        !upload.directMdecVerticalLayoutAuthority ||
        !upload.directRgbaAssemblyAuthority ||
        !upload.translatedGpuDmaStateAuthority ||
        !upload.translatedGpuCompletionAuthority ||
        upload.translatedGpuCommandCount != upload.stripCount ||
        upload.psxGpuDmaExecutionAuthority || upload.psxPointerAuthority ||
        upload.replayValueAuthority || upload.oldWinS0Authority ||
        upload.stage2PlusAuthority || upload.comod2Authority) {
        return result;
    }

    // 8002756C receives decoded movie rectangles.  MOVIE0 is 256x144 at
    // (35,25), whereas MOVIE0T is 320x240 at (0,0).  The PSX routine writes
    // each rectangle into the selected display page; it does not require the
    // source rectangle itself to be a complete page.  Keep the historical
    // full-page path for the existing 320x240 contract (whose synthetic
    // tests intentionally use a non-zero source origin), and compose the
    // proven partial rectangle for the actual MOVIE0 geometry.
    const bool fullPageSource =
        upload.width == runtime.width && upload.height == runtime.height;
    int64_t localDestinationY = upload.destinationY;
    if (pageIndex != 0u) {
        localDestinationY -=
            static_cast<int64_t>(pageIndex) * runtime.height;
    }
    const bool destinationInPage =
        upload.destinationX >= 0 && localDestinationY >= 0 &&
        static_cast<uint64_t>(upload.destinationX) + upload.width <=
            runtime.width &&
        static_cast<uint64_t>(localDestinationY) + upload.height <=
            runtime.height;
    if (!fullPageSource && !destinationInPage) {
        return result;
    }

    StrVramPage8001B120& page = runtime.pages[pageIndex];
    std::vector<uint32_t> pagePixels;
    if (fullPageSource) {
        pagePixels = upload.rgbaPixels;
    } else {
        pagePixels.assign(kStrVramPagePixelCount8001B120, 0u);
        if (page.contentKnown &&
            page.rgbaPixels.size() == kStrVramPagePixelCount8001B120) {
            pagePixels = page.rgbaPixels;
        }
        for (uint32_t y = 0u; y < upload.height; ++y) {
            const std::size_t srcOffset =
                static_cast<std::size_t>(y) * upload.width;
            const std::size_t dstOffset =
                static_cast<std::size_t>(localDestinationY + y) *
                    runtime.width +
                static_cast<std::size_t>(upload.destinationX);
            std::copy_n(upload.rgbaPixels.begin() + srcOffset,
                        upload.width,
                        pagePixels.begin() + dstOffset);
        }
    }
    page.contentKnown = true;
    ++page.uploadCount8002756C;
    page.rgbaPixels = std::move(pagePixels);
    page.rgbaFnv1a64 = Fnv1a64(
        reinterpret_cast<const uint8_t*>(page.rgbaPixels.data()),
        page.rgbaPixels.size() * sizeof(uint32_t));
    result.pixelCount = static_cast<uint32_t>(page.rgbaPixels.size());
    result.sourcePixelCount = static_cast<uint32_t>(upload.rgbaPixels.size());
    result.sourceRgbaFnv1a64 = upload.rgbaFnv1a64;
    result.rgbaFnv1a64 = page.rgbaFnv1a64;
    result.destinationX = upload.destinationX;
    result.destinationY = static_cast<int32_t>(localDestinationY);
    result.partialPageUpload = !fullPageSource;
    result.exactFullPageShape = true;
    result.directVramPixelStateAuthority = true;
    result.executionAccepted = true;
    result.known = true;
    return result;
}

StrDisplayMoveResult8001B120 ExecuteStrDisplayMove8001B120(
    int32_t argument,
    uint16_t drawBufferSlot80096590,
    bool interruptMaskKnown,
    uint16_t interruptMask,
    StrVramPageRuntime8001B120& runtime)
{
    StrDisplayMoveResult8001B120 result{};
    result.called = true;
    result.argument = argument;
    result.drawBufferSlot80096590 = drawBufferSlot80096590;
    if (!runtime.initialized || drawBufferSlot80096590 > 1u ||
        !runtime.currentScusSemanticAuthority ||
        !runtime.directVramPixelStateAuthority ||
        runtime.psxGpuMmioAuthority ||
        runtime.hardwareCompletionTimingAuthority ||
        runtime.hostProjection || runtime.replayValueAuthority ||
        runtime.oldWinS0Authority || runtime.stage2PlusAuthority ||
        runtime.comod2Authority || !interruptMaskKnown) {
        return result;
    }

    result.drawBufferRead8004019C = true;
    if (argument != 0) {
        result.sourcePage = drawBufferSlot80096590 == 0u ? 1u : 0u;
        result.destinationPage = drawBufferSlot80096590;
    } else {
        result.sourcePage = drawBufferSlot80096590;
        result.destinationPage =
            drawBufferSlot80096590 == 0u ? 1u : 0u;
    }
    result.sourceX = 0;
    result.sourceY = static_cast<int16_t>(
        result.sourcePage * kStrVramPageHeight8001B120);
    result.width = static_cast<int16_t>(kStrVramPageWidth8001B120);
    result.height = static_cast<int16_t>(kStrVramPageHeight8001B120);
    result.destinationX = 0u;
    result.destinationY = static_cast<uint16_t>(
        result.destinationPage * kStrVramPageHeight8001B120);

    const StrVramPage8001B120& source = runtime.pages[result.sourcePage];
    if (!source.contentKnown ||
        source.rgbaPixels.size() != kStrVramPagePixelCount8001B120) {
        return result;
    }

    PrPsxDmaSubmitDirect::PsxMoveImageInput80044E2C dmaInput{};
    dmaInput.sourceX = result.sourceX;
    dmaInput.sourceY = result.sourceY;
    dmaInput.width = result.width;
    dmaInput.height = result.height;
    dmaInput.destinationX = result.destinationX;
    dmaInput.destinationY = result.destinationY;
    dmaInput.interruptMaskKnown = interruptMaskKnown;
    dmaInput.interruptMask = interruptMask;
    result.dma =
        PrPsxDmaSubmitDirect::ExecuteMoveImageSoftware80044E2C(dmaInput);
    if (result.dma.status !=
            PrPsxDmaSubmitDirect::PsxMoveImageStatus80044E2C::
                SoftwareTransactionRecorded ||
        !result.dma.softwarePacketStateCommitted ||
        !result.dma.directDmaTransactionRecorded ||
        result.dma.hostMmioWritten || result.dma.hostGpuSubmitted ||
        result.dma.exactPsxHalParity) {
        return result;
    }

    // The translated GPU/DMA register image is the semantic execution owner
    // on Windows.  It is separate from PSX MMIO authority: the latter stays
    // false because no physical 0x1F8018xx register is touched here.
    if (!result.dma.translatedGpuDmaStateCommitted ||
        !result.dma.translatedGpuCompletionPublished ||
        result.dma.translatedGpuGp1Value !=
            PrPsxDmaSubmitDirect::kGp1DmaDirectionCpuToGp0 ||
        result.dma.translatedGpuDma2Madr !=
            PrPsxDmaSubmitDirect::kMoveImagePacketAddress8005D7DC ||
        result.dma.translatedGpuDma2Bcr != 0u ||
        result.dma.translatedGpuDma2Chcr !=
            PrPsxDmaSubmitDirect::kDma2LinkedListChcr) {
        return result;
    }

    StrVramPage8001B120& destination =
        runtime.pages[result.destinationPage];
    destination.contentKnown = true;
    ++destination.moveWriteCount8001B120;
    destination.rgbaFnv1a64 = source.rgbaFnv1a64;
    destination.rgbaPixels = source.rgbaPixels;
    ++runtime.moveCount8001B120;

    result.sourceRgbaFnv1a64 = source.rgbaFnv1a64;
    result.destinationRgbaFnv1a64 = destination.rgbaFnv1a64;
    result.pixelCountCopied =
        static_cast<uint32_t>(destination.rgbaPixels.size());
    result.exact8001B120Branch = true;
    result.softwareVramMoveExecuted = true;
    result.executionAccepted = true;
    result.currentScusSemanticAuthority = true;
    result.currentComod0CallerAuthority = true;
    result.directVramPixelStateAuthority = true;
    result.directGpuPacketStateAuthority = true;
    result.translatedGpuDmaStateAuthority = true;
    result.translatedGpuCompletionAuthority = true;
    result.known = true;
    return result;
}

StrPlayAndWaitReturnResult801C455C
ResolveStrPlayAndWaitMode0Return801C455C()
{
    StrPlayAndWaitReturnResult801C455C result{};
    // The active outer call at 801C4E4C passes a2=0. That mode-zero route
    // reaches 801C4638 and initializes s3 to zero. No later instruction on
    // that route writes s3 before 801C4754 moves it into v0, followed by the
    // exact saved-register epilogue and `jr ra; nop`. Other entry modes are
    // deliberately not generalized: current COMOD0 also contains a route
    // that initializes s3 to one.
    result.callerMode = 0;
    result.activeCallerModeZero = true;
    result.returnCarrierS3InitializedZero801C4638 = true;
    result.returnCarrierS3UnmodifiedAfterInitialization = true;
    result.returnMoveV0FromS3Observed801C4754 = true;
    result.exactEpilogueObserved = true;
    result.returnValue = 0;
    result.currentComod0SemanticAuthority = true;
    result.directReturnStateAuthority = true;
    result.known = true;
    return result;
}

StrOuterPostWaitResult801C4DC4 ExecuteStrOuterPostWait801C4DC4(
    uint16_t drawBufferSlot80096590,
    bool interruptMaskKnown,
    uint16_t interruptMask,
    StrVramPageRuntime8001B120& runtime)
{
    StrOuterPostWaitResult801C4DC4 result{};
    result.called = true;
    result.playAndWaitReturn =
        ResolveStrPlayAndWaitMode0Return801C455C();
    if (!result.playAndWaitReturn.known ||
        !result.playAndWaitReturn.activeCallerModeZero ||
        result.playAndWaitReturn.callerMode != 0 ||
        result.playAndWaitReturn.returnValue != 0 ||
        !result.playAndWaitReturn.currentComod0SemanticAuthority ||
        !result.playAndWaitReturn.directReturnStateAuthority ||
        result.playAndWaitReturn.hostProjection ||
        result.playAndWaitReturn.replayValueAuthority ||
        result.playAndWaitReturn.oldWinS0Authority ||
        result.playAndWaitReturn.stage2PlusAuthority ||
        result.playAndWaitReturn.comod2Authority) {
        return result;
    }

    // PrScene0_RunMovie0_AndMenu does not branch on the zero return. Its next
    // instruction sequence is a second 8001B120(1), then
    // 800201AC(scene0,5,1,2). Execute the software VRAM/GPU-packet state here
    // and leave transition execution to the outer runtime owner.
    result.secondDisplayMove = ExecuteStrDisplayMove8001B120(
        1,
        drawBufferSlot80096590,
        interruptMaskKnown,
        interruptMask,
        runtime);
    if (!result.secondDisplayMove.known ||
        !result.secondDisplayMove.executionAccepted ||
        !result.secondDisplayMove.exact8001B120Branch ||
        !result.secondDisplayMove.softwareVramMoveExecuted ||
        !result.secondDisplayMove.currentScusSemanticAuthority ||
        !result.secondDisplayMove.currentComod0CallerAuthority ||
        !result.secondDisplayMove.directVramPixelStateAuthority ||
        !result.secondDisplayMove.directGpuPacketStateAuthority ||
        result.secondDisplayMove.psxGpuMmioAuthority ||
        result.secondDisplayMove.hardwareCompletionTimingAuthority ||
        result.secondDisplayMove.hostProjection ||
        result.secondDisplayMove.replayValueAuthority ||
        result.secondDisplayMove.oldWinS0Authority ||
        result.secondDisplayMove.stage2PlusAuthority ||
        result.secondDisplayMove.comod2Authority) {
        return result;
    }

    result.fastTransitionPending = true;
    result.word800916D2WritePending = true;
    result.exactCallerOrder = true;
    result.currentComod0SemanticAuthority = true;
    result.directSoftwareStateAuthority = true;
    result.known = true;
    return result;
}

StrRootCallbackRuntime80055F78
InitializeStrRootCallbackRuntime80055F78()
{
    StrRootCallbackRuntime80055F78 runtime{};
    // Current SCUS loads word_80055F78, word_80055FAA, and
    // dword_80055FAC as zero. The PSX interrupt registers are not image
    // data, so they remain unknown until the direct 800358DC reset path
    // writes its exact software values.
    runtime.initialized = true;
    runtime.enabledKnown = true;
    runtime.enabled = false;
    runtime.savedInterruptMaskKnown = true;
    runtime.savedInterruptMask80055FAA = 0u;
    runtime.savedDmaControlKnown = true;
    runtime.savedDmaControl80055FAC = 0u;
    runtime.entryIntHookedKnown = true;
    runtime.entryIntHooked = false;
    runtime.criticalSectionHeldKnown = true;
    runtime.criticalSectionHeld = false;
    runtime.scusLoadedInitialZeroAuthority = true;
    runtime.currentScusSemanticAuthority = true;
    runtime.directSoftwareStateAuthority = true;
    return runtime;
}

StrRootCallbackResetResult800358DC ExecuteStrRootCallbackReset800358DC(
    StrRootCallbackRuntime80055F78& runtime)
{
    StrRootCallbackResetResult800358DC result{};
    result.called = true;
    result.enabledBeforeKnown = runtime.enabledKnown;
    result.enabledBefore = runtime.enabled;
    if (!runtime.initialized || !runtime.enabledKnown ||
        !runtime.savedInterruptMaskKnown ||
        !runtime.savedDmaControlKnown ||
        !runtime.entryIntHookedKnown ||
        !runtime.criticalSectionHeldKnown ||
        !runtime.scusLoadedInitialZeroAuthority ||
        !runtime.currentScusSemanticAuthority ||
        !runtime.directSoftwareStateAuthority ||
        runtime.psxHardwareMmioAuthority ||
        runtime.hardwareCallbackTimingAuthority ||
        runtime.hostProjection || runtime.replayValueAuthority ||
        runtime.oldWinS0Authority || runtime.stage2PlusAuthority ||
        runtime.comod2Authority) {
        return result;
    }

    // 800473EC(0) calls ResetCallback; table slot 80056FEC resolves to
    // 800358DC. An already-enabled root is a true no-op. Otherwise the
    // exact SCUS order clears I_MASK, mirrors that zero to I_STAT, writes
    // DICR=33333333, initializes the callback record, takes the initial
    // setjmp-zero path, hooks the exception entry, enables word_80055F78,
    // queries and stores the default DMA/VSync callbacks, applies the memory
    // configuration call, and exits the critical section. Only the
    // independently owned software state is
    // represented here; no host write to PSX MMIO or callback timing is
    // claimed.
    if (runtime.enabled) {
        result.noOpAlreadyEnabled = true;
        result.returnValue = 0u;
        result.exactSoftwareCallOrder = true;
        result.executionAccepted = true;
        result.currentScusSemanticAuthority = true;
        result.directSoftwareStateAuthority = true;
        result.known = true;
        return result;
    }

    runtime.interruptMaskKnown = true;
    runtime.interruptMask = 0u;
    result.interruptMaskCleared = true;
    runtime.interruptStatusKnown = true;
    runtime.interruptStatus = runtime.interruptMask;
    result.interruptStatusCleared = true;
    runtime.dmaControlKnown = true;
    runtime.dmaControl = kRootResetDmaControlValue;
    result.dmaControlInitialized = true;
    result.callbackStateInitialized80035E28 = true;
    result.setjmpInitialReturnZero80047F5C = true;
    result.unexpectedInterruptDispatchSkipped800359B8 = true;
    runtime.entryIntHooked = true;
    result.entryIntHooked = true;
    runtime.enabled = true;
    result.enabledWritten = true;
    result.defaultVSyncCallbackQueried80035E54 = true;
    result.defaultDmaCallbackQueried80035F7C = true;
    result.memoryConfigurationCalled80048960 = true;
    runtime.criticalSectionHeld = false;
    result.exitCriticalSectionExecuted = true;
    result.returnValue = kWord80055F78;
    result.exactSoftwareCallOrder = true;
    result.executionAccepted = true;
    result.currentScusSemanticAuthority = true;
    result.directSoftwareStateAuthority = true;
    result.known = true;
    return result;
}

StrStopCallbackResult80035838 ExecuteStrStopCallback80035838(
    StrRootCallbackRuntime80055F78& runtime)
{
    StrStopCallbackResult80035838 result{};
    result.called = true;
    result.enabledBeforeKnown = runtime.enabledKnown;
    result.enabledBefore = runtime.enabled;
    if (!runtime.initialized || !runtime.enabledKnown ||
        !runtime.savedInterruptMaskKnown ||
        !runtime.savedDmaControlKnown ||
        !runtime.entryIntHookedKnown ||
        !runtime.criticalSectionHeldKnown ||
        !runtime.scusLoadedInitialZeroAuthority ||
        !runtime.currentScusSemanticAuthority ||
        !runtime.directSoftwareStateAuthority ||
        runtime.psxHardwareMmioAuthority ||
        runtime.hardwareCallbackTimingAuthority ||
        runtime.hostProjection || runtime.replayValueAuthority ||
        runtime.oldWinS0Authority || runtime.stage2PlusAuthority ||
        runtime.comod2Authority) {
        return result;
    }

    // 80035838 loads table slot 80056FF0, whose current SCUS value is
    // 80035CF4. A disabled root returns zero without mutation. The active
    // path enters a critical section, saves I_MASK/DICR, clears I_MASK,
    // writes the cleared mask to I_STAT, masks DICR with 77777777,
    // ResetEntryInt, then clears word_80055F78. 80035CF4 deliberately does
    // not exit the critical section; the later reset/restart path owns it.
    if (!runtime.enabled) {
        result.noOpAlreadyDisabled = true;
        result.returnValue = 0u;
        result.exactSoftwareCallOrder = true;
        result.executionAccepted = true;
        result.currentScusSemanticAuthority = true;
        result.directSoftwareStateAuthority = true;
        result.known = true;
        return result;
    }
    if (!runtime.interruptMaskKnown ||
        !runtime.interruptStatusKnown ||
        !runtime.dmaControlKnown) {
        return result;
    }

    runtime.criticalSectionHeld = true;
    result.enterCriticalSectionExecuted = true;
    runtime.savedInterruptMask80055FAA = runtime.interruptMask;
    runtime.savedInterruptMaskKnown = true;
    result.interruptMaskSaved80055FAA = true;
    runtime.savedDmaControl80055FAC = runtime.dmaControl;
    runtime.savedDmaControlKnown = true;
    result.dmaControlSaved80055FAC = true;
    runtime.interruptMask = 0u;
    result.interruptMaskCleared = true;
    runtime.interruptStatus = runtime.interruptMask;
    result.interruptStatusClearedFromMask = true;
    runtime.dmaControl &= kRootStopDmaControlMask;
    result.dmaControlMasked77777777 = true;
    runtime.entryIntHooked = false;
    result.resetEntryIntExecuted = true;
    runtime.enabled = false;
    result.enabledCleared = true;
    result.returnValue = kWord80055F78;
    result.exactSoftwareCallOrder = true;
    result.executionAccepted = true;
    result.currentScusSemanticAuthority = true;
    result.directSoftwareStateAuthority = true;
    result.known = true;
    return result;
}

StrPostUploadDecision801C455C ResolveStrPostUploadDecision801C455C(
    const StrWorkBaseRuntime80049428& runtime,
    bool requiredFlushExecuted8001ED3C,
    bool playerTickContinuesKnown,
    bool playerTickContinues)
{
    StrPostUploadDecision801C455C result{};
    result.workBaseRead = ReadStrWorkBase8001A3B8(runtime);
    result.playerTickContinuesKnown = playerTickContinuesKnown;
    result.playerTickContinues = playerTickContinues;
    if (!result.workBaseRead.known ||
        !requiredFlushExecuted8001ED3C ||
        !playerTickContinuesKnown) {
        return result;
    }

    // Opening COMOD0 801C455C checks 8001A3B8 after 8001ED74, 8002756C,
    // and its required 8001ED3C submission, before it decides whether the
    // saved PrStrPlayer_Tick result loops.
    result.specialWorkBaseExit = result.workBaseRead.returnsOne;
    if (result.specialWorkBaseExit) {
        result.callWaitCleanup8001A694 = true;
        result.callStopCallback80035838 = true;
        // Both calls now have independently owned direct software state:
        // 8001A694 waits command 8 and clears the CD callback, then
        // 80035838/80035CF4 saves and disables the root callback state.
        // Existing direct 80027664 follows at the shared cleanup label; its
        // later 8001A4A4/8001A694/80024CF0/8001B120 tail remains a separate
        // execution boundary. Hardware MMIO/timing is not claimed.
        result.specialCleanupDirectExecutable = true;
        result.gapSpecialCleanupNotYetDirect = false;
    } else {
        result.continueLoop = playerTickContinues;
        result.terminateFromPlayerTick = !playerTickContinues;
    }
    result.exactBranchOrder = true;
    result.currentScusSemanticAuthority = true;
    result.currentComod0CallerAuthority = true;
    result.directLowerCdStateAuthority = true;
    // COMOD0 801C46E0 stores the return from its own 801C448C tick in s1
    // and 801C4724 branches on that value. There is no Host/Windows player
    // result in the original loop, so projections are explicitly ignored.
    result.directPlayerTickAuthority = true;
    result.hostProjectionResultIgnored = true;
    result.known = true;
    return result;
}

StrPlayerTick801C448C StepStrPlayerTick801C448C(
    StrWorkBaseRuntime80049428& workBase,
    StrLowerCdClockRuntime801C4350& clock)
{
    StrPlayerTick801C448C tick{};
    if (!workBase.initialized ||
        !workBase.dword80049428Known ||
        !workBase.byte80057119Known ||
        !workBase.scusLoadedInitialZeroAuthority ||
        !workBase.currentScusSemanticAuthority ||
        !workBase.currentComod0CallerAuthority ||
        !workBase.directLowerCdStateAuthority ||
        workBase.hardwareCallbackAuthority ||
        workBase.hostProjection ||
        workBase.replayValueAuthority ||
        workBase.oldWinS0Authority ||
        workBase.stage2PlusAuthority ||
        workBase.comod2Authority ||
        !clock.initialized ||
        !clock.discImageDirectoryAuthority ||
        !clock.runtimeInputBound ||
        !clock.psxCdClockAuthority ||
        clock.hostClockAuthority ||
        clock.replayValueAuthority ||
        clock.oldWinS0Authority ||
        clock.stage2PlusAuthority ||
        clock.comod2Authority) {
        return tick;
    }

    const int64_t advanced =
        static_cast<int64_t>(clock.currentLba) +
        static_cast<int64_t>(kStrClockSectorScale);
    const int32_t statusLba =
        advanced >= clock.effectiveEndLba
            ? clock.effectiveEndLba
            : static_cast<int32_t>(advanced);
    const auto msf =
        PrMovieSegmentDirect::PsxCall80036974_LbaToMsf(statusLba);
    if (!msf.known) {
        return tick;
    }

    tick.vblankWaitCalled80035560 = true;
    tick.vblankWaitMode80035560 = 2;
    tick.clockPollCalled8001A3C8 = true;
    tick.currentLba = statusLba;
    tick.lbaToMsfCalled80036974 = true;
    tick.syncFeedbackKnown800364D0 = true;
    tick.syncReturn800364D0 = static_cast<int32_t>(kCdSyncComplete);
    tick.syncMinuteBcd = msf.msf.minute;
    tick.syncSecondBcd = msf.msf.second;
    tick.syncFrameBcd = msf.msf.frame;
    tick.readyStatusKnown800363A4 = true;
    tick.readyStatus800363A4 = workBase.byte80057119;

    PrMovieSegmentDirect::StreamClockPollInput8001A3C8 pollInput{};
    pollInput.sub800364D0Known = true;
    pollInput.sub800364D0Result =
        static_cast<int32_t>(kCdSyncComplete);
    pollInput.syncBytesKnown = true;
    pollInput.syncBytes[0] = msf.msf.minute;
    pollInput.syncBytes[1] = msf.msf.second;
    pollInput.syncBytes[2] = msf.msf.frame;
    pollInput.sub800363A4Known = true;
    pollInput.sub800363A4Result = workBase.byte80057119;
    const auto poll =
        PrMovieSegmentDirect::PsxCall8001A3C8_StreamClockPoll(pollInput);
    ++workBase.clockPollCount8001A3C8;
    tick.clockPollAccepted8001A3C8 =
        poll.acceptedByte800493F4 &&
        !poll.gapMissingSub800364D0Feedback &&
        !poll.gapMissingSub800363A4Feedback;
    if (poll.dword80049428Known) {
        workBase.dword80049428Known = true;
        workBase.dword80049428 = poll.dword80049428;
    }
    tick.dword80049428Known = workBase.dword80049428Known;
    tick.dword80049428 = workBase.dword80049428;

    tick.cleanup = ApplyStrCleanup8001A280(workBase);
    tick.clock = StepStrLowerCdClockRuntime801C4350(clock);
    tick.exactCallOrder =
        tick.vblankWaitCalled80035560 &&
        tick.clockPollCalled8001A3C8 &&
        tick.cleanup.called &&
        tick.clock.polled &&
        tick.clock.currentLba == tick.currentLba;
    tick.currentScusSemanticAuthority = true;
    tick.currentComod0CallerAuthority = true;
    tick.directLowerCdStateAuthority = true;
    tick.known =
        tick.clockPollAccepted8001A3C8 &&
        tick.cleanup.known &&
        tick.clock.known &&
        tick.exactCallOrder;
    return tick;
}

bool UsesStrLowerCdClockRuntime801C4350(StrMovieKind movieKind)
{
    // Active COMOD0 opening playback reaches 801C4350 through
    // 801C455C -> 801C448C. Title MOVIE0T is owned by 801C4894: state 0
    // advances 80027528/8002756C and stops at its own v10 >= 50 gate.
    return movieKind == StrMovieKind::OpeningMovie0;
}

StrStopResetState80027664 BuildStrStopResetState80027664()
{
    StrStopResetState80027664 state{};
    state.known = true;
    state.clearDmaCallbackCalled80047658 = true;
    state.clearDmaCallbackArgument = 0;
    state.clearDmaCallbackChannel = 1u;
    state.stopCdCalled800392C0 = true;
    state.memoryResetCallCount80025AF8 = 4u;
    state.exactCallOrder = true;
    state.dmaCallbackClearExecuted = false;
    state.cdStopExecuted = false;
    state.memoryResetExecuted = false;
    state.hostProjection = false;
    state.replayValueAuthority = false;
    state.oldWinS0Authority = false;
    state.stage2PlusAuthority = false;
    state.comod2Authority = false;
    return state;
}

StrStopResetState80027664 ExecuteStrStopReset80027664(
    StrDecoderMemoryRuntime80027288& runtime)
{
    StrStopResetState80027664 state =
        BuildStrStopResetState80027664();
    state.runtimeWasActive = runtime.initialized;
    if (!runtime.initialized) {
        state.executionSkippedInactive = true;
        return state;
    }
    if (runtime.allocationStackDepth != 4u ||
        !runtime.allocationsZeroFilled ||
        !runtime.allocationAlignmentVerified ||
        !runtime.sectorRingInitialized8003624C ||
        runtime.sectorRingSlots8003624C !=
            kDecoderSectorRingSlots8003624C ||
        !runtime.outputCallbackInstalled80047658 ||
        runtime.outputCallbackFunction80027220 != kFn80027220 ||
        runtime.outputCallbackDmaChannel80047658 != 1u ||
        !runtime.directMemoryAuthority ||
        !runtime.directSectorRingLifecycleAuthority ||
        runtime.psxPointerAuthority ||
        runtime.psxHardwareMmioAuthority ||
        runtime.hardwareCallbackTimingAuthority ||
        runtime.hostProjection ||
        runtime.replayValueAuthority ||
        runtime.oldWinS0Authority ||
        runtime.stage2PlusAuthority ||
        runtime.comod2Authority) {
        state.known = false;
        return state;
    }
    uint64_t validatedAllocationBytes = 0u;
    for (uint32_t index = 0u; index < 4u; ++index) {
        if (!runtime.allocationPresent[index] ||
            runtime.allocations[index].size() !=
                runtime.allocationSizes[index]) {
            state.known = false;
            return state;
        }
        validatedAllocationBytes += runtime.allocationSizes[index];
    }
    if (validatedAllocationBytes != runtime.allocationBytes) {
        state.known = false;
        return state;
    }

    // 80047658(0) -> DMACallback(1,0).
    runtime.outputCallbackInstalled80047658 = false;
    runtime.outputCallbackFunction80027220 = 0u;
    state.dmaCallbackClearExecuted = true;
    state.directDmaCallbackStateAuthority = true;

    // 800392C0 enters a critical section, clears the data/ready callback
    // slots through 80036930(0)/80036528(0), clears CD registers 0 and 3,
    // and exits the critical section. These are direct state mirrors, not
    // host writes to PSX MMIO.
    state.enterCriticalSectionExecuted = true;
    runtime.cdDataCallbackInstalled80036930 = false;
    runtime.cdDataCallbackFunction80039318 = 0u;
    state.cdDataCallbackClearExecuted80036930 = true;
    runtime.cdReadyCallbackInstalled80036528 = false;
    runtime.cdReadyCallbackFunction80039240 = 0u;
    state.cdReadyCallbackClearExecuted80036528 = true;
    runtime.cdromReg0Known = true;
    runtime.cdromReg0 = 0u;
    state.cdromReg0ClearExecuted = true;
    runtime.cdromReg3Known = true;
    runtime.cdromReg3 = 0u;
    state.cdromReg3ClearExecuted = true;
    runtime.streamingCdActive8001A4D0 = false;
    state.exitCriticalSectionExecuted = true;
    state.cdStopExecuted = true;
    state.directCdStateAuthority = true;

    // 80025AF8 pops the allocator stack. 80027288 pushed control, sector,
    // VLC, and image in that order, so four calls release them in reverse.
    for (uint32_t pop = 0u; pop < 4u; ++pop) {
        const uint32_t index = runtime.allocationStackDepth - 1u;
        state.memoryResetPointerSlotOrder[pop] =
            runtime.allocationPointerSlots[index];
        state.memoryBytesReleased +=
            runtime.allocations[index].size();
        std::vector<uint8_t>{}.swap(runtime.allocations[index]);
        runtime.allocationPresent[index] = false;
        --runtime.allocationStackDepth;
        ++state.memoryResetCountExecuted;
    }
    state.memoryResetExecuted =
        state.memoryResetCountExecuted ==
            state.memoryResetCallCount80025AF8 &&
        runtime.allocationStackDepth == 0u;
    state.sectorRingReset = state.memoryResetExecuted;
    state.directMemoryStackAuthority = state.memoryResetExecuted;
    state.psxPointerAuthority = false;
    state.psxHardwareMmioAuthority = false;
    state.hardwareCallbackTimingAuthority = false;
    state.executionAccepted =
        state.dmaCallbackClearExecuted &&
        state.enterCriticalSectionExecuted &&
        state.cdDataCallbackClearExecuted80036930 &&
        state.cdReadyCallbackClearExecuted80036528 &&
        state.cdromReg0ClearExecuted &&
        state.cdromReg3ClearExecuted &&
        state.exitCriticalSectionExecuted &&
        state.cdStopExecuted &&
        state.memoryResetExecuted &&
        state.sectorRingReset;
    runtime.initialized = false;
    runtime.allocationBytes = 0u;
    runtime.allocationSizes.fill(0u);
    runtime.allocationPointerSlots.fill(0u);
    runtime.allocationsZeroFilled = false;
    runtime.allocationAlignmentVerified = false;
    runtime.sectorRingInitialized8003624C = false;
    runtime.sectorRingSlots8003624C = 0u;
    runtime.sectorRingSlotBytes = 0u;
    runtime.sectorRingQueuedSlots = 0u;
    runtime.sectorRingProducerIndex80085C50 = 0u;
    runtime.sectorRingPublishedIndex80085C54 = 0u;
    runtime.sectorRingReadIndex80085C58 = 0u;
    runtime.sectorRingRawSectorCursor = 0u;
    runtime.sectorRingRawSectorsScanned = 0u;
    runtime.sectorRingFilteredVideoSectors = 0u;
    runtime.sectorRingRejectedSectors = 0u;
    runtime.sectorRingPublishedFrames = 0u;
    runtime.sectorRingPublishedFirstSlot = 0u;
    runtime.sectorRingPublishedSectorCount = 0u;
    runtime.sectorRingPublishedFrameNumber = 0u;
    runtime.sectorRingPublishedWidth = 0u;
    runtime.sectorRingPublishedHeight = 0u;
    runtime.sectorRingPublishedPayloadByteOffset = 0u;
    runtime.sectorRingFramePublished = false;
    runtime.sectorRingFrameAcquired = false;
    runtime.directMemoryAuthority = false;
    runtime.directSectorRingLifecycleAuthority = false;
    runtime.directSectorRingInputAuthority = false;
    runtime.directStreamingCdStateAuthority = false;
    // 80027664 releases decoder memory and stops streaming, but it does not
    // erase the global 800367A4 command/response surface. COMOD0 immediately
    // proves that distinction by calling 8001A694 again.
    return state;
}

bool IsKnownExitInput80035510(uint32_t inputMask)
{
    return inputMask == kInputForceReturnOne ||
           inputMask == kInputDirectExit ||
           (inputMask & kInputExitMask) != 0u;
}

int32_t KnownReturnValueForInput80035510(uint32_t inputMask)
{
    if (inputMask == kInputForceReturnOne) {
        return 1;
    }
    if (inputMask == kInputDirectExit ||
        (inputMask & kInputExitMask) != 0u) {
        return 0;
    }
    return 0;
}

StrPlan BuildStrStart801C44E0Plan(uint32_t segmentAddress,
                                  int32_t movieKind,
                                  bool word800916DC)
{
    StrPlan plan = MakePlan(StrPlanKind::Start801C44E0);
    plan.hasOpenP0Gap = true;

    const StrDecodeGeometry geometry =
        KnownDecodeGeometry80027288(movieKind, word800916DC);

    AppendAction(plan,
                 StrActionKind::GateStrStartSource801C44E0,
                 kFn801C44E0,
                 segmentAddress,
                 static_cast<uint32_t>(movieKind),
                 word800916DC ? 1u : 0u,
                 geometry.known ? 1u : 0u,
                 1,
                 true,
                 true);
    AppendAction(plan, StrActionKind::Call801C44E0, kFn801C44E0,
                 segmentAddress, static_cast<uint32_t>(movieKind));
    AppendAction(plan, StrActionKind::Call80026FA4, kFn80026FA4);
    AppendAction(plan, StrActionKind::Call80024E98, kFn80024E98);
    AppendAction(plan, StrActionKind::SetMovieKindNotOneFlag,
                 kFn801C44E0, movieKind == 1 ? 0u : 1u);
    AppendAction(plan, StrActionKind::Call8001A478, kFn8001A478,
                 segmentAddress + 6u);
    AppendAction(plan,
                 StrActionKind::Call80027288AllocDecoder,
                 kFn80027288,
                 static_cast<uint32_t>(movieKind),
                 geometry.width,
                 geometry.height,
                 (geometry.dstX << 16) | geometry.dstY);
    AppendAction(plan, StrActionKind::Call8001A4D0StartStream,
                 kFn8001A4D0, segmentAddress, kStreamStartMode);
    AppendAction(plan, StrActionKind::Call80036A78SetlocLba,
                 kFn80036A78, segmentAddress + 0x10u);
    AppendAction(plan, StrActionKind::Call800367A4SetlocCommand,
                 kFn800367A4,
                 kCdCommandSetloc,
                 kDword800493EC,
                 kDword80049414);
    AppendAction(plan, StrActionKind::Poll800364D0SetlocComplete,
                 kFn800364D0,
                 0,
                 0,
                 kCdSyncComplete,
                 0,
                 0,
                 false,
                 true);
    AppendAction(plan, StrActionKind::Call800367A4SetfilterCommand,
                 kFn800367A4,
                 kCdCommandSetfilter,
                 kByte8004940C,
                 kDword80049414);
    AppendAction(plan, StrActionKind::Poll800364D0SetfilterComplete,
                 kFn800364D0,
                 0,
                 0,
                 kCdSyncComplete,
                 0,
                 0,
                 false,
                 true);
    AppendAction(plan, StrActionKind::Set8001A4D0StreamModeWord,
                 kFn8001A4D0,
                 kStreamingCdModeWord,
                 kStreamingCdModeByte,
                 kStreamingDmaCallbackBit,
                 kStreamStartMode);
    AppendAction(plan, StrActionKind::Set8001A4D0BufferCadence,
                 kFn8001A4D0,
                 kDword80049424,
                 kStreamingBufferCadence,
                 kDword80049420,
                 0u - kStreamingBufferCadence);
    AppendAction(plan,
                 StrActionKind::Call800391ACSetCdMode,
                 kFn800391AC,
                 kStreamingCdModeWord,
                 kStreamingCdModeByte,
                 kStreamingDmaCallbackBit,
                 kStreamingBufferCadence);
    AppendAction(plan, StrActionKind::Call80036540SetModeCommand,
                 kFn80036540,
                 kCdCommandSetmode,
                 kStreamingCdModeByte,
                 kStreamingCdModeWord);
    AppendAction(plan, StrActionKind::Call800391ACInstallDmaCallback,
                 kFn80036930,
                 kFn80039318,
                 kStreamingCdModeWord,
                 kStreamingDmaCallbackBit);
    AppendAction(plan, StrActionKind::Call800391ACInstallDataCallback,
                 kFn80036528,
                 kFn80039240,
                 kStreamingCdModeWord,
                 kStreamingDmaCallbackBit);
    AppendAction(plan, StrActionKind::Call80036540CommitModeCommand,
                 kFn80036540, kCdCommandCommit);
    AppendAction(plan, StrActionKind::Call80035560StartWait,
                 kFn80035560, kStartWaitFrames);
    AppendAction(plan, StrActionKind::Poll800364D0StreamStartComplete,
                 kFn800364D0,
                 0,
                 0,
                 kCdSyncComplete,
                 0,
                 0,
                 false,
                 true);
    AppendAction(plan, StrActionKind::Set8001A4D0StartFlags,
                 kFn8001A4D0,
                 kDword80049410,
                 1,
                 kDword80049420,
                 0u - kStreamingBufferCadence);
    AppendAction(plan, StrActionKind::Call80036678PrimeStatus,
                 kFn80036678, 1, 0);
    AppendAction(plan, StrActionKind::Call800274D4StartStream,
                 kFn800274D4);
    AppendAction(plan, StrActionKind::CallStSetStreamStart,
                 kFnStSetStream, 0, 1, 0xFFFFFFFFu, 0);
    AppendAction(plan, StrActionKind::CallDecDCTvlcSize2Reset,
                 kFnDecDCTvlcSize2, 0);
    AppendAction(plan, StrActionKind::Call800273A4DecodeFallback,
                 kFn800273A4);
    AppendAction(plan, StrActionKind::Gap, kFn8001A4D0,
                 kStreamingCdModeWord);
    return plan;
}

StrPlan BuildStrLoop801C455CPlan(uint32_t segmentAddress,
                                 uint32_t ctxAddress,
                                 int32_t mode,
                                 int32_t movieKind,
                                 bool word800916DC,
                                 bool inputMaskKnown,
                                 uint32_t inputMask)
{
    StrPlan plan = MakePlan(StrPlanKind::Loop801C455C);
    plan.hasOpenP0Gap = true;

    const StrDecodeGeometry geometry =
        KnownDecodeGeometry80027288(movieKind, word800916DC);

    AppendAction(plan,
                 StrActionKind::GateStrLoopSource801C455C,
                 kFn801C455C,
                 segmentAddress,
                 ctxAddress,
                 static_cast<uint32_t>(mode),
                 static_cast<uint32_t>(movieKind),
                 1,
                 true,
                 true);
    AppendAction(plan, StrActionKind::Call801C455C, kFn801C455C,
                 segmentAddress, ctxAddress, static_cast<uint32_t>(mode));
    AppendAction(plan, StrActionKind::WarmupPoll8001A750, kFn8001A750,
                 0, 0, 0, 0, static_cast<int32_t>(kWarmupPollLimit));
    AppendAction(plan, StrActionKind::Call80036678StatusRetry,
                 kFn80036678, 1, 0, 0, 0,
                 static_cast<int32_t>(kWarmupPollLimit), true, true);
    AppendAction(plan,
                 StrActionKind::GateStrInputSource80035510,
                 kFn80035510,
                 inputMaskKnown ? 1u : 0u,
                 inputMask,
                 kInputForceReturnOne,
                 kInputExitMask,
                 1,
                 true,
                 true);
    AppendAction(plan, StrActionKind::Call80035510ReadInput,
                 kFn80035510, inputMaskKnown ? 1u : 0u, inputMask);

    if (inputMaskKnown && IsKnownExitInput80035510(inputMask)) {
        AppendPlan(plan, BuildStrCleanup801C455CPlan(true));
        AppendAction(plan, StrActionKind::ReturnValue, kFn801C455C,
                     static_cast<uint32_t>(
                         KnownReturnValueForInput80035510(inputMask)));
        AppendAction(plan, StrActionKind::Gap, kFn801C455C, inputMask);
        return plan;
    }

    AppendAction(plan, StrActionKind::Call80024CF8Events, kFn80024CF8,
                 ctxAddress);
    AppendAction(plan, StrActionKind::Call8001EC54TextEvent,
                 kFn8001EC54, ctxAddress, 7, 0, 0, 1, true, true);
    AppendAction(plan, StrActionKind::Call8001ED3CTextFlush,
                 kFn8001ED3C, 0, 0, 0, 0, 1, true, true);
    AppendAction(plan, StrActionKind::Call80027528DecodeFrame,
                 kFn80027528);
    AppendAction(plan, StrActionKind::Call800273A4DecodeFallback,
                 kFn800273A4, 0, 0, 0, 0, 1, true, true);
    AppendAction(plan, StrActionKind::Call801C448CStepMovie,
                 kFn801C448C, segmentAddress, ctxAddress);
    AppendAction(plan, StrActionKind::Call801C4350UpdateClock,
                 kFn801C4350, segmentAddress, ctxAddress);
    AppendAction(plan, StrActionKind::Call8001A7A4QueryClock,
                 kFn8001A7A4, segmentAddress + 0x40u);
    AppendAction(plan, StrActionKind::Call8001A7F8CheckEnd,
                 kFn8001A7F8, segmentAddress);
    AppendAction(plan, StrActionKind::Call8001ED74PrepareFlip,
                 kFn8001ED74);
    AppendAction(plan, StrActionKind::Call80040370Flip, kFn80040370);
    AppendAction(plan,
                 StrActionKind::Call8002756CUploadStrips,
                 kFn8002756C,
                 geometry.width,
                 geometry.height,
                 geometry.dstX,
                 geometry.dstY,
                 geometry.known ? static_cast<int32_t>(geometry.stripCount)
                                : 0,
                 geometry.known);
    AppendAction(plan,
                 StrActionKind::Call80044D64LoadImage,
                 kFn80044D64,
                 kUploadStripWidth,
                 geometry.height,
                 kMacroblockBytes,
                 geometry.dstY,
                 geometry.known ? static_cast<int32_t>(geometry.stripCount)
                                : 0,
                 geometry.known);
    AppendAction(plan, StrActionKind::Gap, kFn801C455C,
                 inputMaskKnown ? inputMask : 0xFFFFFFFFu);
    return plan;
}

StrPlan BuildStrCleanup801C455CPlan(bool includeInsideDisplayUpload)
{
    StrPlan plan = MakePlan(StrPlanKind::Cleanup801C455C);
    plan.hasOpenP0Gap = true;

    AppendAction(plan,
                 StrActionKind::GateStrCleanupSource80027664,
                 kFn80027664,
                 includeInsideDisplayUpload ? 1u : 0u,
                 kFn8001A4A4,
                 kFn8001A694,
                 kFn8001B120,
                 1,
                 true,
                 true);
    AppendAction(plan, StrActionKind::Call80027664StopReset, kFn80027664);
    AppendAction(plan, StrActionKind::Call80047658ClearCallback,
                 kFn80047658, 0);
    AppendAction(plan, StrActionKind::CallDMACallbackClear,
                 kFnDMACallback, 1, 0);
    AppendAction(plan, StrActionKind::Call800392C0StopCd, kFn800392C0);
    AppendAction(plan, StrActionKind::Call800392C0EnterCritical,
                 kFnEnterCriticalSection);
    AppendAction(plan, StrActionKind::Call80036930CdReset,
                 kFn80036930, 0);
    AppendAction(plan, StrActionKind::Call80036528CdReset,
                 kFn80036528, 0);
    AppendAction(plan, StrActionKind::Write800392C0CdromMmioClear,
                 kFn800392C0,
                 kCdromIoReg0,
                 0,
                 kCdromIoReg3,
                 0);
    AppendAction(plan, StrActionKind::Call800392C0ExitCritical,
                 kFnExitCriticalSection);
    AppendAction(plan, StrActionKind::Call80025AF8ResetRetry,
                 kFn80025AF8, 0, 0, 0, 0, 4, true);
    AppendAction(plan, StrActionKind::Call8001A4A4StreamCommand,
                 kFn8001A4A4, 1);
    AppendAction(plan, StrActionKind::Call8001A694WaitCleanup,
                 kFn8001A694);
    AppendAction(plan, StrActionKind::Call80024CF0NullSub,
                 kFn80024CF0, kScene0WorkAddress);
    if (includeInsideDisplayUpload) {
        AppendAction(plan, StrActionKind::Call8001B120DisplayUpload,
                     kFn8001B120, 1);
    }
    AppendAction(plan, StrActionKind::Gap, kFn80027664,
                 includeInsideDisplayUpload ? 1u : 0u);
    return plan;
}

StrPlan BuildScene0Movie0PlaybackPlan(uint32_t sceneEntryBase,
                                      bool word800916DC,
                                      bool includeInsideDisplayUpload)
{
    StrPlan plan = MakePlan(StrPlanKind::Scene0Movie0Playback);
    plan.hasOpenP0Gap = true;

    const uint32_t segmentAddress =
        sceneEntryBase + kSceneEntryMovie0SegmentOffset;
    AppendPlan(plan, BuildStrStart801C44E0Plan(
        segmentAddress, static_cast<int32_t>(StrMovieKind::OpeningMovie0),
        word800916DC));
    AppendPlan(plan, BuildStrLoop801C455CPlan(
        segmentAddress, kScene0WorkAddress, 0,
        static_cast<int32_t>(StrMovieKind::OpeningMovie0),
        word800916DC, false, 0));
    AppendPlan(plan, BuildStrCleanup801C455CPlan(includeInsideDisplayUpload));
    AppendAction(plan, StrActionKind::Call8001B120DisplayUpload,
                 kFn8001B120, 1);
    AppendAction(plan, StrActionKind::Gap, kFn801C455C,
                 includeInsideDisplayUpload ? 2u : 1u);
    return plan;
}

StrPlan BuildTitleMovie0TStartPlan(uint32_t sceneEntryBase,
                                   bool word800916DC)
{
    StrPlan plan = MakePlan(StrPlanKind::TitleMovie0TStart);
    plan.hasOpenP0Gap = true;

    const uint32_t segmentAddress =
        sceneEntryBase + kSceneEntryMovie0TSegmentOffset;
    AppendPlan(plan, BuildStrStart801C44E0Plan(
        segmentAddress, static_cast<int32_t>(StrMovieKind::TitleMovie0T),
        word800916DC));
    AppendAction(plan, StrActionKind::Gap, kFn801C44E0, segmentAddress);
    return plan;
}

const char* StrPlanKindName(StrPlanKind kind)
{
    switch (kind) {
    case StrPlanKind::Unknown:
        return "Unknown";
    case StrPlanKind::Start801C44E0:
        return "Start801C44E0";
    case StrPlanKind::Loop801C455C:
        return "Loop801C455C";
    case StrPlanKind::Cleanup801C455C:
        return "Cleanup801C455C";
    case StrPlanKind::Scene0Movie0Playback:
        return "Scene0Movie0Playback";
    case StrPlanKind::TitleMovie0TStart:
        return "TitleMovie0TStart";
    }
    return "Unknown";
}

const char* StrMovieKindName(StrMovieKind kind)
{
    switch (kind) {
    case StrMovieKind::OpeningMovie0:
        return "OpeningMovie0";
    case StrMovieKind::TitleMovie0T:
        return "TitleMovie0T";
    case StrMovieKind::Unknown:
        return "Unknown";
    }
    return "Unknown";
}

const char* StrActionKindName(StrActionKind kind)
{
    switch (kind) {
    case StrActionKind::None:
        return "None";
    case StrActionKind::GateStrStartSource801C44E0:
        return "GateStrStartSource801C44E0";
    case StrActionKind::Call801C44E0:
        return "Call801C44E0";
    case StrActionKind::Call80026FA4:
        return "Call80026FA4";
    case StrActionKind::Call80024E98:
        return "Call80024E98";
    case StrActionKind::SetMovieKindNotOneFlag:
        return "SetMovieKindNotOneFlag";
    case StrActionKind::Call8001A478:
        return "Call8001A478";
    case StrActionKind::Call80027288AllocDecoder:
        return "Call80027288AllocDecoder";
    case StrActionKind::Call8001A4D0StartStream:
        return "Call8001A4D0StartStream";
    case StrActionKind::Call80036A78SetlocLba:
        return "Call80036A78SetlocLba";
    case StrActionKind::Call800367A4SetlocCommand:
        return "Call800367A4SetlocCommand";
    case StrActionKind::Poll800364D0SetlocComplete:
        return "Poll800364D0SetlocComplete";
    case StrActionKind::Call800367A4SetfilterCommand:
        return "Call800367A4SetfilterCommand";
    case StrActionKind::Poll800364D0SetfilterComplete:
        return "Poll800364D0SetfilterComplete";
    case StrActionKind::Set8001A4D0StreamModeWord:
        return "Set8001A4D0StreamModeWord";
    case StrActionKind::Set8001A4D0BufferCadence:
        return "Set8001A4D0BufferCadence";
    case StrActionKind::Call800391ACSetCdMode:
        return "Call800391ACSetCdMode";
    case StrActionKind::Call80036540SetModeCommand:
        return "Call80036540SetModeCommand";
    case StrActionKind::Call800391ACInstallDmaCallback:
        return "Call800391ACInstallDmaCallback";
    case StrActionKind::Call800391ACInstallDataCallback:
        return "Call800391ACInstallDataCallback";
    case StrActionKind::Call80036540CommitModeCommand:
        return "Call80036540CommitModeCommand";
    case StrActionKind::Call80035560StartWait:
        return "Call80035560StartWait";
    case StrActionKind::Poll800364D0StreamStartComplete:
        return "Poll800364D0StreamStartComplete";
    case StrActionKind::Set8001A4D0StartFlags:
        return "Set8001A4D0StartFlags";
    case StrActionKind::Call80036678PrimeStatus:
        return "Call80036678PrimeStatus";
    case StrActionKind::Call800274D4StartStream:
        return "Call800274D4StartStream";
    case StrActionKind::CallStSetStreamStart:
        return "CallStSetStreamStart";
    case StrActionKind::CallDecDCTvlcSize2Reset:
        return "CallDecDCTvlcSize2Reset";
    case StrActionKind::GateStrLoopSource801C455C:
        return "GateStrLoopSource801C455C";
    case StrActionKind::Call801C455C:
        return "Call801C455C";
    case StrActionKind::WarmupPoll8001A750:
        return "WarmupPoll8001A750";
    case StrActionKind::Call80036678StatusRetry:
        return "Call80036678StatusRetry";
    case StrActionKind::GateStrInputSource80035510:
        return "GateStrInputSource80035510";
    case StrActionKind::Call80035510ReadInput:
        return "Call80035510ReadInput";
    case StrActionKind::Call80024CF8Events:
        return "Call80024CF8Events";
    case StrActionKind::Call8001EC54TextEvent:
        return "Call8001EC54TextEvent";
    case StrActionKind::Call8001ED3CTextFlush:
        return "Call8001ED3CTextFlush";
    case StrActionKind::Call80027528DecodeFrame:
        return "Call80027528DecodeFrame";
    case StrActionKind::Call800273A4DecodeFallback:
        return "Call800273A4DecodeFallback";
    case StrActionKind::Call801C448CStepMovie:
        return "Call801C448CStepMovie";
    case StrActionKind::Call801C4350UpdateClock:
        return "Call801C4350UpdateClock";
    case StrActionKind::Call8001A7A4QueryClock:
        return "Call8001A7A4QueryClock";
    case StrActionKind::Call8001A7F8CheckEnd:
        return "Call8001A7F8CheckEnd";
    case StrActionKind::Call8001ED74PrepareFlip:
        return "Call8001ED74PrepareFlip";
    case StrActionKind::Call80040370Flip:
        return "Call80040370Flip";
    case StrActionKind::Call8002756CUploadStrips:
        return "Call8002756CUploadStrips";
    case StrActionKind::Call80044D64LoadImage:
        return "Call80044D64LoadImage";
    case StrActionKind::GateStrCleanupSource80027664:
        return "GateStrCleanupSource80027664";
    case StrActionKind::Call80027664StopReset:
        return "Call80027664StopReset";
    case StrActionKind::Call80047658ClearCallback:
        return "Call80047658ClearCallback";
    case StrActionKind::CallDMACallbackClear:
        return "CallDMACallbackClear";
    case StrActionKind::Call800392C0StopCd:
        return "Call800392C0StopCd";
    case StrActionKind::Call800392C0EnterCritical:
        return "Call800392C0EnterCritical";
    case StrActionKind::Call80036930CdReset:
        return "Call80036930CdReset";
    case StrActionKind::Call80036528CdReset:
        return "Call80036528CdReset";
    case StrActionKind::Write800392C0CdromMmioClear:
        return "Write800392C0CdromMmioClear";
    case StrActionKind::Call800392C0ExitCritical:
        return "Call800392C0ExitCritical";
    case StrActionKind::Call80025AF8ResetRetry:
        return "Call80025AF8ResetRetry";
    case StrActionKind::Call8001A4A4StreamCommand:
        return "Call8001A4A4StreamCommand";
    case StrActionKind::Call8001A694WaitCleanup:
        return "Call8001A694WaitCleanup";
    case StrActionKind::Call80024CF0NullSub:
        return "Call80024CF0NullSub";
    case StrActionKind::Call8001B120DisplayUpload:
        return "Call8001B120DisplayUpload";
    case StrActionKind::ReturnValue:
        return "ReturnValue";
    case StrActionKind::Gap:
        return "Gap";
    }
    return "Unknown";
}

} // namespace PrSS0StrLifecycleDirect
