#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>

#include "pr_psx_dma_submit_direct.h"
#include "pr_ss0_mdec_vlc_direct.h"
#include "pr_ss0_mdec_output_direct.h"

namespace PrSS0StrLifecycleDirect {

static constexpr uint32_t kFn801C44E0 = 0x801C44E0u;
static constexpr uint32_t kFn801C455C = 0x801C455Cu;
static constexpr uint32_t kFn801C4DC4 = 0x801C4DC4u;
static constexpr uint32_t kFn80026FA4 = 0x80026FA4u;
static constexpr uint32_t kFn80024E98 = 0x80024E98u;
static constexpr uint32_t kFn8001A478 = 0x8001A478u;
static constexpr uint32_t kFn80027288 = 0x80027288u;
static constexpr uint32_t kFn80027220 = 0x80027220u;
static constexpr uint32_t kFn80027238 = 0x80027238u;
static constexpr uint32_t kFn80025A70 = 0x80025A70u;
static constexpr uint32_t kFn80025C44 = 0x80025C44u;
static constexpr uint32_t kFn8003624C = 0x8003624Cu;
static constexpr uint32_t kFn800473EC = 0x800473ECu;
static constexpr uint32_t kFn8001A4D0 = 0x8001A4D0u;
static constexpr uint32_t kFn80036A78 = 0x80036A78u;
static constexpr uint32_t kFn800367A4 = 0x800367A4u;
static constexpr uint32_t kFn800364D0 = 0x800364D0u;
static constexpr uint32_t kFn800391AC = 0x800391ACu;
static constexpr uint32_t kFn80036540 = 0x80036540u;
static constexpr uint32_t kFn80035560 = 0x80035560u;
static constexpr uint32_t kFn80039318 = 0x80039318u;
static constexpr uint32_t kFn80039240 = 0x80039240u;
static constexpr uint32_t kFn800274D4 = 0x800274D4u;
static constexpr uint32_t kFnStSetStream = 0x80039408u;
static constexpr uint32_t kFnDecDCTvlcSize2 = 0x80047B00u;
static constexpr uint32_t kFnDecDCTvlc2 = 0x80047B30u;
static constexpr uint32_t kFn8001A750 = 0x8001A750u;
static constexpr uint32_t kFn8001A280 = 0x8001A280u;
static constexpr uint32_t kFn8001A3C8 = 0x8001A3C8u;
static constexpr uint32_t kFn800363A4 = 0x800363A4u;
static constexpr uint32_t kFn80036678 = 0x80036678u;
static constexpr uint32_t kFn80036974 = 0x80036974u;
static constexpr uint32_t kFn800375BC = 0x800375BCu;
static constexpr uint32_t kFn80035510 = 0x80035510u;
static constexpr uint32_t kFn80024CF8 = 0x80024CF8u;
static constexpr uint32_t kFn8001EC54 = 0x8001EC54u;
static constexpr uint32_t kFn8001ED3C = 0x8001ED3Cu;
static constexpr uint32_t kFn80027528 = 0x80027528u;
static constexpr uint32_t kFn800273A4 = 0x800273A4u;
static constexpr uint32_t kFn8003958C = 0x8003958Cu;
static constexpr uint32_t kFn80039490 = 0x80039490u;
static constexpr uint32_t kFn80047558 = 0x80047558u;
static constexpr uint32_t kFn800475D4 = 0x800475D4u;
static constexpr uint32_t kFn80047778 = 0x80047778u;
static constexpr uint32_t kFn8004780C = 0x8004780Cu;
static constexpr uint32_t kFn8004789C = 0x8004789Cu;
static constexpr uint32_t kFn80047934 = 0x80047934u;
static constexpr uint32_t kFn8004019C = 0x8004019Cu;
static constexpr uint32_t kFn801C448C = 0x801C448Cu;
static constexpr uint32_t kFn801C4350 = 0x801C4350u;
static constexpr uint32_t kFn8001A7A4 = 0x8001A7A4u;
static constexpr uint32_t kFn8001A7F8 = 0x8001A7F8u;
static constexpr uint32_t kFn8001ED74 = 0x8001ED74u;
static constexpr uint32_t kFn80040370 = 0x80040370u;
static constexpr uint32_t kFn8002756C = 0x8002756Cu;
static constexpr uint32_t kFn80044D64 = 0x80044D64u;
static constexpr uint32_t kFn80027664 = 0x80027664u;
static constexpr uint32_t kFn80047658 = 0x80047658u;
static constexpr uint32_t kFnDMACallback = 0x800357A4u;
static constexpr uint32_t kFn800392C0 = 0x800392C0u;
static constexpr uint32_t kFnEnterCriticalSection = 0x80048A40u;
static constexpr uint32_t kFnExitCriticalSection = 0x80048A50u;
static constexpr uint32_t kFn80036930 = 0x80036930u;
static constexpr uint32_t kFn80036528 = 0x80036528u;
static constexpr uint32_t kFn80036510 = 0x80036510u;
static constexpr uint32_t kFn80025AF8 = 0x80025AF8u;
static constexpr uint32_t kFn8001A4A4 = 0x8001A4A4u;
static constexpr uint32_t kFn8001A694 = 0x8001A694u;
static constexpr uint32_t kFn8001A3B8 = 0x8001A3B8u;
static constexpr uint32_t kFn80024CF0 = 0x80024CF0u;
static constexpr uint32_t kFn8002AB24 = 0x8002AB24u;
static constexpr uint32_t kFn8002A6FC = 0x8002A6FCu;
static constexpr uint32_t kFnResetCallback = 0x80035744u;
static constexpr uint32_t kFn80035838 = 0x80035838u;
static constexpr uint32_t kFn800358DC = 0x800358DCu;
static constexpr uint32_t kFn80035CF4 = 0x80035CF4u;
static constexpr uint32_t kFnResetEntryInt = 0x80048A20u;
static constexpr uint32_t kFn8001B120 = 0x8001B120u;
static constexpr uint32_t kCallsite801C4E4C = 0x801C4E4Cu;
static constexpr uint32_t kCallsite801C4E54 = 0x801C4E54u;
static constexpr uint32_t kCallsite801C4E70 = 0x801C4E70u;
static constexpr uint32_t kCallsite801C4E74 = 0x801C4E74u;

static constexpr uint32_t kScene0WorkAddress = 0x801C3640u;
static constexpr uint32_t kSceneEntryMovie0SegmentOffset = 0x6Cu;
static constexpr uint32_t kSceneEntryMovie0TSegmentOffset = 0x9Cu;
static constexpr uint32_t kMovieGeometryTable80054614 = 0x80054614u;
static constexpr uint32_t kMovieGeometryTableRowBytes80054614 = 24u;
static constexpr uint32_t kMovieGeometryKnownRowCount80054614 = 3u;
static constexpr uint32_t kDecoderControlBytes80027288 = 0x24u;
static constexpr uint32_t kDecoderSectorBufferBytes80027288 = 0x10000u;
static constexpr uint32_t kDecoderAllocationAlignment80025A70 = 8u;
static constexpr uint32_t kDecoderSectorRingSlots8003624C = 32u;
static constexpr uint32_t kDecoderControlPointerSlot800965A8 = 0x800965A8u;
static constexpr uint32_t kDecoderSectorPointerSlot800965AC = 0x800965ACu;
static constexpr uint32_t kDecoderVlcPointerSlot800965B0 = 0x800965B0u;
static constexpr uint32_t kDecoderImagePointerSlot800965B4 = 0x800965B4u;
static constexpr uint32_t kDecoderRingSlotCount801C3868 = 0x801C3868u;
static constexpr uint32_t kDecoderSectorCounter800965B8 = 0x800965B8u;
static constexpr uint32_t kStreamStartMode = 1u;
static constexpr uint32_t kStreamingCdModeWord = 0x01C8u;
static constexpr uint32_t kStreamingCdModeByte = 0xC8u;
static constexpr uint32_t kStreamingDmaCallbackBit = 0x100u;
static constexpr uint32_t kStreamingBufferCadence = 16u;
static constexpr uint32_t kStreamingDefaultModeWord = 0x0048u;
static constexpr uint32_t kStreamingDefaultBufferCadence = 4u;
static constexpr uint32_t kCdCommandSetloc = 2u;
static constexpr uint32_t kCdCommandSetfilter = 13u;
static constexpr uint32_t kCdCommandSetmode = 14u;
static constexpr uint32_t kCdCommandCommit = 27u;
static constexpr uint8_t kCdCommandNop = 1u;
static constexpr uint8_t kCdCommandGetlocL = 0x10u;
static constexpr uint32_t kCdSyncComplete = 2u;
static constexpr uint32_t kStartWaitFrames = 3u;
static constexpr uint32_t kDword800493EC = 0x800493ECu;
static constexpr uint32_t kDword80049428 = 0x80049428u;
static constexpr uint32_t kByte80057119 = 0x80057119u;
static constexpr uint32_t kByte8004940C = 0x8004940Cu;
static constexpr uint32_t kDword80049410 = 0x80049410u;
static constexpr uint32_t kDword80049414 = 0x80049414u;
static constexpr uint32_t kDword800570F8 = 0x800570F8u;
static constexpr uint32_t kWord80055F78 = 0x80055F78u;
static constexpr uint32_t kWord80055FAA = 0x80055FAAu;
static constexpr uint32_t kDword80055FAC = 0x80055FACu;
static constexpr uint32_t kResetCallbackTableSlot80056FEC = 0x80056FECu;
static constexpr uint32_t kStopCallbackTableSlot80056FF0 = 0x80056FF0u;
static constexpr uint32_t kRootInterruptStatusRegister = 0x1F801070u;
static constexpr uint32_t kRootInterruptMaskRegister = 0x1F801074u;
static constexpr uint32_t kRootDmaControlRegister = 0x1F8010F0u;
static constexpr uint32_t kRootResetDmaControlValue = 0x33333333u;
static constexpr uint32_t kRootStopDmaControlMask = 0x77777777u;
static constexpr uint16_t kSerialVolumeAttributeMask8002AB24 = 0x00C0u;
static constexpr uint8_t kSerialVolumeMaximum8002AB24 = 0x7Fu;
static constexpr uint16_t kSerialVolumeScale8002AB24 = 0x0102u;
static constexpr uint16_t kStrVramPageWidth8001B120 = 320u;
static constexpr uint16_t kStrVramPageHeight8001B120 = 240u;
static constexpr uint32_t kStrVramPagePixelCount8001B120 =
    static_cast<uint32_t>(kStrVramPageWidth8001B120) *
    static_cast<uint32_t>(kStrVramPageHeight8001B120);
static constexpr uint32_t kDword80049420 = 0x80049420u;
static constexpr uint32_t kDword80049424 = 0x80049424u;
static constexpr uint32_t kWarmupPollLimit = 1800u;
static constexpr uint32_t kInputForceReturnOne = 0x100u;
static constexpr uint32_t kInputDirectExit = 0x800u;
static constexpr uint32_t kInputExitMask = 0x840u;
static constexpr uint32_t kUploadStripWidth = 16u;
static constexpr uint32_t kMacroblockBytes = 512u;
static constexpr uint32_t kStrClockDiscontinuityThreshold = 301u;
static constexpr uint32_t kStrClockSectorScale = 5u;
static constexpr uint32_t kStrClockFramesPerSecond = 30u;
static constexpr uint32_t kStrClockFramesPerMinute = 1800u;
static constexpr uint32_t kStrClockDeadlineLead = 150u;
static constexpr uint32_t kCdromIoReg0 = 0x1F801800u;
static constexpr uint32_t kCdromIoReg3 = 0x1F801803u;
static constexpr uint32_t kRawCdSectorBytes8001A4D0 = 2352u;
static constexpr uint32_t kRawCdLogicalBytes8001A4D0 = 2048u;
static constexpr uint32_t kRawCdSyncBytes8001A4D0 = 12u;
static constexpr uint32_t kRawCdHeaderOffset8001A4D0 = 12u;
static constexpr uint32_t kRawCdSubheaderOffset8001A4D0 = 16u;
static constexpr uint8_t kRawCdMode2Byte8001A4D0 = 2u;
static constexpr uint8_t kXaSubmodeVideo8001A4D0 = 0x02u;
static constexpr uint8_t kXaSubmodeAudio8001A4D0 = 0x04u;
static constexpr uint8_t kXaSubmodeRealtime8001A4D0 = 0x40u;
static constexpr uint32_t kXaAudioPayloadBytes8001A4D0 = 2304u;
static constexpr uint32_t kStrSectorMetadataBytes80039670 = 32u;
static constexpr uint32_t kStrSectorPayloadBytes80039670 = 2016u;
static constexpr uint32_t kStrSectorMetadataAreaBytes80039670 =
    kDecoderSectorRingSlots8003624C * kStrSectorMetadataBytes80039670;
static constexpr uint16_t kStrSectorHeaderId80039670 = 0x0160u;
static constexpr uint16_t kStrSectorStateFree = 0u;
static constexpr uint16_t kStrSectorStateWrap = 1u;
static constexpr uint16_t kStrSectorStatePublished = 2u;
static constexpr uint16_t kStrSectorStatePayloadReady = 3u;
static constexpr uint16_t kStrSectorStateAcquired = 4u;

enum class StrPlanKind : uint8_t {
    Unknown = 0,
    Start801C44E0,
    Loop801C455C,
    Cleanup801C455C,
    Scene0Movie0Playback,
    TitleMovie0TStart,
};

enum class StrMovieKind : uint8_t {
    OpeningMovie0 = 0,
    TitleMovie0T = 1,
    Unknown = 0xFFu,
};

enum class StrActionKind : uint8_t {
    None = 0,
    GateStrStartSource801C44E0,
    Call801C44E0,
    Call80026FA4,
    Call80024E98,
    SetMovieKindNotOneFlag,
    Call8001A478,
    Call80027288AllocDecoder,
    Call8001A4D0StartStream,
    Call80036A78SetlocLba,
    Call800367A4SetlocCommand,
    Poll800364D0SetlocComplete,
    Call800367A4SetfilterCommand,
    Poll800364D0SetfilterComplete,
    Set8001A4D0StreamModeWord,
    Set8001A4D0BufferCadence,
    Call800391ACSetCdMode,
    Call80036540SetModeCommand,
    Call800391ACInstallDmaCallback,
    Call800391ACInstallDataCallback,
    Call80036540CommitModeCommand,
    Call80035560StartWait,
    Poll800364D0StreamStartComplete,
    Set8001A4D0StartFlags,
    Call80036678PrimeStatus,
    Call800274D4StartStream,
    CallStSetStreamStart,
    CallDecDCTvlcSize2Reset,
    GateStrLoopSource801C455C,
    Call801C455C,
    WarmupPoll8001A750,
    Call80036678StatusRetry,
    GateStrInputSource80035510,
    Call80035510ReadInput,
    Call80024CF8Events,
    Call8001EC54TextEvent,
    Call8001ED3CTextFlush,
    Call80027528DecodeFrame,
    Call800273A4DecodeFallback,
    Call801C448CStepMovie,
    Call801C4350UpdateClock,
    Call8001A7A4QueryClock,
    Call8001A7F8CheckEnd,
    Call8001ED74PrepareFlip,
    Call80040370Flip,
    Call8002756CUploadStrips,
    Call80044D64LoadImage,
    GateStrCleanupSource80027664,
    Call80027664StopReset,
    Call80047658ClearCallback,
    CallDMACallbackClear,
    Call800392C0StopCd,
    Call800392C0EnterCritical,
    Call80036930CdReset,
    Call80036528CdReset,
    Write800392C0CdromMmioClear,
    Call800392C0ExitCritical,
    Call80025AF8ResetRetry,
    Call8001A4A4StreamCommand,
    Call8001A694WaitCleanup,
    Call80024CF0NullSub,
    Call8001B120DisplayUpload,
    ReturnValue,
    Gap,
};

struct StrDecodeGeometry {
    bool known = false;
    int32_t movieKind = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t dstX = 0;
    uint32_t dstY = 0;
    uint32_t stripCount = 0;
    bool word800916DCApplied = false;
};

struct StrDecoderTableRow80054614 {
    bool known = false;
    int32_t movieKind = 0;
    uint16_t expectedWidth = 0;
    uint16_t expectedHeight = 0;
    int16_t dstX = 0;
    int16_t dstY = 0;
    uint32_t tableWord08 = 0;
    uint32_t vlcBufferBytes = 0;
    uint32_t imageBufferBytes = 0;
    uint32_t tableWord14 = 0;
};

struct StrDecoderControlState80027288 {
    bool known = false;
    bool vlcBufferBound = false;
    bool imageBufferBound = false;
    uint32_t decodedWidth = 0;
    uint32_t decodedHeight = 0;
    uint32_t vlcDecodeResult = 0;
    int32_t dstX = 0;
    int32_t dstY = 0;
    uint32_t decodeInFlight = 0;
    uint32_t outputReady = 0;
};

struct StrDecoderAllocationState80027288 {
    bool accepted = false;
    int32_t movieKind = 0;
    bool word800916DC = false;
    StrDecoderTableRow80054614 tableRow{};
    uint32_t allocationSizes[4]{};
    uint32_t allocationCount = 0;
    uint32_t allocationAlignment = 0;
    bool allocationsZeroFilled = false;
    bool allocationFailureExitsProcess = false;
    uint32_t allocationPointerSlots[4]{};
    bool decDctResetCalled800473EC = false;
    uint32_t decDctResetArgument800473EC = 0;
    bool resetCallbackCalled800473EC = false;
    bool sectorRingInitialized8003624C = false;
    uint32_t sectorRingSlots8003624C = 0;
    uint32_t sectorRingCountAddress801C3868 = 0;
    bool outputCallbackInstalled80047658 = false;
    uint32_t outputCallbackFunction80027220 = 0;
    uint32_t outputCallbackDmaChannel80047658 = 0;
    StrDecoderControlState80027288 control{};
    bool currentScusSemanticAuthority = false;
    bool currentComod0CallerAuthority = false;
    bool comod1ComparisonOnly = false;
    bool hostProjection = false;
    bool psxPointerAuthority = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrDecoderMemoryRuntime80027288 {
    bool initialized = false;
    int32_t movieKind = 0;
    std::array<uint32_t, 4> allocationSizes{};
    std::array<uint32_t, 4> allocationPointerSlots{};
    std::array<bool, 4> allocationPresent{};
    std::array<std::vector<uint8_t>, 4> allocations{};
    uint32_t allocationStackDepth = 0;
    uint64_t allocationBytes = 0;
    bool allocationsZeroFilled = false;
    bool allocationAlignmentVerified = false;
    bool sectorRingInitialized8003624C = false;
    uint32_t sectorRingSlots8003624C = 0;
    uint32_t sectorRingSlotBytes = 0;
    uint32_t sectorRingQueuedSlots = 0;
    uint32_t sectorRingProducerIndex80085C50 = 0;
    uint32_t sectorRingPublishedIndex80085C54 = 0;
    uint32_t sectorRingReadIndex80085C58 = 0;
    uint32_t sectorRingRawSectorCursor = 0;
    uint32_t sectorRingRawSectorsScanned = 0;
    uint32_t sectorRingFilteredVideoSectors = 0;
    uint32_t sectorRingRejectedSectors = 0;
    uint32_t sectorRingPublishedFrames = 0;
    uint32_t sectorRingPublishedFirstSlot = 0;
    uint16_t sectorRingPublishedSectorCount = 0;
    uint32_t sectorRingPublishedFrameNumber = 0;
    uint16_t sectorRingPublishedWidth = 0;
    uint16_t sectorRingPublishedHeight = 0;
    uint32_t sectorRingPublishedPayloadByteOffset = 0;
    bool sectorRingFramePublished = false;
    bool sectorRingFrameAcquired = false;
    bool outputCallbackInstalled80047658 = false;
    uint32_t outputCallbackFunction80027220 = 0;
    uint32_t outputCallbackDmaChannel80047658 = 0;
    bool streamingCdActive8001A4D0 = false;
    bool cdDataCallbackInstalled80036930 = false;
    uint32_t cdDataCallbackFunction80039318 = 0;
    bool cdReadyCallbackInstalled80036528 = false;
    uint32_t cdReadyCallbackFunction80039240 = 0;
    bool cdStatusByteKnown = false;
    uint8_t cdStatusByte = 0u;
    bool cdSyncResponseKnown80049414 = false;
    std::array<uint8_t, 8> cdSyncResponse80049414{};
    uint8_t cdSyncState800573D4 = 0u;
    bool cdromReg0Known = false;
    uint8_t cdromReg0 = 0;
    bool cdromReg3Known = false;
    uint8_t cdromReg3 = 0;
    bool directMemoryAuthority = false;
    bool directSectorRingLifecycleAuthority = false;
    bool directSectorRingInputAuthority = false;
    bool directStreamingCdStateAuthority = false;
    bool directCdCommandStateAuthority = false;
    bool psxPointerAuthority = false;
    bool psxHardwareMmioAuthority = false;
    bool hardwareCallbackTimingAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrStreamStartState800274D4 {
    bool accepted = false;
    StrDecoderControlState80027288 control{};
    bool vlcDecodeResultCleared = false;
    bool decodeInFlightCleared = false;
    bool outputReadyCleared = false;
    bool stSetStreamCalled = false;
    int32_t stSetStreamArgs[5]{};
    bool decDctVlcSize2Called = false;
    int32_t decDctVlcSize2Argument = 0;
    bool initialDecodeAdvanceCalled800273A4 = false;
    bool sectorCounterResetAfterAdvance = false;
    uint32_t sectorCounterAddress800965B8 = 0;
    uint32_t sectorCounterValue = 0;
    bool exactCallOrder = false;
    bool currentScusSemanticAuthority = false;
    bool currentComod0CallerAuthority = false;
    bool comod1ComparisonOnly = false;
    bool lowerCdStartAuthority = false;
    bool initialDecodeAdvanceSemanticTranslated = false;
    bool streamLibraryAuthority = false;
    bool psxPointerAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrLowerCdStartSource8001A4D0 {
    bool segmentKnown = false;
    bool startLbaKnown = false;
    int32_t startLba = 0;
    bool lengthBytesKnown = false;
    uint32_t lengthBytes = 0;
    bool filterKnown = false;
    uint8_t filterFile = 0;
    uint8_t filterChannel = 0;
    std::filesystem::path discBinPath{};
    bool discImageDirectoryAuthority = false;
};

struct StrLowerCdStartRuntime8001A4D0 {
    bool initialized = false;
    uint32_t sourceFunction = kFn8001A4D0;
    int32_t startLba = 0;
    uint32_t lengthBytes = 0;
    uint32_t logicalSectorCount = 0;
    uint8_t filterFile = 0;
    uint8_t filterChannel = 0;
    uint8_t xaSubmode = 0;
    uint8_t xaCoding = 0;
    std::filesystem::path discBinPath{};
    bool rawSectorReadable = false;
    bool syncHeaderMatched = false;
    bool headerLbaMatched = false;
    bool mode2Matched = false;
    bool xaSubheaderDuplicateMatched = false;
    bool xaFilterMatched = false;
    bool xaRealtimeVideoMatched = false;
    bool setlocComplete800364D0 = false;
    bool setfilterComplete800364D0 = false;
    bool setmodeReadSComplete800391AC = false;
    uint32_t startWaitFrames80035560 = 0;
    bool streamStartComplete800364D0 = false;
    bool startFlagsWritten8001A4D0 = false;
    int32_t dword80049410 = 0;
    int32_t dword80049420 = 0;
    bool primeStatusCalled80036678 = false;
    bool commandWrapper80036678Succeeded = false;
    bool statusPollInputKnown8001A750 = false;
    int32_t syncReturn800364D0 = 0;
    uint8_t status0 = 0;
    bool exactCallOrder = false;
    bool currentScusSemanticAuthority = false;
    bool currentComod0CallerAuthority = false;
    bool discImageDirectoryAuthority = false;
    bool discImagePayloadAuthority = false;
    bool directPsxStatusAuthority = false;
    bool hardwareCallbackAuthority = false;
    bool hostProjection = false;
    bool hostExtractedFileAuthority = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrXaAudioDiscRuntime8001A4D0 {
    bool initialized = false;
    uint32_t sourceFunction = kFn8001A4D0;
    uint32_t rawSectorCursor = 0;
    uint32_t rawSectorsScanned = 0;
    uint32_t filteredAudioSectors = 0;
    uint8_t filterFile = 0;
    uint8_t filterChannel = 0;
    bool currentScusSemanticAuthority = false;
    bool currentComod0CallerAuthority = false;
    bool discImagePayloadAuthority = false;
    bool directXaSectorSelectionAuthority = false;
    bool psxCdXaDecodeAuthority = false;
    bool psxSpuHardwareAuthority = false;
    bool hostExtractedFileAuthority = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrXaAudioSectorResult8001A4D0 {
    bool known = false;
    bool available = false;
    uint32_t rawSectorCursorBefore = 0;
    uint32_t rawSectorCursorAfter = 0;
    uint32_t matchedRawSectorCursor = 0;
    uint32_t rawSectorsScanned = 0;
    bool targetCursorReached = false;
    uint8_t file = 0;
    uint8_t channel = 0;
    uint8_t submode = 0;
    uint8_t coding = 0;
    std::array<uint8_t, kXaAudioPayloadBytes8001A4D0> payload{};
    bool syncHeaderMatched = false;
    bool mode2Matched = false;
    bool xaSubheaderDuplicateMatched = false;
    bool xaFilterMatched = false;
    bool xaRealtimeAudioMatched = false;
    bool currentScusSemanticAuthority = false;
    bool currentComod0CallerAuthority = false;
    bool discImagePayloadAuthority = false;
    bool directXaSectorSelectionAuthority = false;
    bool psxCdXaDecodeAuthority = false;
    bool psxSpuHardwareAuthority = false;
    bool hostExtractedFileAuthority = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

enum class StrDecodeAdvanceBranch800273A4 : uint8_t {
    UnknownInput = 0,
    AcquireUnavailable,
    EmptyPayload,
    VlcDecodeFailed,
    OutputQueued,
    MdecBoundary,
    MdecHardwareBoundary,
    MdecOutputSubmittedHardwareBoundary,
};

struct StrDecodeAdvanceInput800273A4 {
    bool acquireReturnKnown = false;
    int32_t acquireReturn = 0;
    bool payloadHandleKnown = false;
    uint32_t payloadHandle = 0;
    bool sectorHeaderKnown = false;
    uint16_t sectorWidth = 0;
    uint16_t sectorHeight = 0;
    bool vlcDecodeResultKnown = false;
    int32_t vlcDecodeResult = 0;
    bool mdecCommandOutputSubmissionKnown = false;
    bool mdecCommandOutputSubmissionAuthority = false;
    uint32_t sectorCounterBefore = 0;
    bool sectorRingExecutionAuthority = false;
};

struct StrDecodeAdvanceState800273A4 {
    bool planned = false;
    bool executed = false;
    bool hasOpenInputGap = true;
    StrDecodeAdvanceBranch800273A4 branch =
        StrDecodeAdvanceBranch800273A4::UnknownInput;
    StrDecoderControlState80027288 control{};
    bool acquireFrameCalled8003958C = false;
    bool acquireReturnKnown = false;
    int32_t acquireReturn = 0;
    bool payloadHandleKnown = false;
    uint32_t payloadHandle = 0;
    bool sectorHeaderApplied = false;
    bool sectorCounterIncremented = false;
    uint32_t sectorCounterAddress800965B8 = 0;
    uint32_t sectorCounterBefore = 0;
    uint32_t sectorCounterAfter = 0;
    bool decDctVlc2Called = false;
    bool reachedDecDctVlc2Boundary = false;
    bool executedThroughAcquire8003958C = false;
    bool blockedAtMdecDecDCTvlc2 = false;
    bool reachedMdecControlBoundary80047558 = false;
    bool blockedAtMdecHardwareExecution = false;
    bool hasOpenMdecHardwareGap = false;
    bool vlcDecodeResultKnown = false;
    int32_t vlcDecodeResult = 0;
    bool releaseFrameCalled80039490 = false;
    bool mdecControlCalled80047558 = false;
    int32_t mdecControlArgument = 0;
    bool outputTransferCalled = false;
    uint32_t outputTransferWordCount = 0;
    bool mdecCommandOutputSubmissionAuthority = false;
    bool blockedAtMdecHardwareOutput = false;
    bool branchSemanticsTranslated = false;
    bool runtimeInputBound = false;
    bool sectorRingExecutionAuthority = false;
    bool mdecExecutionAuthority = false;
    bool psxPointerAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrMdecCommandOutputSubmission800273A4 {
    bool known = false;
    bool executed = false;
    uint32_t controlArgument80047558 = 0;
    uint32_t vlcCommandWordBefore = 0;
    uint32_t vlcCommandWordAfter = 0;
    uint16_t inputDmaWordCount80047778 = 0;
    bool inputSyncCalled8004789C = false;
    uint32_t dmaPriorityOrMask = 0;
    uint32_t inputMadrByteOffset = 0;
    uint32_t inputBcr = 0;
    uint32_t inputChcr = 0;
    bool mdecCommandPortWritten = false;
    bool outputSyncCalled80047934 = false;
    uint32_t outputDmaWordCount8004780C = 0;
    uint32_t outputMadrByteOffset = 0;
    uint32_t outputBcr = 0;
    uint32_t outputChcr = 0;
    bool outputReadyStored800273A4 = false;
    bool dmaCompletionCallbackPending80027220 = false;
    bool outputBufferUntouched = false;
    bool exactCurrentIdaCallOrder = false;
    bool currentScusSemanticAuthority = false;
    bool directCommandStateAuthority = false;
    bool mdecHardwareExecutionAuthority = false;
    bool hardwareCallbackTimingAuthority = false;
    bool psxPointerAuthority = false;
    bool psxHardwareMmioAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrMdecDmaCompletionState80027220 {
    bool known = false;
    bool executed = false;
    StrDecoderControlState80027288 controlBefore{};
    StrDecoderControlState80027288 controlAfter{};
    bool dmaChannelOneCallbackInstalled = false;
    uint32_t callbackFunction80027220 = 0;
    uint32_t callbackDmaChannel = 0;
    bool completedOutputValidated = false;
    uint32_t completedOutputBytes = 0;
    bool outputReadyCleared = false;
    bool decodeInFlightPreserved = false;
    bool exactCurrentIdaStoreAuthority = false;
    bool directSynchronousOutputCompletionAuthority = false;
    bool hardwareCallbackTimingAuthority = false;
    bool psxPointerAuthority = false;
    bool psxHardwareMmioAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrSectorRingPrimeResult80039670 {
    bool known = false;
    bool framePublished = false;
    uint32_t rawSectorsScanned = 0;
    uint32_t filteredVideoSectors = 0;
    uint32_t rejectedSectors = 0;
    uint32_t firstSlot = 0;
    uint16_t sectorCount = 0;
    uint32_t frameNumber = 0;
    uint16_t width = 0;
    uint16_t height = 0;
    uint32_t payloadByteOffset = 0;
    bool directDiscSectorAuthority = false;
    bool synchronousCallbackOrdering = false;
    bool hardwareCallbackTimingAuthority = false;
    bool psxPointerAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrSectorRingAcquireResult8003958C {
    bool known = false;
    int32_t returnValue = 1;
    bool available = false;
    bool payloadHandleKnown = false;
    uint32_t payloadHandle = 0;
    bool sectorHeaderKnown = false;
    uint16_t width = 0;
    uint16_t height = 0;
    uint32_t firstSlot = 0;
    uint16_t sectorCount = 0;
    uint32_t frameNumber = 0;
    bool stateTransition2To4 = false;
    bool directSectorRingAuthority = false;
    bool psxPointerAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrSectorRingReleaseResult80039490 {
    bool known = false;
    int32_t returnValue = 1;
    bool released = false;
    uint32_t firstSlot = 0;
    uint16_t sectorCount = 0;
    uint32_t nextReadIndex = 0;
    bool stateTransition4To0 = false;
    bool directSectorRingAuthority = false;
    bool psxPointerAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrVlcDecodeReleaseResult800273A4 {
    bool known = false;
    bool executed = false;
    uint32_t payloadHandle = 0;
    uint32_t inputBytes = 0;
    PrSS0MdecVlcDirect::DecDctVlc2Result80047B30 vlc{};
    bool releaseFrameCalled80039490 = false;
    StrSectorRingReleaseResult80039490 release{};
    bool exactCurrentIdaCallOrder = false;
    bool currentScusSemanticAuthority = false;
    bool directSectorRingAuthority = false;
    bool mdecHardwareExecutionAuthority = false;
    bool psxPointerAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

enum class StrDecodeGateBranch80027528 : uint8_t {
    OutputNotReady = 0,
    DecodeIdle,
    DecodeBusy,
};

struct StrDecodeGateState80027528 {
    bool known = false;
    StrDecodeGateBranch80027528 branch =
        StrDecodeGateBranch80027528::OutputNotReady;
    uint32_t outputReady = 0;
    uint32_t decodeInFlight = 0;
    bool callDecodeAdvance800273A4 = false;
    bool returnValueKnown = false;
    int32_t returnValue = 0;
    bool branchSemanticsTranslated = false;
    bool decodeAdvanceExecuted = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrStripUploadCommand8002756C {
    int16_t x = 0;
    int16_t y = 0;
    int16_t width = 0;
    int16_t height = 0;
    uint32_t sourceByteOffset = 0;
    uint32_t sourcePitchTerm = 0;
};

struct StrStripUploadState8002756C {
    bool known = false;
    bool skippedForCounterWarmup = false;
    bool skippedForZeroStrips = false;
    uint32_t sectorCounter = 0;
    bool displayBufferResultKnown = false;
    int32_t displayBufferResult8004019C = 0;
    int32_t destinationY = 0;
    uint32_t macroblockRows = 0;
    uint32_t stripCount = 0;
    StrStripUploadCommand8002756C commands[64]{};
    bool truncated = false;
    bool uploadCallsPlanned80044D64 = false;
    bool gpuUploadExecuted = false;
    bool psxImagePointerAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrStripUploadExecution8002756C {
    bool known = false;
    bool executed = false;
    uint32_t width = 0;
    uint32_t height = 0;
    // The upload commands target a PSX VRAM coordinate, while the decoded
    // MDEC pixels are kept in a compact source rectangle.  Preserve that
    // coordinate so 8002756C can publish a partial movie frame into the
    // 320x240 display page instead of incorrectly requiring a full-page
    // source buffer.
    int32_t destinationX = 0;
    int32_t destinationY = 0;
    uint32_t stripCount = 0;
    uint32_t commandsExecuted80044D64 = 0;
    uint32_t sourceBytesConsumed = 0;
    uint32_t rgbaBytesWritten = 0;
    uint64_t sourceFnv1a64 = 0;
    uint64_t rgbaFnv1a64 = 0;
    std::vector<uint32_t> rgbaPixels{};
    bool exactCurrentIdaStripOrder = false;
    bool directMdecVerticalLayoutAuthority = false;
    bool directRgbaAssemblyAuthority = false;
    bool hostTextureUploadPending = false;
    bool hostTextureUploadExecuted = false;
    // 8002756C emits one translated DMA upload transaction per strip.  This
    // records the software GP1/DMA2 owner without confusing it with host
    // texture presentation or real PSX MMIO.
    bool translatedGpuDmaStateAuthority = false;
    bool translatedGpuCompletionAuthority = false;
    uint32_t translatedGpuCommandCount = 0u;
    bool psxGpuDmaExecutionAuthority = false;
    bool psxPointerAuthority = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

enum class StrClockPollBranch801C4350 : uint8_t {
    UnknownInput = 0,
    NegativeClockDelta,
    Discontinuity,
    EndCheckUnknown,
    ContinuePlayback,
    EndPlayback,
};

struct StrClockPollInput801C4350 {
    bool clockDeltaKnown8001A7A4 = false;
    int32_t clockDelta8001A7A4 = 0;
    bool previousClockKnown801C954C = false;
    int32_t previousClock801C954C = 0;
    bool endCheckKnown8001A7F8 = false;
    int32_t endCheckReturn8001A7F8 = 0;
    bool directDiscSegmentAuthority = false;
};

struct StrClockPollState801C4350 {
    bool planned = false;
    bool executed = false;
    bool hasOpenInputGap = true;
    StrClockPollBranch801C4350 branch =
        StrClockPollBranch801C4350::UnknownInput;
    bool clockQueryCalled8001A7A4 = false;
    bool clockDeltaKnown = false;
    int32_t clockDelta = 0;
    bool previousClockKnown = false;
    int32_t previousClock = 0;
    uint32_t absoluteClockDelta = 0;
    bool clockFieldsUpdated = false;
    int32_t updatedPreviousClock = 0;
    int16_t minuteFieldOffset4 = 0;
    uint8_t secondFieldOffset6 = 0;
    uint8_t frameFieldOffset7 = 0;
    bool endCheckCalled8001A7F8 = false;
    bool returnValueKnown = false;
    bool returnValue = true;
    bool branchSemanticsTranslated = false;
    bool currentScusSemanticAuthority = false;
    bool currentComod0CallerAuthority = false;
    bool comod1ComparisonOnly = false;
    bool runtimeInputBound = false;
    bool psxCdClockAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrLowerCdClockSource801C4350 {
    bool segmentTimeBaseKnown = false;
    int32_t segmentTimeBase = 0;
    bool segmentEndKnown = false;
    int32_t segmentEnd = 0;
    bool segmentEndBiasKnown = false;
    int32_t segmentEndBias = 0;
    bool discImageDirectoryAuthority = false;
};

struct StrLowerCdClockRuntime801C4350 {
    bool initialized = false;
    int32_t timeBaseLba = 0;
    int32_t endLba = 0;
    int32_t endBias = 0;
    int32_t effectiveEndLba = 0;
    int32_t currentLba = 0;
    int32_t previousClock = 0;
    uint32_t pollCount = 0;
    bool discImageDirectoryAuthority = false;
    bool runtimeInputBound = false;
    bool psxCdClockAuthority = false;
    bool hostClockAuthority = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrLowerCdClockTick801C4350 {
    bool known = false;
    bool polled = false;
    int32_t currentLba = 0;
    int32_t relativeClock = 0;
    int32_t deadlineLba = 0;
    bool endCheckReturn = false;
    StrClockPollState801C4350 poll{};
    bool terminal = false;
};

struct StrWorkBaseRuntime80049428 {
    bool initialized = false;
    uint32_t sourceAddress = kDword80049428;
    bool dword80049428Known = false;
    int32_t dword80049428 = 0;
    bool byte80057119Known = false;
    uint8_t byte80057119 = 0;
    uint32_t clockPollCount8001A3C8 = 0u;
    uint32_t cleanupCallCount8001A280 = 0u;
    bool scusLoadedInitialZeroAuthority = false;
    bool currentScusSemanticAuthority = false;
    bool currentComod0CallerAuthority = false;
    bool directLowerCdStateAuthority = false;
    bool hardwareCallbackAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrCleanupResult8001A280 {
    bool known = false;
    bool called = false;
    uint32_t sourceFunction = kFn8001A280;
    bool dword80049428Known = false;
    int32_t dword80049428 = 0;
    bool skippedNonZeroWorkBase = false;
    bool commandWrapperCalled80036678 = false;
    uint8_t command80036678 = 0u;
    bool commandWrapperSucceeded80036678 = false;
    bool commandSinkCalled800375BC = false;
    bool commandSinkSkipWait800375BC = false;
    bool byte80057119Written = false;
    uint8_t byte80057119 = 0u;
    bool directCommandSinkAuthority = false;
    bool hardwareCallbackAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrWorkBaseReadResult8001A3B8 {
    bool known = false;
    bool called = false;
    uint32_t sourceFunction = kFn8001A3B8;
    uint32_t sourceAddress = kDword80049428;
    bool valueKnown = false;
    int32_t value = 0;
    bool returnsOne = false;
    bool readOnly = false;
    bool currentScusSemanticAuthority = false;
    bool currentComod0CallerAuthority = false;
    bool directLowerCdStateAuthority = false;
    bool hardwareCallbackAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrCdSyncCallbackRuntime800570F8 {
    bool initialized = false;
    bool callbackKnown = false;
    uint32_t callback = 0u;
    bool scusLoadedInitialZeroAuthority = false;
    bool currentScusSemanticAuthority = false;
    bool directCallbackStateAuthority = false;
    bool hardwareCallbackTimingAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrCdSyncCallbackSwapResult80036510 {
    bool known = false;
    bool called = false;
    uint32_t sourceFunction = kFn80036510;
    uint32_t sourceAddress = kDword800570F8;
    uint32_t requestedCallback = 0u;
    bool priorCallbackKnown = false;
    uint32_t priorCallback = 0u;
    bool callbackWritten = false;
    bool readBeforeWrite = false;
    bool currentScusSemanticAuthority = false;
    bool directCallbackStateAuthority = false;
    bool hardwareCallbackTimingAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrCommand8PauseResult800367A4 {
    bool known = false;
    bool called = false;
    uint32_t sourceFunction = kFn800367A4;
    uint8_t command = 8u;
    bool commandParameterTableSlotZero = false;
    bool commandResponseTableSlotZero = false;
    bool commandCallbackTableSlotZero = false;
    StrCdSyncCallbackSwapResult80036510 clearCallbackBeforeCommand{};
    StrCdSyncCallbackSwapResult80036510 restoreCallbackBeforeSend{};
    bool commandSinkCalled800375BC = false;
    bool commandLatchWritten80057119 = false;
    bool syncWaitCalled80037070 = false;
    uint8_t syncState800573D4 = 0u;
    bool responseWritten80049414 = false;
    std::array<uint8_t, 8> response80049414{};
    bool pauseStreamingStateCommitted = false;
    uint32_t attemptCount = 0u;
    int32_t returnValue = 0;
    bool exactSoftwareCallOrder = false;
    bool currentScusSemanticAuthority = false;
    bool directLowerCdStateAuthority = false;
    bool directCdCommandStateAuthority = false;
    bool psxHardwareMmioAuthority = false;
    bool hardwareCallbackTimingAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrWaitCleanupResult8001A694 {
    bool known = false;
    bool called = false;
    uint32_t sourceFunction = kFn8001A694;
    StrCommand8PauseResult800367A4 command8Pause{};
    StrCdSyncCallbackSwapResult80036510 clearSyncCallback{};
    bool exactCallOrder = false;
    bool executionAccepted = false;
    bool currentScusSemanticAuthority = false;
    bool directLowerCdStateAuthority = false;
    bool directCdCommandStateAuthority = false;
    bool hardwareCallbackTimingAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrSerialVolumeResult8001A4A4 {
    bool known = false;
    bool called = false;
    uint32_t sourceFunction = kFn8001A4A4;
    int32_t argument = 0;
    bool argumentComparedWithOne = false;
    uint8_t mappedVolume = 0u;
    bool wrapperCalled8001A478 = false;
    uint32_t wrapperFunction8001A478 = kFn8001A478;
    uint8_t serialChannel8002AB24 = 0u;
    int16_t leftVolume8002AB24 = 0;
    int16_t rightVolume8002AB24 = 0;
    uint16_t attributeMask8002AB24 = 0u;
    uint16_t scaledLeftVolume8002AB24 = 0u;
    uint16_t scaledRightVolume8002AB24 = 0u;
    bool serialAttributeCommandBuilt8002AB24 = false;
    bool lowerCommonAttributeCalled8002A6FC = false;
    bool exactSoftwareCallOrder = false;
    bool executionAccepted = false;
    bool currentScusSemanticAuthority = false;
    bool currentComod0CallerAuthority = false;
    bool directAudioCommandStateAuthority = false;
    bool psxSpuHardwareAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrNullSubResult80024CF0 {
    bool known = false;
    bool called = false;
    uint32_t sourceFunction = kFn80024CF0;
    uint32_t contextAddress = 0u;
    bool returnInstructionObserved = false;
    bool delaySlotNopObserved = false;
    bool contextUnmodified = false;
    bool exactNoOp = false;
    bool executionAccepted = false;
    bool currentScusSemanticAuthority = false;
    bool currentComod0CallerAuthority = false;
    bool directNoOpAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrCleanupTailPrefixResult801C455C {
    bool known = false;
    bool called = false;
    uint32_t sourceFunction = kFn801C455C;
    bool stopReset80027664RequiredBeforeCall = true;
    bool stopReset80027664Observed = false;
    StrSerialVolumeResult8001A4A4 muteSerialVolume{};
    StrWaitCleanupResult8001A694 secondWaitCleanup{};
    StrNullSubResult80024CF0 contextNoOp{};
    bool nextDisplayMovePending8001B120 = false;
    bool exactCallOrder = false;
    bool executionAccepted = false;
    bool currentScusSemanticAuthority = false;
    bool currentComod0CallerAuthority = false;
    bool directSoftwareStateAuthority = false;
    bool psxSpuHardwareAuthority = false;
    bool psxGpuDmaAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrVramPage8001B120 {
    bool contentKnown = false;
    uint32_t uploadCount8002756C = 0u;
    uint32_t moveWriteCount8001B120 = 0u;
    uint64_t rgbaFnv1a64 = 0u;
    std::vector<uint32_t> rgbaPixels{};
};

struct StrVramPageRuntime8001B120 {
    bool initialized = false;
    uint16_t width = kStrVramPageWidth8001B120;
    uint16_t height = kStrVramPageHeight8001B120;
    uint32_t moveCount8001B120 = 0u;
    std::array<StrVramPage8001B120, 2> pages{};
    bool currentScusSemanticAuthority = false;
    bool directVramPixelStateAuthority = false;
    bool psxGpuMmioAuthority = false;
    bool hardwareCompletionTimingAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

// 8001B1B0 is the bootstrap all-clear used by 8001C470. On PSX it clears
// the complete 320x480 double-buffer area before the first MOVIE0 transfer;
// SS0 keeps the same semantic page state without writing host/PSX MMIO.
struct StrVramClearResult8001B1B0 {
    bool known = false;
    uint32_t sourceFunction = 0x8001B1B0u;
    int32_t red = 0;
    int32_t green = 0;
    int32_t blue = 0;
    uint16_t width = 0u;
    uint16_t height = 0u;
    bool completeDoubleBuffer = false;
    bool directVramPixelStateAuthority = false;
    bool psxGpuMmioAuthority = false;
    bool hardwareCompletionTimingAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
    bool executionAccepted = false;
};

struct StrVramPagePublishResult8002756C {
    bool known = false;
    bool executionAccepted = false;
    uint32_t sourceFunction = kFn8002756C;
    uint16_t pageIndex = 0u;
    uint32_t pixelCount = 0u;
    uint32_t sourcePixelCount = 0u;
    uint64_t sourceRgbaFnv1a64 = 0u;
    uint64_t rgbaFnv1a64 = 0u;
    int32_t destinationX = 0;
    int32_t destinationY = 0;
    bool partialPageUpload = false;
    bool exactFullPageShape = false;
    bool directVramPixelStateAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrDisplayMoveResult8001B120 {
    bool known = false;
    bool called = false;
    uint32_t sourceFunction = kFn8001B120;
    int32_t argument = 0;
    bool drawBufferRead8004019C = false;
    uint16_t drawBufferSlot80096590 = 0u;
    uint16_t sourcePage = 0u;
    uint16_t destinationPage = 0u;
    int16_t sourceX = 0;
    int16_t sourceY = 0;
    int16_t width = 0;
    int16_t height = 0;
    uint16_t destinationX = 0u;
    uint16_t destinationY = 0u;
    uint64_t sourceRgbaFnv1a64 = 0u;
    uint64_t destinationRgbaFnv1a64 = 0u;
    uint32_t pixelCountCopied = 0u;
    PrPsxDmaSubmitDirect::PsxMoveImageResult80044E2C dma{};
    bool exact8001B120Branch = false;
    bool softwareVramMoveExecuted = false;
    bool executionAccepted = false;
    bool currentScusSemanticAuthority = false;
    bool currentComod0CallerAuthority = false;
    bool directVramPixelStateAuthority = false;
    bool directGpuPacketStateAuthority = false;
    bool translatedGpuDmaStateAuthority = false;
    bool translatedGpuCompletionAuthority = false;
    bool psxGpuMmioAuthority = false;
    bool hardwareCompletionTimingAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrPlayAndWaitReturnResult801C455C {
    bool known = false;
    uint32_t sourceFunction = kFn801C455C;
    int32_t callerMode = 0;
    bool activeCallerModeZero = false;
    bool returnCarrierS3InitializedZero801C4638 = false;
    bool returnCarrierS3UnmodifiedAfterInitialization = false;
    bool returnMoveV0FromS3Observed801C4754 = false;
    bool exactEpilogueObserved = false;
    int32_t returnValue = -1;
    bool currentComod0SemanticAuthority = false;
    bool directReturnStateAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrOuterPostWaitResult801C4DC4 {
    bool known = false;
    bool called = false;
    uint32_t sourceFunction = kFn801C4DC4;
    uint32_t playAndWaitCallsite = kCallsite801C4E4C;
    StrPlayAndWaitReturnResult801C455C playAndWaitReturn{};
    uint32_t secondDisplayMoveCallsite = kCallsite801C4E54;
    StrDisplayMoveResult8001B120 secondDisplayMove{};
    uint32_t fastTransitionCallsite = kCallsite801C4E70;
    uint32_t fastTransitionFunction = 0x800201ACu;
    uint32_t fastTransitionContext = kScene0WorkAddress;
    int32_t fastTransitionMode = 5;
    int32_t fastTransitionPreArg = 1;
    int32_t fastTransitionPostArg = 2;
    bool fastTransitionPending = false;
    uint32_t word800916D2WriteCallsite = kCallsite801C4E74;
    uint16_t word800916D2Value = 1u;
    bool word800916D2WritePending = false;
    bool exactCallerOrder = false;
    bool currentComod0SemanticAuthority = false;
    bool directSoftwareStateAuthority = false;
    bool psxGpuMmioAuthority = false;
    bool hardwareCompletionTimingAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrRootCallbackRuntime80055F78 {
    bool initialized = false;
    bool enabledKnown = false;
    bool enabled = false;
    bool interruptStatusKnown = false;
    uint16_t interruptStatus = 0u;
    bool interruptMaskKnown = false;
    uint16_t interruptMask = 0u;
    bool dmaControlKnown = false;
    uint32_t dmaControl = 0u;
    bool savedInterruptMaskKnown = false;
    uint16_t savedInterruptMask80055FAA = 0u;
    bool savedDmaControlKnown = false;
    uint32_t savedDmaControl80055FAC = 0u;
    bool entryIntHookedKnown = false;
    bool entryIntHooked = false;
    bool criticalSectionHeldKnown = false;
    bool criticalSectionHeld = false;
    bool scusLoadedInitialZeroAuthority = false;
    bool currentScusSemanticAuthority = false;
    bool directSoftwareStateAuthority = false;
    bool psxHardwareMmioAuthority = false;
    bool hardwareCallbackTimingAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrRootCallbackResetResult800358DC {
    bool known = false;
    bool called = false;
    uint32_t sourceFunction = kFn800358DC;
    uint32_t callerFunction800473EC = kFn800473EC;
    uint32_t wrapperFunction80035744 = kFnResetCallback;
    uint32_t functionTableSlot80056FEC =
        kResetCallbackTableSlot80056FEC;
    uint32_t functionTableTarget800358DC = kFn800358DC;
    bool enabledBeforeKnown = false;
    bool enabledBefore = false;
    bool noOpAlreadyEnabled = false;
    bool interruptMaskCleared = false;
    bool interruptStatusCleared = false;
    bool dmaControlInitialized = false;
    bool callbackStateInitialized80035E28 = false;
    bool setjmpInitialReturnZero80047F5C = false;
    bool unexpectedInterruptDispatchSkipped800359B8 = false;
    bool entryIntHooked = false;
    bool enabledWritten = false;
    bool defaultVSyncCallbackQueried80035E54 = false;
    bool defaultDmaCallbackQueried80035F7C = false;
    bool memoryConfigurationCalled80048960 = false;
    bool exitCriticalSectionExecuted = false;
    uint32_t returnValue = 0u;
    bool exactSoftwareCallOrder = false;
    bool executionAccepted = false;
    bool currentScusSemanticAuthority = false;
    bool directSoftwareStateAuthority = false;
    bool psxHardwareMmioAuthority = false;
    bool hardwareCallbackTimingAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrStopCallbackResult80035838 {
    bool known = false;
    bool called = false;
    uint32_t sourceFunction = kFn80035838;
    uint32_t functionTableSlot80056FF0 =
        kStopCallbackTableSlot80056FF0;
    uint32_t functionTableTarget80035CF4 = kFn80035CF4;
    bool enabledBeforeKnown = false;
    bool enabledBefore = false;
    bool noOpAlreadyDisabled = false;
    bool enterCriticalSectionExecuted = false;
    bool interruptMaskSaved80055FAA = false;
    bool dmaControlSaved80055FAC = false;
    bool interruptMaskCleared = false;
    bool interruptStatusClearedFromMask = false;
    bool dmaControlMasked77777777 = false;
    bool resetEntryIntExecuted = false;
    bool enabledCleared = false;
    uint32_t returnValue = 0u;
    bool exactSoftwareCallOrder = false;
    bool executionAccepted = false;
    bool currentScusSemanticAuthority = false;
    bool directSoftwareStateAuthority = false;
    bool psxHardwareMmioAuthority = false;
    bool hardwareCallbackTimingAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrPostUploadDecision801C455C {
    bool known = false;
    uint32_t sourceFunction = kFn801C455C;
    StrWorkBaseReadResult8001A3B8 workBaseRead{};
    bool playerTickContinuesKnown = false;
    bool playerTickContinues = false;
    bool specialWorkBaseExit = false;
    bool callWaitCleanup8001A694 = false;
    bool callStopCallback80035838 = false;
    bool specialCleanupDirectExecutable = false;
    bool gapSpecialCleanupNotYetDirect = false;
    bool continueLoop = false;
    bool terminateFromPlayerTick = false;
    bool exactBranchOrder = false;
    bool currentScusSemanticAuthority = false;
    bool currentComod0CallerAuthority = false;
    bool directLowerCdStateAuthority = false;
    bool directPlayerTickAuthority = false;
    bool hostProjectionResultIgnored = false;
    bool hardwareCallbackAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrPlayerTick801C448C {
    bool known = false;
    uint32_t sourceFunction = kFn801C448C;
    bool vblankWaitCalled80035560 = false;
    int32_t vblankWaitMode80035560 = 0;
    bool clockPollCalled8001A3C8 = false;
    int32_t currentLba = 0;
    bool lbaToMsfCalled80036974 = false;
    bool syncFeedbackKnown800364D0 = false;
    int32_t syncReturn800364D0 = 0;
    uint8_t syncMinuteBcd = 0u;
    uint8_t syncSecondBcd = 0u;
    uint8_t syncFrameBcd = 0u;
    bool readyStatusKnown800363A4 = false;
    uint8_t readyStatus800363A4 = 0u;
    bool clockPollAccepted8001A3C8 = false;
    bool dword80049428Known = false;
    int32_t dword80049428 = 0;
    StrCleanupResult8001A280 cleanup{};
    StrLowerCdClockTick801C4350 clock{};
    bool exactCallOrder = false;
    bool currentScusSemanticAuthority = false;
    bool currentComod0CallerAuthority = false;
    bool directLowerCdStateAuthority = false;
    bool hardwareCallbackAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrStopResetState80027664 {
    bool known = false;
    bool clearDmaCallbackCalled80047658 = false;
    int32_t clearDmaCallbackArgument = 0;
    uint32_t clearDmaCallbackChannel = 0;
    bool stopCdCalled800392C0 = false;
    uint32_t memoryResetCallCount80025AF8 = 0;
    bool exactCallOrder = false;
    bool runtimeWasActive = false;
    bool executionSkippedInactive = false;
    bool executionAccepted = false;
    bool dmaCallbackClearExecuted = false;
    bool enterCriticalSectionExecuted = false;
    bool cdDataCallbackClearExecuted80036930 = false;
    bool cdReadyCallbackClearExecuted80036528 = false;
    bool cdromReg0ClearExecuted = false;
    bool cdromReg3ClearExecuted = false;
    bool exitCriticalSectionExecuted = false;
    bool cdStopExecuted = false;
    std::array<uint32_t, 4> memoryResetPointerSlotOrder{};
    uint32_t memoryResetCountExecuted = 0;
    uint64_t memoryBytesReleased = 0;
    bool memoryResetExecuted = false;
    bool sectorRingReset = false;
    bool directDmaCallbackStateAuthority = false;
    bool directCdStateAuthority = false;
    bool directMemoryStackAuthority = false;
    bool psxPointerAuthority = false;
    bool psxHardwareMmioAuthority = false;
    bool hardwareCallbackTimingAuthority = false;
    bool hostProjection = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct TitleEarlyInputWaitCleanupHostProjectionInput8001A694 {
    bool playerPresent = false;
    bool strStarted = false;
    bool playerActive = false;
};

struct TitleEarlyInputWaitCleanupHostProjection8001A694 {
    bool available = false;
    bool hostProjection = false;
    bool hostStrStopped = false;
    bool psxCdAuthority = false;
    bool psxCommand8WaitKnown = false;
    bool psxCallbackClearKnown = false;
};

struct Scene0StrAudioRouteSource801C44E0 {
    bool rowKnown = false;
    bool opaque04Known = false;
    uint32_t opaque04 = 0u;
};

struct Scene0StrAudioRoute801C44E0 {
    bool known = false;
    bool hostProjection = false;
    bool psxCdAuthority = false;
    bool currentScusSemanticAuthority = false;
    bool currentComod0CallerAuthority = false;
    bool volumeKnown = false;
    int16_t volume8001A478 = 0;
    uint32_t volumeCommand8002AB24 = kFn8002AB24;
    uint32_t volumeMask8002AB24 = 0;
    int32_t scaledLeft8002AB24 = 0;
    int32_t scaledRight8002AB24 = 0;
    bool directVolumeCommandAuthority = false;
    bool psxSpuHardwareAuthority = false;
    float normalizedVolume = 1.0f;
    bool filterKnown = false;
    uint8_t filterFile8004940C = 0u;
    uint8_t filterChannel8004940D = 0u;
    uint8_t filterCommand8001A654 = 0u;
    bool filterCommandSerialKnown = false;
    bool directFilterCommandAuthority = false;
    bool replayValueAuthority = false;
    bool oldWinS0Authority = false;
    bool stage2PlusAuthority = false;
    bool comod2Authority = false;
};

struct StrAction {
    StrActionKind kind = StrActionKind::None;
    uint32_t psxFunction = 0;
    uint32_t arg0 = 0;
    uint32_t arg1 = 0;
    uint32_t arg2 = 0;
    uint32_t arg3 = 0;
    int32_t repeatCount = 1;
    bool repeatKnown = true;
    bool conditional = false;
};

struct StrPlan {
    StrPlanKind kind = StrPlanKind::Unknown;
    bool runtimeCutoverAllowed = false;
    bool hasOpenP0Gap = true;
    StrAction actions[96]{};
    uint32_t count = 0;
    bool truncated = false;
};

bool RuntimeCutoverAllowed();
TitleEarlyInputWaitCleanupHostProjection8001A694
BuildTitleEarlyInputWaitCleanupHostProjection8001A694(
    const TitleEarlyInputWaitCleanupHostProjectionInput8001A694& input);
Scene0StrAudioRoute801C44E0 BuildScene0StrAudioRoute801C44E0(
    const Scene0StrAudioRouteSource801C44E0& source);
StrDecodeGeometry KnownDecodeGeometry80027288(int32_t movieKind,
                                              bool word800916DC);
StrDecoderTableRow80054614 KnownDecoderTableRow80054614(
    int32_t movieKind);
bool TryInitializeStrDecoderState80027288(
    int32_t movieKind,
    bool word800916DC,
    StrDecoderAllocationState80027288& state);
bool TryInitializeStrDecoderMemoryRuntime80027288(
    const StrDecoderAllocationState80027288& source,
    StrDecoderMemoryRuntime80027288& runtime);
bool TryStartStrStreamState800274D4(
    const StrDecoderAllocationState80027288& decoderState,
    StrStreamStartState800274D4& state);
bool TryInitializeStrLowerCdStartRuntime8001A4D0(
    const StrLowerCdStartSource8001A4D0& source,
    StrLowerCdStartRuntime8001A4D0& state);
bool TryInitializeStrXaAudioDiscRuntime8001A4D0(
    const StrLowerCdStartRuntime8001A4D0& start,
    StrXaAudioDiscRuntime8001A4D0& runtime);
StrXaAudioSectorResult8001A4D0 ReadNextStrXaAudioSectorFromDisc8001A4D0(
    const StrLowerCdStartRuntime8001A4D0& start,
    uint32_t maxRawSectorCursor,
    StrXaAudioDiscRuntime8001A4D0& runtime);
StrDecodeAdvanceState800273A4 BuildStrDecodeAdvanceState800273A4(
    const StrStreamStartState800274D4& streamState,
    const StrDecodeAdvanceInput800273A4& input);
StrDecodeGateState80027528 BuildStrDecodeGateState80027528(
    const StrDecoderControlState80027288& control);
StrStripUploadState8002756C BuildStrStripUploadState8002756C(
    const StrDecoderControlState80027288& control,
    uint32_t sectorCounter,
    bool displayBufferResultKnown,
    int32_t displayBufferResult8004019C);
StrStripUploadExecution8002756C ExecuteStrStripUpload8002756C(
    const StrStripUploadState8002756C& plan,
    const uint8_t* mdecVerticalOutput,
    size_t mdecVerticalOutputBytes);
StrClockPollState801C4350 BuildStrClockPollState801C4350(
    const StrClockPollInput801C4350& input);
bool TryInitializeStrLowerCdClockRuntime801C4350(
    const StrLowerCdClockSource801C4350& source,
    StrLowerCdClockRuntime801C4350& runtime);
StrLowerCdClockTick801C4350 StepStrLowerCdClockRuntime801C4350(
    StrLowerCdClockRuntime801C4350& runtime);
bool TryInitializeStrWorkBaseRuntime80049428(
    StrWorkBaseRuntime80049428& runtime);
bool PublishStrLowerCdStartCommand8001A4D0(
    const StrLowerCdStartRuntime8001A4D0& start,
    StrWorkBaseRuntime80049428& runtime);
bool PublishStrDecoderStreamingStart8001A4D0(
    const StrLowerCdStartRuntime8001A4D0& start,
    StrDecoderMemoryRuntime80027288& runtime);
StrSectorRingPrimeResult80039670 PrimeStrSectorRingFromDisc80039670(
    const StrLowerCdStartRuntime8001A4D0& start,
    StrDecoderMemoryRuntime80027288& runtime);
StrSectorRingAcquireResult8003958C AcquireStrSectorRingFrame8003958C(
    StrDecoderMemoryRuntime80027288& runtime);
StrSectorRingReleaseResult80039490 ReleaseStrSectorRingFrame80039490(
    uint32_t payloadHandle,
    StrDecoderMemoryRuntime80027288& runtime);
StrVlcDecodeReleaseResult800273A4 ExecuteStrVlcDecodeAndRelease800273A4(
    uint32_t payloadHandle,
    StrDecoderMemoryRuntime80027288& runtime);

StrMdecCommandOutputSubmission800273A4
ExecuteStrMdecCommandOutputSubmission800273A4(
    const StrDecoderControlState80027288& control,
    const StrVlcDecodeReleaseResult800273A4& vlcDecodeRelease,
    StrDecoderMemoryRuntime80027288& runtime);
StrMdecDmaCompletionState80027220 ExecuteStrMdecDmaCompletion80027220(
    const StrDecoderControlState80027288& control,
    const StrMdecCommandOutputSubmission800273A4& submission,
    const PrSS0MdecOutputDirect::Mdec15bppResult& output,
    const StrDecoderMemoryRuntime80027288& runtime);
StrCleanupResult8001A280 ApplyStrCleanup8001A280(
    StrWorkBaseRuntime80049428& runtime);
StrWorkBaseReadResult8001A3B8 ReadStrWorkBase8001A3B8(
    const StrWorkBaseRuntime80049428& runtime);
StrCdSyncCallbackRuntime800570F8
InitializeStrCdSyncCallbackRuntime800570F8();
StrCdSyncCallbackSwapResult80036510 SetStrCdSyncCallback80036510(
    StrCdSyncCallbackRuntime800570F8& runtime,
    uint32_t callback);
StrCommand8PauseResult800367A4 ExecuteStrCommand8Pause800367A4(
    StrDecoderMemoryRuntime80027288& decoder,
    StrCdSyncCallbackRuntime800570F8& callback,
    StrWorkBaseRuntime80049428& workBase);
StrWaitCleanupResult8001A694 ExecuteStrWaitCleanup8001A694(
    StrDecoderMemoryRuntime80027288& decoder,
    StrCdSyncCallbackRuntime800570F8& callback,
    StrWorkBaseRuntime80049428& workBase);
StrSerialVolumeResult8001A4A4 ExecuteStrSerialVolume8001A4A4(
    int32_t argument);
StrNullSubResult80024CF0 ExecuteStrNullSub80024CF0(
    uint32_t contextAddress);
StrCleanupTailPrefixResult801C455C ExecuteStrCleanupTailPrefix801C455C(
    StrDecoderMemoryRuntime80027288& decoder,
    StrCdSyncCallbackRuntime800570F8& callback,
    StrWorkBaseRuntime80049428& workBase,
    uint32_t contextAddress);
StrVramPageRuntime8001B120 InitializeStrVramPageRuntime8001B120();
StrVramClearResult8001B1B0 ClearStrVramPages8001B1B0(
    StrVramPageRuntime8001B120& runtime,
    int32_t red,
    int32_t green,
    int32_t blue);
StrVramPagePublishResult8002756C PublishStrVramPageUpload8002756C(
    StrVramPageRuntime8001B120& runtime,
    uint16_t pageIndex,
    const StrStripUploadExecution8002756C& upload);
StrDisplayMoveResult8001B120 ExecuteStrDisplayMove8001B120(
    int32_t argument,
    uint16_t drawBufferSlot80096590,
    bool interruptMaskKnown,
    uint16_t interruptMask,
    StrVramPageRuntime8001B120& runtime);
StrPlayAndWaitReturnResult801C455C
ResolveStrPlayAndWaitMode0Return801C455C();
StrOuterPostWaitResult801C4DC4 ExecuteStrOuterPostWait801C4DC4(
    uint16_t drawBufferSlot80096590,
    bool interruptMaskKnown,
    uint16_t interruptMask,
    StrVramPageRuntime8001B120& runtime);
StrRootCallbackRuntime80055F78
InitializeStrRootCallbackRuntime80055F78();
StrRootCallbackResetResult800358DC ExecuteStrRootCallbackReset800358DC(
    StrRootCallbackRuntime80055F78& runtime);
StrStopCallbackResult80035838 ExecuteStrStopCallback80035838(
    StrRootCallbackRuntime80055F78& runtime);
StrPostUploadDecision801C455C ResolveStrPostUploadDecision801C455C(
    const StrWorkBaseRuntime80049428& runtime,
    bool requiredFlushExecuted8001ED3C,
    bool playerTickContinuesKnown,
    bool playerTickContinues);
StrPlayerTick801C448C StepStrPlayerTick801C448C(
    StrWorkBaseRuntime80049428& workBase,
    StrLowerCdClockRuntime801C4350& clock);
bool UsesStrLowerCdClockRuntime801C4350(StrMovieKind movieKind);
StrStopResetState80027664 BuildStrStopResetState80027664();
StrStopResetState80027664 ExecuteStrStopReset80027664(
    StrDecoderMemoryRuntime80027288& runtime);
bool IsKnownExitInput80035510(uint32_t inputMask);
int32_t KnownReturnValueForInput80035510(uint32_t inputMask);

StrPlan BuildStrStart801C44E0Plan(uint32_t segmentAddress,
                                  int32_t movieKind,
                                  bool word800916DC);
StrPlan BuildStrLoop801C455CPlan(uint32_t segmentAddress,
                                 uint32_t ctxAddress,
                                 int32_t mode,
                                 int32_t movieKind,
                                 bool word800916DC,
                                 bool inputMaskKnown,
                                 uint32_t inputMask);
StrPlan BuildStrCleanup801C455CPlan(bool includeInsideDisplayUpload);
StrPlan BuildScene0Movie0PlaybackPlan(uint32_t sceneEntryBase,
                                      bool word800916DC,
                                      bool includeInsideDisplayUpload);
StrPlan BuildTitleMovie0TStartPlan(uint32_t sceneEntryBase,
                                   bool word800916DC);

const char* StrPlanKindName(StrPlanKind kind);
const char* StrMovieKindName(StrMovieKind kind);
const char* StrActionKindName(StrActionKind kind);

} // namespace PrSS0StrLifecycleDirect
