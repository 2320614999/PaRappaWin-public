#include "pr_stage1_lifecycle_host_adapter_801c81ec.h"
#include "pr_stage_scene_submit_backend.h"

#include "logger.h"
#include "pr_game_context.h"
#include "pr_pad.h"
#include "pr_main.h"
#include "pr_psx_pad_direct.h"
#include "pr_psx_event_frame_direct.h"
#include "pr_psx_vsync_direct.h"
#include "pr_sfx.h"
#include "pr_sqevs1.h"
#include "pr_ss0_event_frame_loop_direct.h"
#include "pr_ss0_card_image_storage_direct.h"
#include "pr_ss0_resource_audio_direct.h"
#include "pr_ss0_scene0_runtime_direct.h"
#include "pr_stage1_bootstrap_cd_request_direct.h"
#include "pr_stage_runner.h"
#include "pr_stage_scene_submit_runtime_private.h"
#include "pr_stage1_loader_cd_hal.h"
#include "pr_stage1_lower_cd_producer_direct.h"
#include "pr_stage1_loader_producer_adapter.h"
#include "pr_stage1_movie_segment_direct.h"
#include "pr_stage1_save_card_hal_direct.h"
#include "pr_stage1_save_ui_host_bridge_direct.h"
#include "pr_stage1_save_ui_direct.h"
#include "pr_stage_status_bank_host_bridge_direct.h"
#include "pr_stage_status_bank_direct.h"
#include "pr_transition.h"
#include "str_player.h"

#include <array>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <string_view>
#include <system_error>
#include <vector>

namespace PrStage1LifecycleHostAdapter801C81EC {
namespace {

constexpr uint32_t kFn8001EF14 = 0x8001EF14u;
constexpr uint32_t kFn80015744 = 0x80015744u;
constexpr uint32_t kFn8001635C = 0x8001635Cu;
constexpr uint32_t kFn80020008 = 0x80020008u;
constexpr uint32_t kFn80020090 = 0x80020090u;
constexpr uint32_t kFn80020110 = 0x80020110u;
constexpr uint32_t kFn800201AC = 0x800201ACu;
constexpr uint32_t kFn8001A4D0 = 0x8001A4D0u;
constexpr uint32_t kFn80036540 = 0x80036540u;
constexpr uint32_t kFn80017594 = 0x80017594u;
constexpr uint32_t kFn80017A10 = 0x80017A10u;
constexpr uint32_t kFn80017B18 = 0x80017B18u;
constexpr uint32_t kCdCommandTimeoutState80088310 = 0x80088310u;
constexpr uint32_t kCdCommandTimeoutSpin80088314 = 0x80088314u;
constexpr uint32_t kFn80017B60 = 0x80017B60u;
constexpr uint32_t kFn80026B94 = 0x80026B94u;
constexpr uint32_t kFn80026ECC = 0x80026ECCu;
constexpr uint32_t kFn80026EF8 = 0x80026EF8u;
constexpr uint32_t kFn80026FA4 = 0x80026FA4u;
constexpr uint32_t kFn80035898 = 0x80035898u;
constexpr uint32_t kFn800359B8 = 0x800359B8u;
constexpr uint32_t kFn80036AF8 = 0x80036AF8u;
constexpr uint32_t kFn801C44E0 = 0x801C44E0u;
constexpr uint32_t kFn801C455C = 0x801C455Cu;
constexpr uint32_t kFn801CB67C = 0x801CB67Cu;
constexpr uint32_t kFn801C7A60 = 0x801C7A60u;
constexpr uint32_t kInitialMovie1LoaderOffset = 0x6Cu;
constexpr uint32_t kStageLoopLoaderOffset = 0x9Cu;
constexpr uint32_t kClearTailGoodLoaderOffset = 0xCCu;
constexpr uint32_t kClearTailCoolLoaderOffset = 0xFCu;
constexpr uint32_t kCdCallbackPendingWord80055F7A = 0x80055F7Au;
constexpr uint32_t kCdCallbackState80055F78 = 0x80055F78u;
constexpr uint32_t kCdCallbackStateBytes80055F78 = 0x34u;
constexpr uint32_t kCdInterruptRegs1F801070 = 0x1F801070u;
constexpr uint32_t kCdInterruptRegsBytes1F801070 = 8u;
constexpr uint32_t kCdInterruptWatchdog80057010 = 0x80057010u;
constexpr uint32_t kCdInterruptWatchdogBytes80057010 = 4u;
constexpr uint32_t kCdStatusCounter80057110 = 0x80057110u;
constexpr uint32_t kSavePayloadBank80092F10 = 0x80092F10u;
constexpr int32_t kSaveUiWriteRetryCount80017A10 = 4;
constexpr uint32_t kSaveUiFormatArg0_80017B60 = 0x8006EABCu;
constexpr uint32_t kSaveUiDirectoryRawBank8007A318 = 0x8007A318u;
constexpr uint32_t kSaveUiDirectoryRawBankBytes80019458 = 600u;
constexpr uint32_t kSaveUiWriteBlock8007ABE8 = 0x8007ABE8u;
constexpr uint32_t kSaveUiWriteBlockBytes80017A10 = 0x2000u;
constexpr int32_t kSaveUiDirectCardWriteFd80017454 = 2;

enum class CdLowerFirstMissingCode801C81EC : uint8_t {
    Unknown = 0,
    None = 1,
    Sector = 2,
    Dst = 3,
    Mode = 4,
    SavedSync = 5,
    SavedReady = 6,
    Status = 7,
    Clock = 8,
    PreSeek = 9,
    ClearSync = 10,
    ClearReady = 11,
    ShellOpen = 12,
    PreRead = 13,
    ModeCmd = 14,
    Loc = 15,
    Active = 16,
    StartRead = 17,
};

uint8_t CdLowerFirstMissingCodeFromName801C81EC(const char* name) {
    if (!name) {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::Unknown);
    }
    const std::string_view value{name};
    if (value == "none") {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::None);
    }
    if (value == "sector") {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::Sector);
    }
    if (value == "dst") {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::Dst);
    }
    if (value == "mode") {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::Mode);
    }
    if (value == "savedSync") {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::SavedSync);
    }
    if (value == "savedReady") {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::SavedReady);
    }
    if (value == "status") {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::Status);
    }
    if (value == "clock") {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::Clock);
    }
    if (value == "preSeek") {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::PreSeek);
    }
    if (value == "clearSync") {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::ClearSync);
    }
    if (value == "clearReady") {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::ClearReady);
    }
    if (value == "shellOpen") {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::ShellOpen);
    }
    if (value == "preRead") {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::PreRead);
    }
    if (value == "modeCmd") {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::ModeCmd);
    }
    if (value == "loc") {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::Loc);
    }
    if (value == "active") {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::Active);
    }
    if (value == "startRead") {
        return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::StartRead);
    }
    return static_cast<uint8_t>(CdLowerFirstMissingCode801C81EC::Unknown);
}
constexpr uint32_t kSaveUiFormatArg1_80017B60 = 0x8006EAC0u;
constexpr int32_t kSaveUiFormatRetryCount80017B60 = 3;
constexpr uint32_t kCue80094410 = 0x80094410u;
constexpr uint32_t kTransitionWork801C3640 = 0x801C3640u;
constexpr uint16_t kPadCross = static_cast<uint16_t>(PrPadButton::Cross);
constexpr uint16_t kPadCircle = static_cast<uint16_t>(PrPadButton::Circle);

class ScopedRuntimePsxMemoryProviderPc801C81EC {
  public:
    ScopedRuntimePsxMemoryProviderPc801C81EC(
        PrStage1XaCdDirectRuntimePsxMemoryProvider& provider,
        uint32_t pc)
        : provider_(provider),
          oldPcKnown_(provider.pcKnown),
          oldPc_(provider.pc) {
        provider_.pcKnown = true;
        provider_.pc = pc;
    }

    ~ScopedRuntimePsxMemoryProviderPc801C81EC() {
        provider_.pcKnown = oldPcKnown_;
        provider_.pc = oldPc_;
    }

    ScopedRuntimePsxMemoryProviderPc801C81EC(
        const ScopedRuntimePsxMemoryProviderPc801C81EC&) = delete;
    ScopedRuntimePsxMemoryProviderPc801C81EC& operator=(
        const ScopedRuntimePsxMemoryProviderPc801C81EC&) = delete;

  private:
    PrStage1XaCdDirectRuntimePsxMemoryProvider& provider_;
    bool oldPcKnown_ = false;
    uint32_t oldPc_ = 0u;
};

static bool BuildBootstrap15590ReadSyncCompletionRequest801C81EC(
    const PrStage1LifecycleExecutorDirect::
        Bootstrap15590CdLowerHostRequest801C81EC& request,
    PrStage1LoaderCdHal::LowerActionRequestMetadata& out) {
    namespace CdHal = PrStage1LoaderCdHal;
    const CdHal::LowerActionRequestMetadata& readStart =
        request.lowerRequest;
    if (!readStart.known ||
        readStart.actionKind != CdHal::ActionKind::ReadStart80038FC0 ||
        readStart.callerFunction != CdHal::kFn8001A818 ||
        readStart.lowerFunction != CdHal::kFn80038FC0 ||
        readStart.finalFunction != CdHal::kFn800390C8 ||
        !readStart.readSyncRequestKnown ||
        readStart.readSyncFunction != CdHal::kFn800390C8 ||
        readStart.readSyncArg0 != CdHal::kRead8001A818SyncArg0 ||
        readStart.readSyncArg1 != CdHal::kRead8001A818SyncArg1) {
        return false;
    }

    out = CdHal::LowerActionRequestMetadata{};
    out.known = true;
    out.actionKind = CdHal::ActionKind::ReadSync800390C8;
    out.callerFunction = CdHal::kFn8001A818;
    out.lowerFunction = readStart.readSyncFunction;
    out.finalFunction = CdHal::kFn800364F0;
    out.readSyncRequestKnown = true;
    out.readSyncFunction = readStart.readSyncFunction;
    out.readSyncArg0 = readStart.readSyncArg0;
    out.readSyncArg1 = readStart.readSyncArg1;
    return true;
}

static bool TryBuildBootstrap15590FinalReadyFacts801C81EC(
    const PrGameContext& ctx,
    const PrStage1LifecycleExecutorDirect::
        Bootstrap15590CdLowerProducerRuntime801C81EC& runtime,
    const PrStage1LifecycleExecutorDirect::
        Bootstrap15590CdLowerHostRequest801C81EC& request,
    bool clockCarrierAccepted,
    const PrMovieSegmentDirect::StreamClockProducerCarrier800493F4&
        clockCarrier,
    PrStage1LowerCdProducerDirect::LowerCdProducerFacts& facts,
    const char*& firstMissing) {
    firstMissing = "none";
    const auto& xaCd = ctx.stage1XaCdDirect;
    if (!runtime.readStartHalCarrierKnown) {
        firstMissing = "readStartCarrier";
        return false;
    }
    if (runtime.readStartHalCarrierReadS27Serial == 0u ||
        runtime.readStartHalCarrierReadS27Serial != xaCd.readS27Serial) {
        firstMissing = "readStartCarrierSerial";
        return false;
    }
    if (!clockCarrierAccepted) {
        firstMissing = "clock";
        return false;
    }
    if (!runtime.readStartHalCarrierPump.globalsKnown) {
        firstMissing = "readStartPumpGlobals";
        return false;
    }
    PrStage1LoaderCdHal::LowerActionRequestMetadata readSyncRequest{};
    if (!BuildBootstrap15590ReadSyncCompletionRequest801C81EC(
            request,
            readSyncRequest)) {
        firstMissing = "readSyncCompletionRequest";
        return false;
    }
    const bool outputBufferNonNull = request.lowerRequest.readSyncArg1 != 0;
    const bool directHalReadComplete =
        request.lowerRequest.readSyncArg0 == 1 &&
        !outputBufferNonNull &&
        runtime.readStartHalCarrierPump.produced &&
        !runtime.readStartHalCarrierPump.incomplete &&
        runtime.readStartHalCarrierPump.remaining80057424 == 0;
    if (directHalReadComplete) {
        facts.request = readSyncRequest;
        facts.overlayTransferAttempt.known = true;
        facts.overlayTransferAttempt.sourceFunction =
            PrMovieSegmentDirect::kSub800154B0OverlayTransferWrapper;
        facts.overlayTransferAttempt.transferFunction =
            PrMovieSegmentDirect::kSub8001ACF8OverlayTransfer;
        facts.overlayTransferAttempt.attemptIndexKnown = true;
        facts.overlayTransferAttempt.attemptIndex = request.attemptIndex;
        if (runtime.readDstPtrKnown) {
            facts.overlayTransferAttempt.dstKnown = true;
            facts.overlayTransferAttempt.dst = runtime.readDstPtr;
        }
        if (runtime.readSectorCountKnown) {
            facts.overlayTransferAttempt.sectorCountKnown = true;
            facts.overlayTransferAttempt.sectorCount =
                static_cast<uint32_t>(runtime.readSectorCount);
        }
        facts.readSyncWait = true;
        facts.clockKnown = true;
        facts.clockNow = clockCarrier.clockLba;
        facts.startClockKnown = true;
        facts.startClock8005742C =
            runtime.readStartHalCarrierPump.startClock8005742C;
        facts.lastPumpClockKnown = true;
        facts.lastPumpClock80057428 =
            runtime.readStartHalCarrierPump.lastPumpClock80057428;
        facts.remainingKnown = true;
        facts.remaining80057424 = 0;
        facts.retryPump80038DE8 = runtime.readStartHalCarrierPump;
        if (runtime.readStartHalCarrierPayloadBytesKnown &&
            !runtime.readStartHalCarrierPayloadBytes.empty()) {
            facts.payloadBytesKnown = true;
            facts.payloadData =
                runtime.readStartHalCarrierPayloadBytes.data();
            facts.payloadSize =
                runtime.readStartHalCarrierPayloadBytes.size();
        }

        auto& ready = facts.finalReadyInput800372F0;
        ready.a0WaitModeKnown = true;
        ready.a0WaitMode = request.lowerRequest.readSyncArg0;
        ready.a1OutputBufferPtrKnown = true;
        ready.a1OutputBufferPtrNonNull = false;
        ready.timeoutCheckKnown = true;
        ready.timedOut = false;
        ready.callbackCheckKnown = true;
        ready.callbackPending = false;
        ready.byte800573D6Known = true;
        ready.byte800573D6 = 0;
        ready.byte800573D5Known = true;
        ready.byte800573D5 = 0;
        facts.finalReadyInput800372F0Known = true;
        return true;
    }
    if (!xaCd.cdCallbackPending80035898Known) {
        firstMissing = "callbackPending80035898";
        return false;
    }
    if (xaCd.cdCallbackPending80035898) {
        firstMissing = "callbackPump";
        return false;
    }
    if (!xaCd.byte_800573D6Known) {
        firstMissing = "byte800573D6";
        return false;
    }
    if (!xaCd.byte_800573D5Known) {
        firstMissing = "byte800573D5";
        return false;
    }
    if (outputBufferNonNull && xaCd.byte_800573D6 != 0u &&
        !xaCd.response_80088308Known) {
        firstMissing = "response88308";
        return false;
    }
    if (outputBufferNonNull && xaCd.byte_800573D5 != 0u &&
        !xaCd.response_80088300Known) {
        firstMissing = "response88300";
        return false;
    }

    facts.request = readSyncRequest;
    facts.overlayTransferAttempt.known = true;
    facts.overlayTransferAttempt.sourceFunction =
        PrMovieSegmentDirect::kSub800154B0OverlayTransferWrapper;
    facts.overlayTransferAttempt.transferFunction =
        PrMovieSegmentDirect::kSub8001ACF8OverlayTransfer;
    facts.overlayTransferAttempt.attemptIndexKnown = true;
    facts.overlayTransferAttempt.attemptIndex = request.attemptIndex;
    if (runtime.readDstPtrKnown) {
        facts.overlayTransferAttempt.dstKnown = true;
        facts.overlayTransferAttempt.dst = runtime.readDstPtr;
    }
    if (runtime.readSectorCountKnown) {
        facts.overlayTransferAttempt.sectorCountKnown = true;
        facts.overlayTransferAttempt.sectorCount =
            static_cast<uint32_t>(runtime.readSectorCount);
    }
    facts.readSyncWait = request.lowerRequest.readSyncArg0 != 0;
    facts.clockKnown = true;
    facts.clockNow = clockCarrier.clockLba;
    facts.startClockKnown = true;
    facts.startClock8005742C =
        runtime.readStartHalCarrierPump.startClock8005742C;
    facts.lastPumpClockKnown = true;
    facts.lastPumpClock80057428 =
        runtime.readStartHalCarrierPump.lastPumpClock80057428;
    facts.remainingKnown = true;
    facts.remaining80057424 =
        runtime.readStartHalCarrierPump.remaining80057424;
    facts.retryPump80038DE8 = runtime.readStartHalCarrierPump;
    if (runtime.readStartHalCarrierPayloadBytesKnown &&
        !runtime.readStartHalCarrierPayloadBytes.empty()) {
        facts.payloadBytesKnown = true;
        facts.payloadData = runtime.readStartHalCarrierPayloadBytes.data();
        facts.payloadSize = runtime.readStartHalCarrierPayloadBytes.size();
    }

    auto& ready = facts.finalReadyInput800372F0;
    ready.a0WaitModeKnown = true;
    ready.a0WaitMode = request.lowerRequest.readSyncArg0;
    ready.a1OutputBufferPtrKnown = true;
    ready.a1OutputBufferPtrNonNull = outputBufferNonNull;
    ready.timeoutCheckKnown = true;
    ready.timedOut =
        clockCarrier.clockLba >
        runtime.readStartHalCarrierPump.startClock8005742C +
            PrStage1LoaderCdHal::kReadSync800390C8TimeoutVblanks;
    ready.callbackCheckKnown = true;
    ready.callbackPending = xaCd.cdCallbackPending80035898;
    ready.byte800573D6Known = true;
    ready.byte800573D6 = xaCd.byte_800573D6;
    if (xaCd.response_80088308Known) {
        ready.response88308Known = true;
        for (uint32_t i = 0; i < xaCd.response_80088308.size(); ++i) {
            ready.response88308[i] = xaCd.response_80088308[i];
        }
    }
    ready.byte800573D5Known = true;
    ready.byte800573D5 = xaCd.byte_800573D5;
    if (xaCd.response_80088300Known) {
        ready.response88300Known = true;
        for (uint32_t i = 0; i < xaCd.response_80088300.size(); ++i) {
            ready.response88300[i] = xaCd.response_80088300[i];
        }
    }
    facts.finalReadyInput800372F0Known = true;
    return true;
}

PrStage1XaCdDirectCdMmioRuntimeProducerPairResult
TrySubmitBootstrap15590CdMmioSamplePair801C81EC(PrGameContext& ctx) {
    auto& xaCd = ctx.stage1XaCdDirect;
    ++xaCd.cdMmioNaturalAdapterAttemptCount;

    PrStage1XaCdDirectCdMmioRuntimeProducerSample cdReg3{};
    cdReg3.sourceAvailable =
        xaCd.rawEvent80036AF8InitialInterruptKnown &&
        xaCd.rawEvent80036AF8InitialInterruptObservationAcceptedCount != 0u;
    cdReg3.psxAddress = PrStage1LowerCdProducerDirect::kCdReg3Ptr80036AF8;
    cdReg3.byteSize = 1u;
    cdReg3.valueKnown = xaCd.rawEvent80036AF8InitialInterruptKnown;
    cdReg3.value = xaCd.rawEvent80036AF8InitialInterrupt;
    cdReg3.frameKnown = ctx.stage1RuntimePsxMemoryProvider.frameKnown;
    cdReg3.frame = ctx.stage1RuntimePsxMemoryProvider.frame;
    cdReg3.pcKnown = true;
    cdReg3.pc = kFn80036AF8;

    PrStage1XaCdDirectCdMmioRuntimeProducerSample cdReg0{};
    cdReg0.sourceAvailable = xaCd.rawEvent80036AF8CdReg0StatusKnown;
    cdReg0.psxAddress = PrStage1LowerCdProducerDirect::kCdReg0Ptr80036AF8;
    cdReg0.byteSize = 1u;
    cdReg0.valueKnown = xaCd.rawEvent80036AF8CdReg0StatusKnown;
    cdReg0.value = xaCd.rawEvent80036AF8CdReg0Status;
    cdReg0.frameKnown = ctx.stage1RuntimePsxMemoryProvider.frameKnown;
    cdReg0.frame = ctx.stage1RuntimePsxMemoryProvider.frame;
    cdReg0.pcKnown = true;
    cdReg0.pc = kFn80036AF8;

    const auto result =
        PrStage1XaCdDirectSubmitCdMmioRuntimeProducerSamplePair(
            cdReg3,
            cdReg0,
            ctx.stage1RuntimePsxMemoryProvider);
    if (!result.sourceAvailable) {
        ++xaCd.cdMmioNaturalAdapterSourceUnavailableCount;
    }
    if (result.publishAttempted) {
        ++xaCd.cdMmioNaturalAdapterPublishAttemptCount;
    }
    if (result.accepted) {
        ++xaCd.cdMmioNaturalAdapterAcceptedCount;
    }
    const auto reject =
        (result.cdReg3RejectReason !=
         PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None)
            ? result.cdReg3RejectReason
            : result.cdReg0RejectReason;
    xaCd.cdMmioNaturalAdapterLastReject = static_cast<uint8_t>(reject);
    return result;
}

enum class DirectAbortPollStatus801C81EC : uint8_t {
    Waiting = 0,
    Complete = 1,
};

struct DirectAbortPollResult801C81EC {
    DirectAbortPollStatus801C81EC status =
        DirectAbortPollStatus801C81EC::Waiting;
    int32_t result = -1;
    bool frameBodyRequested = false;
    bool retryPendingFrame = false;
    PrSS0EventFrameLoopDirect::Event4FrameTransactionBegin80026B94
        frameBegin{};
};

PrPsxEventFrameDirect::EventFrameState8001E750
    s_abortPollEvent4FrameState8001E750{};
bool s_abortPollEvent4Active8001E750 = false;
uint32_t s_abortPollEvent4ActiveFrame8001E750 = 0u;
int32_t s_abortPollEvent4SelectionState8001E750 = -1;
uint32_t s_abortPollEvent4CueRepeatCount80025E6C = 0u;
uint32_t s_abortPollEvent4CueFrame80025E6C = 0u;
bool s_abortPollEvent4Ss0ModalOwnerInitialized = false;
PrSS0EventFrameLoopDirect::DispatcherPreLoopPadReleaseState80026B94
    s_abortPollEvent4ReleaseGate80026B94{};
PrSS0EventFrameLoopDirect::DispatcherPadChangeState80026744
    s_abortPollEvent4PadChange80026744{};
PrSS0EventFrameLoopDirect::DispatcherTailState80026B94
    s_abortPollEvent4Tail80026B94{};
PrSS0EventFrameLoopDirect::Event4FrameTransaction80026B94
    s_abortPollEvent4FrameTransaction80026B94{};
PrPsxEventFrameDirect::EventFrameTailResult80026B94
    s_abortPollEvent4LastFrameTail80026B94{};
PrSS0EventFrameLoopDirect::Event4ModalInputTransaction80025F0C
    s_abortPollEvent4LastInput80025F0C{};
PrSS0EventFrameLoopDirect::Event4ModalTickTransaction80025E6C
    s_abortPollEvent4LastTick80025E6C{};
PrSS0ResourceAudioDirect::CueSoftwareTransaction80025C8C
    s_abortPollEvent4LastInputCue80025C8C{};
PrSS0ResourceAudioDirect::CueSoftwareTransaction80026EF8
    s_abortPollEvent4LastPeriodicCue80026EF8{};
int32_t s_abortPollEvent4LastFlushResult80026ECC = 0;

PrSS0ResourceAudioDirect::CueDriverBinding800943A8
BuildAbortPollEvent4HostDriverBinding800943A8() {
    PrSS0ResourceAudioDirect::CueDriverBinding800943A8 binding{};
    binding.kind = PrSS0ResourceAudioDirect::
        CueDriverBindingKind800943A8::HostStage1VabProjection;
    binding.value = 0u;
    binding.hostStage1VabReady =
        PrSfx::IsStage1VabReadyForHostProjection800943A8();
    return binding;
}

int16_t PlayAbortPollEvent4HostCue80034240(
    uint16_t driverState800943A8,
    uint8_t byte0,
    uint8_t byte1,
    uint8_t byte2,
    uint16_t arg4,
    uint8_t byte3Left,
    uint8_t byte3Right,
    void*) {
    return PrSfx::PlayStage1Cue80034240HostProjection(
        driverState800943A8,
        byte0,
        byte1,
        byte2,
        arg4,
        byte3Left,
        byte3Right);
}

int32_t FlushAbortPollEvent4HostCue80026ECC(void*) {
    return PrSfx::ApplySharedAudioDriverFlushBarrier26ECCResult();
}

bool PlayAbortPollEvent4InputCue80025C8C(uint32_t code, void*) {
    const PrSS0ResourceAudioDirect::CueKind cue =
        PrSS0ResourceAudioDirect::ResolveInputCue80025C8C(code);
    const PrSS0ResourceAudioDirect::CueSpec source =
        PrSS0ResourceAudioDirect::KnownStage1Event4CueSpec(cue);
    const PrSS0ResourceAudioDirect::CueSoftwareSinks80025C8C sinks{
        PlayAbortPollEvent4HostCue80034240,
        FlushAbortPollEvent4HostCue80026ECC,
        nullptr};
    return PrSS0ResourceAudioDirect::ExecuteInputSfxSoftware80025C8C(
        code,
        source,
        BuildAbortPollEvent4HostDriverBinding800943A8(),
        sinks,
        s_abortPollEvent4LastInputCue80025C8C);
}

bool PlayAbortPollEvent4Cue80026EF8(uint32_t cueAddress, void*) {
    PrSS0ResourceAudioDirect::CueKind cue =
        PrSS0ResourceAudioDirect::CueKind::Unknown;
    if (cueAddress == PrSS0EventFrameLoopDirect::kCueEvent4_8009441C) {
        cue = PrSS0ResourceAudioDirect::
            CueKind::Stage1Event4Primary9441C;
    } else if (
        cueAddress == PrSS0EventFrameLoopDirect::kCueEvent4_8009441C +
                          PrSS0EventFrameLoopDirect::
                              kEvent4ModalSecondCueByteOffset80025E6C) {
        cue = PrSS0ResourceAudioDirect::
            CueKind::Stage1Event4Secondary9441CPlus6;
    }
    return PrSS0ResourceAudioDirect::ExecuteCueSoftware80026EF8(
        PrSS0ResourceAudioDirect::KnownStage1Event4CueSpec(cue),
        BuildAbortPollEvent4HostDriverBinding800943A8(),
        PlayAbortPollEvent4HostCue80034240,
        nullptr,
        s_abortPollEvent4LastPeriodicCue80026EF8);
}

bool FlushAbortPollEvent4Cue80026ECC(void*) {
    s_abortPollEvent4LastFlushResult80026ECC =
        FlushAbortPollEvent4HostCue80026ECC(nullptr);
    return true;
}

bool ResetAbortPollEvent4ModalOwner800267C8() {
    PrSS0EventFrameLoopDirect::Event4ModalInitInput800267C8 input{};
    input.outputPointerKnown = true;
    input.outputPointer = PrSS0EventFrameLoopDirect::kCtxEvent4_8006ED74;
    const PrSS0EventFrameLoopDirect::Event4ModalInitTransaction800267C8 init =
        PrSS0EventFrameLoopDirect::BuildEvent4ModalInitSoftware800267C8(input);
    if (!init.accepted || !init.outputWrite) {
        return false;
    }
    s_abortPollEvent4SelectionState8001E750 = init.outputValue;
    s_abortPollEvent4CueFrame80025E6C = init.cueFrameAfter;
    s_abortPollEvent4CueRepeatCount80025E6C = init.remainingAfter;
    PrSS0EventFrameLoopDirect::ResetDispatcherPreLoopPadRelease80026B94(
        s_abortPollEvent4ReleaseGate80026B94);
    PrSS0EventFrameLoopDirect::ResetDispatcherPadChange80026744(
        s_abortPollEvent4PadChange80026744);
    PrSS0EventFrameLoopDirect::ResetDispatcherTail80026B94(
        s_abortPollEvent4Tail80026B94);
    PrSS0EventFrameLoopDirect::ResetEvent4FrameTransaction80026B94(
        s_abortPollEvent4FrameTransaction80026B94);
    s_abortPollEvent4LastFrameTail80026B94 = {};
    s_abortPollEvent4LastInput80025F0C = {};
    s_abortPollEvent4LastTick80025E6C = {};
    s_abortPollEvent4LastInputCue80025C8C = {};
    s_abortPollEvent4LastPeriodicCue80026EF8 = {};
    s_abortPollEvent4LastFlushResult80026ECC = 0;
    return true;
}

bool TickAbortPollEvent4Cue80025E6C() {
    PrSS0EventFrameLoopDirect::Event4ModalTickInput80025E6C input{};
    input.remainingKnown = true;
    input.remaining = s_abortPollEvent4CueRepeatCount80025E6C;
    input.cueFrameKnown = true;
    input.cueFrame = s_abortPollEvent4CueFrame80025E6C;
    input.cueGlobalAddressKnown = true;
    input.cueGlobalAddress =
        PrSS0EventFrameLoopDirect::kCueEvent4_8009441C;
    const PrSS0EventFrameLoopDirect::Event4ModalTickSinks80025E6C sinks{
        PlayAbortPollEvent4Cue80026EF8,
        FlushAbortPollEvent4Cue80026ECC,
        nullptr};
    s_abortPollEvent4LastTick80025E6C =
        PrSS0EventFrameLoopDirect::ExecuteEvent4ModalTickSoftware80025E6C(
            input,
            sinks);
    if (!s_abortPollEvent4LastTick80025E6C.accepted) {
        return false;
    }
    s_abortPollEvent4CueFrame80025E6C =
        s_abortPollEvent4LastTick80025E6C.cueFrameAfter;
    s_abortPollEvent4CueRepeatCount80025E6C =
        s_abortPollEvent4LastTick80025E6C.remainingAfter;
    return true;
}

void BeginAbortPollEvent4Overlay801C81EC(PrGameContext& ctx) {
    (void)ctx;
    if (s_abortPollEvent4Active8001E750) {
        return;
    }

    s_abortPollEvent4Ss0ModalOwnerInitialized =
        ResetAbortPollEvent4ModalOwner800267C8();
    if (!s_abortPollEvent4Ss0ModalOwnerInitialized) {
        Log::Printf(
            "Scene1 801C81EC direct abort-poll SS0 Event4 init blocked");
        return;
    }

    s_abortPollEvent4Active8001E750 = true;
    s_abortPollEvent4ActiveFrame8001E750 = 0u;
    PrPsxEventFrameDirect::ResetEventFrameState8003FB9C(
        s_abortPollEvent4FrameState8001E750,
        320u,
        240u);
    // Original 80026B94 -> 8001E750 retains the gameplay graph, including
    // 8001C470's screen center and active page. A fresh zero-centered graph
    // interprets 8001B590's centered sprite coordinates as screen coordinates.
    s_abortPollEvent4FrameState8001E750.graph =
        PrStageSceneSubmitDirect::GetOwnedStage1GraphOwner801CBFDC();

    uint32_t stage1Event4SeedGp38C = 0u;
    if (PrPsxEventFrameDirect::ConsumeStage1Event4Gp38CSeed8006EDCC(
            stage1Event4SeedGp38C)) {
        s_abortPollEvent4FrameState8001E750.gp38CEvent4State =
            stage1Event4SeedGp38C;
    }
    Log::Printf("Scene1 801C81EC direct abort-poll event4 overlay init");
}

bool TickAbortPollEvent4Overlay801C81EC(
    PrGameContext& ctx,
    const DirectAbortPollResult801C81EC& direct) {
    if (!s_abortPollEvent4Active8001E750) {
        return false;
    }

    auto& transaction = s_abortPollEvent4FrameTransaction80026B94;
    if (direct.frameBodyRequested) {
        if (!PrSS0EventFrameLoopDirect::BeginEvent4FrameTransaction80026B94(
                transaction,
                direct.frameBegin)) {
            Log::Printf(
                "Scene1 801C81EC direct abort-poll SS0 Event4 frame begin blocked frame=%u",
                static_cast<unsigned>(ctx.frame));
            return false;
        }
        s_abortPollEvent4LastFrameTail80026B94 = {};
    } else if (!direct.retryPendingFrame || !transaction.active) {
        return false;
    }

    if (!transaction.tickCommitted) {
        if (!TickAbortPollEvent4Cue80025E6C() ||
            !PrSS0EventFrameLoopDirect::CommitEvent4FrameTick80025E6C(
                transaction,
                transaction.logicFrame,
                PrSS0EventFrameLoopDirect::kCtxEvent4_8006ED74)) {
            Log::Printf(
                "Scene1 801C81EC direct abort-poll SS0 Event4 tick blocked frame=%u",
                static_cast<unsigned>(transaction.logicFrame));
            return false;
        }
    }

    if (!transaction.directDrawCompleteWithinLimits) {
        const int32_t promptCtx0 =
            (s_abortPollEvent4SelectionState8001E750 == 0 ||
             s_abortPollEvent4SelectionState8001E750 == 1)
                ? s_abortPollEvent4SelectionState8001E750
                : -1;
        s_abortPollEvent4LastFrameTail80026B94 =
            PrPsxEventFrameDirect::PsxExecute80026B94_EventFrameTailDetailed(
                s_abortPollEvent4FrameState8001E750,
                PrPsxVSyncDirect::ProcessVSyncState80035560(),
                4,
                promptCtx0,
                nullptr,
                nullptr);
        if (!s_abortPollEvent4LastFrameTail80026B94.drawRouteExecuted ||
            s_abortPollEvent4LastFrameTail80026B94.frameCloseBlocked ||
            !PrSS0EventFrameLoopDirect::
                CommitEvent4FrameDrawAndBindFormalTail80026B94(
                    transaction,
                    ctx.frame,
                    false)) {
            Log::Printf(
                "Scene1 801C81EC direct abort-poll SS0 Event4 draw blocked frame=%u",
                static_cast<unsigned>(transaction.logicFrame));
            return false;
        }
    }

    if (!transaction.hostFrameBoundaryExecuted) {
        const bool hostWaitExecuted =
            s_abortPollEvent4LastFrameTail80026B94.waitFrameExecuted &&
            s_abortPollEvent4LastFrameTail80026B94.waitFrame.sourceKnown &&
            s_abortPollEvent4LastFrameTail80026B94.waitFrame.
                softwareStateCommitted &&
            s_abortPollEvent4LastFrameTail80026B94.waitFrame.
                softwareWaitComplete &&
            s_abortPollEvent4LastFrameTail80026B94.waitFrame.mode ==
                PrPsxVSyncDirect::PsxVSyncMode80035560::WaitForVblanks;
        if (!PrSS0EventFrameLoopDirect::
                CommitEvent4FrameHostBoundary80026B94(
                    transaction,
                    hostWaitExecuted,
                    s_abortPollEvent4LastFrameTail80026B94.endFrameExecuted,
                    s_abortPollEvent4LastFrameTail80026B94.
                        textFlushExecuted)) {
            Log::Printf(
                "Scene1 801C81EC direct abort-poll SS0 Event4 host boundary blocked frame=%u wait=%d end=%d text=%d",
                static_cast<unsigned>(transaction.logicFrame),
                hostWaitExecuted ? 1 : 0,
                s_abortPollEvent4LastFrameTail80026B94.endFrameExecuted ? 1
                                                                        : 0,
                s_abortPollEvent4LastFrameTail80026B94.textFlushExecuted
                    ? 1
                    : 0);
            return false;
        }
    }
    s_abortPollEvent4ActiveFrame8001E750 = ctx.frame;
    return true;
}

void CompleteAbortPollEvent4Overlay801C81EC(int32_t result) {
    if (!s_abortPollEvent4Active8001E750) {
        return;
    }

    if (result == 1) {
        s_abortPollEvent4SelectionState8001E750 = 0;
    } else if (result == 2) {
        s_abortPollEvent4SelectionState8001E750 = 1;
    }
}

bool IsExactLifecycleFunction801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action,
    uint32_t expectedFunction) {
    return action.psxFunctionKnown && action.psxFunction == expectedFunction;
}

bool IsStageRecordTickAction801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    return IsExactLifecycleFunction801C81EC(action, kFn8001A4D0) &&
           action.loaderOffsetFromSceneEntry == kStageLoopLoaderOffset &&
           action.rawArg0 == kStageLoopLoaderOffset &&
           action.rawArg1 == 0u &&
           action.arg0 == 0;
}

bool IsStageRunnerRunAction801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    return IsExactLifecycleFunction801C81EC(action, kFn801C7A60) &&
           action.loaderOffsetFromSceneEntry == kStageLoopLoaderOffset &&
           action.transitionWorkPtr == kTransitionWork801C3640 &&
           action.rawArg0 == kStageLoopLoaderOffset &&
           action.rawArg1 == kTransitionWork801C3640 &&
           action.rawArg2 == action.sceneId &&
           action.arg0 == static_cast<int32_t>(kStageLoopLoaderOffset) &&
           action.arg1 == static_cast<int32_t>(kTransitionWork801C3640) &&
           action.arg2 == action.sceneId;
}

bool IsConfigMovieViewportAction801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    return IsExactLifecycleFunction801C81EC(action, kFn801CB67C);
}

bool IsQueryAbort26B94Action801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    return IsExactLifecycleFunction801C81EC(action, kFn80026B94) &&
           action.rawArg0 == 4u &&
           action.rawArg1 == 0u &&
           action.arg0 == 4 &&
           action.arg1 == 0;
}

bool IsInitialMovie1StrInitAction801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    return IsExactLifecycleFunction801C81EC(action, kFn801C44E0) &&
           action.strBlockKind ==
               PrStage1LifecycleDirect::StrBlockKind801C81EC::InitialMovie1 &&
           action.rawArg0 == kInitialMovie1LoaderOffset &&
           action.rawArg1 == 0u;
}

bool IsInitialMovie1StrPlayAction801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    return IsExactLifecycleFunction801C81EC(action, kFn801C455C) &&
           action.strBlockKind ==
               PrStage1LifecycleDirect::StrBlockKind801C81EC::InitialMovie1 &&
           action.rawArg0 == kInitialMovie1LoaderOffset &&
           action.rawArg1 == kTransitionWork801C3640 &&
           action.rawArg2 == 0u &&
           action.arg0 == 0;
}

bool IsClearTailLoaderOffset801C81EC(uint32_t loaderOffset) {
    return loaderOffset == kClearTailGoodLoaderOffset ||
           loaderOffset == kClearTailCoolLoaderOffset;
}

bool IsClearTailMovieStrInitAction801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    return IsExactLifecycleFunction801C81EC(action, kFn801C44E0) &&
           action.strBlockKind ==
               PrStage1LifecycleDirect::StrBlockKind801C81EC::ClearTailMovie &&
           IsClearTailLoaderOffset801C81EC(action.rawArg0) &&
           action.rawArg1 == 0u;
}

bool IsClearTailMovieStrPlayAction801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    if (!IsExactLifecycleFunction801C81EC(action, kFn801C455C) ||
        action.strBlockKind !=
            PrStage1LifecycleDirect::StrBlockKind801C81EC::ClearTailMovie ||
        action.rawArg1 != kTransitionWork801C3640) {
        return false;
    }
    if (action.rawArg0 == kClearTailGoodLoaderOffset) {
        return action.rawArg2 == 1u && action.arg0 == 1;
    }
    if (action.rawArg0 == kClearTailCoolLoaderOffset) {
        return action.rawArg2 == 2u && action.arg0 == 2;
    }
    return false;
}

bool IsInitialMovie1Transition201ACAction801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    using StrBlockKind = PrStage1LifecycleDirect::StrBlockKind801C81EC;
    if (action.kind !=
            PrStage1LifecycleDirect::ActionKind801C81EC::Transition201AC ||
        action.strBlockKind != StrBlockKind::InitialMovie1 ||
        !IsExactLifecycleFunction801C81EC(action, kFn800201AC) ||
        action.transitionWorkPtr != kTransitionWork801C3640 ||
        action.transitionA1_801C3640 != kTransitionWork801C3640 ||
        action.transitionFinishFunction != kFn80020090 ||
        action.transitionHasExtraDelay27194 ||
        action.transitionExtraDelayFrames27194 != 0) {
        return false;
    }

    const bool preMovie =
        action.transitionModeA2 == 6 &&
        action.transitionPreFfd4ArgA3 == 2 &&
        action.transitionPostFfd4ArgA4 == 1;
    const bool postMovie =
        action.transitionModeA2 == 5 &&
        action.transitionPreFfd4ArgA3 == 1 &&
        action.transitionPostFfd4ArgA4 == 2;
    return preMovie || postMovie;
}

bool IsClearTailMovieTransition201ACAction801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    using StrBlockKind = PrStage1LifecycleDirect::StrBlockKind801C81EC;
    if (action.kind !=
            PrStage1LifecycleDirect::ActionKind801C81EC::Transition201AC ||
        action.strBlockKind != StrBlockKind::ClearTailMovie ||
        !IsExactLifecycleFunction801C81EC(action, kFn800201AC) ||
        action.transitionWorkPtr != kTransitionWork801C3640 ||
        action.transitionA1_801C3640 != kTransitionWork801C3640 ||
        action.transitionFinishFunction != kFn80020090 ||
        action.transitionHasExtraDelay27194 ||
        action.transitionExtraDelayFrames27194 != 0) {
        return false;
    }

    const bool preMovie =
        action.transitionModeA2 == 6 &&
        action.transitionPreFfd4ArgA3 == 2 &&
        action.transitionPostFfd4ArgA4 == 1;
    const bool postMovie =
        action.transitionModeA2 == 5 &&
        action.transitionPreFfd4ArgA3 == 1 &&
        action.transitionPostFfd4ArgA4 == 2;
    return preMovie || postMovie;
}

bool IsTransition20110Action801C81EC(
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    if (action.kind !=
            PrStage1LifecycleDirect::ActionKind801C81EC::Transition20110 ||
        !IsExactLifecycleFunction801C81EC(action, kFn80020110) ||
        action.transitionWorkPtr != kTransitionWork801C3640 ||
        action.transitionA1_801C3640 != kTransitionWork801C3640 ||
        action.transitionFinishFunction != kFn80020008 ||
        !action.transitionHasExtraDelay27194 ||
        action.transitionExtraDelayFrames27194 != 30) {
        return false;
    }

    const bool stageLoopEntryMode1 =
        action.transitionModeA2 == 1 &&
        action.transitionPreFfd4ArgA3 == 2 &&
        action.transitionPostFfd4ArgA4 == 1;
    const bool mode2Prelude =
        action.transitionModeA2 == 2 &&
        action.transitionPreFfd4ArgA3 == 1 &&
        action.transitionPostFfd4ArgA4 == 2;
    return stageLoopEntryMode1 || mode2Prelude;
}

const PrStage1SaveUi19148LowerFeedbackRequest*
FindSaveUi19148ObservableLowerRequest801C81EC(
    const PrStage1SaveUi19148LowerFeedbackRequestList& requests) {
    for (uint32_t i = 0; i < requests.count; ++i) {
        if (requests.requests[i].kind ==
            PrStage1SaveUi19148LowerFeedbackRequestKind::CardIo80017594) {
            return &requests.requests[i];
        }
    }
    for (uint32_t i = 0; i < requests.count; ++i) {
        if (requests.requests[i].kind ==
            PrStage1SaveUi19148LowerFeedbackRequestKind::
                DirectoryRows80019458) {
            return &requests.requests[i];
        }
    }
    return requests.count > 0 ? &requests.requests[0] : nullptr;
}

const PrStage1SaveUi19148LowerFeedbackRequest*
FindSaveUi19148LowerRequest801C81EC(
    const PrStage1SaveUi19148LowerFeedbackRequestList& requests,
    PrStage1SaveUi19148LowerFeedbackRequestKind kind) {
    for (uint32_t i = 0; i < requests.count; ++i) {
        if (requests.requests[i].kind == kind) {
            return &requests.requests[i];
        }
    }
    return nullptr;
}

struct SaveUi19148CardEventHalState801C81EC {
    bool infoPollReady80016E18 = false;
    bool infoPollCompletionKnown80016E18 = false;
    PrStage1SaveUiCardIoState80017594 expectedInfoPollState{};
    bool loadPollReady80016E18 = false;
    bool loadPollCompletionKnown80016E18 = false;
    PrStage1SaveUiCardIoState80017594 expectedLoadPollState{};
    bool directCardImageKnown80017594 = false;
    bool directDirectoryLoaded80017594 = false;
    int32_t directDirectoryActiveRows80017594 = 0;
    bool probeOnlyFormatPoll4Ready80016E18 = false;
    bool probeOnlyFormatTerminalReady80017B60 = false;
    int32_t probeOnlyFormatTerminalPollResult80017008 = 0;
};

SaveUi19148CardEventHalState801C81EC
    s_saveUi19148CardEventHalState801C81EC{};

struct SaveUi19148DirectoryRawBankSourceAttempt801C81EC {
    bool requestPresent = false;
    bool directDirectoryLoaded = false;
    bool directRawBankKnown = false;
    bool providerInstalled = false;
    bool readAttempted = false;
    bool windowReadable = false;
    uint32_t psxAddress = 0u;
    uint32_t byteSize = 0u;
    bool factsBuilt = false;
    bool carrierPublishAttempted = false;
    bool carrierPublished = false;
};

SaveUi19148DirectoryRawBankSourceAttempt801C81EC
    s_saveUi19148DirectoryRawBankSourceAttempt801C81EC{};

struct SaveUi19148WriteSourceAttempt801C81EC {
    bool requestPresent = false;
    bool writeBlockKnown = false;
    bool saveHeaderBuilt = false;
    bool savePayloadCopied = false;
    uint32_t psxAddress = 0u;
    uint32_t byteSize = 0u;
    bool factsBuilt = false;
    bool carrierPublishAttempted = false;
    bool carrierPublished = false;
};

SaveUi19148WriteSourceAttempt801C81EC
    s_saveUi19148WriteSourceAttempt801C81EC{};

struct SaveUi19148DirectCardBufferSink801C81EC {
    bool attempted = false;
    bool requestShapeOk = false;
    bool blockViewKnown = false;
    bool committed = false;
    bool sinkAuthorityKnown = false;
    uint32_t psxAddress = 0u;
    uint32_t byteSize = 0u;
    uint32_t nameAddress = 0u;
    int32_t blockCount = 0;
    std::array<uint8_t, kSaveUiWriteBlockBytes80017A10> blockBytes{};
};

SaveUi19148DirectCardBufferSink801C81EC
    s_saveUi19148DirectCardBufferSink801C81EC{};

struct SaveUi19148SerializedCardEntry801C81EC {
    bool attempted = false;
    bool cardBufferAuthorityKnown = false;
    bool nameKnown = false;
    bool serialized = false;
    bool directoryUpdateAttempted = false;
    bool directoryKnown = false;
    bool directorySlotKnown = false;
    bool directoryUpdated = false;
    bool directoryOverwrote = false;
    bool durablePolicyKnown = false;
    bool durableCommitted = false;
    int32_t directoryBlockIndex = -1;
    uint32_t nameAddress = 0u;
    uint32_t blockAddress = 0u;
    uint32_t blockBytes = 0u;
    char internalName[32]{};
    std::array<uint8_t, kSaveUiWriteBlockBytes80017A10> bytes{};
};

SaveUi19148SerializedCardEntry801C81EC
    s_saveUi19148SerializedCardEntry801C81EC{};

struct SaveUi19148DirectDurableHandoff801C81EC {
    bool attempted = false;
    bool serializedEntryKnown = false;
    bool cardImageCandidateAttempted = false;
    bool cardImageCandidateKnown = false;
    bool persistenceSinkAttempted = false;
    bool persistenceSinkCommitted = false;
    bool slotPolicyKnown = false;
    int32_t blockIndex = -1;
    bool state16ReadProducerPublished = false;
    bool case17ReadProducerPublished = false;
    bool durablePrimitiveAttempted = false;
    bool directBackendKnown = false;
    bool directBackendCalled = false;
    bool directBackendAccepted = false;
    bool explicitNoSavePersistencePolicyKnown = false;
    bool explicitNoSavePersistencePolicyAccepted = false;
    bool explicitNoSavePersistencePolicyFinalized = false;
    bool explicitNoSaveP0Accepted = false;
    bool durablePolicyKnown = false;
    bool durableCommitted = false;
    const char* missingOwner = nullptr;
};

SaveUi19148DirectDurableHandoff801C81EC
    s_saveUi19148DirectDurableHandoff801C81EC{};

struct SaveUi19148DirectWriteRuntime801C81EC {
    bool active = false;
    bool completed = false;
    bool blocked = false;
    PrStage1SaveUi19148LowerFeedbackRequest request{};
    int32_t attemptIndex = 0;
    bool waitStarted80035560 = false;
    bool durableAttempted = false;
    bool backendCompletionKnown = false;
    bool durableCommitted = false;
    bool originalDirectoryKnown = false;
    std::array<uint8_t, kSaveUiDirectoryRawBankBytes80019458>
        originalDirectory{};
    bool previousCardImageKnown = false;
    int32_t previousCardImageBlockIndex = -1;
    std::array<uint8_t, 128u * 1024u> previousCardImage{};
    PrStage1SaveCardHalDirect::CardWriteHostFacts80017A10 facts{};
    PrStage1SaveUi19148LowerFeedback completedFeedback{};
};

SaveUi19148DirectWriteRuntime801C81EC
    s_saveUi19148DirectWriteRuntime801C81EC{};

struct SaveUi19148DirectFormatRuntime801C81EC {
    bool active = false;
    bool completed = false;
    bool blocked = false;
    PrStage1SaveUi19148LowerFeedbackRequest request{};
    int32_t attemptsUsed = 0;
    bool durableAttempted = false;
    bool backendCompletionKnown = false;
    bool durableCommitted = false;
    bool rollbackCompleted = false;
    bool originalDirectoryKnown = false;
    std::array<uint8_t, kSaveUiDirectoryRawBankBytes80019458>
        originalDirectory{};
    bool previousCardImageKnown = false;
    int32_t previousCardImageBlockIndex = -1;
    std::array<uint8_t, 128u * 1024u> previousCardImage{};
    PrStage1SaveCardHalDirect::SaveUiFormatRuntimeFacts80017B60 facts{};
};

SaveUi19148DirectFormatRuntime801C81EC
    s_saveUi19148DirectFormatRuntime801C81EC{};

bool CommitSaveUi19148DirectCardBufferSink801C81EC(
    const PrStage1SaveUi19148LowerFeedbackRequest& request);
bool SerializeSaveUi19148DirectCardBufferEntry801C81EC();
bool AttemptSaveUi19148DirectDurableHandoff801C81EC();

bool SameSaveUi19148WriteRequest801C81EC(
    const PrStage1SaveUi19148LowerFeedbackRequest& lhs,
    const PrStage1SaveUi19148LowerFeedbackRequest& rhs) {
    return lhs.kind == rhs.kind &&
           lhs.psxFunction == rhs.psxFunction &&
           lhs.retryCount == rhs.retryCount &&
           lhs.nameAddress == rhs.nameAddress &&
           lhs.dataAddress == rhs.dataAddress &&
           lhs.blockCount == rhs.blockCount &&
           lhs.writeCloseGp696FactRequired80017A10 ==
               rhs.writeCloseGp696FactRequired80017A10 &&
           lhs.writeCloseGp696Address80017A10 ==
               rhs.writeCloseGp696Address80017A10 &&
           lhs.writeFdMustMatchCloseGp69680017A10 ==
               rhs.writeFdMustMatchCloseGp69680017A10 &&
           lhs.action.kind == rhs.action.kind;
}

void ResetSaveUi19148DirectWriteRuntime801C81EC() {
    s_saveUi19148DirectWriteRuntime801C81EC = {};
}

bool SameSaveUi19148FormatRequest801C81EC(
    const PrStage1SaveUi19148LowerFeedbackRequest& lhs,
    const PrStage1SaveUi19148LowerFeedbackRequest& rhs) {
    return lhs.kind == rhs.kind &&
           lhs.psxFunction == rhs.psxFunction &&
           lhs.retryCount == rhs.retryCount &&
           lhs.formatArg0 == rhs.formatArg0 &&
           lhs.formatArg1 == rhs.formatArg1 &&
           lhs.action.kind == rhs.action.kind;
}

void ResetSaveUi19148DirectFormatRuntime801C81EC() {
    s_saveUi19148DirectFormatRuntime801C81EC = {};
}

bool RollbackSaveUi19148DirectFormatRuntime801C81EC() {
    SaveUi19148DirectFormatRuntime801C81EC& runtime =
        s_saveUi19148DirectFormatRuntime801C81EC;
    if (!runtime.originalDirectoryKnown) {
        return false;
    }
    const PrStage1SaveUiDirectoryRawBankRestore8007A318 directory =
        PrStage1SaveUiDirect::
            RestoreSaveUiDirectoryRawBankAfterFailedWrite8007A318(
                runtime.originalDirectory.data(),
                runtime.originalDirectory.size());
    const PrStage1SaveUiCardImageWriteRollback8007A318 image =
        PrStage1SaveUiDirect::RollbackSaveUiCardImageAfterFailedWrite8007A318(
            runtime.previousCardImageKnown
                ? runtime.previousCardImage.data()
                : nullptr,
            runtime.previousCardImageKnown
                ? runtime.previousCardImage.size()
                : 0u,
            runtime.previousCardImageBlockIndex,
            runtime.previousCardImageKnown);
    return directory.restored && image.candidateCleared &&
           image.pendingPersistenceCleared &&
           (!runtime.previousCardImageKnown || image.previousImageRestored);
}

bool RollbackSaveUi19148DirectWriteRuntime801C81EC() {
    SaveUi19148DirectWriteRuntime801C81EC& runtime =
        s_saveUi19148DirectWriteRuntime801C81EC;
    if (!runtime.originalDirectoryKnown) {
        return false;
    }
    const PrStage1SaveUiDirectoryRawBankRestore8007A318 directory =
        PrStage1SaveUiDirect::
            RestoreSaveUiDirectoryRawBankAfterFailedWrite8007A318(
                runtime.originalDirectory.data(),
                runtime.originalDirectory.size());
    const PrStage1SaveUiCardImageWriteRollback8007A318 image =
        PrStage1SaveUiDirect::RollbackSaveUiCardImageAfterFailedWrite8007A318(
            runtime.previousCardImageKnown
                ? runtime.previousCardImage.data()
                : nullptr,
            runtime.previousCardImageKnown
                ? runtime.previousCardImage.size()
                : 0u,
            runtime.previousCardImageBlockIndex,
            runtime.previousCardImageKnown);
    return directory.restored && image.candidateCleared &&
           image.pendingPersistenceCleared &&
           (!runtime.previousCardImageKnown || image.previousImageRestored);
}

bool CardIoStateEquals801C81EC(const PrStage1SaveUiCardIoState80017594& lhs,
                               const PrStage1SaveUiCardIoState80017594& rhs) {
    return lhs.dword800917E8 == rhs.dword800917E8 &&
           lhs.dword800917EC == rhs.dword800917EC &&
           lhs.dword800917F0 == rhs.dword800917F0 &&
           lhs.dword800917F4 == rhs.dword800917F4 &&
           lhs.gp700 == rhs.gp700;
}

bool IsSaveUi19148DirectDurableCardImageKnown801C81EC() {
    const PrStage1SaveUiCardImagePersistenceView8007A318 image =
        PrStage1SaveUiDirect::
            GetSaveUiCardImagePersistenceSinkView8007A318();
    return image.known && image.slotPolicyKnown && image.blockIndex >= 0 &&
           image.blockIndex < 15 && image.bytes != nullptr &&
           image.byteCount == 128u * 1024u && image.durablePolicyKnown &&
           image.durableCommitted && image.bytes[0] == 'M' &&
           image.bytes[1] == 'C';
}

void ObserveSaveUi19148DirectCardEventSubmitAfterTick801C81EC(
    const PrStage1SaveUiHostBridgeDirect::SaveUi19148HostTickAttempt& tick) {
    // These typed receipts are set only after matching explicit lower facts.
    // They can arrive after a20-VBlank presentation continuation; the CURRENT
    // caller need not (and must not) resubmit that earlier card feedback.
    if (!tick.cardIoStateBeforeKnown80017594 ||
        !tick.cardIoStateAfterKnown80017594) {
        return;
    }

    SaveUi19148CardEventHalState801C81EC& state =
        s_saveUi19148CardEventHalState801C81EC;
    if (tick.cardIoStateBefore80017594.dword800917E8 == 0 &&
        tick.cardIoStateAfter80017594.dword800917E8 == 1 &&
        tick.cardIoStateAfter80017594.gp700 == 300) {
        state.directCardImageKnown80017594 =
            IsSaveUi19148DirectDurableCardImageKnown801C81EC();
        const auto media = PrSS0CardImageStorageDirect::ProbePrimaryCardMedia80017594();
        const int32_t eventResult80016E18 =
            PrSS0CardImageStorageDirect::ResolveCardInfoEvent80017594(media);
        state.infoPollCompletionKnown80016E18 =
            PrStage1SaveCardHalDirect::SignalTranslatedSwCardEvent80016E18(
                PrStage1SaveCardHalDirect::CardTranslatedEventSignalSource::
                    CardInfo80017594,
                eventResult80016E18);
        state.infoPollReady80016E18 =
            state.infoPollCompletionKnown80016E18;
        state.expectedInfoPollState = tick.cardIoStateAfter80017594;
        Log::Printf(
            "Scene1 801C81EC save-ui 19148 direct card_info signaled translated SwCARD event=%d durableImageKnown=%d signalAccepted=%d mediaPresent=%d imageRead=%d headerValid=%d",
            eventResult80016E18,
            state.directCardImageKnown80017594 ? 1 : 0,
            state.infoPollCompletionKnown80016E18 ? 1 : 0,
            media.mediaPresent ? 1 : 0, media.imageRead ? 1 : 0,
            media.headerValid ? 1 : 0);
    }
    if (tick.cardIoStateBefore80017594.dword800917E8 == 2 &&
        tick.cardIoStateAfter80017594.dword800917E8 == 3 &&
        tick.cardIoStateAfter80017594.gp700 == 300) {
        PrStage1SaveCardHalDirect::DrainTranslatedSwCardEvents80016FC0();
        // 80017594 state2 issues card_load(0). Re-read the current medium
        // before building directory rows; retaining the previous sink here
        // would make a replaced card appear as the old card.
        const auto cardReload =
            PrSS0CardImageStorageDirect::ReloadPrimaryCardImage8007A318();
        PrStage1SaveCardHalDirect::ClearState16CardReadTypedCarrier800179B4();
        PrStage1SaveCardHalDirect::ClearCase17CardReadTypedCarrier800179B4();
        Log::Printf("Scene1 801C81EC save-ui 19148 card_load refreshed medium found=%d read=%d validated=%d imported=%d block=%d",
            cardReload.imageFileFound ? 1 : 0, cardReload.imageRead ? 1 : 0,
            cardReload.imageValidated ? 1 : 0, cardReload.sinkImported ? 1 : 0,
            cardReload.blockIndex);
        const PrStage1SaveUiDirectCardLoadResult80017594 load =
            PrStage1SaveUiDirect::LoadSaveUiDirectCardImageDirectory80017594();
        const int32_t eventResult80016E18 =
            PrSS0CardImageStorageDirect::ResolveCardLoadEvent80017594(
                cardReload, load.directoryLoaded);
        state.loadPollCompletionKnown80016E18 =
            PrStage1SaveCardHalDirect::SignalTranslatedSwCardEvent80016E18(
                PrStage1SaveCardHalDirect::CardTranslatedEventSignalSource::
                    CardLoad80017594,
                eventResult80016E18);
        state.loadPollReady80016E18 =
            state.loadPollCompletionKnown80016E18;
        state.expectedLoadPollState = tick.cardIoStateAfter80017594;
        state.directDirectoryLoaded80017594 = load.directoryLoaded;
        state.directDirectoryActiveRows80017594 = load.activeRows;
        Log::Printf(
            "Scene1 801C81EC save-ui 19148 direct card_load signaled translated SwCARD event=%d directoryLoaded=%d activeRows=%d signalAccepted=%d",
            eventResult80016E18,
            load.directoryLoaded ? 1 : 0,
            load.activeRows,
            state.loadPollCompletionKnown80016E18 ? 1 : 0);
    }
}

bool BuildSaveUi19148CardEventPollLowerFeedback801C81EC(
    const PrStage1SaveUi19148LowerFeedbackRequest& request,
    PrStage1SaveUi19148LowerFeedback& outLowerFeedback) {
    if (outLowerFeedback.cardIoFeedbackKnown80017594 ||
        request.kind !=
            PrStage1SaveUi19148LowerFeedbackRequestKind::CardIo80017594 ||
        request.psxFunction != kFn80017594) {
        return false;
    }

    const PrSS0CardImageStorageDirect::CardMediaChangePoll8007A318
        mediaChange =
            PrSS0CardImageStorageDirect::
                PollPrimaryCardMediaChange8007A318();
    bool physicalState1Event4 = false;
    if (mediaChange.sourceInstalled && mediaChange.changed) {
        if (request.cardIoState.dword800917E8 == 1) {
            physicalState1Event4 = true;
        }
        PrStage1SaveCardHalDirect::SignalTranslatedSwCardEvent80016E18(
            PrStage1SaveCardHalDirect::
                CardTranslatedEventSignalSource::PhysicalHotplug80017594,
            4);
    }

    const bool consumeInfoPoll =
        request.cardIoState.dword800917E8 == 1 &&
        s_saveUi19148CardEventHalState801C81EC.infoPollReady80016E18 &&
        s_saveUi19148CardEventHalState801C81EC
            .infoPollCompletionKnown80016E18 &&
        CardIoStateEquals801C81EC(
            request.cardIoState,
            s_saveUi19148CardEventHalState801C81EC.expectedInfoPollState);
    const bool consumeLoadPoll =
        request.cardIoState.dword800917E8 == 3 &&
        s_saveUi19148CardEventHalState801C81EC.loadPollReady80016E18 &&
        s_saveUi19148CardEventHalState801C81EC
            .loadPollCompletionKnown80016E18 &&
        CardIoStateEquals801C81EC(
            request.cardIoState,
            s_saveUi19148CardEventHalState801C81EC.expectedLoadPollState);
    const bool consumeProbeOnlyFormatPoll4 =
        request.cardIoState.dword800917E8 == 3 &&
        request.cardIoState.dword800917F0 == 1 &&
        request.cardIoState.gp700 > 0 &&
        s_saveUi19148CardEventHalState801C81EC.
            probeOnlyFormatPoll4Ready80016E18;
    PrStage1SaveCardHalDirect::SaveUiCardIoState3TypedPollCarrier80017594
        runtimeState3Carrier{};
    bool runtimeState3CarrierKnown = false;
    // The explicitly armed poll4 fixture is a non-authority route probe. Give
    // it this one state3 observation before the ordinary runtime result=1
    // carrier; otherwise the natural carrier returns first forever and the
    // requested format-route regression can never consume its own fixture.
    if (consumeProbeOnlyFormatPoll4) {
        PrStage1SaveCardHalDirect::
            ClearSaveUiCardIoState3TypedPollCarrier80017594();
    } else {
        runtimeState3CarrierKnown = PrStage1SaveCardHalDirect::
            GetSaveUiCardIoState3TypedPollCarrier80017594(
                &runtimeState3Carrier);
    }
    bool consumeRuntimeState3TypedPoll =
        request.cardIoState.dword800917E8 == 3 &&
        runtimeState3CarrierKnown &&
        runtimeState3Carrier.producerWired80016E18_80017594 &&
        runtimeState3Carrier.typedPollResultKnown80016E18 &&
        runtimeState3Carrier.lower.lowerFeedbackKnown &&
        runtimeState3Carrier.lower.lowerFeedback.cardIoFeedbackKnown80017594 &&
        runtimeState3Carrier.hostFacts.stateBeforeKnown &&
        CardIoStateEquals801C81EC(
            request.cardIoState,
            runtimeState3Carrier.hostFacts.stateBefore);
    if (runtimeState3CarrierKnown && !consumeRuntimeState3TypedPoll) {
        PrStage1SaveCardHalDirect::
            ClearSaveUiCardIoState3TypedPollCarrier80017594();
        runtimeState3CarrierKnown = false;
        runtimeState3Carrier = {};
    }
    if (consumeLoadPoll && !consumeProbeOnlyFormatPoll4 &&
        !runtimeState3CarrierKnown) {
        PrStage1SaveCardHalDirect::CardNaturalSwCardEventInput80016E18
            naturalEvent{};
        PrStage1SaveCardHalDirect::CardIoHostFacts80017594
            runtimeState3Facts{};
        if (!PrStage1SaveCardHalDirect::
                PollTranslatedSwCardEvents80016E18(
                    request.cardIoState.gp700,
                    &naturalEvent) ||
            !PrStage1SaveCardHalDirect::
                BuildSaveUiCardIoPollFactsFromNaturalEvent80016E18(
                    request, naturalEvent, &runtimeState3Facts) ||
            runtimeState3Facts.pollSwResult80016E18 == 0 ||
            !PrStage1SaveCardHalDirect::
                PublishRuntimeSaveUiCardIoState3TypedPollCarrier80017594(
                    request,
                    runtimeState3Facts) ||
            !PrStage1SaveCardHalDirect::
                GetSaveUiCardIoState3TypedPollCarrier80017594(
                    &runtimeState3Carrier)) {
            return false;
        }
        runtimeState3CarrierKnown = true;
        consumeRuntimeState3TypedPoll =
            runtimeState3Carrier.producerWired80016E18_80017594 &&
            runtimeState3Carrier.typedPollResultKnown80016E18 &&
            runtimeState3Carrier.lower.lowerFeedbackKnown &&
            runtimeState3Carrier.lower.lowerFeedback
                .cardIoFeedbackKnown80017594 &&
            runtimeState3Carrier.hostFacts.stateBeforeKnown &&
            CardIoStateEquals801C81EC(
                request.cardIoState,
                runtimeState3Carrier.hostFacts.stateBefore);
        Log::Printf(
            "Scene1 801C81EC save-ui 19148 runtime state3 typed poll published source=translated-card-event-broker-80016e18 result=%d",
            runtimeState3Carrier.pollResult80016E18);
        if (!consumeRuntimeState3TypedPoll) {
            PrStage1SaveCardHalDirect::
                ClearSaveUiCardIoState3TypedPollCarrier80017594();
            return false;
        }
    }
    if (!consumeInfoPoll && !consumeLoadPoll &&
        !consumeRuntimeState3TypedPoll &&
        !consumeProbeOnlyFormatPoll4) {
        return false;
    }

    if (consumeRuntimeState3TypedPoll) {
        outLowerFeedback.cardIoFeedbackKnown80017594 = true;
        outLowerFeedback.cardIoFeedback80017594 =
            runtimeState3Carrier.lower.lowerFeedback.cardIoFeedback80017594;
        PrStage1SaveCardHalDirect::
            ClearSaveUiCardIoState3TypedPollCarrier80017594();
        if (consumeLoadPoll) {
            s_saveUi19148CardEventHalState801C81EC
                .loadPollReady80016E18 = false;
            s_saveUi19148CardEventHalState801C81EC
                .loadPollCompletionKnown80016E18 = false;
            s_saveUi19148CardEventHalState801C81EC.expectedLoadPollState = {};
        }
        Log::Printf(
            "Scene1 801C81EC save-ui 19148 runtime state3 typed poll consumed source=runtime-80016e18-state3 result=%d",
            runtimeState3Carrier.pollResult80016E18);
        return true;
    }

    PrStage1SaveCardHalDirect::CardIoHostFacts80017594 facts{};
    if (consumeProbeOnlyFormatPoll4) {
        if (!PrStage1SaveCardHalDirect::
                BuildSaveUiCardIoPollFactsFromResult80016E18(
                    request, 4, &facts)) {
            return false;
        }
    } else {
        PrStage1SaveCardHalDirect::CardNaturalSwCardEventInput80016E18
            naturalEvent{};
        if (!PrStage1SaveCardHalDirect::
                PollTranslatedSwCardEvents80016E18(
                    request.cardIoState.gp700,
                    &naturalEvent) ||
            !PrStage1SaveCardHalDirect::
                BuildSaveUiCardIoPollFactsFromNaturalEvent80016E18(
                    request, naturalEvent, &facts) ||
            facts.pollSwResult80016E18 == 0) {
            return false;
        }
        if (request.cardIoState.dword800917E8 == 1 &&
            facts.pollSwResult80016E18 == 4 && physicalState1Event4) {
            const PrSS0CardImageStorageDirect::BiosCardResetProbe80047EE4
                probe = PrSS0CardImageStorageDirect::
                    ProbePrimaryCardBiosReset80047EE4();
            PrStage1SaveCardHalDirect::DrainTranslatedHwCardEvents8001707C();
            const int32_t hwEventResult =
                probe.cardWriteResultKnown &&
                        probe.cardWriteResult == 0
                    ? 1
                    : 2;
            const bool hwSignalAccepted =
                probe.sourceInstalled && probe.observationKnown &&
                PrStage1SaveCardHalDirect::SignalTranslatedHwCardEvent80017008(
                    PrStage1SaveCardHalDirect::
                        CardTranslatedEventSignalSource::ResetHwCard80047EE4,
                    hwEventResult);
            PrStage1SaveCardHalDirect::CardNaturalHwCardEventInput80017008
                hwEvent{};
            int32_t hwPollResult = 0;
            const bool hwPollKnown =
                hwSignalAccepted &&
                PrStage1SaveCardHalDirect::PollTranslatedHwCardEvents80017008(
                    &hwEvent) &&
                PrStage1SaveCardHalDirect::ComputeNaturalHwCardPollResult80017008(
                    hwEvent, &hwPollResult);
            PrStage1SaveCardHalDirect::CardBiosResetProviderFacts80047EE4
                provider{};
            provider.sourceKnown = probe.sourceInstalled &&
                                   probe.observationKnown && hwSignalAccepted;
            provider.drainHwEventsKnown8001707C = provider.sourceKnown;
            provider.newCardKnown80047EE4 = provider.sourceKnown;
            provider.cardWriteArgsKnown80047EE4 = provider.sourceKnown;
            provider.cardWriteArg0_80047EE4 = 0;
            provider.cardWriteArg1_80047EE4 = 63;
            provider.cardWriteArg2_80047EE4 = 0;
            provider.cardWriteResultKnown80047EE4 =
                probe.cardWriteResultKnown;
            provider.cardWriteResult80047EE4 = probe.cardWriteResult;
            provider.pollHwKnown80017008 = hwPollKnown;
            provider.pollHwResult80017008 = hwPollResult;
            const bool resetFactsComplete =
                PrStage1SaveCardHalDirect::
                    ApplySaveUiCardIoEvent4ResetProviderFacts80047EE4(
                        provider, &facts);
            Log::Printf(
                "Scene1 801C81EC save-ui 19148 state1 event4 BIOS reset probe mediaPresent=%d imageReadable=%d writable=%d cardWriteResult=%d hwEvent=%d hwPoll=%d factsComplete=%d",
                probe.mediaPresent ? 1 : 0,
                probe.imageReadable ? 1 : 0,
                probe.writable ? 1 : 0,
                probe.cardWriteResult,
                hwEventResult,
                hwPollResult,
                resetFactsComplete ? 1 : 0);
            if (!resetFactsComplete) {
                Log::Printf(
                    "Scene1 801C81EC save-ui 19148 event4 rejected: 80047EE4 BIOS reset/provider facts incomplete");
                return false;
            }
        }
        if (request.cardIoState.dword800917E8 == 1 &&
            facts.pollSwResult80016E18 == 4 &&
            !PrStage1SaveCardHalDirect::
                AreSaveUiCardIoEvent4ResetFactsComplete80047EE4(facts)) {
            Log::Printf(
                "Scene1 801C81EC save-ui 19148 event4 rejected: missing 80047EE4 reset/post-reset 80017008 facts fail-closed");
            return false;
        }
    }

    PrStage1SaveCardHalDirect::CardIoLowerFeedbackBuildResult80017594 build{};
    PrStage1SaveCardHalDirect::BuildSaveUiCardIoLowerFeedbackFromHostFacts80017594(
        facts,
        &build);
    if (!build.lowerFeedbackKnown ||
        !build.lowerFeedback.cardIoFeedbackKnown80017594) {
        return false;
    }

    outLowerFeedback.cardIoFeedbackKnown80017594 = true;
    outLowerFeedback.cardIoFeedback80017594 =
        build.lowerFeedback.cardIoFeedback80017594;
    if (consumeInfoPoll) {
        s_saveUi19148CardEventHalState801C81EC.infoPollReady80016E18 = false;
        s_saveUi19148CardEventHalState801C81EC
            .infoPollCompletionKnown80016E18 = false;
        s_saveUi19148CardEventHalState801C81EC
            .expectedInfoPollState = {};
    }
    if (consumeLoadPoll) {
        s_saveUi19148CardEventHalState801C81EC.loadPollReady80016E18 = false;
        s_saveUi19148CardEventHalState801C81EC
            .loadPollCompletionKnown80016E18 = false;
        s_saveUi19148CardEventHalState801C81EC
            .expectedLoadPollState = {};
    }
    if (consumeProbeOnlyFormatPoll4) {
        s_saveUi19148CardEventHalState801C81EC.
            probeOnlyFormatPoll4Ready80016E18 = false;
        Log::Printf(
            "Scene1 801C81EC save-ui 19148 probe-only poll4 consumed nonAuthority=1 state=3 result=4");
    } else {
        Log::Printf(
            "Scene1 801C81EC save-ui 19148 translated SwCARD event consumed fn=80016E18 state=%d result=%d naturalSource=%d",
            request.cardIoState.dword800917E8,
            facts.pollSwResult80016E18,
            facts.naturalSwCardEventSourceKnown80016E18 ? 1 : 0);
    }
    return true;
}

void LogSaveUi19148CardIoLowerObservable801C81EC(
    const char* source,
    const PrStage1SaveUi19148LowerFeedbackRequestList& pendingRequests,
    const PrStage1SaveUi19148LowerFeedback* effectiveLowerFeedback,
    const PrStage1SaveUiHostBridgeDirect::SaveUi19148HostTickAttempt& tick) {
    const PrStage1SaveUi19148LowerFeedbackRequest* pending =
        FindSaveUi19148ObservableLowerRequest801C81EC(pendingRequests);
    const PrStage1SaveUi19148LowerFeedbackRequest* next =
        FindSaveUi19148ObservableLowerRequest801C81EC(
            tick.lowerFeedbackRequests);
    const PrStage1SaveUi19148LowerFeedbackRequest* directoryPending =
        FindSaveUi19148LowerRequest801C81EC(
            pendingRequests,
            PrStage1SaveUi19148LowerFeedbackRequestKind::
                DirectoryRows80019458);
    const PrStage1SaveUi19148LowerFeedbackRequest* directoryNext =
        FindSaveUi19148LowerRequest801C81EC(
            tick.lowerFeedbackRequests,
            PrStage1SaveUi19148LowerFeedbackRequestKind::
                DirectoryRows80019458);
    const PrStage1SaveUi19148LowerFeedbackRequest* writePending =
        FindSaveUi19148LowerRequest801C81EC(
            pendingRequests,
            PrStage1SaveUi19148LowerFeedbackRequestKind::Write80017A10);
    const PrStage1SaveUi19148LowerFeedbackRequest* writeNext =
        FindSaveUi19148LowerRequest801C81EC(
            tick.lowerFeedbackRequests,
            PrStage1SaveUi19148LowerFeedbackRequestKind::Write80017A10);

    const PrStage1SaveUiCardIoState80017594 emptyState{};
    const PrStage1SaveUiCardIoState80017594& pendingState =
        pending ? pending->cardIoState : emptyState;
    const PrStage1SaveUiCardIoState80017594& nextState =
        next ? next->cardIoState : emptyState;
    const bool feedbackKnown =
        effectiveLowerFeedback &&
        effectiveLowerFeedback->cardIoFeedbackKnown80017594;
    const bool directoryFeedbackKnown =
        effectiveLowerFeedback &&
        effectiveLowerFeedback->directoryRowsFeedbackKnown80019458;
    const PrStage1SaveUiDirectoryRowsFeedback80019458* directoryFeedback =
        directoryFeedbackKnown
            ? &effectiveLowerFeedback->directoryRowsFeedback80019458
            : nullptr;
    const PrStage1SaveUiCardIoFeedback80017594* feedback =
        feedbackKnown ? &effectiveLowerFeedback->cardIoFeedback80017594
                      : nullptr;
    const PrStage1SaveUiDirectoryRawBankSnapshot8007A318 directDirectory =
        PrStage1SaveUiDirect::GetSaveUiDirectoryRawBankSnapshot8007A318();
    const PrStage1SaveUiCardIoState80017594& feedbackBefore =
        feedback ? feedback->stateBefore : emptyState;
    const PrStage1SaveUiCardIoState80017594& before =
        tick.cardIoStateBeforeKnown80017594
            ? tick.cardIoStateBefore80017594
            : emptyState;
    const PrStage1SaveUiCardIoState80017594& after =
        tick.cardIoStateAfterKnown80017594 ? tick.cardIoStateAfter80017594
                                           : emptyState;
    Log::Printf(
        "Scene1 801C81EC save-ui 19148 cardio lower observable "
        "source=%s pendingCount=%u pendingKind=%u pendingFn=%08X "
        "pendingState=%d/%d/%d/%d/%d feedbackKnown=%d "
        "feedbackBeforeKnown=%d feedbackBefore=%d/%d/%d/%d/%d "
        "pollSw=%d/%d gp700=%d/%d/%d/%d timedOut=%d/%d "
        "consumed=%d before=%d/%d/%d/%d/%d afterKnown=%d "
        "after=%d/%d/%d/%d/%d io=%d/%d "
        "tickActive=%d tickDone=%d psxState=%d psxEvent=%d "
        "inputConsumes=%d dispatcherDrive=%d dispatcherPendingBefore=%d "
        "inputRaw=%d inputBeforeDedup=%d inputAfterDedup=%d "
        "gp708=%d/%d duplicateSuppressed=%d inputHandled=%d "
        "inputState=%d/%d "
        "saveResult=%d saveSuccess=%d saveWrite=%d/%d/%d "
        "state15Payload=%d/%08X state15Copy=%d/%d "
        "gp716=%d/%d gp720=%d/%d nextCount=%u nextKind=%u "
        "nextFn=%08X nextState=%d/%d/%d/%d/%d "
        "directoryPendingKind=%u directoryPendingFn=%08X "
        "directoryNextKind=%u directoryNextFn=%08X "
        "directoryFeedback=%d/%d/%d/%d entry=%d/%d free=%d/%d rows=%d "
        "directoryDirectRawBank=%d/%d nonZeroRows=%d prefixRows=%d "
        "firstNonZero=%d firstPrefix=%d "
        "directoryRawBankAttempt=%d/%d/%d/%d/%d/%d addr=%08X bytes=%u "
        "writePendingKind=%u writePendingFn=%08X "
        "writeNextKind=%u writeNextFn=%08X "
        "writeSourceAttempt=%d/%d/%d/%d/%d/%d addr=%08X bytes=%u",
        source ? source : "none",
        pendingRequests.count,
        pending ? static_cast<unsigned>(pending->kind) : 0u,
        pending ? pending->psxFunction : 0u,
        pendingState.dword800917E8,
        pendingState.dword800917EC,
        pendingState.dword800917F0,
        pendingState.dword800917F4,
        pendingState.gp700,
        feedbackKnown ? 1 : 0,
        feedback && feedback->stateBeforeKnown ? 1 : 0,
        feedbackBefore.dword800917E8,
        feedbackBefore.dword800917EC,
        feedbackBefore.dword800917F0,
        feedbackBefore.dword800917F4,
        feedbackBefore.gp700,
        feedback && feedback->pollSwKnown80016E18 ? 1 : 0,
        feedback ? feedback->pollSwResult80016E18 : 0,
        feedback && feedback->pollSwGp700BeforeKnown80016E18 ? 1 : 0,
        feedback ? feedback->pollSwGp700Before80016E18 : 0,
        feedback && feedback->pollSwGp700AfterKnown80016E18 ? 1 : 0,
        feedback ? feedback->pollSwGp700After80016E18 : 0,
        feedback && feedback->pollSwTimedOutKnown80016E18 ? 1 : 0,
        feedback && feedback->pollSwTimedOut80016E18 ? 1 : 0,
        tick.cardIoStateBeforeKnown80017594 ? 1 : 0,
        before.dword800917E8,
        before.dword800917EC,
        before.dword800917F0,
        before.dword800917F4,
        before.gp700,
        tick.cardIoStateAfterKnown80017594 ? 1 : 0,
        after.dword800917E8,
        after.dword800917EC,
        after.dword800917F0,
        after.dword800917F4,
        after.gp700,
        tick.ioResultKnown ? 1 : 0,
        tick.ioResult,
        tick.active ? 1 : 0,
        tick.done ? 1 : 0,
        tick.psxState,
        tick.psxEventId,
        tick.inputStateConsumes80018FB0 ? 1 : 0,
        tick.inputDispatcherResultDrive80018FB0 ? 1 : 0,
        tick.inputDispatcherResultPendingBefore80018FB0,
        tick.inputMaskRaw80035510,
        tick.inputMaskBeforeDedup80018FB0,
        tick.inputMaskAfterDedup80018FB0,
        tick.gp708LastInputBefore80018FB0,
        tick.gp708LastInputAfter80018FB0,
        tick.inputDuplicateSuppressed80018FB0 ? 1 : 0,
        tick.inputHandled800185D0 ? 1 : 0,
        tick.inputStateBefore800185D0,
        tick.inputStateAfter800185D0,
        tick.saveResult,
        tick.saveSucceeded ? 1 : 0,
        tick.saveWriteResultKnown80017A10 ? 1 : 0,
        tick.saveWriteResult80017A10,
        tick.saveWriteSucceeded80019458 ? 1 : 0,
        tick.consumedBy80019458State15Known ? 1 : 0,
        tick.state15PrefixAddress,
        tick.state15CopyTo8007ADE8Known ? 1 : 0,
        tick.state15CopyTo8007ADE8 ? 1 : 0,
        tick.gp716After80019458Known ? 1 : 0,
        tick.gp716After80019458,
        tick.gp720After80019458Known ? 1 : 0,
        tick.gp720After80019458,
        tick.lowerFeedbackRequests.count,
        next ? static_cast<unsigned>(next->kind) : 0u,
        next ? next->psxFunction : 0u,
        nextState.dword800917E8,
        nextState.dword800917EC,
        nextState.dword800917F0,
        nextState.dword800917F4,
        nextState.gp700,
        directoryPending ? static_cast<unsigned>(directoryPending->kind) : 0u,
        directoryPending ? directoryPending->psxFunction : 0u,
        directoryNext ? static_cast<unsigned>(directoryNext->kind) : 0u,
        directoryNext ? directoryNext->psxFunction : 0u,
        directoryFeedbackKnown ? 1 : 0,
        directoryFeedback && directoryFeedback->translated ? 1 : 0,
        directoryFeedback && directoryFeedback->sourceKnown ? 1 : 0,
        directoryFeedback &&
                directoryFeedback->source ==
                    PrStage1SaveUiDirectoryRowsSource80019458::
                        RuntimeCardDirectoryProducer
            ? 1
            : 0,
        directoryFeedback && directoryFeedback->entryCountKnown ? 1 : 0,
        directoryFeedback ? directoryFeedback->entryCount : 0,
        directoryFeedback && directoryFeedback->freeSlotsKnown ? 1 : 0,
        directoryFeedback ? directoryFeedback->freeSlots : 0,
        directoryFeedback && directoryFeedback->rowsKnown ? 1 : 0,
        directDirectory.known ? 1 : 0,
        directDirectory.anyNonZero ? 1 : 0,
        directDirectory.nonZeroRows,
        directDirectory.savePrefixRows,
        directDirectory.firstNonZeroRow,
        directDirectory.firstSavePrefixRow,
        s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.requestPresent ? 1
                                                                          : 0,
        s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.providerInstalled
            ? 1
            : 0,
        s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.readAttempted ? 1
                                                                         : 0,
        s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.windowReadable ? 1
                                                                          : 0,
        s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.factsBuilt ? 1
                                                                      : 0,
        s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.carrierPublished
            ? 1
            : 0,
        s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.psxAddress,
        s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.byteSize,
        writePending ? static_cast<unsigned>(writePending->kind) : 0u,
        writePending ? writePending->psxFunction : 0u,
        writeNext ? static_cast<unsigned>(writeNext->kind) : 0u,
        writeNext ? writeNext->psxFunction : 0u,
        s_saveUi19148WriteSourceAttempt801C81EC.requestPresent ? 1 : 0,
        s_saveUi19148WriteSourceAttempt801C81EC.writeBlockKnown ? 1 : 0,
        s_saveUi19148WriteSourceAttempt801C81EC.saveHeaderBuilt ? 1 : 0,
        s_saveUi19148WriteSourceAttempt801C81EC.savePayloadCopied ? 1 : 0,
        s_saveUi19148WriteSourceAttempt801C81EC.factsBuilt ? 1 : 0,
        s_saveUi19148WriteSourceAttempt801C81EC.carrierPublished ? 1 : 0,
        s_saveUi19148WriteSourceAttempt801C81EC.psxAddress,
        s_saveUi19148WriteSourceAttempt801C81EC.byteSize);
}

bool TryBuildSaveUi19148ObservedCardIoLowerFeedback801C81EC(
    const PrStage1SaveUi19148LowerFeedbackRequestList& pendingRequests,
    PrStage1SaveUi19148LowerFeedback& outLowerFeedback) {
    if (outLowerFeedback.cardIoFeedbackKnown80017594) {
        return false;
    }
    const PrStage1SaveUi19148LowerFeedbackRequest* request =
        FindSaveUi19148LowerRequest801C81EC(
            pendingRequests,
            PrStage1SaveUi19148LowerFeedbackRequestKind::CardIo80017594);
    if (!request) {
        return false;
    }

    PrStage1SaveCardHalDirect::SaveUiCardIoState3TypedPollCarrier80017594
        typedPollCarrier{};
    if (PrStage1SaveCardHalDirect::
            GetSaveUiCardIoState3TypedPollCarrier80017594(
                &typedPollCarrier)) {
        const PrStage1SaveUiCardIoFeedback80017594& feedback =
            typedPollCarrier.lower.lowerFeedback.cardIoFeedback80017594;
        if (!typedPollCarrier.lower.lowerFeedbackKnown ||
            !typedPollCarrier.lower.lowerFeedback.cardIoFeedbackKnown80017594 ||
            !feedback.stateBeforeKnown ||
            !CardIoStateEquals801C81EC(feedback.stateBefore,
                                       request->cardIoState)) {
            PrStage1SaveCardHalDirect::
                ClearSaveUiCardIoState3TypedPollCarrier80017594();
            return false;
        }

        outLowerFeedback.cardIoFeedbackKnown80017594 = true;
        outLowerFeedback.cardIoFeedback80017594 = feedback;
        PrStage1SaveCardHalDirect::
            ClearSaveUiCardIoState3TypedPollCarrier80017594();
        return true;
    }

    PrStage1SaveCardHalDirect::CardIoHostFacts80017594 facts{};
    if (!PrStage1SaveCardHalDirect::
            BuildSaveUiCardIoObservedNormalPathFacts80017594(
                *request,
                &facts)) {
        return false;
    }
    PrStage1SaveCardHalDirect::CardIoLowerFeedbackBuildResult80017594
        build{};
    PrStage1SaveCardHalDirect::
        BuildSaveUiCardIoLowerFeedbackFromHostFacts80017594(facts, &build);
    if (!build.lowerFeedbackKnown ||
        !build.lowerFeedback.cardIoFeedbackKnown80017594) {
        return false;
    }

    outLowerFeedback.cardIoFeedbackKnown80017594 = true;
    outLowerFeedback.cardIoFeedback80017594 =
        build.lowerFeedback.cardIoFeedback80017594;
    return true;
}

bool TryBuildSaveUi19148DirectoryRowsLowerFeedback801C81EC(
    const PrStage1SaveUi19148LowerFeedbackRequestList& pendingRequests,
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& runtimeProvider,
    PrStage1SaveUi19148LowerFeedback& outLowerFeedback) {
    if (outLowerFeedback.directoryRowsFeedbackKnown80019458) {
        return false;
    }
    s_saveUi19148DirectoryRawBankSourceAttempt801C81EC = {};
    const PrStage1SaveUi19148LowerFeedbackRequest* request =
        FindSaveUi19148LowerRequest801C81EC(
            pendingRequests,
            PrStage1SaveUi19148LowerFeedbackRequestKind::
                DirectoryRows80019458);
    if (!request || request->psxFunction != kFn80017B18) {
        return false;
    }
    s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.requestPresent = true;
    s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.psxAddress =
        kSaveUiDirectoryRawBank8007A318;
    s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.byteSize =
        kSaveUiDirectoryRawBankBytes80019458;
    s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.directDirectoryLoaded =
        s_saveUi19148CardEventHalState801C81EC
            .directDirectoryLoaded80017594;

    PrStage1SaveCardHalDirect::SaveUiDirectoryScanTypedCarrier80019458
        carrier{};
    if (!PrStage1SaveCardHalDirect::
            GetSaveUiDirectoryScanTypedCarrier80019458(&carrier)) {
        std::array<uint8_t, kSaveUiDirectoryRawBankBytes80019458> rawBank{};
        const uint8_t* rawBankBytes = nullptr;
        std::size_t rawBankByteCount = 0u;
        const PrStage1SaveUiDirectoryRawBankView8007A318 directRawBank =
            PrStage1SaveUiDirect::GetSaveUiDirectoryRawBankView8007A318();
        s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.directRawBankKnown =
            s_saveUi19148DirectoryRawBankSourceAttempt801C81EC
                .directDirectoryLoaded &&
            directRawBank.known &&
            directRawBank.psxAddress == kSaveUiDirectoryRawBank8007A318 &&
            directRawBank.byteSize == kSaveUiDirectoryRawBankBytes80019458 &&
            directRawBank.bytes != nullptr &&
            directRawBank.byteCount >= kSaveUiDirectoryRawBankBytes80019458;
        if (s_saveUi19148DirectoryRawBankSourceAttempt801C81EC
                .directDirectoryLoaded) {
            if (!s_saveUi19148DirectoryRawBankSourceAttempt801C81EC
                     .directRawBankKnown) {
                return false;
            }
            rawBankBytes = directRawBank.bytes;
            rawBankByteCount = directRawBank.byteCount;
        } else {
            s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.providerInstalled =
                runtimeProvider.installed && runtimeProvider.read != nullptr;
            s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.readAttempted =
                s_saveUi19148DirectoryRawBankSourceAttempt801C81EC
                    .providerInstalled;
            if (!s_saveUi19148DirectoryRawBankSourceAttempt801C81EC
                     .providerInstalled ||
                !runtimeProvider.read(runtimeProvider.userData,
                                      kSaveUiDirectoryRawBank8007A318,
                                      kSaveUiDirectoryRawBankBytes80019458,
                                      rawBank.data(),
                                      rawBank.size())) {
                return false;
            }
            rawBankBytes = rawBank.data();
            rawBankByteCount = rawBank.size();
        }
        s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.windowReadable =
            true;

        PrStage1SaveCardHalDirect::
            SaveUiDirectoryRawBankProducerInput80019458 input{};
        input.requestKnown = true;
        input.request = *request;
        input.rawBankKnown8007A318 = true;
        input.rawBank8007A318 = rawBankBytes;
        input.rawBankByteCount8007A318 = rawBankByteCount;
        PrStage1SaveCardHalDirect::
            SaveUiDirectoryRawBankProducerResult80019458 result{};
        PrStage1SaveCardHalDirect::
            BuildSaveUiDirectoryScanFactsFromRawBankProducerInput80019458(
                input,
                &result);
        s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.factsBuilt =
            result.produced && !result.incomplete && result.requestMatched;
        if (!s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.factsBuilt) {
            return false;
        }
        s_saveUi19148DirectoryRawBankSourceAttempt801C81EC
            .carrierPublishAttempted = true;
        s_saveUi19148DirectoryRawBankSourceAttempt801C81EC.carrierPublished =
            PrStage1SaveCardHalDirect::
                PublishRuntimeSaveUiDirectoryScanTypedCarrier80019458(
                    result.facts);
        if (!s_saveUi19148DirectoryRawBankSourceAttempt801C81EC
                 .carrierPublished ||
            !PrStage1SaveCardHalDirect::
                GetSaveUiDirectoryScanTypedCarrier80019458(&carrier)) {
            return false;
        }
    }

    outLowerFeedback.directoryRowsFeedbackKnown80019458 = true;
    outLowerFeedback.directoryRowsFeedback80019458 = carrier.feedback;
    return true;
}

bool BeginSaveUi19148DirectWriteAttempt801C81EC(
    const PrStage1SaveUi19148LowerFeedbackRequest& request) {
    SaveUi19148DirectWriteRuntime801C81EC& runtime =
        s_saveUi19148DirectWriteRuntime801C81EC;
    if (!runtime.active || runtime.completed || runtime.waitStarted80035560 ||
        runtime.attemptIndex < 0 ||
        runtime.attemptIndex >= kSaveUiWriteRetryCount80017A10) {
        return false;
    }

    const PrStage1SaveUiNameBufferView8007CBE8 name =
        PrStage1SaveUiDirect::GetSaveUiNameBufferView8007CBE8();
    if (!name.known || name.psxAddress != request.nameAddress ||
        name.bytes == nullptr || name.byteCount == 0u) {
        return false;
    }
    const PrStage1SaveUiDirectoryNameScan80017900 scan =
        PrStage1SaveUiDirect::ScanSaveUiDirectoryRawBankName80017900(
            name.bytes);
    if (!scan.attempted || !scan.directoryKnown || !scan.nameKnown) {
        return false;
    }

    PrStage1SaveCardHalDirect::CardWriteHostAttemptFacts80017A10& attempt =
        runtime.facts.attempts[runtime.attemptIndex];
    attempt = {};
    attempt.scanResultKnown80017900 = true;
    attempt.scanResult80017900 = scan.psxReturn;
    if (scan.psxReturn != 1) {
        attempt.openCheckKnown80017454 = true;
        attempt.openCheckReturnKnown80017454 = true;
        attempt.openCheckReturn80017454 = kSaveUiDirectCardWriteFd80017454;
        attempt.openCheckFdKnown80017454 = true;
        attempt.openCheckFd80017454 = kSaveUiDirectCardWriteFd80017454;
        attempt.openCheckCloseKnown80017454 = true;
        attempt.openCheckCloseFd80017454 = kSaveUiDirectCardWriteFd80017454;
    }
    attempt.openWriteKnown80017454 = true;
    attempt.openWriteFdKnown80017454 = true;
    attempt.openWriteFd80017454 = kSaveUiDirectCardWriteFd80017454;
    attempt.openWriteReturnKnown80017454 = true;
    attempt.openWriteReturn80017454 = kSaveUiDirectCardWriteFd80017454;
    attempt.gp696FdWriteKnown80017454 = true;
    attempt.gp696Fd80017454 = kSaveUiDirectCardWriteFd80017454;
    attempt.clearSwEventsKnown80016FC0 = true;
    attempt.writeKnown80017454 = true;
    attempt.writeByteCountKnown80017454 = true;
    attempt.writeByteCount80017454 =
        static_cast<int32_t>(kSaveUiWriteBlockBytes80017A10);
    attempt.submitReturnKnown80017454 = true;
    attempt.submitReturn80017454 = 0;

    const bool writePrepared =
        CommitSaveUi19148DirectCardBufferSink801C81EC(request) &&
        SerializeSaveUi19148DirectCardBufferEntry801C81EC();
    if (!writePrepared) {
        runtime.blocked = true;
        (void)RollbackSaveUi19148DirectWriteRuntime801C81EC();
        return false;
    }
    runtime.durableAttempted = true;
    runtime.durableCommitted =
        AttemptSaveUi19148DirectDurableHandoff801C81EC();
    runtime.backendCompletionKnown =
        s_saveUi19148DirectDurableHandoff801C81EC
            .durablePrimitiveAttempted &&
        s_saveUi19148DirectDurableHandoff801C81EC.directBackendKnown &&
        s_saveUi19148DirectDurableHandoff801C81EC.directBackendCalled;
    if (!runtime.backendCompletionKnown) {
        runtime.blocked = true;
        (void)RollbackSaveUi19148DirectWriteRuntime801C81EC();
        return false;
    }
    attempt.waitCallKnown80035560 = true;
    attempt.waitArg80035560 = 4;

    const PrPsxVSyncDirect::PsxVSyncResult80035560 wait =
        PrPsxVSyncDirect::BeginVSync80035560(
            PrPsxVSyncDirect::ProcessVSyncState80035560(), 4);
    runtime.waitStarted80035560 =
        wait.sourceKnown && wait.waitPending &&
        wait.mode == PrPsxVSyncDirect::PsxVSyncMode80035560::WaitForVblanks;
    if (!runtime.waitStarted80035560) {
        runtime.blocked = true;
    }
    Log::Printf(
        "Scene1 801C81EC save-ui 19148 direct write attempt started: attempt=%d scan=%d durable=%d/%d wait4=%d",
        runtime.attemptIndex,
        scan.psxReturn,
        runtime.durableAttempted ? 1 : 0,
        runtime.durableCommitted ? 1 : 0,
        runtime.waitStarted80035560 ? 1 : 0);
    return runtime.waitStarted80035560;
}

bool CompleteSaveUi19148DirectWriteAttempt801C81EC(
    PrStage1SaveUi19148LowerFeedback& outLowerFeedback) {
    SaveUi19148DirectWriteRuntime801C81EC& runtime =
        s_saveUi19148DirectWriteRuntime801C81EC;
    if (!runtime.active || runtime.completed || !runtime.waitStarted80035560 ||
        runtime.attemptIndex < 0 ||
        runtime.attemptIndex >= kSaveUiWriteRetryCount80017A10) {
        return false;
    }

    const PrPsxVSyncDirect::PsxVSyncResult80035560 wait =
        PrPsxVSyncDirect::ConsumeHostVblanks80035560(
            PrPsxVSyncDirect::ProcessVSyncState80035560(), 1);
    const bool waitComplete =
        wait.sourceKnown && wait.softwareStateCommitted &&
        wait.softwareWaitComplete &&
        wait.mode == PrPsxVSyncDirect::PsxVSyncMode80035560::WaitForVblanks;
    if (!waitComplete) {
        return false;
    }

    PrStage1SaveCardHalDirect::CardWriteHostAttemptFacts80017A10& attempt =
        runtime.facts.attempts[runtime.attemptIndex];
    PrStage1SaveCardHalDirect::SaveUiDirectWriteCompletion80017A10
        completion{};
    completion.backendCompletionKnown = runtime.backendCompletionKnown;
    completion.backendAccepted = runtime.durableCommitted;
    completion.waitCompleted80035560 = true;
    completion.virtualFd80017454 = kSaveUiDirectCardWriteFd80017454;
    if (!PrStage1SaveCardHalDirect::
            FinalizeSaveUiDirectWriteAttempt80017A10(completion, &attempt)) {
        return false;
    }
    runtime.waitStarted80035560 = false;

    const bool terminal =
        runtime.durableCommitted ||
        runtime.attemptIndex + 1 >= kSaveUiWriteRetryCount80017A10;
    if (!terminal) {
        ++runtime.attemptIndex;
        return false;
    }

    if (!runtime.durableCommitted) {
        if (!RollbackSaveUi19148DirectWriteRuntime801C81EC()) {
            return false;
        }
    }

    PrStage1SaveCardHalDirect::CardWriteFeedbackProducerInput80017A10 input{};
    input.requestKnown = true;
    input.request = runtime.request;
    input.hostFactsKnown = true;
    input.hostFacts = runtime.facts;
    PrStage1SaveCardHalDirect::CardWriteLowerFeedbackBuildResult80017A10
        lower{};
    PrStage1SaveCardHalDirect::
        BuildSaveUiWriteLowerFeedbackFromProducerInput80017A10(input, &lower);
    if (!lower.lowerFeedbackKnown || lower.anyMissingRequiredFact ||
        !lower.lowerFeedback.writeFeedbackKnown80017A10) {
        return false;
    }

    runtime.completed = true;
    runtime.completedFeedback = lower.lowerFeedback;
    s_saveUi19148WriteSourceAttempt801C81EC.factsBuilt = true;
    outLowerFeedback.writeFeedbackKnown80017A10 = true;
    outLowerFeedback.writeFeedback80017A10 =
        runtime.completedFeedback.writeFeedback80017A10;
    Log::Printf(
        "Scene1 801C81EC save-ui 19148 direct write transaction completed: attempts=%d poll=%d durable=%d result=%d",
        runtime.attemptIndex + 1,
        attempt.pollResult80016EB8,
        runtime.durableCommitted ? 1 : 0,
        runtime.durableCommitted ? 0 : -1);
    return true;
}

bool TryBuildSaveUi19148WriteLowerFeedback801C81EC(
    const PrStage1SaveUi19148LowerFeedbackRequestList& pendingRequests,
    PrStage1SaveUi19148LowerFeedback& outLowerFeedback) {
    if (outLowerFeedback.writeFeedbackKnown80017A10) {
        return false;
    }
    const PrStage1SaveUi19148LowerFeedbackRequest* request =
        FindSaveUi19148LowerRequest801C81EC(
            pendingRequests,
            PrStage1SaveUi19148LowerFeedbackRequestKind::Write80017A10);
    if (!request || request->psxFunction != kFn80017A10) {
        ResetSaveUi19148DirectWriteRuntime801C81EC();
        s_saveUi19148WriteSourceAttempt801C81EC = {};
        PrStage1SaveCardHalDirect::ClearSaveUiWriteTypedCarrier80017A10();
        return false;
    }
    s_saveUi19148WriteSourceAttempt801C81EC.requestPresent = true;

    PrStage1SaveCardHalDirect::SaveUiWriteTypedCarrier80017A10 carrier{};
    if (PrStage1SaveCardHalDirect::GetSaveUiWriteTypedCarrier80017A10(
            &carrier)) {
        if (!carrier.lower.lowerFeedbackKnown ||
            !carrier.lower.lowerFeedback.writeFeedbackKnown80017A10 ||
            carrier.incomplete ||
            !carrier.producerWired80017900_80017454_80016EB8_80017A10 ||
            !carrier.typedWriteSuccessKnown80017A10) {
            return false;
        }
        outLowerFeedback.writeFeedbackKnown80017A10 = true;
        outLowerFeedback.writeFeedback80017A10 =
            carrier.lower.lowerFeedback.writeFeedback80017A10;
        return true;
    }

    const PrStage1SaveUiWriteBlockView80017A10 writeBlock =
        PrStage1SaveUiDirect::GetSaveUiWriteBlockView80017A10();
    s_saveUi19148WriteSourceAttempt801C81EC.writeBlockKnown =
        writeBlock.known &&
        writeBlock.psxAddress ==
            PrStage1SaveCardHalDirect::kCardReadBlockBufferAddr800179B4 &&
        writeBlock.byteCount >=
            PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4 &&
        writeBlock.bytes != nullptr;
    s_saveUi19148WriteSourceAttempt801C81EC.saveHeaderBuilt =
        writeBlock.saveHeaderBuilt;
    s_saveUi19148WriteSourceAttempt801C81EC.savePayloadCopied =
        writeBlock.savePayloadCopied;
    s_saveUi19148WriteSourceAttempt801C81EC.psxAddress =
        writeBlock.psxAddress;
    s_saveUi19148WriteSourceAttempt801C81EC.byteSize = writeBlock.byteSize;
    if (!s_saveUi19148WriteSourceAttempt801C81EC.writeBlockKnown) {
        ResetSaveUi19148DirectWriteRuntime801C81EC();
        return false;
    }

    SaveUi19148DirectWriteRuntime801C81EC& runtime =
        s_saveUi19148DirectWriteRuntime801C81EC;
    if (!runtime.active ||
        !SameSaveUi19148WriteRequest801C81EC(runtime.request, *request)) {
        ResetSaveUi19148DirectWriteRuntime801C81EC();
        runtime.active = true;
        runtime.request = *request;
        runtime.facts.factsKnown = true;
        const PrStage1SaveUiDirectoryRawBankView8007A318 directory =
            PrStage1SaveUiDirect::GetSaveUiDirectoryRawBankView8007A318();
        runtime.originalDirectoryKnown =
            directory.known && directory.bytes != nullptr &&
            directory.byteCount == runtime.originalDirectory.size();
        if (!runtime.originalDirectoryKnown) {
            ResetSaveUi19148DirectWriteRuntime801C81EC();
            return false;
        }
        std::memcpy(runtime.originalDirectory.data(),
                    directory.bytes,
                    runtime.originalDirectory.size());
        const PrStage1SaveUiCardImagePersistenceView8007A318 previousImage =
            PrStage1SaveUiDirect::
                GetSaveUiCardImagePersistenceSinkView8007A318();
        runtime.previousCardImageKnown =
            previousImage.known && previousImage.slotPolicyKnown &&
            previousImage.blockIndex >= 0 && previousImage.blockIndex < 15 &&
            previousImage.durableCommitted && previousImage.bytes != nullptr &&
            previousImage.byteCount == runtime.previousCardImage.size();
        if (runtime.previousCardImageKnown) {
            runtime.previousCardImageBlockIndex = previousImage.blockIndex;
            std::memcpy(runtime.previousCardImage.data(),
                        previousImage.bytes,
                        runtime.previousCardImage.size());
        }
    }
    if (runtime.completed) {
        outLowerFeedback.writeFeedbackKnown80017A10 = true;
        outLowerFeedback.writeFeedback80017A10 =
            runtime.completedFeedback.writeFeedback80017A10;
        return true;
    }
    if (runtime.blocked) {
        return false;
    }
    if (runtime.waitStarted80035560) {
        return CompleteSaveUi19148DirectWriteAttempt801C81EC(
            outLowerFeedback);
    }
    (void)BeginSaveUi19148DirectWriteAttempt801C81EC(*request);
    return false;
}

bool TryBuildSaveUi19148FormatLowerFeedback801C81EC(
    const PrStage1SaveUi19148LowerFeedbackRequestList& pendingRequests,
    PrStage1SaveUi19148LowerFeedback& outLowerFeedback) {
    if (outLowerFeedback.formatFeedbackKnown80017B60) {
        return false;
    }
    const PrStage1SaveUi19148LowerFeedbackRequest* request =
        FindSaveUi19148LowerRequest801C81EC(
            pendingRequests,
            PrStage1SaveUi19148LowerFeedbackRequestKind::Format80017B60);
    if (!request || request->psxFunction != kFn80017B60 ||
        request->retryCount != kSaveUiFormatRetryCount80017B60 ||
        request->formatArg0 != kSaveUiFormatArg0_80017B60 ||
        request->formatArg1 != kSaveUiFormatArg1_80017B60 ||
        request->action.kind !=
            PrStage1SaveUi19148ActionKind::Call80017B60FormatCard) {
        ResetSaveUi19148DirectFormatRuntime801C81EC();
        PrStage1SaveCardHalDirect::ClearSaveUiFormatTypedCarrier80017B60();
        return false;
    }

    PrStage1SaveCardHalDirect::SaveUiFormatTypedCarrier80017B60 carrier{};
    if (s_saveUi19148CardEventHalState801C81EC.
            probeOnlyFormatTerminalReady80017B60) {
        PrStage1SaveCardHalDirect::SaveUiFormatRuntimeFacts80017B60 facts{};
        facts.factsKnown = true;
        for (int32_t i = 0; i < kSaveUiFormatRetryCount80017B60; ++i) {
            PrStage1SaveCardHalDirect::
                SaveUiFormatRuntimeAttemptFacts80017B60& attempt =
                    facts.attempts[i];
            attempt.drainHwEventsKnown8001707C = true;
            attempt.formatKnown = true;
            attempt.formatArgsKnown = true;
            attempt.formatArg0 = kSaveUiFormatArg0_80017B60;
            attempt.formatArg1 = kSaveUiFormatArg1_80017B60;
            attempt.pollResultKnown80017008 = true;
            attempt.pollResult80017008 =
                s_saveUi19148CardEventHalState801C81EC.
                    probeOnlyFormatTerminalPollResult80017008;
            if (attempt.pollResult80017008 == 3) {
                break;
            }
        }
        const int32_t terminalPollResult =
            s_saveUi19148CardEventHalState801C81EC.
                probeOnlyFormatTerminalPollResult80017008;
        s_saveUi19148CardEventHalState801C81EC.
            probeOnlyFormatTerminalReady80017B60 = false;
        s_saveUi19148CardEventHalState801C81EC.
            probeOnlyFormatTerminalPollResult80017008 = 0;
        PrStage1SaveCardHalDirect::ClearSaveUiFormatTypedCarrier80017B60();
        if (!PrStage1SaveCardHalDirect::
                PublishRuntimeSaveUiFormatTypedCarrier80017B60(*request,
                                                               facts)) {
            return false;
        }
        Log::Printf(
            "Scene1 801C81EC save-ui 19148 probe-only format terminal feedback published nonAuthority=1 pollResult80017008=%d",
            terminalPollResult);
        if (!PrStage1SaveCardHalDirect::GetSaveUiFormatTypedCarrier80017B60(
                &carrier) || carrier.incomplete ||
            !carrier.producerWired8001707C_80017008_80017B60 ||
            !carrier.formatCallCompleted80017B60 ||
            !carrier.lower.lowerFeedbackKnown ||
            !carrier.lower.lowerFeedback.formatFeedbackKnown80017B60) {
            return false;
        }
        outLowerFeedback.formatFeedbackKnown80017B60 = true;
        outLowerFeedback.formatFeedback80017B60 =
            carrier.lower.lowerFeedback.formatFeedback80017B60;
        return true;
    }

    SaveUi19148DirectFormatRuntime801C81EC& runtime =
        s_saveUi19148DirectFormatRuntime801C81EC;
    if (!runtime.active ||
        !SameSaveUi19148FormatRequest801C81EC(runtime.request, *request)) {
        ResetSaveUi19148DirectFormatRuntime801C81EC();
        PrStage1SaveCardHalDirect::ClearSaveUiFormatTypedCarrier80017B60();
        runtime.active = true;
        runtime.request = *request;
        runtime.facts.factsKnown = true;

        const PrStage1SaveUiDirectoryRawBankView8007A318 directory =
            PrStage1SaveUiDirect::GetSaveUiDirectoryRawBankView8007A318();
        runtime.originalDirectoryKnown =
            directory.known && directory.bytes != nullptr &&
            directory.byteCount == runtime.originalDirectory.size();
        if (!runtime.originalDirectoryKnown) {
            runtime.blocked = true;
            return false;
        }
        std::memcpy(runtime.originalDirectory.data(),
                    directory.bytes,
                    runtime.originalDirectory.size());

        const PrStage1SaveUiCardImagePersistenceView8007A318 previousImage =
            PrStage1SaveUiDirect::
                GetSaveUiCardImagePersistenceSinkView8007A318();
        runtime.previousCardImageKnown =
            previousImage.known && previousImage.slotPolicyKnown &&
            previousImage.blockIndex >= 0 && previousImage.blockIndex < 15 &&
            previousImage.durablePolicyKnown &&
            previousImage.durableCommitted && previousImage.bytes != nullptr &&
            previousImage.byteCount == runtime.previousCardImage.size();
        if (previousImage.known && !runtime.previousCardImageKnown) {
            runtime.blocked = true;
            return false;
        }
        if (runtime.previousCardImageKnown) {
            runtime.previousCardImageBlockIndex = previousImage.blockIndex;
            std::memcpy(runtime.previousCardImage.data(),
                        previousImage.bytes,
                        runtime.previousCardImage.size());
        }
    }
    if (runtime.blocked) {
        return false;
    }

    if (!runtime.completed) {
        for (int32_t i = 0; i < kSaveUiFormatRetryCount80017B60; ++i) {
            PrStage1SaveCardHalDirect::SaveUiFormatRuntimeAttemptFacts80017B60&
                attempt = runtime.facts.attempts[i];
            attempt = {};
            PrStage1SaveCardHalDirect::DrainTranslatedHwCardEvents8001707C();
            attempt.drainHwEventsKnown8001707C = true;
            attempt.formatKnown = true;
            attempt.formatArgsKnown = true;
            attempt.formatArg0 = kSaveUiFormatArg0_80017B60;
            attempt.formatArg1 = kSaveUiFormatArg1_80017B60;

            const PrStage1SaveUiDirectFormatResult80017B60 format =
                PrStage1SaveUiDirect::FormatSaveUiDirectCardImage80017B60();
            const PrStage1SaveUiCardImagePersistenceSink8007A318 sink =
                PrStage1SaveUiDirect::
                    CommitSaveUiCardImagePersistenceSink8007A318();
            if (!format.attempted || !format.directoryKnown ||
                !format.formatted || !format.cardImageCandidateKnown ||
                !format.pendingPersistenceCleared ||
                !format.slotPolicyKnown || format.blockIndex < 0 ||
                !sink.attempted || !sink.cardImageCandidateKnown ||
                !sink.sinkCommitted || !sink.slotPolicyKnown ||
                sink.blockIndex != format.blockIndex) {
                runtime.blocked = true;
                runtime.rollbackCompleted =
                    RollbackSaveUi19148DirectFormatRuntime801C81EC();
                return false;
            }

            runtime.durableAttempted = true;
            const PrStage1SaveUiCardImageDurableCommitPrimitive8007A318
                durable = PrStage1SaveUiDirect::
                    CommitSaveUiCardImageDirectDurablePrimitive8007A318();
            runtime.backendCompletionKnown =
                durable.directBackendKnown && durable.directBackendCalled;
            if (!runtime.backendCompletionKnown) {
                runtime.blocked = true;
                runtime.rollbackCompleted =
                    RollbackSaveUi19148DirectFormatRuntime801C81EC();
                return false;
            }

            runtime.durableCommitted = durable.durableCommitted;
            runtime.attemptsUsed = i + 1;
            const int32_t signaledEvent80017008 =
                runtime.durableCommitted ? 1 : 2;
            if (!PrStage1SaveCardHalDirect::
                    SignalTranslatedHwCardEvent80017008(
                        PrStage1SaveCardHalDirect::
                            CardTranslatedEventSignalSource::Format80017B60,
                        signaledEvent80017008)) {
                runtime.blocked = true;
                runtime.rollbackCompleted =
                    RollbackSaveUi19148DirectFormatRuntime801C81EC();
                return false;
            }
            PrStage1SaveCardHalDirect::CardNaturalHwCardEventInput80017008
                naturalEvent{};
            int32_t pollResult80017008 = 0;
            if (!PrStage1SaveCardHalDirect::
                    PollTranslatedHwCardEvents80017008(&naturalEvent) ||
                !PrStage1SaveCardHalDirect::
                    ComputeNaturalHwCardPollResult80017008(
                        naturalEvent,
                        &pollResult80017008)) {
                runtime.blocked = true;
                runtime.rollbackCompleted =
                    RollbackSaveUi19148DirectFormatRuntime801C81EC();
                return false;
            }
            attempt.pollResultKnown80017008 = true;
            attempt.pollResult80017008 = pollResult80017008;
            if (runtime.durableCommitted) {
                PrStage1SaveCardHalDirect::
                    ClearState16CardReadTypedCarrier800179B4();
                PrStage1SaveCardHalDirect::
                    ClearCase17CardReadTypedCarrier800179B4();
                break;
            }
        }

        if (!runtime.durableCommitted) {
            runtime.rollbackCompleted =
                RollbackSaveUi19148DirectFormatRuntime801C81EC();
            if (!runtime.rollbackCompleted) {
                runtime.blocked = true;
                return false;
            }
        }
        runtime.completed = true;
        if (!PrStage1SaveCardHalDirect::
                PublishRuntimeSaveUiFormatTypedCarrier80017B60(
                    *request, runtime.facts)) {
            runtime.blocked = true;
            return false;
        }
        Log::Printf(
            "Scene1 801C81EC save-ui 19148 direct format transaction completed through translated HwCARD events: attempts=%d durable=%d terminalPoll=%d retryExhaustedReturnKnown=%d rollback=%d",
            runtime.attemptsUsed,
            runtime.durableCommitted ? 1 : 0,
            runtime.durableCommitted ? 1 : 2,
            runtime.durableCommitted ? 1 : 0,
            runtime.rollbackCompleted ? 1 : 0);
    }

    if (!PrStage1SaveCardHalDirect::GetSaveUiFormatTypedCarrier80017B60(
            &carrier)) {
        return false;
    }
    if (carrier.incomplete ||
        !carrier.producerWired8001707C_80017008_80017B60 ||
        !carrier.formatCallCompleted80017B60 ||
        !carrier.lower.lowerFeedbackKnown ||
        !carrier.lower.lowerFeedback.formatFeedbackKnown80017B60) {
        return false;
    }

    outLowerFeedback.formatFeedbackKnown80017B60 = true;
    outLowerFeedback.formatFeedback80017B60 =
        carrier.lower.lowerFeedback.formatFeedback80017B60;
    return true;
}

enum class SaveUi19148WriteCommitGate801C81EC : uint8_t {
    NoWriteRequest,
    NoTypedSuccess,
    Blocked,
    Accepted,
};

bool CommitSaveUi19148DirectCardBufferSink801C81EC(
    const PrStage1SaveUi19148LowerFeedbackRequest& request) {
    s_saveUi19148DirectCardBufferSink801C81EC =
        SaveUi19148DirectCardBufferSink801C81EC{};
    s_saveUi19148DirectCardBufferSink801C81EC.attempted = true;
    s_saveUi19148DirectCardBufferSink801C81EC.nameAddress =
        request.nameAddress;
    s_saveUi19148DirectCardBufferSink801C81EC.blockCount =
        request.blockCount;
    s_saveUi19148DirectCardBufferSink801C81EC.requestShapeOk =
        request.kind ==
            PrStage1SaveUi19148LowerFeedbackRequestKind::Write80017A10 &&
        request.psxFunction == kFn80017A10 &&
        request.nameAddress ==
            PrStage1SaveCardHalDirect::kCardReadNameBufferAddr8007CBE8 &&
        request.dataAddress == kSaveUiWriteBlock8007ABE8 &&
        request.blockCount == 1;
    if (!s_saveUi19148DirectCardBufferSink801C81EC.requestShapeOk) {
        return false;
    }

    const PrStage1SaveUiWriteBlockView80017A10 view =
        PrStage1SaveUiDirect::GetSaveUiWriteBlockView80017A10();
    s_saveUi19148DirectCardBufferSink801C81EC.blockViewKnown =
        view.known &&
        view.psxAddress == kSaveUiWriteBlock8007ABE8 &&
        view.byteSize == kSaveUiWriteBlockBytes80017A10 &&
        view.bytes != nullptr &&
        view.byteCount >= kSaveUiWriteBlockBytes80017A10;
    s_saveUi19148DirectCardBufferSink801C81EC.psxAddress =
        view.psxAddress;
    s_saveUi19148DirectCardBufferSink801C81EC.byteSize = view.byteSize;
    if (!s_saveUi19148DirectCardBufferSink801C81EC.blockViewKnown) {
        return false;
    }

    std::memcpy(
        s_saveUi19148DirectCardBufferSink801C81EC.blockBytes.data(),
        view.bytes,
        kSaveUiWriteBlockBytes80017A10);
    s_saveUi19148DirectCardBufferSink801C81EC.committed = true;
    s_saveUi19148DirectCardBufferSink801C81EC.sinkAuthorityKnown = true;
    Log::Printf(
        "Scene1 801C81EC save-ui 19148 direct card-buffer sink committed without host filesystem: name=%08X data=%08X blocks=%d bytes=%u",
        request.nameAddress,
        request.dataAddress,
        request.blockCount,
        kSaveUiWriteBlockBytes80017A10);
    return true;
}

bool SerializeSaveUi19148DirectCardBufferEntry801C81EC() {
    s_saveUi19148SerializedCardEntry801C81EC =
        SaveUi19148SerializedCardEntry801C81EC{};
    s_saveUi19148SerializedCardEntry801C81EC.attempted = true;
    s_saveUi19148SerializedCardEntry801C81EC.cardBufferAuthorityKnown =
        s_saveUi19148DirectCardBufferSink801C81EC.committed &&
        s_saveUi19148DirectCardBufferSink801C81EC.sinkAuthorityKnown &&
        s_saveUi19148DirectCardBufferSink801C81EC.psxAddress ==
            kSaveUiWriteBlock8007ABE8 &&
        s_saveUi19148DirectCardBufferSink801C81EC.byteSize ==
            kSaveUiWriteBlockBytes80017A10;
    if (!s_saveUi19148SerializedCardEntry801C81EC.cardBufferAuthorityKnown) {
        return false;
    }

    const PrStage1SaveUiNameBufferView8007CBE8 nameView =
        PrStage1SaveUiDirect::GetSaveUiNameBufferView8007CBE8();
    s_saveUi19148SerializedCardEntry801C81EC.nameKnown =
        nameView.known &&
        nameView.psxAddress ==
            PrStage1SaveCardHalDirect::kCardReadNameBufferAddr8007CBE8 &&
        nameView.bytes != nullptr &&
        nameView.byteCount > 1u &&
        nameView.byteCount <=
            sizeof(s_saveUi19148SerializedCardEntry801C81EC.internalName);
    if (!s_saveUi19148SerializedCardEntry801C81EC.nameKnown) {
        return false;
    }

    std::memcpy(s_saveUi19148SerializedCardEntry801C81EC.internalName,
                nameView.bytes,
                nameView.byteCount);
    std::memcpy(s_saveUi19148SerializedCardEntry801C81EC.bytes.data(),
                s_saveUi19148DirectCardBufferSink801C81EC.blockBytes.data(),
                kSaveUiWriteBlockBytes80017A10);
    s_saveUi19148SerializedCardEntry801C81EC.nameAddress =
        nameView.psxAddress;
    s_saveUi19148SerializedCardEntry801C81EC.blockAddress =
        s_saveUi19148DirectCardBufferSink801C81EC.psxAddress;
    s_saveUi19148SerializedCardEntry801C81EC.blockBytes =
        kSaveUiWriteBlockBytes80017A10;
    s_saveUi19148SerializedCardEntry801C81EC.serialized = true;
    const PrStage1SaveUiDirectoryRawBankUpdate8007A318 directoryUpdate =
        PrStage1SaveUiDirect::
            ApplySaveUiDirectoryRawBankSerializedEntry8007A318(
                s_saveUi19148SerializedCardEntry801C81EC.internalName,
                s_saveUi19148SerializedCardEntry801C81EC.blockBytes);
    s_saveUi19148SerializedCardEntry801C81EC.directoryUpdateAttempted =
        directoryUpdate.attempted;
    s_saveUi19148SerializedCardEntry801C81EC.directoryKnown =
        directoryUpdate.directoryKnown;
    s_saveUi19148SerializedCardEntry801C81EC.directorySlotKnown =
        directoryUpdate.slotKnown;
    s_saveUi19148SerializedCardEntry801C81EC.directoryUpdated =
        directoryUpdate.updated;
    s_saveUi19148SerializedCardEntry801C81EC.directoryOverwrote =
        directoryUpdate.overwrote;
    s_saveUi19148SerializedCardEntry801C81EC.directoryBlockIndex =
        directoryUpdate.blockIndex;
    s_saveUi19148SerializedCardEntry801C81EC.durablePolicyKnown = false;
    s_saveUi19148SerializedCardEntry801C81EC.durableCommitted = false;
    Log::Printf(
        "Scene1 801C81EC save-ui 19148 direct card-buffer serialized pending durable policy directory update: name=%s data=%08X bytes=%u dirUpdate=%d/%d/%d block=%d durableKnown=0 durableCommitted=0",
        s_saveUi19148SerializedCardEntry801C81EC.internalName,
        s_saveUi19148SerializedCardEntry801C81EC.blockAddress,
        s_saveUi19148SerializedCardEntry801C81EC.blockBytes,
        s_saveUi19148SerializedCardEntry801C81EC.directoryUpdateAttempted ? 1
                                                                          : 0,
        s_saveUi19148SerializedCardEntry801C81EC.directorySlotKnown ? 1 : 0,
        s_saveUi19148SerializedCardEntry801C81EC.directoryUpdated ? 1 : 0,
        s_saveUi19148SerializedCardEntry801C81EC.directoryBlockIndex);
    return true;
}

bool AttemptSaveUi19148DirectDurableHandoff801C81EC() {
    s_saveUi19148DirectDurableHandoff801C81EC =
        SaveUi19148DirectDurableHandoff801C81EC{};
    s_saveUi19148DirectDurableHandoff801C81EC.attempted = true;
    s_saveUi19148DirectDurableHandoff801C81EC.serializedEntryKnown =
        s_saveUi19148SerializedCardEntry801C81EC.serialized &&
        s_saveUi19148SerializedCardEntry801C81EC.directoryUpdateAttempted &&
        s_saveUi19148SerializedCardEntry801C81EC.directorySlotKnown &&
        s_saveUi19148SerializedCardEntry801C81EC.directoryUpdated;
    if (!s_saveUi19148DirectDurableHandoff801C81EC.serializedEntryKnown) {
        s_saveUi19148DirectDurableHandoff801C81EC.missingOwner =
            "Scene8-final-save-direct-serialized-entry";
        return false;
    }

    const PrStage1SaveUiCardImageSerialization8007A318 cardImage =
        PrStage1SaveUiDirect::
            SerializeSaveUiCardImageCandidateFromDirectBuffers8007A318();
    s_saveUi19148DirectDurableHandoff801C81EC.cardImageCandidateAttempted =
        cardImage.attempted;
    s_saveUi19148DirectDurableHandoff801C81EC.cardImageCandidateKnown =
        cardImage.imageSerialized;
    if (!cardImage.imageSerialized) {
        s_saveUi19148DirectDurableHandoff801C81EC.missingOwner =
            "Scene8-final-save-direct-card-image-candidate";
        return false;
    }

    const PrStage1SaveUiCardImagePersistenceSink8007A318 sink =
        PrStage1SaveUiDirect::CommitSaveUiCardImagePersistenceSink8007A318();
    s_saveUi19148DirectDurableHandoff801C81EC.persistenceSinkAttempted =
        sink.attempted;
    s_saveUi19148DirectDurableHandoff801C81EC.persistenceSinkCommitted =
        sink.sinkCommitted;
    s_saveUi19148DirectDurableHandoff801C81EC.slotPolicyKnown =
        sink.slotPolicyKnown;
    s_saveUi19148DirectDurableHandoff801C81EC.blockIndex = sink.blockIndex;
    if (!sink.sinkCommitted || !sink.slotPolicyKnown) {
        s_saveUi19148DirectDurableHandoff801C81EC.missingOwner =
            "Scene8-final-save-card-image-persistence-sink";
        return false;
    }

    const PrStage1SaveUiCardImageDurableCommitPrimitive8007A318 durable =
        PrStage1SaveUiDirect::
            CommitSaveUiCardImageDirectDurablePrimitive8007A318();
    s_saveUi19148DirectDurableHandoff801C81EC.durablePrimitiveAttempted =
        durable.attempted;
    s_saveUi19148DirectDurableHandoff801C81EC.directBackendKnown =
        durable.directBackendKnown;
    s_saveUi19148DirectDurableHandoff801C81EC.directBackendCalled =
        durable.directBackendCalled;
    s_saveUi19148DirectDurableHandoff801C81EC.directBackendAccepted =
        durable.directBackendAccepted;
    s_saveUi19148DirectDurableHandoff801C81EC
        .explicitNoSavePersistencePolicyKnown =
        durable.explicitNoSavePersistencePolicyKnown;
    s_saveUi19148DirectDurableHandoff801C81EC
        .explicitNoSavePersistencePolicyAccepted =
        durable.explicitNoSavePersistencePolicyAccepted;
    s_saveUi19148DirectDurableHandoff801C81EC
        .explicitNoSavePersistencePolicyFinalized =
        durable.explicitNoSavePersistencePolicyFinalized &&
        !durable.durableCommitted;
    s_saveUi19148DirectDurableHandoff801C81EC.explicitNoSaveP0Accepted =
        s_saveUi19148DirectDurableHandoff801C81EC
            .explicitNoSavePersistencePolicyFinalized &&
        !s_saveUi19148DirectDurableHandoff801C81EC.directBackendKnown &&
        !s_saveUi19148DirectDurableHandoff801C81EC.durableCommitted;
    s_saveUi19148DirectDurableHandoff801C81EC.durablePolicyKnown =
        durable.durablePolicyKnown;
    s_saveUi19148DirectDurableHandoff801C81EC.durableCommitted =
        durable.durableCommitted;
    s_saveUi19148DirectDurableHandoff801C81EC.missingOwner =
        durable.missingOwner;
    if (durable.durableCommitted) {
        const auto sinkView =
            PrStage1SaveUiDirect::
                GetSaveUiCardImagePersistenceSinkView8007A318();
        s_saveUi19148DirectDurableHandoff801C81EC
            .state16ReadProducerPublished =
            PrStage1SaveCardHalDirect::
                PublishRuntimeState16CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(
                    sinkView, sink.blockIndex);
        s_saveUi19148DirectDurableHandoff801C81EC
            .case17ReadProducerPublished =
            PrStage1SaveCardHalDirect::
                PublishRuntimeCase17CardReadTypedCarrier800179B4FromDirectCardImagePersistenceSink(
                    sinkView, sink.blockIndex);
    }
    Log::Printf(
        "Scene1 801C81EC save-ui 19148 direct durable handoff attempted: serialized=%d candidate=%d sink=%d slot=%d block=%d state16Read=%d case17Read=%d backend=%d/%d/%d noSave=%d/%d/%d durable=%d/%d missing=%s p0NoSave=%d",
        s_saveUi19148DirectDurableHandoff801C81EC.serializedEntryKnown ? 1
                                                                       : 0,
        s_saveUi19148DirectDurableHandoff801C81EC.cardImageCandidateKnown
            ? 1
            : 0,
        s_saveUi19148DirectDurableHandoff801C81EC.persistenceSinkCommitted
            ? 1
            : 0,
        s_saveUi19148DirectDurableHandoff801C81EC.slotPolicyKnown ? 1 : 0,
        s_saveUi19148DirectDurableHandoff801C81EC.blockIndex,
        s_saveUi19148DirectDurableHandoff801C81EC
                .state16ReadProducerPublished
            ? 1
            : 0,
        s_saveUi19148DirectDurableHandoff801C81EC
                .case17ReadProducerPublished
            ? 1
            : 0,
        s_saveUi19148DirectDurableHandoff801C81EC.directBackendKnown ? 1 : 0,
        s_saveUi19148DirectDurableHandoff801C81EC.directBackendCalled ? 1 : 0,
        s_saveUi19148DirectDurableHandoff801C81EC.directBackendAccepted ? 1
                                                                        : 0,
        s_saveUi19148DirectDurableHandoff801C81EC
                .explicitNoSavePersistencePolicyKnown
            ? 1
            : 0,
        s_saveUi19148DirectDurableHandoff801C81EC
                .explicitNoSavePersistencePolicyAccepted
            ? 1
            : 0,
        s_saveUi19148DirectDurableHandoff801C81EC
                .explicitNoSavePersistencePolicyFinalized
            ? 1
            : 0,
        s_saveUi19148DirectDurableHandoff801C81EC.durablePolicyKnown ? 1 : 0,
        s_saveUi19148DirectDurableHandoff801C81EC.durableCommitted ? 1 : 0,
        s_saveUi19148DirectDurableHandoff801C81EC.missingOwner
            ? s_saveUi19148DirectDurableHandoff801C81EC.missingOwner
            : "none",
        s_saveUi19148DirectDurableHandoff801C81EC.explicitNoSaveP0Accepted
            ? 1
            : 0);
    return durable.durableCommitted;
}

SaveUi19148WriteCommitGate801C81EC
TryCommitSaveUi19148WriteAfterTypedFeedbackRequests801C81EC(
    const PrStage1SaveUi19148LowerFeedbackRequestList& requests,
    const PrStage1SaveUi19148LowerFeedback& lowerFeedback) {
    if (!lowerFeedback.writeFeedbackKnown80017A10) {
        return SaveUi19148WriteCommitGate801C81EC::NoTypedSuccess;
    }

    for (uint32_t i = 0; i < requests.count; ++i) {
        const PrStage1SaveUi19148LowerFeedbackRequest& request =
            requests.requests[i];
        if (request.kind !=
            PrStage1SaveUi19148LowerFeedbackRequestKind::Write80017A10) {
            continue;
        }
        if (request.psxFunction != kFn80017A10 ||
            request.retryCount != kSaveUiWriteRetryCount80017A10 ||
            !request.writeCloseGp696FactRequired80017A10 ||
            request.writeCloseGp696Address80017A10 != 0x8006ECF8u ||
            !request.writeFdMustMatchCloseGp69680017A10 ||
            request.action.kind !=
                PrStage1SaveUi19148ActionKind::Call80017A10WriteSaveBlock) {
            return SaveUi19148WriteCommitGate801C81EC::Blocked;
        }

        const PrStage1SaveUiWriteFeedbackCarrier80017A10 carrier =
            PrStage1SaveUiDirect::BuildWriteFeedback80017A10(
                request.nameAddress,
                request.dataAddress,
                request.blockCount,
                &lowerFeedback.writeFeedback80017A10);
        if (!carrier.resultKnown || carrier.result != 0 ||
            carrier.helperGap) {
            return SaveUi19148WriteCommitGate801C81EC::NoTypedSuccess;
        }

        const SaveUi19148DirectWriteRuntime801C81EC& runtime =
            s_saveUi19148DirectWriteRuntime801C81EC;
        if (!runtime.completed ||
            !SameSaveUi19148WriteRequest801C81EC(runtime.request, request) ||
            !runtime.durableAttempted || !runtime.durableCommitted ||
            !s_saveUi19148DirectDurableHandoff801C81EC.attempted ||
            !s_saveUi19148DirectDurableHandoff801C81EC.durableCommitted) {
            return SaveUi19148WriteCommitGate801C81EC::Blocked;
        }
        Log::Printf(
            "Scene1 801C81EC save-ui 19148 write typed feedback accepted after direct durable event completion: name=%08X data=%08X blocks=%d attempts=%d",
            request.nameAddress,
            request.dataAddress,
            request.blockCount,
            runtime.attemptIndex + 1);
        return SaveUi19148WriteCommitGate801C81EC::Accepted;
    }
    return SaveUi19148WriteCommitGate801C81EC::NoWriteRequest;
}

SaveUi19148WriteCommitGate801C81EC
TryCommitSaveUi19148WriteAfterTypedFeedback801C81EC(
    const PrStage1LifecycleExecutorDirect::State801C81EC& state,
    const PrStage1SaveUi19148LowerFeedback& lowerFeedback) {
    return TryCommitSaveUi19148WriteAfterTypedFeedbackRequests801C81EC(
        state.saveUi19148LowerFeedbackRequests,
        lowerFeedback);
}

enum class SaveUi19148FormatCommitGate801C81EC : uint8_t {
    NoFormatRequest,
    NoTypedSuccess,
    Blocked,
    Accepted,
};

SaveUi19148FormatCommitGate801C81EC
TryCommitSaveUi19148FormatAfterTypedFeedbackRequests801C81EC(
    const PrStage1SaveUi19148LowerFeedbackRequestList& requests,
    const PrStage1SaveUi19148LowerFeedback& lowerFeedback) {
    if (!lowerFeedback.formatFeedbackKnown80017B60) {
        return SaveUi19148FormatCommitGate801C81EC::NoTypedSuccess;
    }

    for (uint32_t i = 0; i < requests.count; ++i) {
        const PrStage1SaveUi19148LowerFeedbackRequest& request =
            requests.requests[i];
        if (request.kind !=
            PrStage1SaveUi19148LowerFeedbackRequestKind::Format80017B60) {
            continue;
        }
        if (request.psxFunction != kFn80017B60 ||
            request.retryCount != kSaveUiFormatRetryCount80017B60 ||
            request.formatArg0 != kSaveUiFormatArg0_80017B60 ||
            request.formatArg1 != kSaveUiFormatArg1_80017B60 ||
            request.action.kind !=
                PrStage1SaveUi19148ActionKind::Call80017B60FormatCard) {
            return SaveUi19148FormatCommitGate801C81EC::Blocked;
        }

        const PrStage1SaveUiFormatFeedbackCarrier80017B60 carrier =
            PrStage1SaveUiDirect::BuildFormatFeedback80017B60(
                &lowerFeedback.formatFeedback80017B60);
        if (!carrier.callCompleted || carrier.helperGap) {
            return SaveUi19148FormatCommitGate801C81EC::NoTypedSuccess;
        }

        const SaveUi19148DirectFormatRuntime801C81EC& runtime =
            s_saveUi19148DirectFormatRuntime801C81EC;
        if (!runtime.completed || runtime.blocked ||
            !SameSaveUi19148FormatRequest801C81EC(runtime.request, request) ||
            !runtime.durableAttempted || !runtime.backendCompletionKnown ||
            runtime.attemptsUsed <= 0 ||
            runtime.attemptsUsed > kSaveUiFormatRetryCount80017B60) {
            return SaveUi19148FormatCommitGate801C81EC::Blocked;
        }
        if (runtime.durableCommitted) {
            if (!carrier.resultKnown || carrier.result != 1 ||
                carrier.retryExhaustedReturnUnknown) {
                return SaveUi19148FormatCommitGate801C81EC::Blocked;
            }
            Log::Printf(
                "Scene1 801C81EC save-ui 19148 format typed feedback accepted after direct durable completion: result=%d arg0=%08X arg1=%08X attempts=%d",
                carrier.result,
                request.formatArg0,
                request.formatArg1,
                runtime.attemptsUsed);
            return SaveUi19148FormatCommitGate801C81EC::Accepted;
        }
        if (!runtime.rollbackCompleted || carrier.resultKnown ||
            !carrier.retryExhaustedReturnUnknown) {
            return SaveUi19148FormatCommitGate801C81EC::Blocked;
        }
        Log::Printf(
            "Scene1 801C81EC save-ui 19148 format failure completed after direct backend rejection and rollback: arg0=%08X arg1=%08X attempts=%d returnKnown=0",
            request.formatArg0,
            request.formatArg1,
            runtime.attemptsUsed);
        return SaveUi19148FormatCommitGate801C81EC::Accepted;
    }
    return SaveUi19148FormatCommitGate801C81EC::NoFormatRequest;
}

SaveUi19148FormatCommitGate801C81EC
TryCommitSaveUi19148FormatAfterTypedFeedback801C81EC(
    const PrStage1LifecycleExecutorDirect::State801C81EC& state,
    const PrStage1SaveUi19148LowerFeedback& lowerFeedback) {
    return TryCommitSaveUi19148FormatAfterTypedFeedbackRequests801C81EC(
        state.saveUi19148LowerFeedbackRequests,
        lowerFeedback);
}

bool TryResolveIso9660BinPath(
    const PrGameContext& ctx,
    std::filesystem::path& outPath) {
    outPath.clear();
    std::error_code ec;
    std::filesystem::path p = ctx.dataRoot;
    for (int i = 0; i < 8 && !p.empty(); ++i) {
        const std::filesystem::path candidate =
            p / "PaRappa the Rapper.bin";
        if (std::filesystem::exists(candidate, ec)) {
            outPath = candidate;
            return true;
        }
        const std::filesystem::path parent = p.parent_path();
        if (parent == p) {
            break;
        }
        p = parent;
    }

    p = std::filesystem::current_path(ec);
    for (int i = 0; i < 8 && !p.empty(); ++i) {
        const std::filesystem::path candidate =
            p / "PaRappa the Rapper.bin";
        if (std::filesystem::exists(candidate, ec)) {
            outPath = candidate;
            return true;
        }
        const std::filesystem::path parent = p.parent_path();
        if (parent == p) {
            break;
        }
        p = parent;
    }
    return false;
}

struct Bootstrap15590DiscUserDataProbe801C81EC {
    bool attempted = false;
    bool success = false;
    bool binPathKnown = false;
    bool fileBaseLbaKnown = false;
    int32_t fileBaseLba = 0;
    bool lastSeekLbaKnown = false;
    int32_t lastSeekLba = 0;
    bool byteCountKnown = false;
    uint32_t byteCount = 0;
    std::size_t bytesRead = 0;
    std::vector<uint8_t> bytes{};
};

Bootstrap15590DiscUserDataProbe801C81EC
ProbeBootstrap15590ReadStartDiscUserData801C81EC(
    const PrGameContext& ctx,
    const PrStage1LifecycleExecutorDirect::State801C81EC& state,
    const PrStage1LifecycleExecutorDirect::
        Bootstrap15590CdLowerHostRequest801C81EC& request) {
    Bootstrap15590DiscUserDataProbe801C81EC out{};
    out.fileBaseLbaKnown = state.bootstrap15590CdLowerFileBaseLbaKnown;
    out.fileBaseLba = state.bootstrap15590CdLowerFileBaseLba;
    out.lastSeekLbaKnown = state.bootstrap15590CdLowerLastSeekLbaKnown;
    out.lastSeekLba = state.bootstrap15590CdLowerLastSeekLba;

    if (request.lowerRequest.readStartSectorCountKnown &&
        request.lowerRequest.readStartSectorCount > 0 &&
        request.lowerRequest.readStartSectorCount <=
            static_cast<int32_t>(0xFFFFFFFFu / 2048u)) {
        out.byteCountKnown = true;
        out.byteCount =
            static_cast<uint32_t>(request.lowerRequest.readStartSectorCount) *
            2048u;
    }
    if (!out.lastSeekLbaKnown || !out.byteCountKnown) {
        return out;
    }

    std::filesystem::path binPath;
    if (!TryResolveIso9660BinPath(ctx, binPath)) {
        return out;
    }
    out.binPathKnown = true;

    PrStage1LoaderCdHal::Iso9660UserDataReadInput8001A818 isoInput{};
    isoInput.valid = true;
    isoInput.binPath = binPath;
    isoInput.lba = out.lastSeekLba;
    isoInput.byteCount = out.byteCount;
    const PrStage1LoaderCdHal::Iso9660UserDataReadResult8001A818 isoResult =
        PrStage1LoaderCdHal::ReadIso9660UserDataBytes8001A818(isoInput);
    out.attempted = isoResult.attempted;
    out.success = isoResult.success;
    out.bytesRead = isoResult.bytes.size();
    if (isoResult.success) {
        out.bytes = isoResult.bytes;
    }
    return out;
}

bool TryPumpBootstrap15590CdLookupIsoHal(
    PrGameContext& ctx,
    PrStage1LifecycleExecutorDirect::State801C81EC& state,
    const PrStage1LifecycleExecutorDirect::
        Bootstrap15590LoaderPumpResult801C81EC& pump) {
    (void)pump;
    if (!pump.waitingForFeedback) {
        return false;
    }

    PrStage1LifecycleExecutorDirect::Bootstrap15590CdLookupHostRequest801C81EC
        request{};
    if (!PrStage1LifecycleExecutorDirect::
            BuildBootstrap15590CdLookupLowerProducerRequest801C81EC(
                state,
                request) ||
        (request.binNeeded && !request.psxPathKnown)) {
        return false;
    }

    PrStage1LoaderCdHal::Iso9660LookupResult800381F8 isoResult{};
    PrStage1LoaderCdHal::LookupFeedback800381F8 lookupFeedback{};
    if (request.binNeeded) {
        std::filesystem::path binPath;
        if (!TryResolveIso9660BinPath(ctx, binPath)) {
            Log::Printf(
                "Scene1 801C81EC bootstrap15590 CD lookup blocked: no authority BIN for path=%08X psxPath='%s'",
                request.pathPtr,
                request.psxPath != nullptr ? request.psxPath : "");
            return false;
        }

        PrStage1LoaderCdHal::Iso9660LookupInput800381F8 isoInput{};
        isoInput.valid = true;
        isoInput.binPath = binPath;
        isoInput.psxPath = request.psxPath;
        isoInput.cdlFilePtr = request.cdlFilePtr;
        isoInput.pathPtr = request.pathPtr;
        isoInput.retryIndex = request.retryIndex;
        isoResult =
            PrStage1LoaderCdHal::BuildIso9660LookupFeedback800381F8(
                isoInput);
        lookupFeedback = isoResult.feedback;
    }

    const bool applied =
        PrStage1LifecycleExecutorDirect::
            RunBootstrap15590CdLookupLowerProducerFeedback801C81EC(
                state,
                lookupFeedback,
                &ctx.stage1XaCdDirect);
    if (applied) {
        if (isoResult.success &&
            isoResult.matchedExtentLba <=
                static_cast<uint32_t>(0x7FFFFFFFu)) {
            state.bootstrap15590CdLowerFileBaseLbaKnown = true;
            state.bootstrap15590CdLowerFileBaseLba =
                static_cast<int32_t>(isoResult.matchedExtentLba);
        }
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lookup ISO HAL: path=%08X psxPath='%s' success=%d lba=%u size=%u fileBase=%d/%d",
            request.pathPtr,
            request.psxPath,
            isoResult.success ? 1 : 0,
            isoResult.matchedExtentLba,
            isoResult.matchedSize,
            state.bootstrap15590CdLowerFileBaseLbaKnown ? 1 : 0,
            state.bootstrap15590CdLowerFileBaseLba);
    }
    return applied;
}

bool TryPumpBootstrap15590CdLowerExternalPlan(
    PrGameContext& ctx,
    PrStage1LifecycleExecutorDirect::State801C81EC& state,
    const PrStage1LifecycleExecutorDirect::
        Bootstrap15590LoaderPumpResult801C81EC& pump) {
    if (!pump.waitingForFeedback) {
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower live final-ready facts: skipped waitingForFeedback=0");
        return false;
    }

    PrStage1LifecycleExecutorDirect::Bootstrap15590CdLowerHostRequest801C81EC
        request{};
    if (!PrStage1LifecycleExecutorDirect::
            BuildBootstrap15590CdLowerProducerRequest801C81EC(
                state,
                request)) {
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower live final-ready facts: skipped requestBuild=0");
        return false;
    }
    if (!request.valid ||
        request.cdActionKind ==
            PrStage1LoaderCdHal::ActionKind::Lookup800381F8) {
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower live final-ready facts: skipped requestValid=%d cd=%u",
            request.valid ? 1 : 0,
            static_cast<unsigned>(request.cdActionKind));
        return false;
    }
    if (request.status !=
            PrStage1LifecycleExecutorDirect::
                Bootstrap15590CdLowerAttemptStatus801C81EC::RequestReady &&
        request.status !=
            PrStage1LifecycleExecutorDirect::
                Bootstrap15590CdLowerAttemptStatus801C81EC::
                    RequestAlreadyPending) {
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower live final-ready facts: skipped status=%u cd=%u",
            static_cast<unsigned>(request.status),
            static_cast<unsigned>(request.cdActionKind));
        return false;
    }

    if (request.cdActionKind ==
        PrStage1LoaderCdHal::ActionKind::SeekSync800367A4) {
        PrStage1LoaderCdHal::SeekSyncHalInput800367A4 seekInput{};
        seekInput.requestKnown = request.lowerRequest.known;
        seekInput.request = request.lowerRequest;
        const PrStage1LoaderCdHal::SeekSyncHalFacts800367A4 seekFacts =
            PrStage1LoaderCdHal::BuildSeekSyncHalFacts800367A4(seekInput);

        PrStage1LowerCdProducerDirect::LowerCdProducerFacts facts{};
        facts.request = request.lowerRequest;
        facts.overlayTransferAttempt.known = true;
        facts.overlayTransferAttempt.sourceFunction =
            PrMovieSegmentDirect::kSub800154B0OverlayTransferWrapper;
        facts.overlayTransferAttempt.transferFunction =
            PrMovieSegmentDirect::kSub8001ACF8OverlayTransfer;
        facts.overlayTransferAttempt.attemptIndexKnown = true;
        facts.overlayTransferAttempt.attemptIndex = request.attemptIndex;
        facts.seekSyncHalFacts800367A4Known = seekFacts.known;
        facts.seekSyncHalFacts800367A4 = seekFacts;

        PrStage1LifecycleExecutorDirect::
            Bootstrap15590CdLowerFactsApplyResult801C81EC apply{};
        const bool applied =
            PrStage1LifecycleExecutorDirect::
                RunBootstrap15590CdLowerFacts801C81EC(
                    state,
                    ctx.stage1XaCdDirect,
                    facts,
                    &apply);
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower seek-sync HAL facts: applied=%d requestMatched=%d xaCdAccepted=%d feedbackReady=%d status=%u reject=%u pending=%d/%d commandAccepted=%d/%d seekCompleted=%d/%d seekFailed=%d/%d timeout=%d/%d",
            applied ? 1 : 0,
            apply.requestMatched ? 1 : 0,
            apply.xaCdAccepted ? 1 : 0,
            apply.feedbackReady ? 1 : 0,
            static_cast<unsigned>(apply.status),
            static_cast<unsigned>(apply.rejectReason),
            apply.requestPendingBefore ? 1 : 0,
            apply.requestPendingAfter ? 1 : 0,
            seekFacts.commandAcceptedKnown ? 1 : 0,
            seekFacts.commandAccepted ? 1 : 0,
            seekFacts.seekCompletedKnown ? 1 : 0,
            seekFacts.seekCompleted ? 1 : 0,
            seekFacts.seekFailedKnown ? 1 : 0,
            seekFacts.seekFailed ? 1 : 0,
            seekFacts.timeoutKnown ? 1 : 0,
            seekFacts.timeout ? 1 : 0);
        if (applied) {
            if (request.seekLbaKnown) {
                state.bootstrap15590CdLowerLastSeekLbaKnown = true;
                state.bootstrap15590CdLowerLastSeekLba = request.seekLba;
            }
            return true;
        }
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower seek-sync facts gap: status=%u order=%u step=%u record=%u attempt=%u lower=%08X sync=%08X a0Known=%d a0=%d needSeekHalFacts=1",
            static_cast<unsigned>(request.status),
            request.psxOrder,
            static_cast<unsigned>(request.stepKind),
            static_cast<unsigned>(request.recordIndex),
            static_cast<unsigned>(request.attemptIndex),
            request.lowerRequest.lowerFunction,
            request.cdSyncLoopFunction80037070,
            request.cdSyncLoopA0WaitModeKnown80037070 ? 1 : 0,
            request.cdSyncLoopA0WaitMode80037070);
        return false;
    }

    if (request.cdActionKind ==
            PrStage1LoaderCdHal::ActionKind::ReadStart80038FC0 &&
        !request.readStartHalProgressAccepted) {
        PrStage1LoaderCdHal::ReadStartHalInput80038FC0 readStartInput{};
        readStartInput.requestKnown = request.lowerRequest.known;
        readStartInput.request = request.lowerRequest;
        if (request.readDstPtrKnown) {
            readStartInput.dstPtrKnown = true;
            readStartInput.dstPtr = request.readDstPtr;
        }
        if (request.readSectorCountKnown) {
            readStartInput.sectorCountKnown = true;
            readStartInput.sectorCount = request.readSectorCount;
        }
        const bool setupPathShapeKnown =
            request.lowerRequest.readStartDstPtrKnown &&
            request.lowerRequest.readStartSectorCountKnown &&
            request.lowerRequest.readStartModeFlagKnown;
        const Bootstrap15590DiscUserDataProbe801C81EC discProbe =
            ProbeBootstrap15590ReadStartDiscUserData801C81EC(ctx,
                                                            state,
                                                            request);
        const bool discReadStartHalResultKnown =
            setupPathShapeKnown &&
            discProbe.success &&
            discProbe.byteCountKnown &&
            discProbe.bytesRead == discProbe.byteCount;
        if (discReadStartHalResultKnown) {
            readStartInput.commandAcceptedKnown = true;
            readStartInput.commandAccepted = true;
            readStartInput.readStartedKnown = true;
            readStartInput.readStarted = true;
            readStartInput.readFailedKnown = true;
            readStartInput.readFailed = false;
            readStartInput.timeoutKnown = true;
            readStartInput.timeout = false;
        }
        const PrStage1LoaderCdHal::ReadStartHalFacts80038FC0 readStartFacts =
            PrStage1LoaderCdHal::BuildReadStartHalFacts80038FC0(
                readStartInput);

        PrStage1LowerCdProducerDirect::LowerCdProducerFacts facts{};
        facts.request = request.lowerRequest;
        facts.overlayTransferAttempt.known = true;
        facts.overlayTransferAttempt.sourceFunction =
            PrMovieSegmentDirect::kSub800154B0OverlayTransferWrapper;
        facts.overlayTransferAttempt.transferFunction =
            PrMovieSegmentDirect::kSub8001ACF8OverlayTransfer;
        facts.overlayTransferAttempt.attemptIndexKnown = true;
        facts.overlayTransferAttempt.attemptIndex = request.attemptIndex;
        if (request.readDstPtrKnown) {
            facts.overlayTransferAttempt.dstKnown = true;
            facts.overlayTransferAttempt.dst = request.readDstPtr;
        }
        if (request.readSectorCountKnown) {
            facts.overlayTransferAttempt.sectorCountKnown = true;
            facts.overlayTransferAttempt.sectorCount =
                static_cast<uint32_t>(request.readSectorCount);
        }
        facts.readStartHalFacts80038FC0Known = readStartFacts.known;
        facts.readStartHalFacts80038FC0 = readStartFacts;
        const PrStage1XaCdDirectStreamClockProbe800493F4 clockProbe =
            PrStage1XaCdDirectProbeStreamClockProducer800493F4(
                ctx.stage1XaCdDirect);
        const bool clockCarrierAccepted =
            clockProbe.carrier.clockKnown &&
            clockProbe.carrier.acceptedByte800493F4 &&
            !clockProbe.gapMissingStreamClock800493F4Producer;
        PrStage1LowerCdProducerDirect::ReadStartSetupInput80038FC0
            setupProbe{};
        setupProbe.sectorCountKnown =
            request.lowerRequest.readStartSectorCountKnown;
        setupProbe.sectorCount = request.lowerRequest.readStartSectorCount;
        setupProbe.dstKnown = request.lowerRequest.readStartDstPtrKnown;
        setupProbe.dst =
            static_cast<int32_t>(request.lowerRequest.readStartDstPtr);
        setupProbe.modeKnown = request.lowerRequest.readStartModeFlagKnown;
        setupProbe.mode =
            static_cast<uint32_t>(request.lowerRequest.readStartModeFlag);
        if (ctx.stage1XaCdDirect.statusRead80036384Known) {
            PrStage1LowerCdProducerDirect::StatusReadInput80036384
                statusRead{};
            statusRead.statusKnown = true;
            statusRead.status = ctx.stage1XaCdDirect.statusRead80036384;
            setupProbe.status =
                PrStage1LowerCdProducerDirect::
                    BuildStatusReadResult80036384(statusRead);
        }
        if (ctx.stage1XaCdDirect.dword_800570F8Known) {
            PrStage1LowerCdProducerDirect::CallbackSwapInput80036510
                savedSync{};
            savedSync.oldCallbackKnown = true;
            savedSync.oldCallback = ctx.stage1XaCdDirect.dword_800570F8;
            savedSync.newCallback = 0u;
            setupProbe.savedSyncCallback =
                PrStage1LowerCdProducerDirect::
                    BuildCallbackSwapResult80036510(savedSync);
        }
        if (ctx.stage1XaCdDirect.dword_800570FCKnown) {
            PrStage1LowerCdProducerDirect::CallbackSwapInput80036528
                savedReady{};
            savedReady.oldCallbackKnown = true;
            savedReady.oldCallback = ctx.stage1XaCdDirect.dword_800570FC;
            savedReady.newCallback = 0u;
            setupProbe.savedReadyCallback =
                PrStage1LowerCdProducerDirect::
                    BuildCallbackSwapResult80036528(savedReady);
        }
        if (clockCarrierAccepted) {
            setupProbe.clockKnown = true;
            setupProbe.clockNow = clockProbe.carrier.clockLba;
            facts.clockKnown = true;
            facts.clockNow = clockProbe.carrier.clockLba;
        }
        const PrStage1LowerCdProducerDirect::ReadStartSetupResult80038FC0
            setupProbeResult =
                PrStage1LowerCdProducerDirect::
                    BuildReadStartSetupResult80038FC0(setupProbe);
        PrStage1LowerCdProducerDirect::ReadPumpInput80038DE8 pumpProbe{};
        pumpProbe.status = setupProbe.status;
        if (ctx.stage1XaCdDirect.dword_800570F8Known) {
            PrStage1LowerCdProducerDirect::CallbackSwapInput80036510
                clearSync{};
            clearSync.oldCallbackKnown = true;
            clearSync.oldCallback = ctx.stage1XaCdDirect.dword_800570F8;
            clearSync.newCallback = 0u;
            pumpProbe.clearSyncCallback =
                PrStage1LowerCdProducerDirect::
                    BuildCallbackSwapResult80036510(clearSync);
        }
        if (ctx.stage1XaCdDirect.dword_800570FCKnown) {
            PrStage1LowerCdProducerDirect::CallbackSwapInput80036528
                clearReady{};
            clearReady.oldCallbackKnown = true;
            clearReady.oldCallback = ctx.stage1XaCdDirect.dword_800570FC;
            clearReady.newCallback = 0u;
            pumpProbe.clearReadyCallback =
                PrStage1LowerCdProducerDirect::
                    BuildCallbackSwapResult80036528(clearReady);
        }
        if (clockCarrierAccepted) {
            pumpProbe.clockKnown = true;
            pumpProbe.clockNow = clockProbe.carrier.clockLba;
        }
        pumpProbe.modeKnown = setupProbeResult.produced;
        pumpProbe.modeWord8005741C = setupProbeResult.modeWord8005741C;
        pumpProbe.activeDstKnown = setupProbeResult.produced;
        pumpProbe.activeDst80057414 = setupProbeResult.dst80057414;
        pumpProbe.activeSectorCountKnown = setupProbeResult.produced;
        pumpProbe.activeSectorCount80057410 =
            setupProbeResult.sectorCount80057410;
        const bool preReadSetupAccepted =
            setupProbeResult.produced &&
            !setupProbeResult.incomplete &&
            pumpProbe.status.produced &&
            !pumpProbe.status.incomplete &&
            (pumpProbe.status.status & 0x10u) == 0u;
        if (preReadSetupAccepted) {
            pumpProbe.preReadSetupKnown = true;
        }
        const bool modeCmdNeeded =
            pumpProbe.modeKnown &&
            ((static_cast<uint8_t>(pumpProbe.modeWord8005741C) != 0u) ||
             pumpProbe.retry);
        const bool modeCmdArgsKnown = pumpProbe.modeKnown;
        const uint8_t modeCmdArg0 =
            static_cast<uint8_t>(pumpProbe.modeWord8005741C);
        const PrStage1XaCdDirectStatusFlagsRuntimeSourceWindow80057108
            modeCmdStatus57108RuntimeSourceWindow = [&]() {
                ScopedRuntimePsxMemoryProviderPc801C81EC scopedPc(
                    ctx.stage1RuntimePsxMemoryProvider,
                    kFn80036540);
                return PrStage1XaCdDirectReadStatusFlagsRuntimeSourceWindow80057108(
                    ctx.stage1RuntimePsxMemoryProvider);
            }();
        const PrStage1XaCdDirectStatusFlagsRuntimeSource80057108
            modeCmdStatus57108RuntimeSource =
                PrStage1XaCdDirectBuildStatusFlagsRuntimeSource80057108(
                    modeCmdStatus57108RuntimeSourceWindow);
        const PrStage1XaCdDirectStatusFlagsRuntimeSourceResult80057108
            modeCmdStatus57108RuntimeSourceResult =
                PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(
                    modeCmdStatus57108RuntimeSource,
                    ctx.stage1XaCdDirect);
        const PrStage1XaCdDirectStatusCounterRuntimeSourceWindow80057110
            modeCmdStatus57110RuntimeSourceWindow = [&]() {
                ScopedRuntimePsxMemoryProviderPc801C81EC scopedPc(
                    ctx.stage1RuntimePsxMemoryProvider,
                    kFn80036AF8);
                return PrStage1XaCdDirectReadStatusCounterRuntimeSourceWindow80057110(
                    ctx.stage1RuntimePsxMemoryProvider);
            }();
        const PrStage1XaCdDirectStatusCounterRuntimeSource80057110
            modeCmdStatus57110RuntimeSource =
                PrStage1XaCdDirectBuildStatusCounterRuntimeSource80057110(
                    modeCmdStatus57110RuntimeSourceWindow);
        const PrStage1XaCdDirectStatusCounterRuntimeSourceResult80057110
            modeCmdStatus57110RuntimeSourceResult =
                PrStage1XaCdDirectPublishStatusCounterRuntimeSource80057110(
                    modeCmdStatus57110RuntimeSource,
                    ctx.stage1XaCdDirect);
        const PrMainStage1RuntimePsxMemorySourceReadAudit
            modeCmdStatus57110SourceReadAudit =
                PrMain::AuditStage1RuntimePsxMemorySources(
                    ctx,
                    kCdStatusCounter80057110,
                    4u);
        const bool modeCmdStatus57108Known =
            ctx.stage1XaCdDirect.dword_80057108Known;
        const uint8_t modeCmdStatus57108 =
            static_cast<uint8_t>(ctx.stage1XaCdDirect.dword_80057108 & 0xFFu);
        const PrStage1LowerCdProducerDirect::CommandAttr800375BC
            modeCmdAttr =
                PrStage1LowerCdProducerDirect::ResolveCommandAttr800375BC(
                    0x0Eu);
        const bool modeCmdNeedsSetlocKnown =
            modeCmdAttr.needsSetlocKnown;
        const bool modeCmdNeedsSetloc = modeCmdAttr.needsSetloc;
        const bool modeCmdCommand800375BCOwnerGate =
            modeCmdNeeded &&
            modeCmdArgsKnown &&
            modeCmdStatus57108Known &&
            modeCmdNeedsSetlocKnown &&
            ((modeCmdStatus57108 & 0x10u) == 0u) &&
            !modeCmdNeedsSetloc;
        if (modeCmdCommand800375BCOwnerGate && clockCarrierAccepted) {
            PrStage1XaCdDirectPrimeSub800375BCTimeoutState(
                ctx.stage1XaCdDirect,
                clockProbe.carrier.clockLba);
        }
        const PrMainStage1RuntimePsxMemorySourceReadAudit
            modeCmdTimeoutState80088310Audit =
                PrMain::AuditStage1RuntimePsxMemorySources(
                    ctx,
                    kCdCommandTimeoutState80088310,
                    4u);
        const PrMainStage1RuntimePsxMemorySourceReadAudit
            modeCmdTimeoutSpin80088314Audit =
                PrMain::AuditStage1RuntimePsxMemorySources(
                    ctx,
                    kCdCommandTimeoutSpin80088314,
                    4u);
        const bool modeCmdTimeoutProviderInstalled =
            ctx.stage1RuntimePsxMemoryProvider.installed &&
            ctx.stage1RuntimePsxMemoryProvider.read != nullptr;
        const bool modeCmdTimeoutExactCdSourceInstalled =
            ctx.stage1RuntimePsxMemoryProvider.exactCdSourceInstalled;
        const bool modeCmdTimeoutExactCdReadFnInstalled =
            ctx.stage1RuntimePsxMemoryProvider.exactCdRead != nullptr;
        const bool modeCmdTimeoutExactCdUserDataInstalled =
            ctx.stage1RuntimePsxMemoryProvider.exactCdUserData != nullptr;
        const bool modeCmdTimeoutStateReadAttempted =
            modeCmdTimeoutState80088310Audit.loaderHeapReadAttempted ||
            modeCmdTimeoutState80088310Audit.knownStateRelayReadAttempted ||
            modeCmdTimeoutState80088310Audit.cdMmioSnapshotReadAttempted;
        const bool modeCmdTimeoutStateReadable =
            modeCmdTimeoutState80088310Audit.loaderHeapReadable ||
            modeCmdTimeoutState80088310Audit.knownStateRelayReadable ||
            modeCmdTimeoutState80088310Audit.cdMmioSnapshotReadable;
        const bool modeCmdTimeoutSpinReadAttempted =
            modeCmdTimeoutSpin80088314Audit.loaderHeapReadAttempted ||
            modeCmdTimeoutSpin80088314Audit.knownStateRelayReadAttempted ||
            modeCmdTimeoutSpin80088314Audit.cdMmioSnapshotReadAttempted;
        const bool modeCmdTimeoutSpinReadable =
            modeCmdTimeoutSpin80088314Audit.loaderHeapReadable ||
            modeCmdTimeoutSpin80088314Audit.knownStateRelayReadable ||
            modeCmdTimeoutSpin80088314Audit.cdMmioSnapshotReadable;
        const bool modeCmdTimeoutSourceAvailable =
            modeCmdTimeoutStateReadable && modeCmdTimeoutSpinReadable;
        const char* modeCmdTimeoutReadPathFirstMissing =
            !modeCmdTimeoutStateReadable
                ? "timeoutState80088310"
                : (!modeCmdTimeoutSpinReadable ? "timeoutSpin80088314"
                                                : "none");
        const PrStage1XaCdDirectInterruptSnapshotRuntimeSourceWindow800359B8
            modeCmd359B8RuntimeSourceWindow = [&]() {
                ScopedRuntimePsxMemoryProviderPc801C81EC scopedPc(
                    ctx.stage1RuntimePsxMemoryProvider,
                    kFn800359B8);
                return PrStage1XaCdDirectReadInterruptSnapshotRuntimeSourceWindow800359B8(
                    ctx.stage1RuntimePsxMemoryProvider);
            }();
        const PrMainStage1RuntimePsxMemorySourceReadAudit
            modeCmd359B8CbStateSourceAudit =
                PrMain::AuditStage1RuntimePsxMemorySources(
                    ctx,
                    kCdCallbackState80055F78,
                    kCdCallbackStateBytes80055F78);
        const PrMainStage1RuntimePsxMemorySourceReadAudit
            modeCmd359B8InitialRegsSourceAudit =
                PrMain::AuditStage1RuntimePsxMemorySources(
                    ctx,
                    kCdInterruptRegs1F801070,
                    kCdInterruptRegsBytes1F801070);
        const PrMainStage1RuntimePsxMemorySourceReadAudit
            modeCmd359B8TerminalRegsSourceAudit =
                PrMain::AuditStage1RuntimePsxMemorySources(
                    ctx,
                    kCdInterruptRegs1F801070,
                    kCdInterruptRegsBytes1F801070);
        const PrMainStage1RuntimePsxMemorySourceReadAudit
            modeCmd359B8WatchdogSourceAudit =
                PrMain::AuditStage1RuntimePsxMemorySources(
                    ctx,
                    kCdInterruptWatchdog80057010,
                    kCdInterruptWatchdogBytes80057010);
        const PrStage1XaCdDirectInterruptSnapshotRuntimeSource800359B8
            modeCmd359B8RuntimeSource =
                PrStage1XaCdDirectBuildInterruptSnapshotRuntimeSource800359B8(
                    modeCmd359B8RuntimeSourceWindow);
        const PrStage1XaCdDirectInterruptSnapshotRuntimeSourceResult800359B8
            modeCmd359B8RuntimeSourceResult =
                PrStage1XaCdDirectPublishInterruptSnapshotRuntimeSource800359B8(
                    modeCmd359B8RuntimeSource,
                    ctx.stage1XaCdDirect);
        const PrStage1XaCdDirectInterruptSnapshotTypedSourceAudit800359B8
            modeCmd359B8TypedAudit =
                PrStage1XaCdDirectAuditInterruptSnapshotTypedSource800359B8(
                    ctx.stage1XaCdDirect);
        const auto modeCmd359B8TypedFirstMissing = [&]() -> const char* {
            if (!modeCmd359B8TypedAudit.callbackStateTableKnown) {
                return "callbackStateTable";
            }
            if (!modeCmd359B8TypedAudit.initialInterruptRegsKnown) {
                return "initialRegs";
            }
            if (!modeCmd359B8TypedAudit.terminalInterruptRegsKnown) {
                return "terminalRegs";
            }
            if (!modeCmd359B8TypedAudit.watchdogKnown) {
                return "watchdog";
            }
            return "none";
        }();
        const PrMainStage1RuntimePsxMemorySourceReadAudit
            modeCmdCheckCallbackWord55F7AAudit =
                PrMain::AuditStage1RuntimePsxMemorySources(
                    ctx,
                    kCdCallbackPendingWord80055F7A,
                    2u);
        const bool modeCmdCheckCallbackProviderInstalled =
            ctx.stage1RuntimePsxMemoryProvider.installed &&
            ctx.stage1RuntimePsxMemoryProvider.read != nullptr;
        PrStage1XaCdDirectCallbackPendingRuntimeSourceWindow80055F7A
            modeCmdCheckCallbackWindow{};
        PrStage1XaCdDirectCallbackPendingRuntimeSource80055F7A
            modeCmdCheckCallbackSource{};
        PrStage1XaCdDirectCallbackPendingRuntimeSourceResult80055F7A
            modeCmdCheckCallbackSourceResult{};
        {
            ScopedRuntimePsxMemoryProviderPc801C81EC scopedPc(
                ctx.stage1RuntimePsxMemoryProvider,
                kFn80035898);
            modeCmdCheckCallbackWindow =
                PrStage1XaCdDirectReadCallbackPendingRuntimeSourceWindow80055F7A(
                    ctx.stage1RuntimePsxMemoryProvider);
            modeCmdCheckCallbackSource =
                PrStage1XaCdDirectBuildCallbackPendingRuntimeSource80055F7A(
                    modeCmdCheckCallbackWindow);
            modeCmdCheckCallbackSourceResult =
                PrStage1XaCdDirectPublishCallbackPendingRuntimeSource80055F7A(
                    modeCmdCheckCallbackSource,
                    ctx.stage1XaCdDirect);
        }
        PrMovieSegmentDirect::CheckCallbackInput80035898
            modeCmdCheckCallbackInput{};
        modeCmdCheckCallbackInput.word80055F7AKnown =
            modeCmdCheckCallbackSource.valueKnown ||
            ctx.stage1XaCdDirect.word_80055F7AKnown;
        modeCmdCheckCallbackInput.word80055F7A =
            modeCmdCheckCallbackSource.valueKnown
                ? modeCmdCheckCallbackSource.observation.value
                : ctx.stage1XaCdDirect.word_80055F7A;
        const PrMovieSegmentDirect::CheckCallbackResult80035898
            modeCmdCheckCallback =
                PrMovieSegmentDirect::PsxCall80035898_CheckCallback(
                    modeCmdCheckCallbackInput);
        const bool modeCmdCheckCallbackSourceAvailable =
            modeCmdCheckCallbackSource.sourceAvailable;
        const char* modeCmdCheckCallbackReadPathFirstMissing =
            !modeCmdCheckCallbackProviderInstalled
                ? "provider"
                : (!modeCmdCheckCallbackWindow.readAttempted
                       ? "readAttempt"
                       : (!modeCmdCheckCallbackWindow.windowReadable
                              ? "word80055F7A"
                              : "none"));
        PrStage1LowerCdProducerDirect::CommandWrapperInput80036540
            modeCmdProbe{};
        modeCmdProbe.command = 0x0Eu;
        modeCmdProbe.argsKnown = modeCmdArgsKnown;
        modeCmdProbe.argsPresent = modeCmdNeeded;
        modeCmdProbe.status57108Known = modeCmdStatus57108Known;
        modeCmdProbe.status57108 = modeCmdStatus57108;
        modeCmdProbe.commandNeedsSetlocKnown = modeCmdNeedsSetlocKnown;
        modeCmdProbe.commandNeedsSetloc = modeCmdNeedsSetloc;
        PrStage1LowerCdProducerDirect::CommandInput800375BC
            modeCmdCommand800375BCInput{};
        PrStage1LowerCdProducerDirect::CommandResult800375BC
            modeCmdCommand800375BC{};
        const PrStage1XaCdDirectCommandWaitLoopRuntimeSourceWindow800375BC
            modeCmdWaitLoopRuntimeSourceWindow = [&]() {
                ScopedRuntimePsxMemoryProviderPc801C81EC scopedPc(
                    ctx.stage1RuntimePsxMemoryProvider,
                    PrMovieSegmentDirect::kSub800375BCCdCommand);
                return PrStage1XaCdDirectReadCommandWaitLoopRuntimeSourceWindow800375BC(
                    ctx.stage1RuntimePsxMemoryProvider,
                    clockCarrierAccepted,
                    clockCarrierAccepted ? clockProbe.carrier.clockLba : 0);
            }();
        const PrStage1XaCdDirectCommandWaitLoopRuntimeSource800375BC
            modeCmdWaitLoopRuntimeSource =
                PrStage1XaCdDirectBuildCommandWaitLoopRuntimeSource800375BC(
                    modeCmdWaitLoopRuntimeSourceWindow);
        const PrStage1XaCdDirectCommandWaitLoopRuntimeSourceResult800375BC
            modeCmdWaitLoopRuntimeSourceResult =
                PrStage1XaCdDirectResolveCommandWaitLoopRuntimeSource800375BC(
                    modeCmdWaitLoopRuntimeSource);
        const bool modeCmdCommand800375BCInputBuilt =
            modeCmdCommand800375BCOwnerGate;
        auto& runtime = state.bootstrap15590CdLowerProducerRuntime;
        PrStage1XaCdDirectCommandResult800375BC
            modeCmdDirectOwner800375BC{};
        bool modeCmdDirectOwnerCommitAttempted = false;
        bool modeCmdDirectOwnerCommitSkippedSameRequest = false;
        if (modeCmdCommand800375BCInputBuilt) {
            uint8_t modeCmdArgBytes[1]{modeCmdArg0};
            const bool preSyncKnown =
                ctx.stage1XaCdDirect.cdSync80037070Known &&
                ctx.stage1XaCdDirect.cdSync80037070.syncResultKnown;
            const bool modeCmdCommand800375BCTimedOut =
                modeCmdWaitLoopRuntimeSourceResult.feedsCommand &&
                modeCmdWaitLoopRuntimeSourceResult.psxReturn == -1;
            modeCmdCommand800375BCInput =
                PrStage1LowerCdProducerDirect::BuildCommandInput800375BC(
                    0x0Eu,
                    true,
                    1u,
                    modeCmdArgBytes,
                    preSyncKnown,
                    preSyncKnown
                        ? ctx.stage1XaCdDirect.cdSync80037070.psxReturn
                        : 0,
                    false,
                    clockCarrierAccepted,
                    clockCarrierAccepted ? clockProbe.carrier.clockLba : 0,
                    clockCarrierAccepted,
                    modeCmdCommand800375BCTimedOut,
                    modeCmdCheckCallback.pendingKnown,
                    modeCmdCheckCallback.pending,
                    false,
                    0u,
                    0u,
                    false,
                    false,
                    0u,
                    false,
                    nullptr,
                    nullptr,
                    false,
                    modeCmdWaitLoopRuntimeSourceResult.feedsCommand,
                    modeCmdWaitLoopRuntimeSourceResult.psxReturn);
            modeCmdCommand800375BC =
                PrStage1LowerCdProducerDirect::BuildCommandResult800375BC(
                    modeCmdCommand800375BCInput);
            const bool modeCmdDirectOwnerAlreadyCommitted =
                runtime.modeCmd800375BCDirectOwnerCommitted &&
                runtime.modeCmd800375BCDirectOwnerReadS27Serial ==
                    ctx.stage1XaCdDirect.readS27Serial &&
                runtime.modeCmd800375BCDirectOwnerCommand == 0x0Eu &&
                runtime.modeCmd800375BCDirectOwnerArg0 == modeCmdArg0;
            modeCmdDirectOwnerCommitSkippedSameRequest =
                modeCmdDirectOwnerAlreadyCommitted;
            if (!modeCmdDirectOwnerAlreadyCommitted) {
                PrStage1XaCdDirectCommandInput800375BC directCommand{};
                directCommand.command = 0x0Eu;
                directCommand.argsKnown = true;
                directCommand.argCount = 1u;
                directCommand.args[0] = modeCmdArg0;
                directCommand.a3 = 0;
                directCommand.skipWait = false;
                directCommand.clockKnown = clockCarrierAccepted;
                directCommand.clockNow =
                    clockCarrierAccepted ? clockProbe.carrier.clockLba : 0;
                directCommand.waitLoopResultKnown =
                    modeCmdWaitLoopRuntimeSourceResult.feedsCommand;
                directCommand.waitLoopPsxReturn =
                    modeCmdWaitLoopRuntimeSourceResult.psxReturn;
                directCommand.checkCallback = modeCmdCheckCallbackInput;
                modeCmdDirectOwner800375BC =
                    PrStage1XaCdDirectApplySub800375BCCommand(
                        ctx.stage1XaCdDirect,
                        directCommand);
                modeCmdDirectOwnerCommitAttempted = true;
                runtime.modeCmd800375BCDirectOwnerCommitted = true;
                runtime.modeCmd800375BCDirectOwnerReadS27Serial =
                    ctx.stage1XaCdDirect.readS27Serial;
                runtime.modeCmd800375BCDirectOwnerCommand = 0x0Eu;
                runtime.modeCmd800375BCDirectOwnerArg0 = modeCmdArg0;
            }
            modeCmdProbe.attempts[0].commandKnown = true;
            modeCmdProbe.attempts[0].command = modeCmdCommand800375BC;
        }
        const PrStage1LowerCdProducerDirect::CommandWrapperResult80036540
            modeCmdProbeResult =
                PrStage1LowerCdProducerDirect::
                    BuildCommandWrapperResult80036540(modeCmdProbe);
        pumpProbe.modeCommandResult = modeCmdProbeResult;
        const bool modeCmdBlockedBeforeCommandAttempts =
            modeCmdNeeded && modeCmdArgsKnown && !modeCmdStatus57108Known &&
            modeCmdProbeResult.attemptsUsed == 0u;
        const PrStage1LowerCdProducerDirect::ReadPumpResult80038DE8
            pumpProbeResult =
                PrStage1LowerCdProducerDirect::
                    BuildReadPumpResult80038DE8(pumpProbe);
        const auto modeCmdFirstMissingForHalPath = [&]() -> const char* {
            if (!modeCmdNeeded) {
                return "none";
            }
            if (!modeCmdArgsKnown) {
                return "args";
            }
            if (!modeCmdStatus57108Known) {
                return "status57108";
            }
            if (!modeCmdNeedsSetlocKnown) {
                return "needsSetloc";
            }
            if ((modeCmdStatus57108 & 0x10u) != 0u) {
                return "restartCommand1";
            }
            if (modeCmdNeedsSetloc) {
                return "preSetloc";
            }
            if (!modeCmdCommand800375BC.produced ||
                modeCmdCommand800375BC.incomplete) {
                return "command800375BC";
            }
            return "none";
        }();
        const auto modeCmdCommand800375BCFirstMissingForHalPath =
            [&]() -> const char* {
            if (!modeCmdCommand800375BCInputBuilt) {
                return "notAttempted";
            }
            if (!modeCmdCommand800375BCInput.requiredArgCountKnown) {
                return "commandAttr";
            }
            if (modeCmdCommand800375BCInput.requiredArgCount != 0u &&
                (!modeCmdCommand800375BCInput.argsKnown ||
                 modeCmdCommand800375BCInput.args == nullptr ||
                 modeCmdCommand800375BCInput.argCount <
                     modeCmdCommand800375BCInput.requiredArgCount)) {
                return "args";
            }
            if (!modeCmdCommand800375BCInput.preSyncResultKnown) {
                return "preSync";
            }
            if (!modeCmdCommand800375BCInput.skipWait) {
                if (!modeCmdCommand800375BCInput.clockKnown) {
                    return "clock";
                }
                if (!modeCmdCommand800375BCInput.timeoutKnown) {
                    return "timeout";
                }
                if (modeCmdCommand800375BCInput.timedOut) {
                    return "none";
                }
                if (!modeCmdCommand800375BCInput.checkCallbackKnown) {
                    return "checkCallback";
                }
                if (modeCmdCommand800375BCInput.callbackPending &&
                    (!modeCmdCommand800375BCInput
                          .savedCdReg0SelectorKnown ||
                     !modeCmdCommand800375BCInput.callbackPumpKnown ||
                     !modeCmdCommand800375BCInput.callbackPumpDrained ||
                     modeCmdCommand800375BCInput.lastCallbackPumpReturn != 0 ||
                     !modeCmdCommand800375BCInput.selectorRestored ||
                     !modeCmdCommand800375BCInput
                          .rawCallbackTransactionSequenceKnown)) {
                    return "callbackPump";
                }
                if (!modeCmdCommand800375BCInput.waitLoopResultKnown) {
                    return "waitLoopResult";
                }
            }
            return modeCmdCommand800375BC.produced ? "none" : "unknown";
        }();
        const auto setupFirstMissing = [&]() -> const char* {
            if (!setupProbe.sectorCountKnown) {
                return "sector";
            }
            if (!setupProbe.dstKnown) {
                return "dst";
            }
            if (!setupProbe.modeKnown) {
                return "mode";
            }
            if (!setupProbe.savedSyncCallback.produced) {
                return "savedSync";
            }
            if (!setupProbe.savedReadyCallback.produced) {
                return "savedReady";
            }
            if (!setupProbe.status.produced || setupProbe.status.incomplete) {
                return "status";
            }
            if (!setupProbe.clockKnown) {
                return "clock";
            }
            if (setupProbeResult.preSeekRequested &&
                !setupProbe.preSeekResult800367A4.produced) {
                return "preSeek";
            }
            return setupProbeResult.produced ? "none" : "unknown";
        }();
        const auto pumpFirstMissing = [&]() -> const char* {
            if (!pumpProbe.clearSyncCallback.produced) {
                return "clearSync";
            }
            if (!pumpProbe.clearReadyCallback.produced) {
                return "clearReady";
            }
            if (!pumpProbe.status.produced || pumpProbe.status.incomplete) {
                return "status";
            }
            if (!pumpProbe.clockKnown) {
                return "clock";
            }
            if ((pumpProbe.status.status & 0x10u) != 0u &&
                (!pumpProbe.shellOpenCommandResultKnown ||
                 !pumpProbe.shellOpenCommandResult.produced ||
                 pumpProbe.shellOpenCommandResult.incomplete)) {
                return "shellOpen";
            }
            if (!pumpProbe.preReadSetupKnown) {
                return "preRead";
            }
            if (!pumpProbe.modeKnown) {
                return "mode";
            }
            if ((static_cast<uint8_t>(pumpProbe.modeWord8005741C) != 0u ||
                 pumpProbe.retry) &&
                (!pumpProbe.modeCommandResult.produced ||
                 pumpProbe.modeCommandResult.incomplete)) {
                return "modeCmd";
            }
            if (!pumpProbe.locSectorKnown) {
                return "loc";
            }
            if (!pumpProbe.activeDstKnown ||
                !pumpProbe.activeSectorCountKnown) {
                return "active";
            }
            if (!pumpProbe.startReadResultKnown ||
                !pumpProbe.startReadResult.produced ||
                pumpProbe.startReadResult.incomplete) {
                return "startRead";
            }
            return pumpProbeResult.produced ? "none" : "unknown";
        }();
        const uint8_t setupFirstMissingCode =
            CdLowerFirstMissingCodeFromName801C81EC(setupFirstMissing);
        const uint8_t pumpFirstMissingCode =
            CdLowerFirstMissingCodeFromName801C81EC(pumpFirstMissing);
        if (!facts.readStartHalFacts80038FC0Known) {
            facts.readStartSetup80038FC0 = setupProbeResult;
            facts.readPump80038DE8 = pumpProbeResult;
        }

        PrStage1LifecycleExecutorDirect::
            Bootstrap15590CdLowerFactsApplyResult801C81EC apply{};
        const bool applied =
            PrStage1LifecycleExecutorDirect::
                RunBootstrap15590CdLowerFacts801C81EC(
                    state,
                    ctx.stage1XaCdDirect,
                    facts,
                    &apply,
                    false);
        if (applied && discReadStartHalResultKnown &&
            discProbe.bytesRead == discProbe.bytes.size() &&
            discProbe.byteCountKnown &&
            discProbe.bytes.size() == discProbe.byteCount) {
            runtime.readStartHalCarrierPayloadBytesKnown = true;
            runtime.readStartHalCarrierPayloadBytes = discProbe.bytes;
        } else if (!runtime.readStartHalCarrierKnown ||
                   runtime.readStartHalCarrierReadS27Serial !=
                       ctx.stage1XaCdDirect.readS27Serial) {
            runtime.readStartHalCarrierPayloadBytesKnown = false;
            runtime.readStartHalCarrierPayloadBytes.clear();
        }
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower read-start HAL facts: progressApplied=%d requestMatched=%d xaCdAccepted=%d feedbackReady=%d status=%u reject=%u pending=%d/%d source=request_bound_hal streamStarted=%d readS27Serial=%u statusObs=%d/%u statusObsAccepted=%u statusObsRejected=%u statusObsReject=%u commandAccepted=%d/%d readStarted=%d/%d readFailed=%d/%d timeout=%d/%d dst=%d/%08X sectors=%d/%d setupFacts=%d/%d readPumpFacts=%d/%d setupFirstMissing=%s/%u pumpFirstMissing=%s/%u setupShape=%d discReadStartHal=%d requestDstKnown=%d requestSectorKnown=%d requestModeKnown=%d payloadBytesKnown=%d bridgeProduced=%d bridgeIncomplete=%d bridgeIn359B8Known=%d bridgeIn36AF8Known=%d bridgeIn37070Known=%d bridgePending359B8=%d bridgeEvent36AF8=%d bridgeCdSync37070=%d bridgeLowerCdFacts=%d discUserDataProbe=%d/%d bin=%d fileBase=%d/%d lastSeek=%d/%d byteCount=%d/%u bytes=%zu published=0",
            applied ? 1 : 0,
            apply.requestMatched ? 1 : 0,
            apply.xaCdAccepted ? 1 : 0,
            apply.feedbackReady ? 1 : 0,
            static_cast<unsigned>(apply.status),
            static_cast<unsigned>(apply.rejectReason),
            apply.requestPendingBefore ? 1 : 0,
            apply.requestPendingAfter ? 1 : 0,
            ctx.stage1XaCdDirect.streamStarted ? 1 : 0,
            ctx.stage1XaCdDirect.readS27Serial,
            ctx.stage1XaCdDirect.statusRead80036384Known ? 1 : 0,
            ctx.stage1XaCdDirect.statusRead80036384,
            ctx.stage1XaCdDirect.statusByte800573D4ObservationAcceptedCount,
            ctx.stage1XaCdDirect.statusByte800573D4ObservationRejectedCount,
            ctx.stage1XaCdDirect.statusByte800573D4LastReject,
            readStartFacts.commandAcceptedKnown ? 1 : 0,
            readStartFacts.commandAccepted ? 1 : 0,
            readStartFacts.readStartedKnown ? 1 : 0,
            readStartFacts.readStarted ? 1 : 0,
            readStartFacts.readFailedKnown ? 1 : 0,
            readStartFacts.readFailed ? 1 : 0,
            readStartFacts.timeoutKnown ? 1 : 0,
            readStartFacts.timeout ? 1 : 0,
            readStartFacts.dstPtrKnown ? 1 : 0,
            readStartFacts.dstPtr,
            readStartFacts.sectorCountKnown ? 1 : 0,
            readStartFacts.sectorCount,
            facts.readStartSetup80038FC0.produced ? 1 : 0,
            facts.readStartSetup80038FC0.incomplete ? 1 : 0,
            facts.readPump80038DE8.produced ? 1 : 0,
            facts.readPump80038DE8.incomplete ? 1 : 0,
            setupFirstMissing,
            static_cast<unsigned>(setupFirstMissingCode),
            pumpFirstMissing,
            static_cast<unsigned>(pumpFirstMissingCode),
            setupPathShapeKnown ? 1 : 0,
            discReadStartHalResultKnown ? 1 : 0,
            request.lowerRequest.readStartDstPtrKnown ? 1 : 0,
            request.lowerRequest.readStartSectorCountKnown ? 1 : 0,
            request.lowerRequest.readStartModeFlagKnown ? 1 : 0,
            facts.payloadBytesKnown ? 1 : 0,
            apply.bridgeProduced ? 1 : 0,
            apply.bridgeIncomplete ? 1 : 0,
            apply.bridgeInputInterruptSnapshot800359B8Known ? 1 : 0,
            apply.bridgeInputRawEvent80036AF8Known ? 1 : 0,
            apply.bridgeInputCdSyncLoopFacts80037070Known ? 1 : 0,
            apply.bridgePendingProducer800359B8Bridged ? 1 : 0,
            apply.bridgeLowerEventRegisters80036AF8Bridged ? 1 : 0,
            apply.bridgeCdSyncLoopFacts80037070Bridged ? 1 : 0,
            apply.bridgeLowerCdFactsBridged ? 1 : 0,
            discProbe.attempted ? 1 : 0,
            discProbe.success ? 1 : 0,
            discProbe.binPathKnown ? 1 : 0,
            discProbe.fileBaseLbaKnown ? 1 : 0,
            discProbe.fileBaseLba,
            discProbe.lastSeekLbaKnown ? 1 : 0,
            discProbe.lastSeekLba,
            discProbe.byteCountKnown ? 1 : 0,
            discProbe.byteCount,
            discProbe.bytesRead);
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower read-start HAL mode-command gate: modeCmdNeeded=%d modeCmdArg0=%u modeCmdFirstMissing=%s modeCmdStatus57108=%d/%u modeCmdStatus57108Accepted=%u modeCmd800375BCInputBuilt=%d modeCmd800375BCFirstMissing=%s modeCmd800375BCResult=%d/%d modeCmd800375BCTimedOut=%d modeCmd800375BCCheckCallbackKnown=%d modeCmd800375BCWaitLoopFeedsCommand=%d modeCmd800375BCWaitLoopPsxReturn=%d modeCmd800375BCWaitLoopBlocker=%u modeCmd800375BCDirectOwnerAttempted=%d modeCmd800375BCDirectOwnerSkippedSameRequest=%d modeCmd800375BCDirectOwnerCommitted=%d modeCmd800375BCDirectOwnerCalled=%d modeCmd800375BCDirectOwnerIncomplete=%d modeCmd80035898CheckCallbackSourceAvailable=%d modeCmd80035898CheckCallbackValueKnown=%d modeCmd80035898CheckCallbackReadPathFirstMissing=%s modeCmd80035898CheckCallbackPublishesState=%d modeCmd359B8RuntimeSourceBundleKnown=%d modeCmd359B8RuntimeSourcePublishAttempted=%d modeCmd359B8TypedFirstMissing=%s published=0",
            modeCmdNeeded ? 1 : 0,
            modeCmdArg0,
            modeCmdFirstMissingForHalPath,
            modeCmdStatus57108Known ? 1 : 0,
            modeCmdStatus57108,
            ctx.stage1XaCdDirect.statusFlags80057108ObservationAcceptedCount,
            modeCmdCommand800375BCInputBuilt ? 1 : 0,
            modeCmdCommand800375BCFirstMissingForHalPath,
            modeCmdCommand800375BC.produced ? 1 : 0,
            modeCmdCommand800375BC.incomplete ? 1 : 0,
            modeCmdCommand800375BCInput.timedOut ? 1 : 0,
            modeCmdCommand800375BCInput.checkCallbackKnown ? 1 : 0,
            modeCmdWaitLoopRuntimeSourceResult.feedsCommand ? 1 : 0,
            modeCmdWaitLoopRuntimeSourceResult.psxReturn,
            static_cast<unsigned>(modeCmdWaitLoopRuntimeSourceResult.blocker),
            modeCmdDirectOwnerCommitAttempted ? 1 : 0,
            modeCmdDirectOwnerCommitSkippedSameRequest ? 1 : 0,
            runtime.modeCmd800375BCDirectOwnerCommitted ? 1 : 0,
            modeCmdDirectOwner800375BC.called ? 1 : 0,
            modeCmdDirectOwner800375BC.incomplete ? 1 : 0,
            modeCmdCheckCallbackSourceAvailable ? 1 : 0,
            modeCmdCheckCallbackSource.valueKnown ? 1 : 0,
            modeCmdCheckCallbackReadPathFirstMissing,
            modeCmdCheckCallbackSourceResult.publishAttempted &&
                    modeCmdCheckCallbackSourceResult.observation.accepted
                ? 1
                : 0,
            modeCmd359B8RuntimeSourceResult.bundleKnown ? 1 : 0,
            modeCmd359B8RuntimeSourceResult.publishAttempted ? 1 : 0,
            modeCmd359B8TypedFirstMissing);
        runtime.lastFactsReadStartSetupFirstMissing = setupFirstMissingCode;
        runtime.lastFactsReadPumpFirstMissing = pumpFirstMissingCode;
        if (applied) {
            Log::Printf(
                "Scene1 801C81EC bootstrap15590 CD lower read-start applied 800359B8 source split tail: modeCmd359B8CbStateReadAttempted=%d modeCmd359B8CbStateReadable=%d modeCmd359B8InitialRegsReadAttempted=%d modeCmd359B8InitialRegsReadable=%d modeCmd359B8TerminalRegsReadAttempted=%d modeCmd359B8TerminalRegsReadable=%d modeCmd359B8WatchdogReadAttempted=%d modeCmd359B8WatchdogReadable=%d modeCmd359B8RuntimeSourceBundleKnown=%d modeCmd359B8RuntimeSourcePublishAttempted=%d modeCmd359B8TypedCallbackSlotsKnown=%d modeCmd359B8TypedCallbackStateWordsKnown=%d modeCmd359B8TypedCallbackStateTableKnown=%d modeCmd359B8TypedInitialRegsKnown=%d modeCmd359B8TypedTerminalRegsKnown=%d modeCmd359B8TypedWatchdogKnown=%d modeCmd359B8TypedMissingMask=%u modeCmd359B8TypedFirstMissing=%s modeCmd359B8TypedCanFeedObservation=%d modeCmd80035898CheckCallbackReadPathFirstMissing=%s modeCmd80035898CheckCallbackPublishesState=%d hostBlock4ReleasePending=1 saveUi19148StartPending=unknown saveUi19148Active=unknown published=0",
                modeCmd359B8RuntimeSourceWindow.callbackStateReadAttempted
                    ? 1
                    : 0,
                modeCmd359B8RuntimeSourceWindow.callbackStateReadable ? 1
                                                                     : 0,
                modeCmd359B8RuntimeSourceWindow.initialRegsReadAttempted ? 1
                                                                         : 0,
                modeCmd359B8RuntimeSourceWindow.initialRegsReadable ? 1
                                                                    : 0,
                modeCmd359B8RuntimeSourceWindow.terminalRegsReadAttempted ? 1
                                                                          : 0,
                modeCmd359B8RuntimeSourceWindow.terminalRegsReadable ? 1
                                                                     : 0,
                modeCmd359B8RuntimeSourceWindow.watchdogReadAttempted ? 1
                                                                      : 0,
                modeCmd359B8RuntimeSourceWindow.watchdogReadable ? 1 : 0,
                modeCmd359B8RuntimeSourceResult.bundleKnown ? 1 : 0,
                modeCmd359B8RuntimeSourceResult.publishAttempted ? 1 : 0,
                modeCmd359B8TypedAudit.callbackSlotsKnown ? 1 : 0,
                modeCmd359B8TypedAudit.callbackStateWordsKnown ? 1 : 0,
                modeCmd359B8TypedAudit.callbackStateTableKnown ? 1 : 0,
                modeCmd359B8TypedAudit.initialInterruptRegsKnown ? 1 : 0,
                modeCmd359B8TypedAudit.terminalInterruptRegsKnown ? 1 : 0,
                modeCmd359B8TypedAudit.watchdogKnown ? 1 : 0,
                modeCmd359B8TypedAudit.missingMask,
                modeCmd359B8TypedFirstMissing,
                modeCmd359B8TypedAudit.canFeedRuntimeObservation ? 1 : 0,
                modeCmdCheckCallbackReadPathFirstMissing,
                modeCmdCheckCallbackSourceResult.publishAttempted &&
                        modeCmdCheckCallbackSourceResult.observation.accepted
                    ? 1
                    : 0);
            if (runtime.readStartHalCarrierKnown &&
                runtime.readStartHalCarrierReadS27Serial ==
                    ctx.stage1XaCdDirect.readS27Serial) {
                if (setupProbeResult.produced && !setupProbeResult.incomplete) {
                    runtime.readStartHalCarrierSetup = setupProbeResult;
                }
                if (pumpProbeResult.produced && !pumpProbeResult.incomplete) {
                    runtime.readStartHalCarrierPump = pumpProbeResult;
                }
            }
            return true;
        }
        const PrStage1XaCdDirectRawEventRuntimeSourceWindow80036AF8
            modeCmdRawEventRuntimeSourceWindow = [&]() {
                ScopedRuntimePsxMemoryProviderPc801C81EC scopedPc(
                    ctx.stage1RuntimePsxMemoryProvider,
                    kFn80036AF8);
                return PrStage1XaCdDirectReadRawEventRuntimeSourceWindow80036AF8(
                    ctx.stage1RuntimePsxMemoryProvider);
            }();
        const PrStage1XaCdDirectRawEventInitialInterruptRuntimeSource80036AF8
            modeCmdRawEventInitialSource =
                PrStage1XaCdDirectBuildRawEventInitialInterruptRuntimeSource80036AF8(
                    modeCmdRawEventRuntimeSourceWindow);
        PrStage1XaCdDirectRawEventInitialInterruptRuntimeSourceResult80036AF8
            modeCmdRawEventInitialResult{};
        if (!ctx.stage1XaCdDirect.rawEvent80036AF8InitialInterruptKnown) {
            modeCmdRawEventInitialResult =
                PrStage1XaCdDirectPublishRawEventInitialInterruptRuntimeSource80036AF8(
                    modeCmdRawEventInitialSource,
                    ctx.stage1XaCdDirect);
        }
        const auto modeCmdCdMmioSamplePairSubmit =
            TrySubmitBootstrap15590CdMmioSamplePair801C81EC(ctx);
        const PrStage1XaCdDirectRawEventTypedSourceAudit80036AF8
            modeCmdRawEventTypedAudit =
                PrStage1XaCdDirectAuditRawEventTypedSource80036AF8(
                    ctx.stage1XaCdDirect);
        const PrStage1XaCdDirectRawEventRuntimeSource80036AF8
            modeCmdRawEventSource =
                PrStage1XaCdDirectBuildRawEventRuntimeSource80036AF8(
                    ctx.stage1XaCdDirect);
        const bool modeCmdRawEventCdMmioSubmitBlockedByTransaction =
            modeCmdRawEventSource.sourceAvailable &&
            !modeCmdRawEventSource.transactionKnown;
        const bool modeCmdRawEventWouldReachCdMmioSubmit =
            modeCmdRawEventSource.sourceAvailable &&
            modeCmdRawEventSource.transactionKnown &&
            ctx.stage1RuntimePsxMemoryProvider.cdMmioSourceInstalled &&
            ctx.stage1RuntimePsxMemoryProvider.cdMmioRead != nullptr &&
            ctx.stage1RuntimePsxMemoryProvider.cdMmioUserData != nullptr;
        const auto modeCmdRawEventTypedFirstMissingName =
            [&]() -> const char* {
                using FirstMissing =
                    PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8;
                switch (modeCmdRawEventTypedAudit.firstMissing) {
                case FirstMissing::None:
                    return "none";
                case FirstMissing::RegisterPointers:
                    return "registerPointers";
                case FirstMissing::InitialInterrupt:
                    return "initialInterrupt";
                case FirstMissing::StableInterrupt:
                    return "stableInterrupt";
                case FirstMissing::CdReg0Status:
                    return "cdReg0Status";
                case FirstMissing::FifoStatusSamples:
                    return "fifoStatusSamples";
                case FirstMissing::ResultByteCount:
                    return "resultByteCount";
                case FirstMissing::ResultBytes:
                    return "resultBytes";
                case FirstMissing::AckWrites:
                    return "ackWrites";
                case FirstMissing::PriorDword80057108:
                    return "priorDword80057108";
                case FirstMissing::PriorDword80057110:
                    return "priorDword80057110";
                case FirstMissing::PriorByte80057119:
                    return "priorByte80057119";
                }
                return "invalid";
            }();
        const auto cdMmioSnapshotAudit =
            PrStage1XaCdDirectAuditCdMmioSnapshotRuntimeSource(
                ctx.stage1RuntimePsxMemoryProvider);
        const auto modeCmdStatus57108ReadPathFirstMissing =
            [&]() -> const char* {
                if (modeCmdStatus57108RuntimeSourceWindow.windowReadable) {
                    return "none";
                }
                if (!ctx.stage1RuntimePsxMemoryProvider.cdMmioSourceInstalled) {
                    return "cdMmioSource";
                }
                if (ctx.stage1RuntimePsxMemoryProvider.cdMmioRead == nullptr) {
                    return "cdMmioReadFn";
                }
                if (cdMmioSnapshotAudit.producerObservationCallCount == 0u) {
                    return "cdMmioProducerCall";
                }
                return "cdMmioWindow";
            }();
        const auto modeCmd36AF8CdReg3InitialReadPathFirstMissing =
            [&]() -> const char* {
                if (modeCmdRawEventRuntimeSourceWindow
                        .cdReg3InitialReadable) {
                    return "none";
                }
                if (!ctx.stage1RuntimePsxMemoryProvider.cdMmioSourceInstalled) {
                    return "cdMmioSource";
                }
                if (ctx.stage1RuntimePsxMemoryProvider.cdMmioRead == nullptr) {
                    return "cdMmioReadFn";
                }
                if (cdMmioSnapshotAudit.producerObservationCallCount == 0u) {
                    return "cdMmioProducerCall";
                }
                return "cdMmioWindow";
            }();
        const auto modeCmdStatus57110ReadPathFirstMissing =
            [&]() -> const char* {
                if (modeCmdStatus57110RuntimeSourceWindow.windowReadable) {
                    return "none";
                }
                if (!ctx.stage1RuntimePsxMemoryProvider.cdMmioSourceInstalled) {
                    return "cdMmioSource";
                }
                if (ctx.stage1RuntimePsxMemoryProvider.cdMmioRead == nullptr) {
                    return "cdMmioReadFn";
                }
                if (cdMmioSnapshotAudit.producerObservationCallCount == 0u) {
                    return "cdMmioProducerCall";
                }
                return "cdMmioWindow";
            }();
        const auto modeCmdFirstMissing = [&]() -> const char* {
            if (!modeCmdNeeded) {
                return "none";
            }
            if (!modeCmdArgsKnown) {
                return "args";
            }
            if (!modeCmdStatus57108Known) {
                return "status57108";
            }
            if (!modeCmdNeedsSetlocKnown) {
                return "needsSetloc";
            }
            if ((modeCmdStatus57108 & 0x10u) != 0u) {
                return "restartCommand1";
            }
            if (modeCmdNeedsSetloc) {
                return "preSetloc";
            }
            if (!modeCmdCommand800375BC.produced ||
                modeCmdCommand800375BC.incomplete) {
                return "command800375BC";
            }
            return "none";
        }();
        const auto modeCmdCommand800375BCFirstMissing =
            [&]() -> const char* {
            if (!modeCmdCommand800375BCInputBuilt) {
                return "notAttempted";
            }
            if (!modeCmdCommand800375BCInput.requiredArgCountKnown) {
                return "commandAttr";
            }
            if (modeCmdCommand800375BCInput.requiredArgCount != 0u &&
                (!modeCmdCommand800375BCInput.argsKnown ||
                 modeCmdCommand800375BCInput.args == nullptr ||
                 modeCmdCommand800375BCInput.argCount <
                     modeCmdCommand800375BCInput.requiredArgCount)) {
                return "args";
            }
            if (!modeCmdCommand800375BCInput.preSyncResultKnown) {
                return "preSync";
            }
            if (!modeCmdCommand800375BCInput.skipWait) {
                if (!modeCmdCommand800375BCInput.clockKnown) {
                    return "clock";
                }
                if (!modeCmdCommand800375BCInput.timeoutKnown) {
                    return "timeout";
                }
                if (modeCmdCommand800375BCInput.timedOut) {
                    return "none";
                }
                if (!modeCmdCommand800375BCInput.checkCallbackKnown) {
                    return "checkCallback";
                }
                if (modeCmdCommand800375BCInput.callbackPending &&
                    (!modeCmdCommand800375BCInput
                          .savedCdReg0SelectorKnown ||
                     !modeCmdCommand800375BCInput.callbackPumpKnown ||
                     !modeCmdCommand800375BCInput.callbackPumpDrained ||
                     modeCmdCommand800375BCInput.lastCallbackPumpReturn != 0 ||
                     !modeCmdCommand800375BCInput.selectorRestored ||
                     !modeCmdCommand800375BCInput
                          .rawCallbackTransactionSequenceKnown)) {
                    return "callbackPump";
                }
                if (!modeCmdCommand800375BCInput.waitLoopResultKnown) {
                    return "waitLoopResult";
                }
            }
            return modeCmdCommand800375BC.produced ? "none" : "unknown";
        }();
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower read-start direct producer gap: setupFirstMissing=%s pumpFirstMissing=%s modeCmdFirstMissing=%s modeCmdNeeded=%d modeCmdArg0=%u modeCmdStatus57108=%d/%u modeCmdStatus57108ObsAccepted=%u modeCmdStatus57108ObsRejected=%u modeCmdStatus57108ObsReject=%u modeCmdStatusRead36384=%d/%u modeCmdStatusReadCanFeed57108=0 modeCmdStatus57108KnownStateRelayKnown=%d modeCmdStatus57108KnownStateRelayCanFeed=%d modeCmdStatus57108ReadPathFirstMissing=%s modeCmdRuntimeProviderLoaderHeap=%d modeCmdRuntimeProviderKnownStateRelay=%d modeCmdRuntimeProviderCdMmioSourceInstalled=%d modeCmdRuntimeProviderCdMmioReadFnInstalled=%d modeCmdRuntimeProviderCdMmioUserDataInstalled=%d modeCmdRuntimeProviderExactCdSourceInstalled=%d modeCmdRuntimeProviderExactCdReadFnInstalled=%d modeCmdRuntimeProviderExactCdUserDataInstalled=%d modeCmd359B8RuntimeSourceAdapter=1 modeCmd359B8ProviderInstalled=%d modeCmd359B8PcKnown=%d modeCmd359B8Pc=%08X modeCmd359B8CbStateReadAttempted=%d modeCmd359B8CbStateReadable=%d modeCmd359B8InitialRegsReadAttempted=%d modeCmd359B8InitialRegsReadable=%d modeCmd359B8TerminalRegsReadAttempted=%d modeCmd359B8TerminalRegsReadable=%d modeCmd359B8WatchdogReadAttempted=%d modeCmd359B8WatchdogReadable=%d modeCmd359B8RuntimeSourceAvailable=%d modeCmd359B8RuntimeSourceBundleKnown=%d modeCmd359B8RuntimeSourcePublishAttempted=%d modeCmd359B8RuntimeSourceAccepted=%d modeCmd359B8RuntimeSourceReject=%u modeCmd359B8TypedCallbackSlotsKnown=%d modeCmd359B8TypedCallbackStateWordsKnown=%d modeCmd359B8TypedCallbackStateTableKnown=%d modeCmd359B8TypedInitialRegsKnown=%d modeCmd359B8TypedTerminalRegsKnown=%d modeCmd359B8TypedWatchdogKnown=%d modeCmd359B8TypedMissingMask=%u modeCmd359B8TypedCanReconstructBundle=%d modeCmd359B8TypedFirstMissing=%s modeCmd359B8TypedCanFeedObservation=%d modeCmd36AF8InitialRuntimeSourceAdapter=1 modeCmd36AF8InitialProviderInstalled=%d modeCmd36AF8PcKnown=%d modeCmd36AF8Pc=%08X modeCmd36AF8InitialCdReg3ReadAttempted=%d modeCmd36AF8InitialCdReg3Readable=%d modeCmd36AF8InitialCdReg0ReadAttempted=%d modeCmd36AF8InitialCdReg0Readable=%d modeCmd36AF8InitialRuntimeSourceAvailable=%d modeCmd36AF8InitialRuntimeSourceValueKnown=%d modeCmd36AF8InitialRuntimeSourcePublishAttempted=%d modeCmd36AF8InitialRuntimeSourceBlocker=%u modeCmd36AF8InitialRuntimeSourceAccepted=%d modeCmd36AF8TypedMissingMask=%u modeCmd36AF8TypedFirstMissing=%u modeCmd36AF8CanFeedRuntimeSource=%d modeCmdStatus57108RuntimeSourceAdapter=1 modeCmdStatus57108RuntimeSourceProviderInstalled=%d modeCmdStatus57108RuntimeSourcePcKnown=%d modeCmdStatus57108RuntimeSourcePc=%08X modeCmdStatus57108RuntimeSourceReadAttempted=%d modeCmdStatus57108RuntimeSourceWindowReadable=%d modeCmdStatus57108RuntimeSourceAvailable=%d modeCmdStatus57108RuntimeSourceValueKnown=%d modeCmdStatus57108RuntimeSourcePublishAttempted=%d modeCmdStatus57108RuntimeSourceBlocker=%u modeCmdStatus57108RuntimeSourceAccepted=%d modeCmdNeedsSetloc=%d/%d modeCmdProbe=%d/%d modeCmdAttemptsUsed=%u modeCmdBlockedBeforeCommandAttempts=%d modeCmd800375BCInputBuilt=%d modeCmd800375BCFirstMissing=%s modeCmd800375BCResult=%d/%d modeCmd800375BCPreSyncKnown=%d modeCmd800375BCClockKnown=%d modeCmd800375BCTimeoutKnown=%d modeCmd800375BCCheckCallbackKnown=%d modeCmd800375BCWaitLoopKnown=%d modeCmd800375BCWaitLoopRuntimeSourceAdapter=1 modeCmd800375BCWaitLoopSourceAvailable=%d modeCmd800375BCWaitLoopResultKnown=%d modeCmd800375BCWaitLoopFeedsCommand=%d modeCmd800375BCWaitLoopBlocker=%u modeCmdStatus57108RequiredBeforeCommand=1 callbackSlots=sync:%d/%08X ready:%d/%08X statusObs=%d/%u statusObsAccepted=%u statusObsRejected=%u statusObsReject=%u callbackSwapProbe=sync:%d ready:%d readyCallbackProducer=writes:%u clears:%u install39240:%u source:%08X setupInputs=sector/dst/mode/savedSync/savedReady/status/clock/preSeek=%d/%d/%d/%d/%d/%d/%d/%d setupProbe=%d/%d pumpInputs=clearSync/clearReady/status/clock/shellOpen/preRead/mode/modeCmd/startRead/loc/active=%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d pumpProbe=%d/%d published=0",
            setupFirstMissing,
            pumpFirstMissing,
            modeCmdFirstMissing,
            modeCmdNeeded ? 1 : 0,
            modeCmdArg0,
            modeCmdStatus57108Known ? 1 : 0,
            modeCmdStatus57108,
            ctx.stage1XaCdDirect.statusFlags80057108ObservationAcceptedCount,
            ctx.stage1XaCdDirect.statusFlags80057108ObservationRejectedCount,
            ctx.stage1XaCdDirect.statusFlags80057108LastReject,
            ctx.stage1XaCdDirect.statusRead80036384Known ? 1 : 0,
            ctx.stage1XaCdDirect.statusRead80036384,
            ctx.stage1XaCdDirect.dword_80057108Known ? 1 : 0,
            ctx.stage1XaCdDirect.dword_80057108Known ? 1 : 0,
            modeCmdStatus57108ReadPathFirstMissing,
            ctx.stage1RuntimePsxMemoryProvider.loaderHeapSourceInstalled ? 1
                                                                         : 0,
            ctx.stage1RuntimePsxMemoryProvider.xaCdKnownStateRelayInstalled
                ? 1
                : 0,
            ctx.stage1RuntimePsxMemoryProvider.cdMmioSourceInstalled ? 1 : 0,
            ctx.stage1RuntimePsxMemoryProvider.cdMmioRead != nullptr ? 1 : 0,
            ctx.stage1RuntimePsxMemoryProvider.cdMmioUserData != nullptr ? 1
                                                                         : 0,
            ctx.stage1RuntimePsxMemoryProvider.exactCdSourceInstalled ? 1 : 0,
            ctx.stage1RuntimePsxMemoryProvider.exactCdRead != nullptr ? 1 : 0,
            ctx.stage1RuntimePsxMemoryProvider.exactCdUserData != nullptr ? 1
                                                                          : 0,
            modeCmd359B8RuntimeSourceWindow.providerInstalled ? 1 : 0,
            modeCmd359B8RuntimeSourceWindow.pcKnown ? 1 : 0,
            modeCmd359B8RuntimeSourceWindow.pc,
            modeCmd359B8RuntimeSourceWindow.callbackStateReadAttempted ? 1 : 0,
            modeCmd359B8RuntimeSourceWindow.callbackStateReadable ? 1 : 0,
            modeCmd359B8RuntimeSourceWindow.initialRegsReadAttempted ? 1 : 0,
            modeCmd359B8RuntimeSourceWindow.initialRegsReadable ? 1 : 0,
            modeCmd359B8RuntimeSourceWindow.terminalRegsReadAttempted ? 1 : 0,
            modeCmd359B8RuntimeSourceWindow.terminalRegsReadable ? 1 : 0,
            modeCmd359B8RuntimeSourceWindow.watchdogReadAttempted ? 1 : 0,
            modeCmd359B8RuntimeSourceWindow.watchdogReadable ? 1 : 0,
            modeCmd359B8RuntimeSourceResult.sourceAvailable ? 1 : 0,
            modeCmd359B8RuntimeSourceResult.bundleKnown ? 1 : 0,
            modeCmd359B8RuntimeSourceResult.publishAttempted ? 1 : 0,
            modeCmd359B8RuntimeSourceResult.observation.accepted ? 1 : 0,
            static_cast<unsigned>(
                modeCmd359B8RuntimeSourceResult.observation.rejectReason),
            modeCmd359B8TypedAudit.callbackSlotsKnown ? 1 : 0,
            modeCmd359B8TypedAudit.callbackStateWordsKnown ? 1 : 0,
            modeCmd359B8TypedAudit.callbackStateTableKnown ? 1 : 0,
            modeCmd359B8TypedAudit.initialInterruptRegsKnown ? 1 : 0,
            modeCmd359B8TypedAudit.terminalInterruptRegsKnown ? 1 : 0,
            modeCmd359B8TypedAudit.watchdogKnown ? 1 : 0,
            modeCmd359B8TypedAudit.missingMask,
            modeCmd359B8TypedAudit.typedCanReconstructBundle ? 1 : 0,
            modeCmd359B8TypedFirstMissing,
            modeCmd359B8TypedAudit.canFeedRuntimeObservation ? 1 : 0,
            modeCmdRawEventRuntimeSourceWindow.providerInstalled ? 1 : 0,
            modeCmdRawEventRuntimeSourceWindow.pcKnown ? 1 : 0,
            modeCmdRawEventRuntimeSourceWindow.pc,
            modeCmdRawEventRuntimeSourceWindow.cdReg3InitialReadAttempted ? 1
                                                                          : 0,
            modeCmdRawEventRuntimeSourceWindow.cdReg3InitialReadable ? 1 : 0,
            modeCmdRawEventRuntimeSourceWindow.cdReg0StatusReadAttempted ? 1
                                                                         : 0,
            modeCmdRawEventRuntimeSourceWindow.cdReg0StatusReadable ? 1 : 0,
            modeCmdRawEventInitialResult.sourceAvailable ? 1 : 0,
            modeCmdRawEventInitialResult.valueKnown ? 1 : 0,
            modeCmdRawEventInitialResult.publishAttempted ? 1 : 0,
            static_cast<unsigned>(modeCmdRawEventInitialResult.blocker),
            modeCmdRawEventInitialResult.observation.accepted ? 1 : 0,
            modeCmdRawEventTypedAudit.missingMask,
            static_cast<unsigned>(modeCmdRawEventTypedAudit.firstMissing),
            modeCmdRawEventTypedAudit.canFeedRuntimeSource ? 1 : 0,
            modeCmdStatus57108RuntimeSourceWindow.providerInstalled ? 1 : 0,
            modeCmdStatus57108RuntimeSourceWindow.pcKnown ? 1 : 0,
            modeCmdStatus57108RuntimeSourceWindow.pc,
            modeCmdStatus57108RuntimeSourceWindow.readAttempted ? 1 : 0,
            modeCmdStatus57108RuntimeSourceWindow.windowReadable ? 1 : 0,
            modeCmdStatus57108RuntimeSourceResult.sourceAvailable ? 1 : 0,
            modeCmdStatus57108RuntimeSourceResult.valueKnown ? 1 : 0,
            modeCmdStatus57108RuntimeSourceResult.publishAttempted ? 1 : 0,
            static_cast<unsigned>(
                modeCmdStatus57108RuntimeSourceResult.blocker),
            modeCmdStatus57108RuntimeSourceResult.observation.accepted ? 1
                                                                       : 0,
            modeCmdNeedsSetlocKnown ? 1 : 0,
            modeCmdNeedsSetloc ? 1 : 0,
            modeCmdProbeResult.produced ? 1 : 0,
            modeCmdProbeResult.incomplete ? 1 : 0,
            modeCmdProbeResult.attemptsUsed,
            modeCmdBlockedBeforeCommandAttempts ? 1 : 0,
            modeCmdCommand800375BCInputBuilt ? 1 : 0,
            modeCmdCommand800375BCFirstMissing,
            modeCmdCommand800375BC.produced ? 1 : 0,
            modeCmdCommand800375BC.incomplete ? 1 : 0,
            modeCmdCommand800375BCInput.preSyncResultKnown ? 1 : 0,
            modeCmdCommand800375BCInput.clockKnown ? 1 : 0,
            modeCmdCommand800375BCInput.timeoutKnown ? 1 : 0,
            modeCmdCommand800375BCInput.checkCallbackKnown ? 1 : 0,
            modeCmdCommand800375BCInput.waitLoopResultKnown ? 1 : 0,
            modeCmdWaitLoopRuntimeSourceResult.sourceAvailable ? 1 : 0,
            modeCmdWaitLoopRuntimeSourceResult.resultKnown ? 1 : 0,
            modeCmdWaitLoopRuntimeSourceResult.feedsCommand ? 1 : 0,
            static_cast<unsigned>(modeCmdWaitLoopRuntimeSourceResult.blocker),
            ctx.stage1XaCdDirect.dword_800570F8Known ? 1 : 0,
            ctx.stage1XaCdDirect.dword_800570F8,
            ctx.stage1XaCdDirect.dword_800570FCKnown ? 1 : 0,
            ctx.stage1XaCdDirect.dword_800570FC,
            ctx.stage1XaCdDirect.statusRead80036384Known ? 1 : 0,
            ctx.stage1XaCdDirect.statusRead80036384,
            ctx.stage1XaCdDirect.statusByte800573D4ObservationAcceptedCount,
            ctx.stage1XaCdDirect.statusByte800573D4ObservationRejectedCount,
            ctx.stage1XaCdDirect.statusByte800573D4LastReject,
            setupProbe.savedSyncCallback.produced ? 1 : 0,
            setupProbe.savedReadyCallback.produced ? 1 : 0,
            ctx.stage1XaCdDirect.readyCallbackRegisterWriteCount,
            ctx.stage1XaCdDirect.readyCallbackRegisterClearCount,
            ctx.stage1XaCdDirect.readyCallbackRegisterInstall39240Count,
            ctx.stage1XaCdDirect.readyCallbackRegisterSourceFunction,
            setupProbe.sectorCountKnown ? 1 : 0,
            setupProbe.dstKnown ? 1 : 0,
            setupProbe.modeKnown ? 1 : 0,
            setupProbe.savedSyncCallback.produced ? 1 : 0,
            setupProbe.savedReadyCallback.produced ? 1 : 0,
            setupProbe.status.produced ? 1 : 0,
            setupProbe.clockKnown ? 1 : 0,
            setupProbe.preSeekResult800367A4.produced ? 1 : 0,
            setupProbeResult.produced ? 1 : 0,
            setupProbeResult.incomplete ? 1 : 0,
            pumpProbe.clearSyncCallback.produced ? 1 : 0,
            pumpProbe.clearReadyCallback.produced ? 1 : 0,
            pumpProbe.status.produced ? 1 : 0,
            pumpProbe.clockKnown ? 1 : 0,
            pumpProbe.shellOpenCommandResultKnown ? 1 : 0,
            pumpProbe.preReadSetupKnown ? 1 : 0,
            pumpProbe.modeKnown ? 1 : 0,
            pumpProbe.modeCommandResult.produced ? 1 : 0,
            pumpProbe.startReadResultKnown ? 1 : 0,
            pumpProbe.locSectorKnown ? 1 : 0,
            (pumpProbe.activeDstKnown && pumpProbe.activeSectorCountKnown) ? 1
                                                                           : 0,
            pumpProbeResult.produced ? 1 : 0,
            pumpProbeResult.incomplete ? 1 : 0);
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower read-start direct producer 800359B8 source split tail: modeCmd359B8CbStateLoaderHeapReadAttempted=%d modeCmd359B8CbStateLoaderHeapReadable=%d modeCmd359B8CbStateKnownStateRelayReadAttempted=%d modeCmd359B8CbStateKnownStateRelayReadable=%d modeCmd359B8CbStateCdMmioSnapshotReadAttempted=%d modeCmd359B8CbStateCdMmioSnapshotReadable=%d modeCmd359B8InitialRegsLoaderHeapReadAttempted=%d modeCmd359B8InitialRegsLoaderHeapReadable=%d modeCmd359B8InitialRegsKnownStateRelayReadAttempted=%d modeCmd359B8InitialRegsKnownStateRelayReadable=%d modeCmd359B8InitialRegsCdMmioSnapshotReadAttempted=%d modeCmd359B8InitialRegsCdMmioSnapshotReadable=%d modeCmd359B8TerminalRegsLoaderHeapReadAttempted=%d modeCmd359B8TerminalRegsLoaderHeapReadable=%d modeCmd359B8TerminalRegsKnownStateRelayReadAttempted=%d modeCmd359B8TerminalRegsKnownStateRelayReadable=%d modeCmd359B8TerminalRegsCdMmioSnapshotReadAttempted=%d modeCmd359B8TerminalRegsCdMmioSnapshotReadable=%d modeCmd359B8WatchdogLoaderHeapReadAttempted=%d modeCmd359B8WatchdogLoaderHeapReadable=%d modeCmd359B8WatchdogKnownStateRelayReadAttempted=%d modeCmd359B8WatchdogKnownStateRelayReadable=%d modeCmd359B8WatchdogCdMmioSnapshotReadAttempted=%d modeCmd359B8WatchdogCdMmioSnapshotReadable=%d modeCmd359B8RuntimeSourceBundleKnown=%d modeCmd359B8RuntimeSourcePublishAttempted=%d modeCmd359B8TypedCanFeedObservation=%d published=0",
            modeCmd359B8CbStateSourceAudit.loaderHeapReadAttempted ? 1 : 0,
            modeCmd359B8CbStateSourceAudit.loaderHeapReadable ? 1 : 0,
            modeCmd359B8CbStateSourceAudit.knownStateRelayReadAttempted ? 1
                                                                         : 0,
            modeCmd359B8CbStateSourceAudit.knownStateRelayReadable ? 1 : 0,
            modeCmd359B8CbStateSourceAudit.cdMmioSnapshotReadAttempted ? 1
                                                                       : 0,
            modeCmd359B8CbStateSourceAudit.cdMmioSnapshotReadable ? 1 : 0,
            modeCmd359B8InitialRegsSourceAudit.loaderHeapReadAttempted ? 1
                                                                       : 0,
            modeCmd359B8InitialRegsSourceAudit.loaderHeapReadable ? 1 : 0,
            modeCmd359B8InitialRegsSourceAudit.knownStateRelayReadAttempted
                ? 1
                : 0,
            modeCmd359B8InitialRegsSourceAudit.knownStateRelayReadable ? 1
                                                                       : 0,
            modeCmd359B8InitialRegsSourceAudit.cdMmioSnapshotReadAttempted
                ? 1
                : 0,
            modeCmd359B8InitialRegsSourceAudit.cdMmioSnapshotReadable ? 1
                                                                      : 0,
            modeCmd359B8TerminalRegsSourceAudit.loaderHeapReadAttempted ? 1
                                                                        : 0,
            modeCmd359B8TerminalRegsSourceAudit.loaderHeapReadable ? 1
                                                                  : 0,
            modeCmd359B8TerminalRegsSourceAudit.knownStateRelayReadAttempted
                ? 1
                : 0,
            modeCmd359B8TerminalRegsSourceAudit.knownStateRelayReadable ? 1
                                                                        : 0,
            modeCmd359B8TerminalRegsSourceAudit.cdMmioSnapshotReadAttempted
                ? 1
                : 0,
            modeCmd359B8TerminalRegsSourceAudit.cdMmioSnapshotReadable ? 1
                                                                       : 0,
            modeCmd359B8WatchdogSourceAudit.loaderHeapReadAttempted ? 1 : 0,
            modeCmd359B8WatchdogSourceAudit.loaderHeapReadable ? 1 : 0,
            modeCmd359B8WatchdogSourceAudit.knownStateRelayReadAttempted ? 1
                                                                         : 0,
            modeCmd359B8WatchdogSourceAudit.knownStateRelayReadable ? 1 : 0,
            modeCmd359B8WatchdogSourceAudit.cdMmioSnapshotReadAttempted ? 1
                                                                        : 0,
            modeCmd359B8WatchdogSourceAudit.cdMmioSnapshotReadable ? 1 : 0,
            modeCmd359B8RuntimeSourceResult.bundleKnown ? 1 : 0,
            modeCmd359B8RuntimeSourceResult.publishAttempted ? 1 : 0,
            modeCmd359B8TypedAudit.canFeedRuntimeObservation ? 1 : 0);
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower read-start direct producer 800359B8 exactCd source split tail: modeCmd359B8CbStateExactCdReadAttempted=%d modeCmd359B8CbStateExactCdReadable=%d modeCmd359B8InitialRegsExactCdReadAttempted=%d modeCmd359B8InitialRegsExactCdReadable=%d modeCmd359B8TerminalRegsExactCdReadAttempted=%d modeCmd359B8TerminalRegsExactCdReadable=%d modeCmd359B8WatchdogExactCdReadAttempted=%d modeCmd359B8WatchdogExactCdReadable=%d published=0",
            modeCmd359B8CbStateSourceAudit.exactCdReadAttempted ? 1 : 0,
            modeCmd359B8CbStateSourceAudit.exactCdReadable ? 1 : 0,
            modeCmd359B8InitialRegsSourceAudit.exactCdReadAttempted ? 1 : 0,
            modeCmd359B8InitialRegsSourceAudit.exactCdReadable ? 1 : 0,
            modeCmd359B8TerminalRegsSourceAudit.exactCdReadAttempted ? 1 : 0,
            modeCmd359B8TerminalRegsSourceAudit.exactCdReadable ? 1 : 0,
            modeCmd359B8WatchdogSourceAudit.exactCdReadAttempted ? 1 : 0,
            modeCmd359B8WatchdogSourceAudit.exactCdReadable ? 1 : 0);
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower read-start direct producer cdmmio producer tail: modeCmdCdMmioProducerIngressInstalled=%d modeCmdCdMmioProviderSubmitAvailable=%d modeCmdCdMmioProducerObservationCalls=%u modeCmdCdMmioSamplePairAvailable=%d modeCmdCdMmioObsAccepted=%u modeCmdCdMmioObsRejected=%u modeCmdCdMmioObsReject=%u modeCmdCdMmioSamplePairSubmitAttempted=%d modeCmdCdMmioSamplePairSourceAvailable=%d modeCmdCdMmioSamplePairPublishAttempted=%d modeCmdCdMmioSamplePairAccepted=%d modeCmdCdMmioSamplePairCdReg3Reject=%u modeCmdCdMmioSamplePairCdReg0Reject=%u modeCmdCdMmioRouteCallsite=bootstrap15590 modeCmdCdMmioRouteCallsiteAuthority=fail-closed modeCmdStatus57108ReadPathFirstMissing=%s modeCmdStatus57110ReadPathFirstMissing=%s modeCmd36AF8CdReg3InitialReadPathFirstMissing=%s modeCmd36AF8CdReg3InitialExactCdFallbackRequiresAcceptedObservation=1 modeCmd36AF8CdReg3InitialExactCdFallbackAcceptedCount=%u modeCmd36AF8CdReg3InitialExactCdFallbackCanCreate=0 published=0",
            cdMmioSnapshotAudit.producerIngressInstalled ? 1 : 0,
            cdMmioSnapshotAudit.providerSubmitAvailable ? 1 : 0,
            cdMmioSnapshotAudit.producerObservationCallCount,
            cdMmioSnapshotAudit.samplePairAvailable ? 1 : 0,
            cdMmioSnapshotAudit.observationAcceptedCount,
            cdMmioSnapshotAudit.observationRejectedCount,
            static_cast<unsigned>(cdMmioSnapshotAudit.lastRejectReason),
            modeCmdCdMmioSamplePairSubmit.attempted ? 1 : 0,
            modeCmdCdMmioSamplePairSubmit.sourceAvailable ? 1 : 0,
            modeCmdCdMmioSamplePairSubmit.publishAttempted ? 1 : 0,
            modeCmdCdMmioSamplePairSubmit.accepted ? 1 : 0,
            static_cast<unsigned>(
                modeCmdCdMmioSamplePairSubmit.cdReg3RejectReason),
            static_cast<unsigned>(
                modeCmdCdMmioSamplePairSubmit.cdReg0RejectReason),
            modeCmdStatus57108ReadPathFirstMissing,
            modeCmdStatus57110ReadPathFirstMissing,
            modeCmd36AF8CdReg3InitialReadPathFirstMissing,
            ctx.stage1XaCdDirect
                .rawEvent80036AF8InitialInterruptObservationAcceptedCount);
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower read-start direct producer gap tail: callbackSlots=sync:%d/%08X ready:%d/%08X setupInputs=sector/dst/mode/savedSync/savedReady/status/clock/preSeek=%d/%d/%d/%d/%d/%d/%d/%d pumpInputs=clearSync/clearReady/status/clock/shellOpen/preRead/mode/modeCmd/startRead/loc/active=%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d pumpFirstMissing=%s pumpProbe=%d/%d published=0",
            ctx.stage1XaCdDirect.dword_800570F8Known ? 1 : 0,
            ctx.stage1XaCdDirect.dword_800570F8,
            ctx.stage1XaCdDirect.dword_800570FCKnown ? 1 : 0,
            ctx.stage1XaCdDirect.dword_800570FC,
            setupProbe.sectorCountKnown ? 1 : 0,
            setupProbe.dstKnown ? 1 : 0,
            setupProbe.modeKnown ? 1 : 0,
            setupProbe.savedSyncCallback.produced ? 1 : 0,
            setupProbe.savedReadyCallback.produced ? 1 : 0,
            setupProbe.status.produced ? 1 : 0,
            setupProbe.clockKnown ? 1 : 0,
            setupProbe.preSeekResult800367A4.produced ? 1 : 0,
            pumpProbe.clearSyncCallback.produced ? 1 : 0,
            pumpProbe.clearReadyCallback.produced ? 1 : 0,
            pumpProbe.status.produced ? 1 : 0,
            pumpProbe.clockKnown ? 1 : 0,
            pumpProbe.shellOpenCommandResultKnown ? 1 : 0,
            pumpProbe.preReadSetupKnown ? 1 : 0,
            pumpProbe.modeKnown ? 1 : 0,
            pumpProbe.modeCommandResult.produced ? 1 : 0,
            pumpProbe.startReadResultKnown ? 1 : 0,
            pumpProbe.locSectorKnown ? 1 : 0,
            (pumpProbe.activeDstKnown && pumpProbe.activeSectorCountKnown) ? 1
                                                                           : 0,
            pumpFirstMissing,
            pumpProbeResult.produced ? 1 : 0,
            pumpProbeResult.incomplete ? 1 : 0);
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower read-start direct producer raw-event tail: modeCmd36AF8RawEventSourceAvailable=%d modeCmd36AF8RawEventSourceTransactionKnown=%d modeCmd36AF8RawEventSourcePublishAttempted=0 modeCmd36AF8RawEventCdMmioSubmitAttempted=0 modeCmd36AF8RawEventCdMmioSubmitBlockedByTransaction=%d modeCmd36AF8RawEventCdMmioSubmitWouldReach=%d modeCmd36AF8RawEventCdMmioSubmitAccepted=0 modeCmd36AF8TypedRegisterPointersKnown=%d modeCmd36AF8TypedInitialInterruptKnown=%d modeCmd36AF8TypedStableInterruptKnown=%d modeCmd36AF8TypedCdReg0StatusKnown=%d modeCmd36AF8TypedFifoStatusSamplesKnown=%d modeCmd36AF8TypedResultByteCountKnown=%d modeCmd36AF8TypedResultBytesKnown=%d modeCmd36AF8TypedAckWritesKnown=%d modeCmd36AF8TypedPrior57108Known=%d modeCmd36AF8TypedPrior57110Known=%d modeCmd36AF8TypedPrior57119Known=%d modeCmd36AF8TypedPrior57119Value=%u modeCmd36AF8TypedPrior57119Producer=%08X modeCmd36AF8LastCdCommand=%u modeCmd36AF8CommandSerial=%u modeCmd36AF8TypedCanReconstructTransaction=%d modeCmd36AF8TypedFirstMissingName=%s published=0",
            modeCmdRawEventSource.sourceAvailable ? 1 : 0,
            modeCmdRawEventSource.transactionKnown ? 1 : 0,
            modeCmdRawEventCdMmioSubmitBlockedByTransaction ? 1 : 0,
            modeCmdRawEventWouldReachCdMmioSubmit ? 1 : 0,
            modeCmdRawEventTypedAudit.registerPointersKnown ? 1 : 0,
            modeCmdRawEventTypedAudit.initialInterruptKnown ? 1 : 0,
            modeCmdRawEventTypedAudit.stableInterruptKnown ? 1 : 0,
            modeCmdRawEventTypedAudit.cdReg0StatusKnown ? 1 : 0,
            modeCmdRawEventTypedAudit.fifoStatusSamplesKnown ? 1 : 0,
            modeCmdRawEventTypedAudit.resultByteCountKnown ? 1 : 0,
            modeCmdRawEventTypedAudit.resultBytesKnown ? 1 : 0,
            modeCmdRawEventTypedAudit.ackWritesKnown ? 1 : 0,
            modeCmdRawEventTypedAudit.priorDword80057108Known ? 1 : 0,
            modeCmdRawEventTypedAudit.priorDword80057110Known ? 1 : 0,
            modeCmdRawEventTypedAudit.priorByte80057119Known ? 1 : 0,
            ctx.stage1XaCdDirect.byte_80057119,
            ctx.stage1XaCdDirect.byte80057119ProducerFunction,
            ctx.stage1XaCdDirect.lastCdCommand,
            ctx.stage1XaCdDirect.commandSerial,
            modeCmdRawEventTypedAudit.typedCanReconstructTransaction ? 1 : 0,
            modeCmdRawEventTypedFirstMissingName);
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower read-start direct producer status tail: modeCmdStatus57108Known=%d modeCmdStatus57108Value=%u modeCmdStatus57108ObsAccepted=%u modeCmdStatus57108ObsRejected=%u modeCmdStatus57108ObsReject=%u modeCmdStatus57108RuntimeSourceAdapter=1 modeCmdStatus57108RuntimeSourceProviderInstalled=%d modeCmdStatus57108RuntimeSourcePcKnown=%d modeCmdStatus57108RuntimeSourcePc=%08X modeCmdStatus57108RuntimeSourceReadAttempted=%d modeCmdStatus57108RuntimeSourceWindowReadable=%d modeCmdStatus57108RuntimeSourceAvailable=%d modeCmdStatus57108RuntimeSourceValueKnown=%d modeCmdStatus57108RuntimeSourcePublishAttempted=%d modeCmdStatus57108RuntimeSourceBlocker=%u modeCmdStatus57108RuntimeSourceAccepted=%d modeCmdNeedsSetloc=%d/%d modeCmdProbe=%d/%d modeCmdAttemptsUsed=%u modeCmdBlockedBeforeCommandAttempts=%d modeCmd800375BCInputBuilt=%d modeCmd800375BCFirstMissing=%s modeCmd800375BCResult=%d/%d modeCmd800375BCPreSyncKnown=%d modeCmd800375BCClockKnown=%d modeCmd800375BCTimeoutKnown=%d modeCmd800375BCCheckCallbackKnown=%d modeCmd800375BCWaitLoopKnown=%d modeCmd800375BCWaitLoopRuntimeSourceAdapter=1 modeCmd800375BCWaitLoopSourceAvailable=%d modeCmd800375BCWaitLoopResultKnown=%d modeCmd800375BCWaitLoopFeedsCommand=%d modeCmd800375BCWaitLoopBlocker=%u modeCmdStatus57108RequiredBeforeCommand=1 modeCmdStatus57108ConsumerBeforeCommand=1 modeCmdCommandWrapperConsumes57108=1 modeCmdCommandWrapperProduces57108=0 published=0",
            modeCmdStatus57108Known ? 1 : 0,
            modeCmdStatus57108,
            ctx.stage1XaCdDirect.statusFlags80057108ObservationAcceptedCount,
            ctx.stage1XaCdDirect.statusFlags80057108ObservationRejectedCount,
            ctx.stage1XaCdDirect.statusFlags80057108LastReject,
            modeCmdStatus57108RuntimeSourceWindow.providerInstalled ? 1 : 0,
            modeCmdStatus57108RuntimeSourceWindow.pcKnown ? 1 : 0,
            modeCmdStatus57108RuntimeSourceWindow.pc,
            modeCmdStatus57108RuntimeSourceWindow.readAttempted ? 1 : 0,
            modeCmdStatus57108RuntimeSourceWindow.windowReadable ? 1 : 0,
            modeCmdStatus57108RuntimeSourceResult.sourceAvailable ? 1 : 0,
            modeCmdStatus57108RuntimeSourceResult.valueKnown ? 1 : 0,
            modeCmdStatus57108RuntimeSourceResult.publishAttempted ? 1 : 0,
            static_cast<unsigned>(
                modeCmdStatus57108RuntimeSourceResult.blocker),
            modeCmdStatus57108RuntimeSourceResult.observation.accepted ? 1
                                                                       : 0,
            modeCmdNeedsSetlocKnown ? 1 : 0,
            modeCmdNeedsSetloc ? 1 : 0,
            modeCmdProbeResult.produced ? 1 : 0,
            modeCmdProbeResult.incomplete ? 1 : 0,
            modeCmdProbeResult.attemptsUsed,
            modeCmdBlockedBeforeCommandAttempts ? 1 : 0,
            modeCmdCommand800375BCInputBuilt ? 1 : 0,
            modeCmdCommand800375BCFirstMissing,
            modeCmdCommand800375BC.produced ? 1 : 0,
            modeCmdCommand800375BC.incomplete ? 1 : 0,
            modeCmdCommand800375BCInput.preSyncResultKnown ? 1 : 0,
            modeCmdCommand800375BCInput.clockKnown ? 1 : 0,
            modeCmdCommand800375BCInput.timeoutKnown ? 1 : 0,
            modeCmdCommand800375BCInput.checkCallbackKnown ? 1 : 0,
            modeCmdCommand800375BCInput.waitLoopResultKnown ? 1 : 0,
            modeCmdWaitLoopRuntimeSourceResult.sourceAvailable ? 1 : 0,
            modeCmdWaitLoopRuntimeSourceResult.resultKnown ? 1 : 0,
            modeCmdWaitLoopRuntimeSourceResult.feedsCommand ? 1 : 0,
            static_cast<unsigned>(
                modeCmdWaitLoopRuntimeSourceResult.blocker));
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower read-start direct producer 80057110 source tail: modeCmdStatus57110RuntimeSourceAdapter=1 modeCmdStatus57110RuntimeSourceProviderInstalled=%d modeCmdStatus57110RuntimeSourcePcKnown=%d modeCmdStatus57110RuntimeSourcePc=%08X modeCmdStatus57110RuntimeSourceReadAttempted=%d modeCmdStatus57110RuntimeSourceWindowReadable=%d modeCmdStatus57110RuntimeSourceAvailable=%d modeCmdStatus57110RuntimeSourceValueKnown=%d modeCmdStatus57110RuntimeSourceValue=%u modeCmdStatus57110RuntimeSourcePublishAttempted=%d modeCmdStatus57110RuntimeSourceBlocker=%u modeCmdStatus57110RuntimeSourceAccepted=%d modeCmdStatus57110RuntimeSourceReject=%u modeCmdStatus57110ReadPathFirstMissing=%s modeCmdStatus57110LoaderHeapAttempted=%d modeCmdStatus57110LoaderHeapReadable=%d modeCmdStatus57110KnownStateRelayAttempted=%d modeCmdStatus57110KnownStateRelayReadable=%d modeCmdStatus57110CdMmioAttempted=%d modeCmdStatus57110CdMmioReadable=%d modeCmdStatus57110ExactCdAttempted=%d modeCmdStatus57110ExactCdReadable=%d modeCmdStatus57110FeedsRawEventPrior=%d published=0",
            modeCmdStatus57110RuntimeSourceWindow.providerInstalled ? 1 : 0,
            modeCmdStatus57110RuntimeSourceWindow.pcKnown ? 1 : 0,
            modeCmdStatus57110RuntimeSourceWindow.pc,
            modeCmdStatus57110RuntimeSourceWindow.readAttempted ? 1 : 0,
            modeCmdStatus57110RuntimeSourceWindow.windowReadable ? 1 : 0,
            modeCmdStatus57110RuntimeSourceResult.sourceAvailable ? 1 : 0,
            modeCmdStatus57110RuntimeSourceResult.valueKnown ? 1 : 0,
            modeCmdStatus57110RuntimeSourceWindow.value,
            modeCmdStatus57110RuntimeSourceResult.publishAttempted ? 1 : 0,
            static_cast<unsigned>(
                modeCmdStatus57110RuntimeSourceResult.blocker),
            modeCmdStatus57110RuntimeSourceResult.observation.accepted ? 1
                                                                       : 0,
            static_cast<unsigned>(
                modeCmdStatus57110RuntimeSourceResult.observation
                    .rejectReason),
            modeCmdStatus57110ReadPathFirstMissing,
            modeCmdStatus57110SourceReadAudit.loaderHeapReadAttempted ? 1 : 0,
            modeCmdStatus57110SourceReadAudit.loaderHeapReadable ? 1 : 0,
            modeCmdStatus57110SourceReadAudit.knownStateRelayReadAttempted
                ? 1
                : 0,
            modeCmdStatus57110SourceReadAudit.knownStateRelayReadable ? 1
                                                                      : 0,
            modeCmdStatus57110SourceReadAudit.cdMmioSnapshotReadAttempted ? 1
                                                                         : 0,
            modeCmdStatus57110SourceReadAudit.cdMmioSnapshotReadable ? 1 : 0,
            modeCmdStatus57110SourceReadAudit.exactCdReadAttempted ? 1 : 0,
            modeCmdStatus57110SourceReadAudit.exactCdReadable ? 1 : 0,
            modeCmdRawEventTypedAudit.priorDword80057110Known ? 1 : 0);
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower read-start direct producer 80035898 checkCallback source tail: modeCmd80035898CheckCallbackRuntimeSourceAdapter=1 modeCmd80035898CheckCallbackProviderInstalled=%d modeCmd80035898CheckCallbackReadAttempted=%d modeCmd80035898CheckCallbackWindowReadable=%d modeCmd80035898CheckCallbackSourceAvailable=%d modeCmd80035898CheckCallbackValueKnown=%d modeCmd80035898CheckCallbackWord55F7A=%u modeCmd80035898CheckCallbackPendingKnown=%d modeCmd80035898CheckCallbackPending=%d modeCmd80035898CheckCallbackReadPathFirstMissing=%s modeCmd80035898CheckCallbackLoaderHeapAttempted=%d modeCmd80035898CheckCallbackLoaderHeapReadable=%d modeCmd80035898CheckCallbackKnownStateRelayAttempted=%d modeCmd80035898CheckCallbackKnownStateRelayReadable=%d modeCmd80035898CheckCallbackCdMmioAttempted=%d modeCmd80035898CheckCallbackCdMmioReadable=%d modeCmd80035898CheckCallbackExactCdAttempted=%d modeCmd80035898CheckCallbackExactCdReadable=%d modeCmd80035898DirectStateWord55F7AKnown=%d modeCmd80035898DirectStateWord55F7A=%u modeCmd80035898DirectStatePendingKnown=%d modeCmd80035898DirectStatePending=%d modeCmd80035898DirectStateGapCount=%u modeCmd359B8DirectStateWriteCount=%u modeCmd359B8DirectStateGapCount=%u modeCmd80035898CheckCallbackFeedsCommand=%d modeCmd80035898CheckCallbackPublishesState=%d published=0",
            modeCmdCheckCallbackProviderInstalled ? 1 : 0,
            modeCmdCheckCallbackWindow.readAttempted ? 1 : 0,
            modeCmdCheckCallbackWindow.windowReadable ? 1 : 0,
            modeCmdCheckCallbackSourceAvailable ? 1 : 0,
            modeCmdCheckCallbackSource.valueKnown ? 1 : 0,
            modeCmdCheckCallbackSource.observation.value,
            modeCmdCheckCallback.pendingKnown ? 1 : 0,
            modeCmdCheckCallback.pending ? 1 : 0,
            modeCmdCheckCallbackReadPathFirstMissing,
            modeCmdCheckCallbackWord55F7AAudit.loaderHeapReadAttempted ? 1
                                                                       : 0,
            modeCmdCheckCallbackWord55F7AAudit.loaderHeapReadable ? 1 : 0,
            modeCmdCheckCallbackWord55F7AAudit.knownStateRelayReadAttempted
                ? 1
                : 0,
            modeCmdCheckCallbackWord55F7AAudit.knownStateRelayReadable ? 1
                                                                       : 0,
            modeCmdCheckCallbackWord55F7AAudit.cdMmioSnapshotReadAttempted ? 1
                                                                          : 0,
            modeCmdCheckCallbackWord55F7AAudit.cdMmioSnapshotReadable ? 1 : 0,
            modeCmdCheckCallbackWord55F7AAudit.exactCdReadAttempted ? 1 : 0,
            modeCmdCheckCallbackWord55F7AAudit.exactCdReadable ? 1 : 0,
            ctx.stage1XaCdDirect.word_80055F7AKnown ? 1 : 0,
            ctx.stage1XaCdDirect.word_80055F7A,
            ctx.stage1XaCdDirect.cdCallbackPending80035898Known ? 1 : 0,
            ctx.stage1XaCdDirect.cdCallbackPending80035898 ? 1 : 0,
            ctx.stage1XaCdDirect.cdCallbackPending80035898GapCount,
            ctx.stage1XaCdDirect.cdCallbackPending800359B8WriteCount,
            ctx.stage1XaCdDirect.cdCallbackPending800359B8GapCount,
            modeCmdCommand800375BCInput.checkCallbackKnown ? 1 : 0,
            modeCmdCheckCallbackSourceResult.publishAttempted &&
                    modeCmdCheckCallbackSourceResult.observation.accepted
                ? 1
                : 0);
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower read-start direct producer 800375BC timeout source tail: modeCmd800375BCTimeoutRuntimeSourceAdapter=1 modeCmd800375BCTimeoutProviderInstalled=%d modeCmd800375BCTimeoutState80088310Attempted=%d modeCmd800375BCTimeoutState80088310Readable=%d modeCmd800375BCTimeoutSpin80088314Attempted=%d modeCmd800375BCTimeoutSpin80088314Readable=%d modeCmd800375BCTimeoutExactCdSourceInstalled=%d modeCmd800375BCTimeoutExactCdReadFnInstalled=%d modeCmd800375BCTimeoutExactCdUserDataInstalled=%d modeCmd800375BCTimeoutSourceAvailable=%d modeCmd800375BCTimeoutReadPathFirstMissing=%s modeCmd800375BCTimeoutInitialModeledFromClock=%d modeCmd800375BCTimeoutSourceFeedsCommand=%d modeCmd800375BCTimeoutPsxReturn=%d modeCmd800375BCTimedOut=%d modeCmd800375BCTimeoutKnown=%d published=0",
            modeCmdTimeoutProviderInstalled ? 1 : 0,
            modeCmdTimeoutStateReadAttempted ? 1 : 0,
            modeCmdTimeoutStateReadable ? 1 : 0,
            modeCmdTimeoutSpinReadAttempted ? 1 : 0,
            modeCmdTimeoutSpinReadable ? 1 : 0,
            modeCmdTimeoutExactCdSourceInstalled ? 1 : 0,
            modeCmdTimeoutExactCdReadFnInstalled ? 1 : 0,
            modeCmdTimeoutExactCdUserDataInstalled ? 1 : 0,
            modeCmdTimeoutSourceAvailable ? 1 : 0,
            modeCmdTimeoutReadPathFirstMissing,
            modeCmdCommand800375BCInput.timeoutKnown &&
                    !modeCmdCommand800375BCInput.timedOut
                ? 1
                : 0,
            modeCmdWaitLoopRuntimeSourceResult.feedsCommand ? 1 : 0,
            modeCmdWaitLoopRuntimeSourceResult.psxReturn,
            modeCmdCommand800375BCInput.timedOut ? 1 : 0,
            modeCmdCommand800375BCInput.timeoutKnown ? 1 : 0);
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower read-start direct owner 800375BC commit: attempted=%d skippedSameRequest=%d committed=%d serial=%u command=%u arg0=%u called=%d incomplete=%d psxReturnKnown=%d psxReturn=%d checkCallbackKnown=%d checkCallbackPending=%d waitLoopResultKnown=%d waitLoopPsxReturn=%d hostBlockRelease=0 saveUi19148StartActive=0 published=0",
            modeCmdDirectOwnerCommitAttempted ? 1 : 0,
            modeCmdDirectOwnerCommitSkippedSameRequest ? 1 : 0,
            runtime.modeCmd800375BCDirectOwnerCommitted ? 1 : 0,
            runtime.modeCmd800375BCDirectOwnerReadS27Serial,
            static_cast<unsigned>(runtime.modeCmd800375BCDirectOwnerCommand),
            static_cast<unsigned>(runtime.modeCmd800375BCDirectOwnerArg0),
            modeCmdDirectOwner800375BC.called ? 1 : 0,
            modeCmdDirectOwner800375BC.incomplete ? 1 : 0,
            modeCmdDirectOwner800375BC.psxReturnKnown ? 1 : 0,
            modeCmdDirectOwner800375BC.psxReturn,
            modeCmdDirectOwner800375BC.checkCallbackResult.pendingKnown ? 1
                                                                        : 0,
            modeCmdDirectOwner800375BC.checkCallbackResult.pending ? 1 : 0,
            modeCmdDirectOwner800375BC.waitLoopResultKnown ? 1 : 0,
            modeCmdDirectOwner800375BC.waitLoopPsxReturn);
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower read-start facts gap: status=%u order=%u step=%u record=%u attempt=%u lower=%08X final=%08X source=request_bound_hal streamStarted=%d readS27Serial=%u readStartArg0=%d readStartArg1=%d readSync=%08X readSyncArg0=%d readSyncArg1=%d setupFacts=%d/%d readPumpFacts=%d/%d setupShape=%d requestDstKnown=%d requestSectorKnown=%d requestModeKnown=%d payloadBytesKnown=%d discUserDataProbe=%d/%d bin=%d fileBase=%d/%d lastSeek=%d/%d byteCount=%d/%u bytes=%zu published=0 needReadStartHalFacts=1",
            static_cast<unsigned>(request.status),
            request.psxOrder,
            static_cast<unsigned>(request.stepKind),
            static_cast<unsigned>(request.recordIndex),
            static_cast<unsigned>(request.attemptIndex),
            request.lowerRequest.lowerFunction,
            request.lowerRequest.finalFunction,
            ctx.stage1XaCdDirect.streamStarted ? 1 : 0,
            ctx.stage1XaCdDirect.readS27Serial,
            request.lowerRequest.readStartArg0,
            request.lowerRequest.readStartArg1,
            request.lowerRequest.readSyncFunction,
            request.lowerRequest.readSyncArg0,
            request.lowerRequest.readSyncArg1,
            facts.readStartSetup80038FC0.produced ? 1 : 0,
            facts.readStartSetup80038FC0.incomplete ? 1 : 0,
            facts.readPump80038DE8.produced ? 1 : 0,
            facts.readPump80038DE8.incomplete ? 1 : 0,
            setupPathShapeKnown ? 1 : 0,
            request.lowerRequest.readStartDstPtrKnown ? 1 : 0,
            request.lowerRequest.readStartSectorCountKnown ? 1 : 0,
            request.lowerRequest.readStartModeFlagKnown ? 1 : 0,
            facts.payloadBytesKnown ? 1 : 0,
            discProbe.attempted ? 1 : 0,
            discProbe.success ? 1 : 0,
            discProbe.binPathKnown ? 1 : 0,
            discProbe.fileBaseLbaKnown ? 1 : 0,
            discProbe.fileBaseLba,
            discProbe.lastSeekLbaKnown ? 1 : 0,
            discProbe.lastSeekLba,
            discProbe.byteCountKnown ? 1 : 0,
            discProbe.byteCount,
            discProbe.bytesRead);
        return false;
    }

    const bool finalReadyProgressShapeFromReadStart =
        PrStage1LifecycleExecutorDirect::
            IsFinalReadyProgressGapForPendingReadStart801C81EC(
                request,
                ctx.stage1XaCdDirect.readS27Serial);

    if (!finalReadyProgressShapeFromReadStart) {
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 CD lower live final-ready facts: skipped shape cd=%u readStartKnown=%d caller=%08X lower=%08X final=%08X readSyncKnown=%d readSync=%08X arg0=%d arg1=%d readStartProgress=%d/%u progressShape=0",
            static_cast<unsigned>(request.cdActionKind),
            request.lowerRequest.known ? 1 : 0,
            request.lowerRequest.callerFunction,
            request.lowerRequest.lowerFunction,
            request.lowerRequest.finalFunction,
            request.lowerRequest.readSyncRequestKnown ? 1 : 0,
            request.lowerRequest.readSyncFunction,
            request.lowerRequest.readSyncArg0,
            request.lowerRequest.readSyncArg1,
            request.readStartHalProgressAccepted ? 1 : 0,
            request.readStartHalProgressReadS27Serial);
        return false;
    }

    const bool finalReadyFactsRequirementRecorded =
        PrStage1LifecycleExecutorDirect::
            MarkBootstrap15590FinalReadyHalFactsRequired801C81EC(
                state.bootstrap15590CdLowerProducerRuntime,
                request,
                ctx.stage1XaCdDirect.readS27Serial);
    const PrStage1XaCdDirectStreamClockProbe800493F4 finalReadyClockProbe =
        PrStage1XaCdDirectProbeStreamClockProducer800493F4(
            ctx.stage1XaCdDirect);
    const bool finalReadyClockCarrierAccepted =
        finalReadyClockProbe.carrier.clockKnown &&
        finalReadyClockProbe.carrier.acceptedByte800493F4 &&
        !finalReadyClockProbe.gapMissingStreamClock800493F4Producer;
    PrStage1LowerCdProducerDirect::LowerCdProducerFacts finalReadyFacts{};
    const char* finalReadySourceFirstMissing = "none";
    const bool finalReadyFactsKnown =
        TryBuildBootstrap15590FinalReadyFacts801C81EC(
            ctx,
            state.bootstrap15590CdLowerProducerRuntime,
            request,
            finalReadyClockCarrierAccepted,
            finalReadyClockProbe.carrier,
            finalReadyFacts,
            finalReadySourceFirstMissing);
    PrStage1LifecycleExecutorDirect::
        Bootstrap15590CdLowerFactsApplyResult801C81EC finalReadyApply{};
    const bool finalReadyApplied =
        finalReadyFactsKnown &&
        PrStage1LifecycleExecutorDirect::RunBootstrap15590CdLowerFacts801C81EC(
            state,
            ctx.stage1XaCdDirect,
            finalReadyFacts,
            &finalReadyApply);
    Log::Printf(
        "Scene1 801C81EC bootstrap15590 CD lower final-ready direct producer ingress: source=read_start_carrier_xacd_state attempted=1 factsKnown=%d firstMissing=%s applied=%d requestMatched=%d xaCdAccepted=%d feedbackReady=%d status=%u reject=%u carrier=%d/%u currentReadS27Serial=%u clock=%d/%d readyBankD6=%d/%u readyBankD5=%d/%u callbackPending=%d/%d published=0",
        finalReadyFactsKnown ? 1 : 0,
        finalReadySourceFirstMissing,
        finalReadyApplied ? 1 : 0,
        finalReadyApply.requestMatched ? 1 : 0,
        finalReadyApply.xaCdAccepted ? 1 : 0,
        finalReadyApply.feedbackReady ? 1 : 0,
        static_cast<unsigned>(finalReadyApply.status),
        static_cast<unsigned>(finalReadyApply.rejectReason),
        state.bootstrap15590CdLowerProducerRuntime.readStartHalCarrierKnown ? 1
                                                                           : 0,
        state.bootstrap15590CdLowerProducerRuntime.
            readStartHalCarrierReadS27Serial,
        ctx.stage1XaCdDirect.readS27Serial,
        finalReadyClockProbe.carrier.clockKnown ? 1 : 0,
        finalReadyClockCarrierAccepted ? 1 : 0,
        ctx.stage1XaCdDirect.byte_800573D6Known ? 1 : 0,
        static_cast<unsigned>(ctx.stage1XaCdDirect.byte_800573D6),
        ctx.stage1XaCdDirect.byte_800573D5Known ? 1 : 0,
        static_cast<unsigned>(ctx.stage1XaCdDirect.byte_800573D5),
        ctx.stage1XaCdDirect.cdCallbackPending80035898Known ? 1 : 0,
        ctx.stage1XaCdDirect.cdCallbackPending80035898 ? 1 : 0);
    if (finalReadyApplied) {
        return true;
    }
    Log::Printf(
        "Scene1 801C81EC bootstrap15590 CD lower final-ready facts gap: status=%u order=%u step=%u record=%u attempt=%u lower=%08X final=%08X readSync=%08X arg0=%d arg1=%d readStartProgress=%d/%u currentReadS27Serial=%u progressShape=read_start_progress needFinalReadyHalFacts=1 finalReadyFactsRequirementRecorded=%d finalReadySourceFirstMissing=%s finalReadyDirectFactsKnown=%d",
        static_cast<unsigned>(request.status),
        request.psxOrder,
        static_cast<unsigned>(request.stepKind),
        static_cast<unsigned>(request.recordIndex),
        static_cast<unsigned>(request.attemptIndex),
        PrStage1LoaderCdHal::kFn800390C8,
        PrStage1LoaderCdHal::kFn800364F0,
        PrStage1LoaderCdHal::kFn800390C8,
        PrStage1LoaderCdHal::kRead8001A818SyncArg0,
        PrStage1LoaderCdHal::kRead8001A818SyncArg1,
        request.readStartHalProgressAccepted ? 1 : 0,
        request.readStartHalProgressReadS27Serial,
        ctx.stage1XaCdDirect.readS27Serial,
        finalReadyFactsRequirementRecorded ? 1 : 0,
        finalReadySourceFirstMissing,
        finalReadyFactsKnown ? 1 : 0);
    return false;
}

} // namespace

void ArmProbeOnlySaveUi19148FormatPoll4Feedback801C81EC() {
    s_saveUi19148CardEventHalState801C81EC.
        probeOnlyFormatPoll4Ready80016E18 = true;
    Log::Printf(
        "Scene1 801C81EC save-ui 19148 probe-only poll4 armed nonAuthority=1");
}

void ArmProbeOnlySaveUi19148FormatTerminalFeedback801C81EC(
    int32_t terminalPollResult80017008) {
    s_saveUi19148CardEventHalState801C81EC.
        probeOnlyFormatTerminalReady80017B60 = true;
    s_saveUi19148CardEventHalState801C81EC.
        probeOnlyFormatTerminalPollResult80017008 =
            terminalPollResult80017008;
    PrStage1SaveCardHalDirect::ClearSaveUiFormatTypedCarrier80017B60();
    Log::Printf(
        "Scene1 801C81EC save-ui 19148 probe-only format terminal armed nonAuthority=1 pollResult80017008=%d",
        terminalPollResult80017008);
}

static bool TryResolveStage1LifecycleActionRowPath(
    const PrGameContext& ctx,
    const PrStage1LifecycleDirect::Action801C81EC& action,
    std::filesystem::path& outPath) {
    outPath.clear();
    if (!action.sceneLoaderRecordKnown || !action.sceneLoaderSlotPresent) {
        return false;
    }

    const auto& record = action.sceneLoaderMovieSegmentRecord;
    const PrStage1MovieSegmentDirect::Stage1MovieSegmentIdentity801C4780
        identity =
            PrStage1MovieSegmentDirect::
                IdentifyStage1MovieSegmentRecord801C4780(record);
    if (!identity.known || identity.relativeWinPath == nullptr) {
        return false;
    }

    outPath = ctx.dataRoot / std::filesystem::path(identity.relativeWinPath);
    return true;
}

bool ApplyBootstrap15590CdLowerFacts801C81EC(
    PrGameContext& ctx,
    ActionHostRefs801C81EC& host,
    const PrStage1LowerCdProducerDirect::LowerCdProducerFacts& facts,
    PrStage1LifecycleExecutorDirect::
        Bootstrap15590CdLowerFactsApplyResult801C81EC* out) {
    return PrStage1LifecycleExecutorDirect::RunBootstrap15590CdLowerFacts801C81EC(
        host.executor,
        ctx.stage1XaCdDirect,
        facts,
        out);
}

static PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC
StartSaveUi19148Block(
    PrGameContext& ctx,
    PrStage1LifecycleExecutorDirect::State801C81EC& state,
    const PrStage1LifecycleDirect::Action801C81EC& action);

static void ApplyLifecycleStatusWrites801C81EC(
    PrGameContext& ctx,
    PrStage1LifecycleDirect::Runtime801C81EC& runtime,
    const PrStage1LifecycleExecutorDirect::LifecycleStatusWrites801C81EC&
        writes);

uint32_t ResolveMovie1HostFrame30(PrGameContext& ctx) {
    if (!ctx.strPlayer) {
        return 0u;
    }
    const double frame30 =
        std::floor(ctx.strPlayer->GetPlayedSecondsPrecise() * 30.0 + 1e-6);
    if (frame30 > 0.0) {
        return static_cast<uint32_t>(frame30);
    }
    const uint32_t decodedFrame = ctx.strPlayer->GetCurrentFrame();
    if (decodedFrame == 0u) {
        return 0u;
    }
    const float fps = ctx.strPlayer->GetFrameRate();
    if (fps > 0.1f) {
        const double decodedFrame30 =
            std::floor((static_cast<double>(decodedFrame) * 30.0 /
                        static_cast<double>(fps)) +
                       1e-6);
        return decodedFrame30 <= 0.0 ? 0u
                                     : static_cast<uint32_t>(decodedFrame30);
    }
    return decodedFrame;
}

static PrPsxPadDirect::PadReadResult80035510 ReadInputMaskSub80035510(
    PrGameContext& ctx) {
    const PrPadState padState = PrPad::GetState(0);
    const uint16_t returnedMask =
        PrPsxPadDirect::BuildReturnedMask80035510FromLocalAndDebugPad(
            padState.held,
            ctx.debugPadInput);
    return PrPsxPadDirect::PsxReadPadMask80035510(returnedMask);
}

void BindAbortPollEvent4FrameRequest80026B94(
    DirectAbortPollResult801C81EC& out,
    uint32_t logicFrame,
    PrSS0EventFrameLoopDirect::Event4FrameClass80026B94 frameClass,
    bool inputClosed,
    int32_t tailFramesRemainingBefore,
    int32_t tailFramesRemainingAfter) {
    out.frameBodyRequested = true;
    out.frameBegin.requestBound = true;
    out.frameBegin.ctxAddress =
        PrSS0EventFrameLoopDirect::kCtxEvent4_8006ED74;
    out.frameBegin.logicFrame = logicFrame;
    out.frameBegin.frameClass = frameClass;
    out.frameBegin.inputClosed = inputClosed;
    out.frameBegin.tailFramesRemainingBefore = tailFramesRemainingBefore;
    out.frameBegin.tailFramesRemainingAfter = tailFramesRemainingAfter;
}

DirectAbortPollResult801C81EC TickDirectAbortPollSub80026B94801C81EC(
    PrGameContext& ctx) {
    DirectAbortPollResult801C81EC out{};
    if (!s_abortPollEvent4Active8001E750 ||
        !s_abortPollEvent4Ss0ModalOwnerInitialized) {
        return out;
    }
    if (!PrSS0EventFrameLoopDirect::CanAdvanceEvent4FrameLogic80026B94(
            s_abortPollEvent4FrameTransaction80026B94)) {
        out.retryPendingFrame = true;
        return out;
    }
    if (s_abortPollEvent4Tail80026B94.active) {
        PrSS0EventFrameLoopDirect::DispatcherTailStep80026B94 tail{};
        if (!PrSS0EventFrameLoopDirect::TryStepEvent4DispatcherTail80026B94(
                s_abortPollEvent4FrameTransaction80026B94,
                s_abortPollEvent4Tail80026B94,
                tail)) {
            out.retryPendingFrame = true;
            return out;
        }
        if (tail.kind ==
            PrSS0EventFrameLoopDirect::
                DispatcherTailStepKind80026B94::ReleaseResult) {
            out.status = DirectAbortPollStatus801C81EC::Complete;
            out.result = tail.releasedResult;
        } else if (
            tail.kind ==
            PrSS0EventFrameLoopDirect::
                DispatcherTailStepKind80026B94::HoldFrame) {
            BindAbortPollEvent4FrameRequest80026B94(
                out,
                ctx.frame,
                PrSS0EventFrameLoopDirect::
                    Event4FrameClass80026B94::ClosedInputTail,
                true,
                tail.framesRemainingBefore,
                tail.framesRemainingAfter);
        }
        return out;
    }

    const PrPsxPadDirect::PadReadResult80035510 input =
        ReadInputMaskSub80035510(ctx);
    const PrSS0EventFrameLoopDirect::
        DispatcherPreLoopPadReleaseStep80026B94 release =
            PrSS0EventFrameLoopDirect::
                StepDispatcherPreLoopPadRelease80026B94(
                    s_abortPollEvent4ReleaseGate80026B94,
                    input.psxReturnMask);
    if (release.kind ==
        PrSS0EventFrameLoopDirect::
            DispatcherPreLoopPadReleaseStepKind80026B94::WaitingForRelease) {
        return out;
    }
    if (release.kind ==
        PrSS0EventFrameLoopDirect::
            DispatcherPreLoopPadReleaseStepKind80026B94::ReleasedEnterLoop) {
        BindAbortPollEvent4FrameRequest80026B94(
            out,
            ctx.frame,
            PrSS0EventFrameLoopDirect::
                Event4FrameClass80026B94::OpenInputLoop,
            false,
            -1,
            -1);
        return out;
    }

    uint32_t changedPadMask80026744 = 0u;
    if (ctx.debugEsc_Ev4Exit) {
        // Debug Esc is a host-only request.  Once the formal release gate is
        // open, route it through the exact Circle input owner instead of
        // bypassing the SS0 transaction or publishing an immediate result.
        changedPadMask80026744 = kPadCircle;
    } else {
        const PrSS0EventFrameLoopDirect::DispatcherPadChangeStep80026744 pad =
            PrSS0EventFrameLoopDirect::StepDispatcherPadChange80026744(
                s_abortPollEvent4PadChange80026744,
                input.psxReturnMask);
        if (!pad.changed) {
            BindAbortPollEvent4FrameRequest80026B94(
                out,
                ctx.frame,
                PrSS0EventFrameLoopDirect::
                    Event4FrameClass80026B94::OpenInputLoop,
                false,
                -1,
                -1);
            return out;
        }
        changedPadMask80026744 = pad.changedPadMask80026744;
    }

    const PrSS0EventFrameLoopDirect::Event4ModalInputSinks80025F0C sinks{
        PlayAbortPollEvent4InputCue80025C8C,
        nullptr};
    s_abortPollEvent4LastInput80025F0C =
        PrSS0EventFrameLoopDirect::ExecuteEvent4ModalInputSoftware80025F0C(
            changedPadMask80026744,
            sinks);
    if (!s_abortPollEvent4LastInput80025F0C.accepted) {
        return out;
    }
    if (s_abortPollEvent4LastInput80025F0C.outputWrite) {
        s_abortPollEvent4SelectionState8001E750 =
            s_abortPollEvent4LastInput80025F0C.outputValue;
    }
    if (s_abortPollEvent4LastInput80025F0C.result != 0) {
        if (!PrSS0EventFrameLoopDirect::ArmDispatcherTail80026B94(
                s_abortPollEvent4Tail80026B94,
                s_abortPollEvent4LastInput80025F0C.result)) {
            return out;
        }
        BindAbortPollEvent4FrameRequest80026B94(
            out,
            ctx.frame,
            PrSS0EventFrameLoopDirect::
                Event4FrameClass80026B94::ResultAction,
            false,
            PrSS0EventFrameLoopDirect::
                kDispatcherResultTailFrames80026B94,
            PrSS0EventFrameLoopDirect::
                kDispatcherResultTailFrames80026B94);
        return out;
    }
    BindAbortPollEvent4FrameRequest80026B94(
        out,
        ctx.frame,
        PrSS0EventFrameLoopDirect::Event4FrameClass80026B94::OpenInputLoop,
        false,
        -1,
        -1);
    return out;
}

PrStage1Scene1Movie1Direct::Movie1HostFeedback BuildMovie1HostFeedback(
    PrGameContext& ctx,
    bool lifecyclePathResolved,
    const std::filesystem::path& lifecyclePath,
    bool freezeSubbox) {
    PrStage1Scene1Movie1Direct::Movie1HostFeedback host{};
    host.strPlayerReady = ctx.strPlayer != nullptr;
    host.movie1StrExists =
        lifecyclePathResolved && std::filesystem::exists(lifecyclePath);
    host.debugStage1DirectBootRequested = ctx.debugStage1DirectBoot;
    host.debugF1StrSkipRequested = ctx.debugF1_StrSkip;
    host.subtitleEnabled = ctx.subtitleFlag != 0;
    host.freezeSubbox = freezeSubbox;
    host.languageIndex = static_cast<uint8_t>(ctx.languageIndex);
    host.movieFrame30 = ResolveMovie1HostFrame30(ctx);
    host.lastStrUpdateResult = StrPlayerResult::Playing;
    host.strVideoFinished =
        ctx.strPlayer != nullptr && ctx.strPlayer->IsVideoFinished();
    const PrPsxPadDirect::PadReadResult80035510 inputMask =
        ReadInputMaskSub80035510(ctx);
    host.inputMaskSub80035510Known =
        inputMask.called && inputMask.padDrCalled;
    host.inputMaskSub80035510 = inputMask.psxReturnMask;
    return host;
}

void AdvanceMovie1HostStrPlayer(
    PrGameContext& ctx,
    PrStage1Scene1Movie1Direct::Movie1RuntimeState& runtime,
    PrStage1Scene1Movie1Direct::Movie1HostFeedback& host) {
    host.lastStrUpdateResult = StrPlayerResult::Playing;
    const PrStage1Scene1Movie1Direct::Movie1HostStrPollPlan pollPlan =
        PrStage1Scene1Movie1Direct::BuildHostStrPollPlan(runtime, host);
    if (pollPlan.shouldUpdateStr && ctx.strPlayer) {
        host.lastStrUpdateResult = ctx.strPlayer->Update(pollPlan.skipAllowed);
        if (host.lastStrUpdateResult == StrPlayerResult::Skipped) {
            // 801C455C cleanup: 8001A4A4(1) mutes XA; 8001A694 stops CD.
            // Keep the last decoded frame for the following native mode-5.
            ctx.strPlayer->FinishPlaybackKeepFrame();
            Log::Printf("Scene1 movie 801C455C completion: native=%d debug=%d frame=%u audio=%d",
                host.nativePlayAndWaitComplete801C455C ? 1 : 0,
                host.debugF1StrSkipRequested ? 1 : 0,
                ctx.strPlayer->GetCurrentFrame(), ctx.strPlayer->HasAudio() ? 1 : 0);
        }
        host.movieFrame30 = ResolveMovie1HostFrame30(ctx);
    } else if (ctx.strPlayer) {
        switch (ctx.strPlayer->GetState()) {
        case StrPlayerState::Finished:
            host.lastStrUpdateResult = StrPlayerResult::Finished;
            break;
        case StrPlayerState::Skipped:
            host.lastStrUpdateResult = StrPlayerResult::Skipped;
            break;
        case StrPlayerState::Error:
            host.lastStrUpdateResult = StrPlayerResult::Error;
            break;
        case StrPlayerState::Idle:
        case StrPlayerState::Loading:
        case StrPlayerState::Playing:
        case StrPlayerState::Paused:
            break;
        }
    }
    host.strVideoFinished =
        ctx.strPlayer != nullptr && ctx.strPlayer->IsVideoFinished();
}

PrStage1Scene1Movie1Direct::Movie1HostActionFeedback
ExecuteMovie1HostActions(
    PrGameContext& ctx,
    bool lifecyclePathResolved,
    const std::filesystem::path& lifecyclePath,
    const PrStage1Scene1Movie1Direct::Movie1HostActionList& actions) {
    using ActionKind = PrStage1Scene1Movie1Direct::Movie1HostActionKind;
    PrStage1Scene1Movie1Direct::Movie1HostActionFeedback feedback{};
    for (uint32_t i = 0; i < actions.count; ++i) {
        const PrStage1Scene1Movie1Direct::Movie1HostAction& action =
            actions.actions[i];
        switch (action.kind) {
        case ActionKind::None:
            break;
        case ActionKind::StopStr:
            if (ctx.strPlayer) {
                ctx.strPlayer->Stop();
            }
            break;
        case ActionKind::PlayMovie1Str:
            feedback.kind =
                PrStage1Scene1Movie1Direct::Movie1HostActionFeedbackKind::
                    PlayMovie1Str;
            feedback.actionIndex = i;
            feedback.playAttempted = true;
            feedback.playSucceeded =
                ctx.strPlayer &&
                lifecyclePathResolved &&
                ctx.strPlayer->Play(lifecyclePath);
            return feedback;
        case ActionKind::PauseStr:
            if (ctx.strPlayer) {
                ctx.strPlayer->Pause();
            }
            break;
        case ActionKind::PlayMovie1Cue9441C:
            PrSfx::PlayMovie1ShellCue9441C(action.cue9441CIndex);
            break;
        case ActionKind::ApplyMovieTransitionCueCadence80027194:
            (void)PrSfx::ApplyMovieTransitionCueCadence80027194();
            break;
        case ActionKind::LogDebugDirectBootSkip:
            ctx.debugStage1DirectBoot = false;
            Log::Printf(
                "Scene1::Fn2 debug direct boot -> skip MOVIE1 and enter Stage1 loop");
            break;
        }
    }
    return feedback;
}

static PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC
TickTransition20110801C81EC(
    PrGameContext& ctx,
    ActionHostRefs801C81EC& host,
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    if (!IsTransition20110Action801C81EC(action) ||
        host.movieRuntime == nullptr) {
        return PrStage1LifecycleExecutorDirect::
            MakeBlockedActionRetryResult801C81EC();
    }

    const bool clearTailPreludeMode2 =
        action.transitionModeA2 == 2 &&
        action.transitionPreFfd4ArgA3 == 1 &&
        action.transitionPostFfd4ArgA4 == 2;

    if (PrStage1Scene1Movie1Direct::
            ConsumeTransitionSub800201ACCompleted(*host.movieRuntime)) {
        if (clearTailPreludeMode2) {
            PrStage1LifecycleExecutorDirect::
                SetClearTailMovieVisualActive801C81EC(host.executor, true);
        }
        return PrStage1LifecycleExecutorDirect::
            MakeImmediateInputResult801C81EC();
    }

    const PrStage1Scene1Movie1Direct::Movie1DrawableStateQueryResult drawable =
        PrStage1Scene1Movie1Direct::QueryDrawableState(*host.movieRuntime);
    if (!drawable.outroTailActive) {
        uint32_t sourceFrame30 = host.movieRuntime->currentMovieFrame30;
        if (sourceFrame30 == 0u) {
            sourceFrame30 = host.movieRuntime->outroSourceFrame30;
        }
        PrStage1Scene1Movie1Direct::BeginTransitionSub80020110(
            *host.movieRuntime,
            action.transitionA1_801C3640,
            static_cast<uint32_t>(action.transitionModeA2),
            static_cast<uint32_t>(action.transitionPreFfd4ArgA3),
            static_cast<uint32_t>(action.transitionPostFfd4ArgA4),
            sourceFrame30,
            false);
        if (clearTailPreludeMode2) {
            PrStage1LifecycleExecutorDirect::
                SetClearTailMovieVisualActive801C81EC(host.executor, true);
        }
    }

    const uint32_t drainGuard = clearTailPreludeMode2 ? 256u : 1u;
    for (uint32_t guard = 0; guard < drainGuard; ++guard) {
        PrStage1Scene1Movie1Direct::Movie1HostFeedback movieHost =
            BuildMovie1HostFeedback(ctx, false, std::filesystem::path{}, false);
        const PrStage1Scene1Movie1Direct::Movie1AdvanceResult rawMovieResult =
            PrStage1Scene1Movie1Direct::AdvanceRuntimePure(
                *host.movieRuntime,
                movieHost,
                PrStage1MovieTextDirect::GetActiveMovieSubtitleTrack(host.movieText));
        const PrStage1Scene1Movie1Direct::Movie1HostActionFeedback feedback =
            ExecuteMovie1HostActions(
                ctx,
                false,
                std::filesystem::path{},
                rawMovieResult.hostActions);
        const PrStage1Scene1Movie1Direct::Movie1HostActionFeedbackResolution
            feedbackResolution =
                PrStage1Scene1Movie1Direct::ApplyHostActionFeedback(
                    *host.movieRuntime,
                    rawMovieResult,
                    feedback);
        (void)ExecuteMovie1HostActions(
            ctx,
            false,
            std::filesystem::path{},
            feedbackResolution.followupHostActions);

        if (PrStage1Scene1Movie1Direct::
                ConsumeTransitionSub800201ACCompleted(*host.movieRuntime)) {
            if (clearTailPreludeMode2) {
                PrStage1LifecycleExecutorDirect::
                    SetClearTailMovieVisualActive801C81EC(host.executor, true);
            }
            return PrStage1LifecycleExecutorDirect::
                MakeImmediateInputResult801C81EC();
        }
        if (rawMovieResult.transitionFrameReadyForPresent ||
            !rawMovieResult.handledFrame) {
            break;
        }
    }

    return PrStage1LifecycleExecutorDirect::
        MakeBlockedActionRetryResult801C81EC();
}

static PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC
TickClearTailMovieTransition201AC801C81EC(
    PrGameContext& ctx,
    ActionHostRefs801C81EC& host,
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    if (!IsClearTailMovieTransition201ACAction801C81EC(action) ||
        host.movieRuntime == nullptr) {
        return PrStage1LifecycleExecutorDirect::
            MakeBlockedActionRetryResult801C81EC();
    }

    const bool preMovieTransition =
        action.transitionModeA2 == 6 &&
        action.transitionPreFfd4ArgA3 == 2 &&
        action.transitionPostFfd4ArgA4 == 1;
    const bool postMovieTransition =
        action.transitionModeA2 == 5 &&
        action.transitionPreFfd4ArgA3 == 1 &&
        action.transitionPostFfd4ArgA4 == 2;

    if (!preMovieTransition && !postMovieTransition) {
        return PrStage1LifecycleExecutorDirect::
            MakeBlockedActionRetryResult801C81EC();
    }

    if (PrStage1Scene1Movie1Direct::
            ConsumeTransitionSub800201ACCompleted(*host.movieRuntime)) {
        if (preMovieTransition) {
            PrStage1LifecycleExecutorDirect::
                SetClearTailMovieVisualActive801C81EC(host.executor, true);
        } else {
            if (ctx.strPlayer) {
                ctx.strPlayer->Stop();
            }
            PrStage1LifecycleExecutorDirect::
                ClearClearTailMovieVisualActive801C81EC(host.executor);
        }
        return PrStage1LifecycleExecutorDirect::
            MakeImmediateInputResult801C81EC();
    }

    PrStage1Scene1Movie1Direct::Movie1HostFeedback movieHost =
        BuildMovie1HostFeedback(ctx, false, std::filesystem::path{}, false);
    const PrStage1Scene1Movie1Direct::Movie1DrawableStateQueryResult drawable =
        PrStage1Scene1Movie1Direct::QueryDrawableState(*host.movieRuntime);
    if (!drawable.outroTailActive) {
        uint32_t sourceFrame30 = host.movieRuntime->currentMovieFrame30;
        if (sourceFrame30 == 0u) {
            sourceFrame30 = host.movieRuntime->outroSourceFrame30;
        }
        if (sourceFrame30 == 0u) {
            sourceFrame30 = movieHost.movieFrame30;
        }
        PrStage1Scene1Movie1Direct::BeginTransitionSub800201AC(
            *host.movieRuntime,
            action.transitionA1_801C3640,
            static_cast<uint32_t>(action.transitionModeA2),
            static_cast<uint32_t>(action.transitionPreFfd4ArgA3),
            static_cast<uint32_t>(action.transitionPostFfd4ArgA4),
            sourceFrame30,
            false);
        PrStage1LifecycleExecutorDirect::SetClearTailMovieVisualActive801C81EC(
            host.executor,
            true);
    }

    constexpr uint32_t kClearTailTransition201ACDrainGuard = 256u;
    for (uint32_t guard = 0; guard < kClearTailTransition201ACDrainGuard;
         ++guard) {
        const PrStage1Scene1Movie1Direct::Movie1AdvanceResult rawMovieResult =
            PrStage1Scene1Movie1Direct::AdvanceRuntimePure(
                *host.movieRuntime,
                movieHost,
                PrStage1MovieTextDirect::GetActiveMovieSubtitleTrack(host.movieText));
        const PrStage1Scene1Movie1Direct::Movie1HostActionFeedback feedback =
            ExecuteMovie1HostActions(
                ctx,
                false,
                std::filesystem::path{},
                rawMovieResult.hostActions);
        const PrStage1Scene1Movie1Direct::Movie1HostActionFeedbackResolution
            feedbackResolution =
                PrStage1Scene1Movie1Direct::ApplyHostActionFeedback(
                    *host.movieRuntime,
                    rawMovieResult,
                    feedback);
        (void)ExecuteMovie1HostActions(
            ctx,
            false,
            std::filesystem::path{},
            feedbackResolution.followupHostActions);

        if (PrStage1Scene1Movie1Direct::
                ConsumeTransitionSub800201ACCompleted(*host.movieRuntime)) {
            if (preMovieTransition) {
                PrStage1LifecycleExecutorDirect::
                    SetClearTailMovieVisualActive801C81EC(host.executor, true);
            } else {
                if (ctx.strPlayer) {
                    ctx.strPlayer->Stop();
                }
                PrStage1LifecycleExecutorDirect::
                    ClearClearTailMovieVisualActive801C81EC(host.executor);
            }
            return PrStage1LifecycleExecutorDirect::
                MakeImmediateInputResult801C81EC();
        }
        if (rawMovieResult.transitionFrameReadyForPresent ||
            !rawMovieResult.handledFrame) {
            break;
        }
    }

    return PrStage1LifecycleExecutorDirect::
        MakeBlockedActionRetryResult801C81EC();
}

PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC ApplyAction(
    PrGameContext& ctx,
    const PrStage1LifecycleDirect::Action801C81EC& action,
    const PrStage1Scene1FrameDriverDirect::LoopFrameWindow& window,
    ActionHostRefs801C81EC& host) {
    using ActionKind = PrStage1LifecycleDirect::ActionKind801C81EC;
    using HostBlockKind =
        PrStage1LifecycleExecutorDirect::HostBlockKind801C81EC;
    PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC out{};
    const HostBlockKind hostBlockKind =
        PrStage1LifecycleExecutorDirect::GetActionHostBlockKind801C81EC(
            action);
    switch (action.kind) {
    case ActionKind::None:
        break;
    case ActionKind::ConfigMovieViewport:
        if (!IsConfigMovieViewportAction801C81EC(action)) {
            out = PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
        }
        break;
    case ActionKind::AudioReset26FA4:
        if (!IsExactLifecycleFunction801C81EC(action, kFn80026FA4)) {
            out = PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
            break;
        }
        PrSfx::ApplySharedAudioResetBarrier26FA4();
        break;
    case ActionKind::StageRecordTick1A4D0:
    {
        std::filesystem::path stageRuntimePath;
        if (!IsStageRecordTickAction801C81EC(action) ||
            host.stageRecordTick == nullptr ||
            !TryResolveStage1LifecycleActionRowPath(
                ctx,
                action,
                stageRuntimePath) ||
            !host.stageRecordTick(ctx, stageRuntimePath, window)) {
            out = PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
        }
        break;
    }
    case ActionKind::Transition201AC:
    case ActionKind::Transition20110:
    {
        if (action.kind == ActionKind::Transition201AC &&
            action.strBlockKind ==
                PrStage1LifecycleDirect::StrBlockKind801C81EC::InitialMovie1) {
            if (!IsInitialMovie1Transition201ACAction801C81EC(action)) {
                out = PrStage1LifecycleExecutorDirect::
                    MakeBlockedActionRetryResult801C81EC();
                break;
            }
            out = host.tickInitialMovie1Transition201AC != nullptr
                ? host.tickInitialMovie1Transition201AC(ctx, action)
                : PrStage1LifecycleExecutorDirect::
                      MakeBlockedActionRetryResult801C81EC();
            break;
        }
        if (action.kind == ActionKind::Transition201AC &&
            action.strBlockKind ==
                PrStage1LifecycleDirect::StrBlockKind801C81EC::
                    ClearTailMovie) {
            out = TickClearTailMovieTransition201AC801C81EC(ctx, host, action);
            break;
        }
        if (action.kind == ActionKind::Transition20110) {
            out = TickTransition20110801C81EC(ctx, host, action);
            break;
        }
        PrStage1LifecycleExecutorDirect::TransitionStartInput801C81EC input{};
        input.transitionActive = PrTransition::IsActive();
        input.action = action;
        const PrStage1LifecycleExecutorDirect::TransitionStartPlan801C81EC
            plan =
                PrStage1LifecycleExecutorDirect::BuildTransitionStartPlan801C81EC(
                    input);
        if (plan.activeAlready || !plan.validPayload || !plan.shouldStart) {
            out = PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
            break;
        }
        if (plan.shouldStart) {
            using TransitionPreset =
                PrStage1LifecycleExecutorDirect::TransitionStartPreset801C81EC;
            bool transitionStarted = false;
            switch (plan.preset) {
            case TransitionPreset::StageLoopEntry20110Mode1:
            case TransitionPreset::ClearTailPrelude20110Mode2:
                break;
            case TransitionPreset::ClearTailResultMovieMode6:
            case TransitionPreset::ClearTailResultMovieMode5:
            case TransitionPreset::None:
                break;
            }
            if (!transitionStarted) {
                out = PrStage1LifecycleExecutorDirect::
                    MakeBlockedActionRetryResult801C81EC();
            }
        }
        break;
    }
    case ActionKind::StrInit:
        if (hostBlockKind == HostBlockKind::ClearTailMovie) {
            if (!IsClearTailMovieStrInitAction801C81EC(action)) {
                out = PrStage1LifecycleExecutorDirect::
                    MakeBlockedActionRetryResult801C81EC();
                break;
            }
            std::filesystem::path clearTailMoviePath;
            const bool pathResolved =
                TryResolveStage1LifecycleActionRowPath(
                    ctx,
                    action,
                    clearTailMoviePath);
            if (!pathResolved) {
                out = PrStage1LifecycleExecutorDirect::
                    MakeBlockedActionRetryResult801C81EC();
                break;
            }
            PrStage1MovieTextDirect::ApplyPrStrPlayerInitSub801C7744(
                host.movieText,
                action.rawArg1);
            out = PrStage1LifecycleExecutorDirect::BeginHostBlock801C81EC(
                host.executor,
                PrStage1LifecycleExecutorDirect::
                    BuildClearTailMovieHostBlockStart801C81EC(
                        pathResolved,
                        clearTailMoviePath));
            if (ctx.strPlayer) {
                ctx.strPlayer->Stop();
            }
            out.waitingForHostBlock = false;
        } else {
            if (!IsInitialMovie1StrInitAction801C81EC(action)) {
                out = PrStage1LifecycleExecutorDirect::
                    MakeBlockedActionRetryResult801C81EC();
                break;
            }
            PrStage1MovieTextDirect::ApplyPrStrPlayerInitSub801C7744(
                host.movieText,
                action.rawArg1);
        }
        break;
    case ActionKind::StrPlayAndWait:
        if (hostBlockKind == HostBlockKind::Movie1) {
            if (!IsInitialMovie1StrPlayAction801C81EC(action)) {
                out = PrStage1LifecycleExecutorDirect::
                    MakeBlockedActionRetryResult801C81EC();
                break;
            }
            std::filesystem::path movie1Path;
            const bool movie1PathResolved =
                TryResolveStage1LifecycleActionRowPath(
                    ctx,
                    action,
                    movie1Path);
            if (!movie1PathResolved || movie1Path.empty()) {
                out = PrStage1LifecycleExecutorDirect::
                    MakeBlockedActionRetryResult801C81EC();
                break;
            }
            PrStage1MovieTextOuterLoopDirect::BeginMovieTextOuterLoopSub801C455C(
                host.movieTextOuterLoop,
                host.movieText,
                static_cast<uint8_t>(action.arg0),
                host.commonLyricsLanguageIndex);
            PrStage1LifecycleExecutorDirect::MergeActionApplyResult801C81EC(
                out,
                PrStage1LifecycleExecutorDirect::BeginHostBlock801C81EC(
                    host.executor,
                    PrStage1LifecycleExecutorDirect::
                        BuildMovie1HostBlockStart801C81EC(
                            movie1PathResolved,
                            movie1Path)));
            if (host.tickMovie1Block != nullptr) {
                PrStage1LifecycleExecutorDirect::MergeActionApplyResult801C81EC(
                    out,
                    host.tickMovie1Block(ctx));
            }
        } else if (hostBlockKind == HostBlockKind::ClearTailMovie) {
            if (!IsClearTailMovieStrPlayAction801C81EC(action)) {
                out = PrStage1LifecycleExecutorDirect::
                    MakeBlockedActionRetryResult801C81EC();
                break;
            }
            if (host.movieRuntime != nullptr) {
                PrStage1Scene1Movie1Direct::ResetRuntime(*host.movieRuntime);
            }
            PrStage1MovieTextOuterLoopDirect::BeginMovieTextOuterLoopSub801C455C(
                host.movieTextOuterLoop,
                host.movieText,
                static_cast<uint8_t>(action.arg0),
                host.commonLyricsLanguageIndex);
            PrStage1LifecycleExecutorDirect::MergeActionApplyResult801C81EC(
                out,
                TickClearTailMovieBlock(
                    ctx,
                    host.executor,
                    host.movieTextOuterLoop,
                    host.movieText,
                    host.movieRuntime));
        }
        break;
    case ActionKind::StageRunnerRun7A60: {
        if (!IsStageRunnerRunAction801C81EC(action)) {
            out = PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
            break;
        }
        const StageRunnerHostResult801C81EC runnerResult =
            host.runStageRunner != nullptr ? host.runStageRunner(ctx, window)
                                           : StageRunnerHostResult801C81EC{};
        if (runnerResult.known) {
            if (runnerResult.result != 0) {
                // sub_801C7A60 has returned; sub_801C81EC now runs event4
                // outside the gameplay loop.
                ctx.stageRunning = false;
                PrStage1RuntimeSlotsDirectApplyStageLoopExit801C7A60(
                    host.runtimeSlots);
                GetStageRunner().Reset();
            }
            PrStage1LifecycleExecutorDirect::SetStageResult801C7A60(
                host.executor,
                runnerResult.result);
            out =
                PrStage1LifecycleExecutorDirect::MakeImmediateInputResult801C81EC();
        } else {
            out = PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
        }
        break;
    }
    case ActionKind::QueryStageStatus166AC:
    {
        const PrStageStatusBankDirectCallRequest request =
            PrStage1LifecycleExecutorDirect::
                BuildStatusBankDirectMemoryRequest801C81EC(action);
        const PrStage1LifecycleExecutorDirect::
            StatusBankDirectMemoryFeedback801C81EC feedback =
                PrStageStatusBankHostBridgeDirect::
                    ExecuteStatusBankDirectMemoryRequest801C81EC(request);
        out = PrStage1LifecycleExecutorDirect::
            ApplyStatusBankDirectMemoryFeedback801C81EC(
                host.executor,
                action,
                feedback);
        const PrStage1LifecycleExecutorDirect::
            StatusBankDirectMemoryGap801C81EC gap =
                PrStage1LifecycleExecutorDirect::
                    BuildStatusBankDirectMemoryGap801C81EC(
                        action,
                        feedback);
        if (gap.shouldLog) {
            static bool loggedStatus166ACGap = false;
            if (!loggedStatus166ACGap) {
                loggedStatus166ACGap = true;
                Log::Printf(
                    "Scene1 801C81EC QueryStageStatus166AC gap: scene=%d request=%d fn=%08X fnMatched=%d mapped=%d slot=%d statusBankKnown=%d helperGap=%d",
                    gap.sceneId,
                    gap.requestValid ? 1 : 0,
                    gap.psxFunction,
                    gap.psxFunctionMatched ? 1 : 0,
                    gap.status166ACMapped ? 1 : 0,
                    gap.status166ACSlotIndex,
                    gap.status166ACStatusBankKnown ? 1 : 0,
                    gap.helperGap ? 1 : 0);
            }
        }
        if (!feedback.requestHandled || !feedback.status166ACKnown) {
            out = PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
        }
        break;
    }
    case ActionKind::ResetHoldTiles1EF14:
        if (!IsExactLifecycleFunction801C81EC(action, kFn8001EF14)) {
            out = PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
            break;
        }
        PrTransition::ResetHoldOverlayState1EF14();
        PrSS0TransitionDirect::ResetLoadingPatternState8001EF14(
            host.executor.bootstrap15590Loading.pattern);
        break;
    case ActionKind::RestoreTransitionPayload15744:
    {
        if (!IsExactLifecycleFunction801C81EC(action, kFn80015744) ||
            action.rawArg0 != kSavePayloadBank80092F10) {
            out = PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
            break;
        }
        const PrStage1SaveStatusBackupResult80015700 restore =
            PrStage1SaveUiDirect::Sub80015744(action.rawArg0);
        if (!restore.ok) {
            static bool loggedRestore15744Gap = false;
            if (!loggedRestore15744Gap) {
                loggedRestore15744Gap = true;
                Log::Printf(
                    "Scene1 801C81EC RestoreTransitionPayload15744 direct gap: restoreKnown=%d backupKnown=%d backupStatusKnown=%d fault=%08X",
                    restore.restoreKnown ? 1 : 0,
                    restore.backupKnown ? 1 : 0,
                    restore.backupStatusBankKnown80092F1D ? 1 : 0,
                    restore.lastFaultAddress);
            }
            out = PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
        }
        break;
    }
    case ActionKind::QueryAbort26B94:
        if (!IsQueryAbort26B94Action801C81EC(action)) {
            out = PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
            break;
        }
        PrStage1LifecycleExecutorDirect::MergeActionApplyResult801C81EC(
            out,
            PrStage1LifecycleExecutorDirect::BeginHostBlock801C81EC(
                host.executor,
                PrStage1LifecycleExecutorDirect::
                    BuildAbortPollHostBlockStart801C81EC()));
        PrStage1LifecycleExecutorDirect::MergeActionApplyResult801C81EC(
            out,
            TickAbortPollBlock(ctx, host.executor));
        break;
    case ActionKind::SfxCue26EF8:
        if (!IsExactLifecycleFunction801C81EC(action, kFn80026EF8) ||
            action.rawArg0 != kCue80094410) {
            out = PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
            break;
        }
        PrSfx::PlaySceneTransitionCue94410();
        break;
    case ActionKind::AudioFlush26ECC:
        if (!IsExactLifecycleFunction801C81EC(action, kFn80026ECC)) {
            out = PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
            break;
        }
        PrSfx::ApplySharedAudioDriverFlushBarrier26ECC();
        break;
    case ActionKind::SaveStatus1635C:
    {
        const PrStage1LifecycleExecutorDirect::
            SaveStatus1635CReplayBackupHostRequest801C81EC replayRequest =
                PrStage1LifecycleExecutorDirect::
                    BuildSaveStatus1635CReplayBackupHostRequest801C81EC(
                        action);
        PrStage1LifecycleExecutorDirect::
            SaveStatus1635CReplayBackupHostFeedback801C81EC replayFeedback{};
        replayFeedback.requestValid = replayRequest.valid;
        replayFeedback.psxFunctionKnown = replayRequest.psxFunctionKnown;
        replayFeedback.psxFunction = replayRequest.psxFunction;
        replayFeedback.psxFunctionMatched =
            replayRequest.psxFunctionKnown &&
            replayRequest.psxFunction == kFn8001635C;
        replayFeedback.prevGrade92F40Known =
            replayRequest.prevGrade92F40Known;
        replayFeedback.hostCallbackKnown = host.captureReplayBackup != nullptr;
        replayFeedback.replayMirrorKnown8008EEF8 =
            host.numericRuntime.acceptedProducerReplayBuffer
                .replayMirrorKnown8008EEF8;
        replayFeedback.replayMirrorProducerKnown8008EEF8 =
            host.numericRuntime.acceptedProducerReplayBuffer
                .replayMirrorProducerKnown8008EEF8;
        replayFeedback.replayMirrorProducerFunction =
            host.numericRuntime.acceptedProducerReplayBuffer
                .replayMirrorProducerFunction;
        replayFeedback.replayMirrorByteCountKnown8008EEF8 =
            host.numericRuntime.acceptedProducerReplayBuffer
                .replayMirrorByteCountKnown8008EEF8;
        replayFeedback.replayMirrorKnownByteCount8008EEF8 =
            host.numericRuntime.acceptedProducerReplayBuffer
                .replayMirrorKnownByteCount8008EEF8;
        replayFeedback.replayMirrorFullBackingKnown8008EEF8 =
            host.numericRuntime.acceptedProducerReplayBuffer
                .replayMirrorFullBackingKnown8008EEF8;
        replayFeedback.replayPublishedCount901BC =
            host.numericRuntime.acceptedProducerReplayBuffer
                .publishedCount901BC;
        replayFeedback.replayWriteCount901C0 =
            host.numericRuntime.acceptedProducerReplayBuffer.writeCount901C0;
        replayFeedback.saveStatusQueryFrame =
            static_cast<int32_t>(host.numericRuntime.queryFrame);
        replayFeedback.replayLastAppendKnown =
            host.numericRuntime.rightRankLastReplayAppendKnown;
        replayFeedback.replayLastAppendQueryFrame =
            host.numericRuntime.rightRankLastReplayAppendQueryFrame;
        replayFeedback.replayLastAppendDeltaToSaveStatus =
            replayFeedback.replayLastAppendKnown
                ? replayFeedback.saveStatusQueryFrame -
                      replayFeedback.replayLastAppendQueryFrame
                : -1;
        replayFeedback.replayLastAppendKnownByteCount8008EEF8 =
            host.numericRuntime.rightRankLastReplayAppendKnownBytes;
        replayFeedback.replayLastAppendPublishedCount901BC =
            host.numericRuntime.rightRankLastReplayAppendPublishedCount;
        replayFeedback.replayLastAppendWriteCount901C0 =
            host.numericRuntime.rightRankLastReplayAppendWriteCount;
        replayFeedback.replayLastAppendFullBackingKnown8008EEF8 =
            host.numericRuntime
                .rightRankLastReplayAppendFullBackingKnown8008EEF8;
        if (replayFeedback.requestValid &&
            replayFeedback.prevGrade92F40Known &&
            replayFeedback.hostCallbackKnown &&
            replayFeedback.psxFunctionMatched) {
            replayFeedback.captureAttempted = true;
            host.acceptedReplayBackup =
                PrScn1::Stage1AcceptedProducerReplayBackupRuntime{};
            host.captureReplayBackup(
                replayRequest.prevGrade92F40,
                host.numericRuntime.acceptedProducerReplayBuffer,
                host.acceptedReplayBackup);
            replayFeedback.backupValid = host.acceptedReplayBackup.valid;
            replayFeedback.backupPrevGrade92F40Known =
                host.acceptedReplayBackup.prevGrade92F40Valid;
            replayFeedback.backupPrevGrade92F40 =
                host.acceptedReplayBackup.prevGrade92F40;
            replayFeedback.backupPublishedCount901BC =
                host.acceptedReplayBackup.publishedCount901BC;
            replayFeedback.requestHandled =
                replayFeedback.backupValid &&
                replayFeedback.backupPrevGrade92F40Known &&
                replayFeedback.backupPrevGrade92F40 ==
                    replayRequest.prevGrade92F40;
        }
        const PrStage1LifecycleExecutorDirect::
            ActionApplyResult801C81EC apply =
                PrStage1LifecycleExecutorDirect::
                    ApplySaveStatus1635CReplayBackupHostFeedback801C81EC(
                        host.executor,
                        action,
                        replayFeedback);
        PrStage1LifecycleExecutorDirect::MergeActionApplyResult801C81EC(
            out,
            apply);
        const PrStage1LifecycleExecutorDirect::
            SaveStatus1635CReplayBackupHostGap801C81EC replayGap =
                PrStage1LifecycleExecutorDirect::
                    BuildSaveStatus1635CReplayBackupHostGap801C81EC(
                        action,
                        replayFeedback);
        if (replayGap.shouldLog) {
            static bool loggedReplayBackupGap = false;
            if (!loggedReplayBackupGap) {
                loggedReplayBackupGap = true;
                Log::Printf(
                    "Scene1 801C81EC SaveStatus1635C replay backup gap: order=%u request=%d fnKnown=%d fn=%08X fnMatched=%d prevGrade=%d hostCallback=%d replayKnown=%d producerKnown=%d producer=%08X bytesKnown=%d knownBytes=%u fullBacking=%d published=%u writeCount=%u saveQuery=%d lastAppendKnown=%d lastAppendQuery=%d lastAppendDeltaToSave=%d lastAppendKnownBytes=%u lastAppendPublished=%u lastAppendWriteCount=%u lastAppendFullBacking=%d capture=%d backupValid=%d backupPrevKnown=%d backupPrev=%u backupPublished=%u handled=%d",
                    replayGap.psxOrder,
                    replayGap.requestValid ? 1 : 0,
                    replayGap.psxFunctionKnown ? 1 : 0,
                    replayGap.psxFunction,
                    replayGap.psxFunctionMatched ? 1 : 0,
                    replayGap.prevGrade92F40Known ? 1 : 0,
                    replayGap.hostCallbackKnown ? 1 : 0,
                    replayGap.replayMirrorKnown8008EEF8 ? 1 : 0,
                    replayGap.replayMirrorProducerKnown8008EEF8 ? 1 : 0,
                    replayGap.replayMirrorProducerFunction,
                    replayGap.replayMirrorByteCountKnown8008EEF8 ? 1 : 0,
                    replayGap.replayMirrorKnownByteCount8008EEF8,
                    replayGap.replayMirrorFullBackingKnown8008EEF8 ? 1 : 0,
                    replayGap.replayPublishedCount901BC,
                    replayGap.replayWriteCount901C0,
                    replayGap.saveStatusQueryFrame,
                    replayGap.replayLastAppendKnown ? 1 : 0,
                    replayGap.replayLastAppendQueryFrame,
                    replayGap.replayLastAppendDeltaToSaveStatus,
                    replayGap.replayLastAppendKnownByteCount8008EEF8,
                    replayGap.replayLastAppendPublishedCount901BC,
                    replayGap.replayLastAppendWriteCount901C0,
                    replayGap.replayLastAppendFullBackingKnown8008EEF8
                        ? 1
                        : 0,
                    replayGap.captureAttempted ? 1 : 0,
                    replayGap.backupValid ? 1 : 0,
                    replayGap.backupPrevGrade92F40Known ? 1 : 0,
                    replayGap.backupPrevGrade92F40,
                    replayGap.backupPublishedCount901BC,
                    replayGap.requestHandled ? 1 : 0);
            }
        }
        if (!replayFeedback.requestHandled) {
            out = PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
            break;
        }
        const PrStageStatusBankDirectCallRequest request =
            PrStage1LifecycleExecutorDirect::
                BuildStatusBankDirectMemoryRequest801C81EC(action);
        const PrStage1LifecycleExecutorDirect::
            StatusBankDirectMemoryFeedback801C81EC feedback =
                PrStageStatusBankHostBridgeDirect::
                    ExecuteStatusBankDirectMemoryRequest801C81EC(request);
        const PrStage1LifecycleExecutorDirect::
            StatusBankDirectMemoryGap801C81EC gap =
                PrStage1LifecycleExecutorDirect::
                    BuildStatusBankDirectMemoryGap801C81EC(
                        action,
                        feedback);
        if (gap.shouldLog) {
            static bool loggedSaveStatus1635CGap = false;
            if (!loggedSaveStatus1635CGap) {
                loggedSaveStatus1635CGap = true;
                Log::Printf(
                    "Scene1 801C81EC SaveStatus1635C gap: scene=%d request=%d fn=%08X fnMatched=%d minStatus=%d prevGrade=%d payloadKnown=%d prefixKnown=%d statusKnown=%d writer=%08X seedAuthority=%08X wrote1635C=%d helperGap=%d lastFault=%08X",
                    gap.sceneId,
                    gap.requestValid ? 1 : 0,
                    gap.psxFunction,
                    gap.psxFunctionMatched ? 1 : 0,
                    gap.arg0,
                    gap.arg1,
                    gap.payloadKnown ? 1 : 0,
                    gap.payloadPrefixKnown80092F10 ? 1 : 0,
                    gap.payloadPrefixStatusBankKnown80092F1D ? 1 : 0,
                    gap.payloadLastWriterFunction,
                    gap.payloadSeedAuthorityFunction,
                    gap.payloadWrote8001635C ? 1 : 0,
                    gap.helperGap ? 1 : 0,
                    gap.lastFaultAddress);
            }
        }
        const bool saveStatus1635CWriterPublished =
            feedback.payloadKnown &&
            feedback.payloadPrefixKnown80092F10 &&
            feedback.payloadPrefixStatusBankKnown80092F1D &&
            feedback.payloadSeedAuthorityFunction == kFn8001635C &&
            feedback.payloadWrote8001635C;
        if (!feedback.requestHandled ||
            !feedback.payloadOk ||
            !saveStatus1635CWriterPublished) {
            PrStage1LifecycleExecutorDirect::MergeActionApplyResult801C81EC(
                out,
                PrStage1LifecycleExecutorDirect::
                    MakeBlockedActionRetryResult801C81EC());
        }
        break;
    }
    case ActionKind::UnlockNextStage1628C:
    {
        const PrStageStatusBankDirectCallRequest request =
            PrStage1LifecycleExecutorDirect::
                BuildStatusBankDirectMemoryRequest801C81EC(action);
        const PrStage1LifecycleExecutorDirect::
            StatusBankDirectMemoryFeedback801C81EC feedback =
                PrStageStatusBankHostBridgeDirect::
                    ExecuteStatusBankDirectMemoryRequest801C81EC(request);
        const PrStage1LifecycleExecutorDirect::
            StatusBankDirectMemoryGap801C81EC gap =
                PrStage1LifecycleExecutorDirect::
                    BuildStatusBankDirectMemoryGap801C81EC(
                        action,
                        feedback);
        if (gap.shouldLog) {
            static bool loggedUnlock1628CGap = false;
            if (!loggedUnlock1628CGap) {
                loggedUnlock1628CGap = true;
                Log::Printf(
                    "Scene1 801C81EC UnlockNextStage1628C gap: scene=%d request=%d fn=%08X fnMatched=%d helperGap=%d lastFault=%08X",
                    gap.sceneId,
                    gap.requestValid ? 1 : 0,
                    gap.psxFunction,
                    gap.psxFunctionMatched ? 1 : 0,
                    gap.helperGap ? 1 : 0,
                    gap.lastFaultAddress);
            }
        }
        if (!feedback.requestHandled || !feedback.payloadOk) {
            out = PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
        }
        break;
    }
    case ActionKind::Bootstrap15590:
    {
        const PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC
            block =
                PrStage1LifecycleExecutorDirect::GetHostBlockSnapshot801C81EC(
                    host.executor);
        PrStage1LifecycleExecutorDirect::Bootstrap15590HostStartInput801C81EC
            input{};
        input.block = block;
        input.action = action;
        PrStage1LifecycleExecutorDirect::Bootstrap15590HostStartPlan801C81EC
            plan =
                PrStage1LifecycleExecutorDirect::
                    BuildBootstrap15590HostStartPlan801C81EC(input);
        if (plan.alreadyActive) {
            out = PrStage1LifecycleExecutorDirect::MakeHostBlockResult801C81EC(
                true);
            break;
        }
        if (plan.shouldStartCurtain) {
            ctx.sceneExitReason = plan.sceneExitReason;
            input.curtainStartAttempted = true;
            // 801C81EC calls 8001EF14 after its post-movie transition and
            // before 80015590. That shared reset, not the disposed movie
            // runtime, owns the grid/cursors consumed by this callback.
            input.curtainStarted = PrStage1LoadingDirect::BeginAfterReset8001EF14(
                    host.executor.bootstrap15590Loading,
                    static_cast<int16_t>(ctx.sceneExitReason),
                    PrSfx::ApplySharedAudioDriverFlushBarrier26ECC);
            if (input.curtainStarted && PrStage1LoadingDirect::Tick(
                    host.executor.bootstrap15590Loading, ctx.frame)) {
                Log::Printf("Scene1 bootstrap15590 direct Loading started mode=%d style=%u reset1EF14=1 frame=%u",
                    ctx.sceneExitReason, host.executor.bootstrap15590Loading.frame.style, ctx.frame);
            }
            plan =
                PrStage1LifecycleExecutorDirect::
                    BuildBootstrap15590HostStartPlan801C81EC(input);
        }
        if (plan.shouldResolvePath) {
            input.pathResolveAttempted = true;
            input.pathResolved =
                TryResolveStage1LifecycleActionRowPath(
                    ctx,
                    action,
                    input.path);
            plan =
                PrStage1LifecycleExecutorDirect::
                    BuildBootstrap15590HostStartPlan801C81EC(input);
        }
        if (plan.shouldBeginHostBlock) {
            out = PrStage1LifecycleExecutorDirect::BeginHostBlock801C81EC(
                host.executor,
                plan.start);
        } else {
            out = PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
        }
        break;
    }
    case ActionKind::SaveUi19148:
        out = StartSaveUi19148Block(
            ctx,
            host.executor,
            action);
        break;
    }
    return out;
}

void DrainPendingActions801C81EC(
    PrGameContext& ctx,
    PrStage1LifecycleDirect::Runtime801C81EC& runtime,
    const PrStage1Scene1FrameDriverDirect::LoopFrameWindow& window,
    ActionHostRefs801C81EC& host) {
    for (;;) {
        const PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC
            block =
                PrStage1LifecycleExecutorDirect::GetHostBlockSnapshot801C81EC(
                    host.executor);
        const PrStage1LifecycleExecutorDirect::PendingActionDrainPlan801C81EC
            drainPlan =
                PrStage1LifecycleExecutorDirect::
                    BuildPendingActionDrainPlan801C81EC(block);
        if (!drainPlan.shouldPopAction) {
            break;
        }

        PrStage1LifecycleDirect::Action801C81EC action{};
        if (!PrStage1LifecycleExecutorDirect::PopNextAction801C81EC(
                host.executor,
                action)) {
            break;
        }
        const PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC
            applyResult = ApplyAction(ctx, action, window, host);
        if (applyResult.retryBlockedAction) {
            host.executor.pendingActions.insert(
                host.executor.pendingActions.begin(),
                action);
        }
        if (applyResult.waitingForHostBlock) {
            break;
        }
    }
    PrStage1LifecycleExecutorDirect::LifecycleStatusWrites801C81EC
        pendingStatusWrites{};
    if (PrStage1LifecycleExecutorDirect::PopReadyPendingStatusWrites801C81EC(
            host.executor,
            pendingStatusWrites)) {
        ApplyLifecycleStatusWrites801C81EC(ctx, runtime, pendingStatusWrites);
    }
}

static void ApplyLifecycleStatusWrites801C81EC(
    PrGameContext& ctx,
    PrStage1LifecycleDirect::Runtime801C81EC& runtime,
    const PrStage1LifecycleExecutorDirect::LifecycleStatusWrites801C81EC&
        writes) {
    if (writes.write800916D0) {
        ctx.transitionState = writes.word800916D0;
    }
    if (writes.write800916DA) {
        ctx.transitionStateDA = writes.word800916DA;
    }
    if (writes.write800916E0) {
        ctx.sceneExitReason = writes.word800916E0;
    }
    PrStage1LifecycleExecutorDirect::CommitClearTailStatusContinuation801C81EC(runtime, writes);
}

static uint32_t CountStepAction801C81EC(
    const PrStage1LifecycleDirect::StepResult801C81EC& step,
    PrStage1LifecycleDirect::ActionKind801C81EC kind) {
    uint32_t count = 0;
    for (const PrStage1LifecycleDirect::Action801C81EC& action :
         step.actions) {
        if (action.kind == kind) {
            ++count;
        }
    }
    return count;
}

static void LogBlockedUnknownWord800916F0ClearTail801C81EC(
    const PrStage1LifecycleDirect::StepResult801C81EC& step) {
    const auto observation =
        PrSS0Scene0RuntimeDirect::GetWord800916F0ObservationDebugSnapshot();
    const PrMainWord800916F0ProviderIngressDebug ingress =
        PrMain::GetWord800916F0ProviderIngressDebug();
    Log::Printf(
        "Scene1 801C81EC clear-tail F0 gate blocked: word_800916F0 source unknown accepted=%d runtimeAccepted=%d startupAccepted=%d startupSource=%u providerAttempted=%d providerReadable=%d providerPublished=%d mainGlobal=%d/%d mainGlobalSelfBootstrap=%d loaderHeap=%d/%d knownStateRelay=%d/%d cdMmio=%d/%d exactCd=%d/%d preSaveStatusActions=%u preUnlockActions=%u bootstrapActions=%u saveUiActions=%u sceneResultKnown=%d sceneResult=%d sceneResultReleaseBlocked=1 statusProducer8001635C=%d statusArgs=%d/%d/%d/%d unlock8001628C=%d/%d saveMenuCalled=%d",
        observation.acceptedCount,
        observation.runtimeObservationAcceptedCount,
        observation.startupObservationAcceptedCount,
        static_cast<unsigned>(observation.lastStartupSource),
        ingress.attempted ? 1 : 0,
        ingress.readable ? 1 : 0,
        ingress.published ? 1 : 0,
        ingress.sourceMainGlobalAttempted ? 1 : 0,
        ingress.sourceMainGlobalReadable ? 1 : 0,
        ingress.sourceMainGlobalSelfBootstrapBlocked ? 1 : 0,
        ingress.sourceLoaderHeapAttempted ? 1 : 0,
        ingress.sourceLoaderHeapReadable ? 1 : 0,
        ingress.sourceKnownStateRelayAttempted ? 1 : 0,
        ingress.sourceKnownStateRelayReadable ? 1 : 0,
        ingress.sourceCdMmioSnapshotAttempted ? 1 : 0,
        ingress.sourceCdMmioSnapshotReadable ? 1 : 0,
        ingress.sourceExactCdAttempted ? 1 : 0,
        ingress.sourceExactCdReadable ? 1 : 0,
        CountStepAction801C81EC(
            step,
            PrStage1LifecycleDirect::ActionKind801C81EC::SaveStatus1635C),
        CountStepAction801C81EC(
            step,
            PrStage1LifecycleDirect::ActionKind801C81EC::UnlockNextStage1628C),
        CountStepAction801C81EC(
            step,
            PrStage1LifecycleDirect::ActionKind801C81EC::Bootstrap15590),
        CountStepAction801C81EC(
            step,
            PrStage1LifecycleDirect::ActionKind801C81EC::SaveUi19148),
        step.sceneResultKnown ? 1 : 0,
        step.sceneResult,
        step.clearTailStatusProducerCalled8001635C ? 1 : 0,
        step.clearTailStatusA1,
        step.clearTailStatusA2,
        step.clearTailStatusA3,
        step.clearTailStatusA4,
        step.clearTailNextStageUnlockCalled8001628C ? 1 : 0,
        step.clearTailNextStageUnlockArg,
        step.clearTailSaveMenuCalled ? 1 : 0);
}

LifecycleStepPassResult801C81EC RunLifecycleStepPass801C81EC(
    PrGameContext& ctx,
    PrStage1LifecycleDirect::Runtime801C81EC& runtime,
    const PrStage1LifecycleDirect::SceneEntry801C7284& sceneEntry,
    const PrStage1LifecycleDirect::FrameInput801C81EC& input,
    const PrStage1Scene1FrameDriverDirect::LoopFrameWindow& window,
    ActionHostRefs801C81EC& host) {
    LifecycleStepPassResult801C81EC out{};
    const PrStage1LifecycleDirect::StepResult801C81EC step =
        PrStage1LifecycleDirect::Step801C81EC(
            runtime,
            sceneEntry,
            input);
    if (step.blockedByUnknownWord800916F0) {
        LogBlockedUnknownWord800916F0ClearTail801C81EC(step);
    }
    PrStage1LifecycleExecutorDirect::ConsumeFrameInputKnownFlags801C81EC(
        host.executor,
        input);
    if (step.blockedByUnknownWord800916F0 &&
        input.clearTailPlayAndWaitResultKnown) {
        host.executor.clearTailMoviePlayAndWaitResultKnown = true;
        host.executor.clearTailMoviePlayAndWaitResult =
            input.clearTailPlayAndWaitResult;
        Log::Printf(
            "Scene1 801C81EC clear-tail F0 gate preserved movie result for runtime-source retry result=%d",
            input.clearTailPlayAndWaitResult);
    }

    for (size_t actionIndex = 0; actionIndex < step.actions.size();
         actionIndex++) {
        const PrStage1LifecycleDirect::Action801C81EC& action =
            step.actions[actionIndex];
        const PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC
            applyResult = ApplyAction(ctx, action, window, host);
        PrStage1LifecycleExecutorDirect::MergeActionApplyResult801C81EC(
            out.aggregateApplyResult,
            applyResult);
        if (applyResult.waitingForHostBlock) {
            const size_t firstPendingActionIndex =
                applyResult.retryBlockedAction ? actionIndex
                                               : actionIndex + 1u;
            PrStage1LifecycleExecutorDirect::EnqueueActions801C81EC(
                host.executor,
                step.actions,
                firstPendingActionIndex);
            break;
        }
    }

    const PrStage1LifecycleExecutorDirect::LifecycleStatusWrites801C81EC
        statusWrites =
            PrStage1LifecycleExecutorDirect::BuildStatusWrites801C81EC(step);
    if (!out.aggregateApplyResult.waitingForHostBlock) {
        ApplyLifecycleStatusWrites801C81EC(ctx, runtime, statusWrites);
    } else {
        PrStage1LifecycleExecutorDirect::SetPendingStatusWrites801C81EC(
            host.executor,
            statusWrites);
    }

    out.sceneDispatch =
        PrStage1LifecycleExecutorDirect::ResolveStepSceneResult801C81EC(
            host.executor,
            step,
            out.aggregateApplyResult.waitingForHostBlock);
    return out;
}

LifecycleFrameTransactionResult801C81EC RunLifecycleFrameTransaction801C81EC(
    PrGameContext& ctx,
    PrStage1LifecycleDirect::Runtime801C81EC& runtime,
    const PrStage1LifecycleDirect::SceneEntry801C7284& sceneEntry,
    BuildFrameHostInputFn801C81EC buildFrameHostInput,
    const PrStage1Scene1FrameDriverDirect::LoopFrameWindow& window,
    ActionHostRefs801C81EC& host) {
    LifecycleFrameTransactionResult801C81EC out{};
    if (buildFrameHostInput == nullptr) {
        return out;
    }

    constexpr int kMaxExecutorPasses801C81EC = 8;
    for (int pass = 0; pass < kMaxExecutorPasses801C81EC; pass++) {
        const PrStage1LifecycleExecutorDirect::FrameHostInput801C81EC
            hostInput = buildFrameHostInput(ctx);
        const PrStage1LifecycleDirect::FrameInput801C81EC input =
            PrStage1LifecycleExecutorDirect::BuildFrameInput801C81EC(
                host.executor,
                hostInput);
        const LifecycleStepPassResult801C81EC passResult =
            RunLifecycleStepPass801C81EC(
                ctx,
                runtime,
                sceneEntry,
                input,
                window,
                host);
        const PrStage1LifecycleExecutorDirect::SceneResultDispatch801C81EC&
            sceneDispatch = passResult.sceneDispatch;
        if (sceneDispatch.sceneResultKnown) {
            if (!sceneDispatch.deferred) {
                out.sceneResultKnown = true;
                out.sceneResult = sceneDispatch.sceneResult;
            }
            break;
        }
        if (!passResult.aggregateApplyResult.producedImmediateInput ||
            passResult.aggregateApplyResult.waitingForHostBlock) {
            break;
        }
    }
    return out;
}

PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC
TickClearTailMovieBlock(
    PrGameContext& ctx,
    PrStage1LifecycleExecutorDirect::State801C81EC& state,
    PrStage1MovieTextOuterLoopDirect::MovieTextOuterLoopRuntimeSub801C455C&
        movieTextOuterLoop,
    PrStage1MovieTextDirect::Movie1TextRuntime& movieText,
    PrStage1Scene1Movie1Direct::Movie1RuntimeState* movieRuntime) {
    const PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC block =
        PrStage1LifecycleExecutorDirect::GetHostBlockSnapshot801C81EC(state);
    PrStage1LifecycleExecutorDirect::ClearTailMovieHostTickInput801C81EC
        tickInput{};
    tickInput.block = block;
    tickInput.strPlayerReady = ctx.strPlayer != nullptr;
    tickInput.movieFileExists =
        block.pathResolved && std::filesystem::exists(block.path);
    tickInput.strPlayerPlaying =
        ctx.strPlayer && ctx.strPlayer->IsPlaying();
    if (movieRuntime != nullptr) {
        tickInput.visualActive =
            PrStage1Scene1Movie1Direct::QueryDrawableState(*movieRuntime)
                .drawableActive;
    } else {
        tickInput.visualActive =
            PrStage1LifecycleExecutorDirect::
                IsClearTailMovieVisualActive801C81EC(state);
    }
    tickInput.debugSkipRequested = ctx.debugF1_StrSkip;
    const PrStage1LifecycleExecutorDirect::ClearTailMovieHostTickPlan801C81EC
        tickPlan =
            PrStage1LifecycleExecutorDirect::
                BuildClearTailMovieHostTickPlan801C81EC(tickInput);
    if (!tickPlan.active) {
        return {};
    }
    if (tickPlan.waitingForPendingActions) {
        return {};
    }
    if (movieRuntime != nullptr) {
        auto completeClearTailMovieBlock =
            [&]() -> PrStage1LifecycleExecutorDirect::
                ActionApplyResult801C81EC {
            if (ctx.strPlayer) ctx.strPlayer->FinishPlaybackKeepFrame();
            const int32_t clearTailPlayAndWaitResult =
                movieTextOuterLoop.psxReturnValue;
            PrStage1LifecycleExecutorDirect::
                ClearClearTailMovieVisualActive801C81EC(state);
            PrStage1MovieTextOuterLoopDirect::EndMovieTextOuterLoopSub801C455C(
                movieTextOuterLoop,
                movieText);
            PrStage1Scene1Movie1Direct::ClearPlayAndWaitCompletionPending(
                *movieRuntime);
            return PrStage1LifecycleExecutorDirect::ApplyHostBlockFeedback801C81EC(
                state,
                PrStage1LifecycleExecutorDirect::
                    BuildClearTailMovieCompletedHostBlockFeedback801C81EC(
                        clearTailPlayAndWaitResult));
        };
        if (PrStage1Scene1Movie1Direct::IsPlayAndWaitCompletionPending(
                *movieRuntime)) {
            if (!movieTextOuterLoop.complete) {
                PrStage1MovieTextOuterLoopDirect::
                    CompleteMovieTextOuterLoopFromHostMovieEndSub801C455C(
                        movieTextOuterLoop,
                        movieText);
            }
            return completeClearTailMovieBlock();
        }
        if (movieTextOuterLoop.complete) {
            return completeClearTailMovieBlock();
        }

        PrStage1Scene1Movie1Direct::Movie1HostFeedback movieHost =
            BuildMovie1HostFeedback(ctx,
                                    tickPlan.pathResolved,
                                    tickPlan.path,
                                    false);
        AdvanceMovie1HostStrPlayer(ctx, *movieRuntime, movieHost);
        const PrStage1Scene1Movie1Direct::Movie1AdvanceResult rawMovieResult =
            PrStage1Scene1Movie1Direct::AdvanceRuntimePure(
                *movieRuntime,
                movieHost,
                PrStage1MovieTextDirect::GetActiveMovieSubtitleTrack(movieText));
        const PrStage1Scene1Movie1Direct::Movie1HostActionFeedback feedback =
            ExecuteMovie1HostActions(ctx,
                                     tickPlan.pathResolved,
                                     tickPlan.path,
                                     rawMovieResult.hostActions);
        const PrStage1Scene1Movie1Direct::Movie1HostActionFeedbackResolution
            feedbackResolution =
                PrStage1Scene1Movie1Direct::ApplyHostActionFeedback(
                    *movieRuntime,
                    rawMovieResult,
                    feedback);
        (void)ExecuteMovie1HostActions(
            ctx,
            tickPlan.pathResolved,
            tickPlan.path,
            feedbackResolution.followupHostActions);

        const PrStage1Scene1Movie1Direct::Movie1AdvanceResult movieResult =
            feedbackResolution.advanceResult;
        // Keep clear-tail rendering on the Movie1 route until PlayAndWait ends.
        PrStage1LifecycleExecutorDirect::SetClearTailMovieVisualActive801C81EC(
            state,
            true);

        if (movieResult.completedToStage1) {
            if (!movieTextOuterLoop.complete) {
                PrStage1MovieTextOuterLoopDirect::
                    CompleteMovieTextOuterLoopFromHostMovieEndSub801C455C(
                        movieTextOuterLoop,
                        movieText);
            }
            return completeClearTailMovieBlock();
        }
        if (movieHost.lastStrUpdateResult == StrPlayerResult::Error) {
            PrStage1LifecycleExecutorDirect::
                ClearClearTailMovieVisualActive801C81EC(state);
            PrStage1MovieTextOuterLoopDirect::EndMovieTextOuterLoopSub801C455C(
                movieTextOuterLoop,
                movieText);
        }
        return PrStage1LifecycleExecutorDirect::MakeHostBlockResult801C81EC(
            true);
    }
    if (tickPlan.shouldPlay) {
        if (!ctx.strPlayer || !ctx.strPlayer->Play(tickPlan.path)) {
            PrStage1MovieTextOuterLoopDirect::EndMovieTextOuterLoopSub801C455C(
                movieTextOuterLoop,
                movieText);
            return PrStage1LifecycleExecutorDirect::MakeHostBlockResult801C81EC(
                true);
        }
        PrStage1LifecycleExecutorDirect::SetClearTailMovieVisualActive801C81EC(
            state,
            true);
    }

    if (!tickPlan.shouldUpdate) {
        return PrStage1LifecycleExecutorDirect::MakeHostBlockResult801C81EC(
            true);
    }
    if (!ctx.strPlayer) {
        return PrStage1LifecycleExecutorDirect::MakeHostBlockResult801C81EC(
            true);
    }
    const StrPlayerResult updateResult =
        ctx.strPlayer->Update(tickPlan.skipAllowed);
    if (updateResult == StrPlayerResult::Finished ||
        updateResult == StrPlayerResult::Skipped) {
        if (!movieTextOuterLoop.complete) {
            PrStage1MovieTextOuterLoopDirect::
                CompleteMovieTextOuterLoopFromHostMovieEndSub801C455C(
                    movieTextOuterLoop,
                    movieText);
        }
        const int32_t clearTailPlayAndWaitResult =
            movieTextOuterLoop.psxReturnValue;
        PrStage1LifecycleExecutorDirect::ClearClearTailMovieVisualActive801C81EC(
            state);
        PrStage1MovieTextOuterLoopDirect::EndMovieTextOuterLoopSub801C455C(
            movieTextOuterLoop,
            movieText);
        return PrStage1LifecycleExecutorDirect::ApplyHostBlockFeedback801C81EC(
            state,
            PrStage1LifecycleExecutorDirect::
                BuildClearTailMovieCompletedHostBlockFeedback801C81EC(
                    clearTailPlayAndWaitResult));
    }
    if (updateResult == StrPlayerResult::Error) {
        PrStage1LifecycleExecutorDirect::ClearClearTailMovieVisualActive801C81EC(
            state);
        PrStage1MovieTextOuterLoopDirect::EndMovieTextOuterLoopSub801C455C(
            movieTextOuterLoop,
            movieText);
    }
    return PrStage1LifecycleExecutorDirect::MakeHostBlockResult801C81EC(true);
}

PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC
TickAbortPollBlock(
    PrGameContext& ctx,
    PrStage1LifecycleExecutorDirect::State801C81EC& state) {
    const PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC block =
        PrStage1LifecycleExecutorDirect::GetHostBlockSnapshot801C81EC(state);
    PrStage1LifecycleExecutorDirect::AbortPollHostTickInput801C81EC input{};
    input.block = block;
    const bool ownsAbortPollBlock =
        block.kind ==
            PrStage1LifecycleExecutorDirect::HostBlockKind801C81EC::
                AbortPoll &&
        block.active;
    input.dispatcherRunning = false;
    input.dispatcherStartAttempted = ownsAbortPollBlock;
    input.dispatcherStartSucceeded = ownsAbortPollBlock;
    if (ownsAbortPollBlock) {
        BeginAbortPollEvent4Overlay801C81EC(ctx);
        const DirectAbortPollResult801C81EC direct =
            TickDirectAbortPollSub80026B94801C81EC(ctx);
        if (direct.status == DirectAbortPollStatus801C81EC::Complete) {
            input.dispatcherResult = direct.result;
            CompleteAbortPollEvent4Overlay801C81EC(direct.result);
        } else if (direct.frameBodyRequested || direct.retryPendingFrame) {
            (void)TickAbortPollEvent4Overlay801C81EC(ctx, direct);
        }
    } else {
        ResetAbortPollEvent4Overlay801C81EC();
    }
    PrStage1LifecycleExecutorDirect::AbortPollHostTickPlan801C81EC plan =
        PrStage1LifecycleExecutorDirect::BuildAbortPollHostTickPlan801C81EC(
            input);
    if (!plan.active) {
        return {};
    }
    if (plan.shouldComplete) {
        Log::Printf("Scene1 801C81EC direct abort-poll 80026B94 result=%d",
                    plan.result);
        return PrStage1LifecycleExecutorDirect::ApplyHostBlockFeedback801C81EC(
            state,
            PrStage1LifecycleExecutorDirect::
                BuildAbortPollCompletedHostBlockFeedback801C81EC(plan.result));
    }
    return PrStage1LifecycleExecutorDirect::MakeHostBlockResult801C81EC(true);
}

AbortPollEvent4OverlayState801C81EC GetActiveAbortPollEvent4OverlayState801C81EC(
    uint32_t frame) {
    AbortPollEvent4OverlayState801C81EC out{};
    if (!s_abortPollEvent4Active8001E750 ||
        s_abortPollEvent4ActiveFrame8001E750 != frame) {
        return out;
    }
    out.frameState = &s_abortPollEvent4FrameState8001E750;
    out.selectionState = s_abortPollEvent4SelectionState8001E750;
    out.ss0ModalOwnerInitialized =
        s_abortPollEvent4Ss0ModalOwnerInitialized;
    out.inputClosed = s_abortPollEvent4Tail80026B94.active;
    out.tailFramesRemaining = s_abortPollEvent4Tail80026B94.framesRemaining;
    out.cueFrame80025E6C = s_abortPollEvent4CueFrame80025E6C;
    out.cueRemaining80025E6C =
        s_abortPollEvent4CueRepeatCount80025E6C;
    out.lastInputStatus80025F0C = static_cast<uint8_t>(
        s_abortPollEvent4LastInput80025F0C.status);
    out.lastTickStatus80025E6C = static_cast<uint8_t>(
        s_abortPollEvent4LastTick80025E6C.status);
    out.frameTransactionActive80026B94 =
        s_abortPollEvent4FrameTransaction80026B94.active;
    out.frameTransactionCompleteWithinLimits80026B94 =
        s_abortPollEvent4FrameTransaction80026B94.
            directDrawCompleteWithinLimits &&
        s_abortPollEvent4FrameTransaction80026B94.
            hostFrameBoundaryExecuted;
    out.frameHostBoundaryExecuted80026B94 =
        s_abortPollEvent4FrameTransaction80026B94.
            hostFrameBoundaryExecuted;
    out.frameClass80026B94 = static_cast<uint8_t>(
        s_abortPollEvent4FrameTransaction80026B94.frameClass);
    out.frameLogicFrame80026B94 =
        s_abortPollEvent4FrameTransaction80026B94.logicFrame;
    out.inputCueSourceAccepted80025C8C =
        s_abortPollEvent4LastInputCue80025C8C.sourceAccepted;
    out.inputCueHostProjection80025C8C =
        s_abortPollEvent4LastInputCue80025C8C.hostStage1VabProjection;
    out.inputCuePlayResult80034240 =
        s_abortPollEvent4LastInputCue80025C8C.playResult80034240;
    out.inputCueFlushResult80026ECC =
        s_abortPollEvent4LastInputCue80025C8C.flushResult80026ECC;
    out.periodicCueSourceAccepted80026EF8 =
        s_abortPollEvent4LastPeriodicCue80026EF8.sourceAccepted;
    out.periodicCueHostProjection80026EF8 =
        s_abortPollEvent4LastPeriodicCue80026EF8.hostStage1VabProjection;
    out.periodicCuePlayResult80034240 =
        s_abortPollEvent4LastPeriodicCue80026EF8.playResult80034240;
    out.periodicCueFlushResult80026ECC =
        s_abortPollEvent4LastFlushResult80026ECC;
    out.exactPsxAudioHalParity80034240 =
        s_abortPollEvent4LastInputCue80025C8C.exactPsxHalParity ||
        s_abortPollEvent4LastPeriodicCue80026EF8.exactPsxHalParity;
    return out;
}

void ResetAbortPollEvent4Overlay801C81EC() {
    s_abortPollEvent4FrameState8001E750 =
        PrPsxEventFrameDirect::EventFrameState8001E750{};
    s_abortPollEvent4Active8001E750 = false;
    s_abortPollEvent4ActiveFrame8001E750 = 0u;
    s_abortPollEvent4SelectionState8001E750 = -1;
    s_abortPollEvent4CueRepeatCount80025E6C = 0u;
    s_abortPollEvent4CueFrame80025E6C = 0u;
    s_abortPollEvent4Ss0ModalOwnerInitialized = false;
    PrSS0EventFrameLoopDirect::ResetDispatcherPreLoopPadRelease80026B94(
        s_abortPollEvent4ReleaseGate80026B94);
    PrSS0EventFrameLoopDirect::ResetDispatcherPadChange80026744(
        s_abortPollEvent4PadChange80026744);
    PrSS0EventFrameLoopDirect::ResetDispatcherTail80026B94(
        s_abortPollEvent4Tail80026B94);
    PrSS0EventFrameLoopDirect::ResetEvent4FrameTransaction80026B94(
        s_abortPollEvent4FrameTransaction80026B94);
    s_abortPollEvent4LastFrameTail80026B94 = {};
    s_abortPollEvent4LastInput80025F0C = {};
    s_abortPollEvent4LastTick80025E6C = {};
    s_abortPollEvent4LastInputCue80025C8C = {};
    s_abortPollEvent4LastPeriodicCue80026EF8 = {};
    s_abortPollEvent4LastFlushResult80026ECC = 0;
}

static PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC
StartSaveUi19148Block(
    PrGameContext& ctx,
    PrStage1LifecycleExecutorDirect::State801C81EC& state,
    const PrStage1LifecycleDirect::Action801C81EC& action) {
    const PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC block =
        PrStage1LifecycleExecutorDirect::GetHostBlockSnapshot801C81EC(state);
    PrStage1LifecycleExecutorDirect::SaveUi19148HostStartInput801C81EC
        input{};
    input.block = block;
    input.action = action;
    PrStage1LifecycleExecutorDirect::SaveUi19148HostStartPlan801C81EC plan =
        PrStage1LifecycleExecutorDirect::BuildSaveUi19148HostStartPlan801C81EC(
            input);
    if (plan.alreadyActive) {
        return PrStage1LifecycleExecutorDirect::MakeHostBlockResult801C81EC(
            true);
    }
    if (!plan.actionValid) {
        return PrStage1LifecycleExecutorDirect::
            MakeBlockedActionRetryResult801C81EC();
    }
    if (plan.shouldStart) {
        if (!plan.seed80092F10Known ||
            plan.seed80092F10Address !=
                PrStage1SaveStatusPrefix80092F10::kPsxAddress) {
            return PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
        }
        input.startAttempted = true;
        const PrStage1SaveStatusPrefix80092F10 seed80092F10 =
            PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
        if (!seed80092F10.known ||
            seed80092F10.helperGap ||
            !seed80092F10.statusBankKnown80092F1D ||
            seed80092F10.psxAddress !=
                PrStage1SaveStatusPrefix80092F10::kPsxAddress ||
            seed80092F10.byteCount !=
                PrStage1SaveStatusPrefix80092F10::kByteCount) {
            Log::Printf(
                "Scene1 801C81EC SaveUi19148 start blocked: "
                "seed known=%d helperGap=%d statusKnown=%d addr=%08X bytes=%u",
                seed80092F10.known ? 1 : 0,
                seed80092F10.helperGap ? 1 : 0,
                seed80092F10.statusBankKnown80092F1D ? 1 : 0,
                seed80092F10.psxAddress,
                seed80092F10.byteCount);
            return PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
        }
        const bool seedWriterIsSaveStatus1635CChain =
            seed80092F10.wrote8001635C &&
            (seed80092F10.seedAuthorityFunction ==
                 PrStagePayloadBankDirect::kFn8001635C ||
             (seed80092F10.seedAuthorityFunction ==
                  PrStagePayloadBankDirect::kFn8001628C &&
              seed80092F10.wrote8001628C));
        if (!seedWriterIsSaveStatus1635CChain) {
            Log::Printf(
                "Scene1 801C81EC SaveUi19148 start blocked: "
                "seed writer not SaveStatus1635C chain writer=%08X seedAuthority=%08X wrote1635C=%d wrote1628C=%d",
                seed80092F10.lastWriterFunction,
                seed80092F10.seedAuthorityFunction,
                seed80092F10.wrote8001635C ? 1 : 0,
                seed80092F10.wrote8001628C ? 1 : 0);
            return PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
        }
        input.startSucceeded =
            PrStage1SaveUiHostBridgeDirect::StartSaveUiWithEntry19148(ctx, seed80092F10);
        // RunSaveUi19148HostTickAttempt performs
        // ExecuteCardCommunicationSetup80017524 at the exact 80019148
        // prelude boundary; ExecuteCardCommunicationTeardown80017574 is
        // paired at block completion. This adapter only owns the lifecycle
        // block and does not repeat either call.
        if (!input.startSucceeded) {
            Log::Printf(
                "Scene1 801C81EC SaveUi19148 start blocked: "
                "Start19148 rejected seed addr=%08X statusKnown=%d",
                seed80092F10.psxAddress,
                seed80092F10.statusBankKnown80092F1D ? 1 : 0);
            return PrStage1LifecycleExecutorDirect::
                MakeBlockedActionRetryResult801C81EC();
        }
        s_saveUi19148CardEventHalState801C81EC = {};
        plan =
            PrStage1LifecycleExecutorDirect::
                BuildSaveUi19148HostStartPlan801C81EC(input);
    }
    if (!plan.shouldBeginHostBlock) {
        return {};
    }
    return PrStage1LifecycleExecutorDirect::BeginHostBlock801C81EC(
        state,
        PrStage1LifecycleExecutorDirect::
            BuildSaveUi19148HostBlockStart801C81EC(
                plan.started,
                plan.seed80092F10Address));
}

PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC
TickSaveUi19148Block(
    PrGameContext& ctx,
    PrStage1LifecycleExecutorDirect::State801C81EC& state,
    const PrStage1SaveUi19148LowerFeedback* lowerFeedback) {
    const PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC block =
        PrStage1LifecycleExecutorDirect::GetHostBlockSnapshot801C81EC(state);
    PrStage1LifecycleExecutorDirect::SaveUi19148HostTickInput801C81EC input{};
    input.block = block;
    PrStage1LifecycleExecutorDirect::SaveUi19148HostTickPlan801C81EC plan =
        PrStage1LifecycleExecutorDirect::BuildSaveUi19148HostTickPlan801C81EC(
            input);
    if (plan.shouldStart) {
        PrStage1LifecycleDirect::Action801C81EC retryAction{};
        return StartSaveUi19148Block(ctx, state, retryAction);
    }
    if (!plan.active) {
        return {};
    }
    if (plan.shouldTick) {
        PrStage1SaveUi19148LowerFeedback mergedLowerFeedback{};
        const PrStage1SaveUi19148LowerFeedback* effectiveLowerFeedback =
            lowerFeedback;
        bool mergedLowerFeedbackActive = false;
        bool formatFeedbackBlocked = false;
        bool writeFeedbackBlocked = false;
        const char* lowerFeedbackSource = lowerFeedback ? "caller" : "none";
        auto ensureMergedLowerFeedback =
            [&]() -> PrStage1SaveUi19148LowerFeedback& {
            if (!mergedLowerFeedbackActive) {
                mergedLowerFeedback =
                    lowerFeedback ? *lowerFeedback
                                  : PrStage1SaveUi19148LowerFeedback{};
                mergedLowerFeedbackActive = true;
                effectiveLowerFeedback = &mergedLowerFeedback;
            }
            return mergedLowerFeedback;
        };
        const PrStage1SaveUi19148LowerFeedbackRequestList& pendingRequests =
            block.saveUi19148LowerFeedbackRequests;
        if (!PrStage1SaveUiHostBridgeDirect::HasPendingSaveUiPresentation19148()) {
        if (!effectiveLowerFeedback ||
            !effectiveLowerFeedback->cardIoFeedbackKnown80017594) {
            PrStage1SaveUi19148LowerFeedback& dst =
                ensureMergedLowerFeedback();
            const PrStage1SaveUi19148LowerFeedbackRequest* cardIoRequest =
                FindSaveUi19148LowerRequest801C81EC(
                    pendingRequests,
                    PrStage1SaveUi19148LowerFeedbackRequestKind::
                        CardIo80017594);
            if (cardIoRequest &&
                BuildSaveUi19148CardEventPollLowerFeedback801C81EC(
                    *cardIoRequest,
                    dst)) {
                lowerFeedbackSource =
                    lowerFeedback ? "caller-card-event-hal-80016e18"
                                  : "card-event-hal-80016e18";
            } else if (TryBuildSaveUi19148ObservedCardIoLowerFeedback801C81EC(
                    pendingRequests,
                    dst)) {
                lowerFeedbackSource =
                    lowerFeedback ? "caller-observed-cardio-80017594"
                                  : "observed-cardio-80017594";
            } else if (!lowerFeedback) {
                effectiveLowerFeedback = nullptr;
                mergedLowerFeedbackActive = false;
            }
        }
        if (!effectiveLowerFeedback ||
            !effectiveLowerFeedback->directoryRowsFeedbackKnown80019458) {
            PrStage1SaveUi19148LowerFeedback& dst =
                ensureMergedLowerFeedback();
            if (TryBuildSaveUi19148DirectoryRowsLowerFeedback801C81EC(
                    pendingRequests,
                    ctx.stage1RuntimePsxMemoryProvider,
                    dst)) {
                lowerFeedbackSource =
                    lowerFeedback ? "caller-saveui-directory-runtime-80019458"
                                  : "saveui-directory-runtime-80019458";
            } else if (!lowerFeedback &&
                       !dst.cardIoFeedbackKnown80017594 &&
                       !dst.directoryRowsFeedbackKnown80019458) {
                effectiveLowerFeedback = nullptr;
                mergedLowerFeedbackActive = false;
            }
        }
        if (!effectiveLowerFeedback ||
            !effectiveLowerFeedback->writeFeedbackKnown80017A10) {
            PrStage1SaveUi19148LowerFeedback& dst =
                ensureMergedLowerFeedback();
            if (TryBuildSaveUi19148WriteLowerFeedback801C81EC(
                    pendingRequests,
                    dst)) {
                lowerFeedbackSource =
                    lowerFeedback ? "caller-saveui-write-runtime-80017a10"
                                  : "saveui-write-runtime-80017a10";
            } else if (!lowerFeedback &&
                       !dst.cardIoFeedbackKnown80017594 &&
                       !dst.directoryRowsFeedbackKnown80019458 &&
                       !dst.writeFeedbackKnown80017A10) {
                effectiveLowerFeedback = nullptr;
                mergedLowerFeedbackActive = false;
            }
        }
        if (!effectiveLowerFeedback ||
            !effectiveLowerFeedback->formatFeedbackKnown80017B60) {
            PrStage1SaveUi19148LowerFeedback& dst =
                ensureMergedLowerFeedback();
            if (TryBuildSaveUi19148FormatLowerFeedback801C81EC(
                    pendingRequests,
                    dst)) {
                lowerFeedbackSource =
                    lowerFeedback ? "caller-saveui-format-runtime-80017b60"
                                  : "saveui-format-runtime-80017b60";
            } else if (!lowerFeedback &&
                       !dst.cardIoFeedbackKnown80017594 &&
                       !dst.directoryRowsFeedbackKnown80019458 &&
                       !dst.writeFeedbackKnown80017A10 &&
                       !dst.formatFeedbackKnown80017B60) {
                effectiveLowerFeedback = nullptr;
                mergedLowerFeedbackActive = false;
            }
        }
        if (effectiveLowerFeedback &&
            effectiveLowerFeedback->formatFeedbackKnown80017B60) {
            const SaveUi19148FormatCommitGate801C81EC formatCommitGate =
                TryCommitSaveUi19148FormatAfterTypedFeedback801C81EC(
                    state,
                    *effectiveLowerFeedback);
            if (formatCommitGate ==
                SaveUi19148FormatCommitGate801C81EC::Blocked) {
                PrStage1SaveUi19148LowerFeedback& dst =
                    ensureMergedLowerFeedback();
                dst.formatFeedbackKnown80017B60 = false;
                dst.formatFeedback80017B60 =
                    PrStage1SaveUiFormatFeedbackInput80017B60{};
                formatFeedbackBlocked = true;
            }
        }
        if (effectiveLowerFeedback &&
            effectiveLowerFeedback->writeFeedbackKnown80017A10) {
            const SaveUi19148WriteCommitGate801C81EC writeCommitGate =
                TryCommitSaveUi19148WriteAfterTypedFeedbackRequests801C81EC(
                    state.saveUi19148LowerFeedbackRequests,
                    *effectiveLowerFeedback);
            if (writeCommitGate ==
                SaveUi19148WriteCommitGate801C81EC::Blocked) {
                PrStage1SaveUi19148LowerFeedback& dst =
                    ensureMergedLowerFeedback();
                dst.writeFeedbackKnown80017A10 = false;
                dst.writeFeedback80017A10 =
                    PrStage1SaveUiWriteFeedbackInput80017A10{};
                writeFeedbackBlocked = true;
            }
        }
        if (formatFeedbackBlocked && writeFeedbackBlocked) {
            lowerFeedbackSource = "caller-format-write-blocked";
        } else if (formatFeedbackBlocked) {
            lowerFeedbackSource = "caller-format-blocked";
        } else if (writeFeedbackBlocked) {
            lowerFeedbackSource = "caller-write-blocked";
        }
        } else {
            effectiveLowerFeedback = nullptr;
            lowerFeedbackSource = "native-feedback-presentation-wait";
        }
        const PrStage1SaveUiHostBridgeDirect::SaveUi19148HostTickAttempt tick =
            PrStage1SaveUiHostBridgeDirect::RunSaveUi19148HostTickAttempt(
                ctx,
            effectiveLowerFeedback);
        ObserveSaveUi19148DirectCardEventSubmitAfterTick801C81EC(
            tick);
        if (tick.done) {
            s_saveUi19148CardEventHalState801C81EC = {};
            PrStage1SaveCardHalDirect::
                ExecuteCardCommunicationTeardown80017574();
        }
        if (lowerFeedback ||
            block.saveUi19148LowerFeedbackRequests.count > 0 ||
            tick.lowerFeedbackRequests.count > 0) {
            LogSaveUi19148CardIoLowerObservable801C81EC(
                lowerFeedbackSource,
                block.saveUi19148LowerFeedbackRequests,
                effectiveLowerFeedback,
                tick);
        }
        input.tickAttempted = tick.attempted;
        input.done = tick.done;
        input.saveResult = tick.saveResult;
        PrStage1LifecycleExecutorDirect::
            RecordSaveUi19148LowerFeedbackRequests801C81EC(
                state,
                tick.lowerFeedbackRequests);
        plan =
            PrStage1LifecycleExecutorDirect::
                BuildSaveUi19148HostTickPlan801C81EC(input);
        if (plan.shouldComplete) {
            Log::Printf(
                "Scene1 801C81EC save-ui 19148 done result=%d success=%d",
                tick.saveResult,
                tick.saveSucceeded ? 1 : 0);
        }
    }
    if (plan.shouldComplete) {
        return PrStage1LifecycleExecutorDirect::ApplyHostBlockFeedback801C81EC(
            state,
            PrStage1LifecycleExecutorDirect::
                BuildSaveUi19148CompletedHostBlockFeedback801C81EC(
                    plan.saveResult));
    }
    return PrStage1LifecycleExecutorDirect::MakeHostBlockResult801C81EC(true);
}

PrStage1SaveUiHostBridgeDirect::SaveUi19148HostTickAttempt
TickSaveUi19148Standalone801C81EC(
    PrGameContext& ctx,
    const PrStage1SaveUi19148LowerFeedbackRequestList& pendingRequests,
    PrStage1SaveUi19148LowerFeedbackRequestList* outNextRequests) {
    PrStage1SaveUi19148LowerFeedback mergedLowerFeedback{};
    const PrStage1SaveUi19148LowerFeedback* effectiveLowerFeedback = nullptr;
    bool mergedLowerFeedbackActive = false;
    const char* lowerFeedbackSource = "none";
    auto ensureMergedLowerFeedback =
        [&]() -> PrStage1SaveUi19148LowerFeedback& {
        if (!mergedLowerFeedbackActive) {
            mergedLowerFeedback = PrStage1SaveUi19148LowerFeedback{};
            mergedLowerFeedbackActive = true;
            effectiveLowerFeedback = &mergedLowerFeedback;
        }
        return mergedLowerFeedback;
    };

    if (!PrStage1SaveUiHostBridgeDirect::HasPendingSaveUiPresentation19148()) {
    PrStage1SaveUi19148LowerFeedback& dst = ensureMergedLowerFeedback();
    const PrStage1SaveUi19148LowerFeedbackRequest* cardIoRequest =
        FindSaveUi19148LowerRequest801C81EC(
            pendingRequests,
            PrStage1SaveUi19148LowerFeedbackRequestKind::CardIo80017594);
    if (cardIoRequest &&
        BuildSaveUi19148CardEventPollLowerFeedback801C81EC(
            *cardIoRequest,
            dst)) {
        lowerFeedbackSource = "standalone-card-event-hal-80016e18";
    } else if (TryBuildSaveUi19148ObservedCardIoLowerFeedback801C81EC(
            pendingRequests,
            dst)) {
        lowerFeedbackSource = "standalone-observed-cardio-80017594";
    }
    if (TryBuildSaveUi19148DirectoryRowsLowerFeedback801C81EC(
            pendingRequests,
            ctx.stage1RuntimePsxMemoryProvider,
            dst)) {
        lowerFeedbackSource =
            dst.cardIoFeedbackKnown80017594
                ? "standalone-cardio-directory-runtime-80019458"
                : "standalone-saveui-directory-runtime-80019458";
    }
    if (TryBuildSaveUi19148WriteLowerFeedback801C81EC(
            pendingRequests,
            dst)) {
        lowerFeedbackSource =
            dst.directoryRowsFeedbackKnown80019458
                ? "standalone-directory-write-runtime-80017a10"
                : "standalone-saveui-write-runtime-80017a10";
    }
    if (TryBuildSaveUi19148FormatLowerFeedback801C81EC(
            pendingRequests,
            dst)) {
        lowerFeedbackSource =
            dst.writeFeedbackKnown80017A10
                ? "standalone-write-format-runtime-80017b60"
                : "standalone-saveui-format-runtime-80017b60";
    }
    if (dst.formatFeedbackKnown80017B60) {
        const SaveUi19148FormatCommitGate801C81EC formatCommitGate =
            TryCommitSaveUi19148FormatAfterTypedFeedbackRequests801C81EC(
                pendingRequests,
                dst);
        if (formatCommitGate == SaveUi19148FormatCommitGate801C81EC::Blocked) {
            dst.formatFeedbackKnown80017B60 = false;
            dst.formatFeedback80017B60 =
                PrStage1SaveUiFormatFeedbackInput80017B60{};
            lowerFeedbackSource = "standalone-format-blocked";
        }
    }
    if (dst.writeFeedbackKnown80017A10) {
        const SaveUi19148WriteCommitGate801C81EC writeCommitGate =
            TryCommitSaveUi19148WriteAfterTypedFeedbackRequests801C81EC(
                pendingRequests,
                dst);
        if (writeCommitGate == SaveUi19148WriteCommitGate801C81EC::Blocked) {
            dst.writeFeedbackKnown80017A10 = false;
            dst.writeFeedback80017A10 =
                PrStage1SaveUiWriteFeedbackInput80017A10{};
            lowerFeedbackSource = "standalone-write-blocked";
        }
    }
    if (!dst.cardIoFeedbackKnown80017594 &&
        !dst.directoryRowsFeedbackKnown80019458 &&
        !dst.writeFeedbackKnown80017A10 &&
        !dst.formatFeedbackKnown80017B60) {
        effectiveLowerFeedback = nullptr;
        mergedLowerFeedbackActive = false;
    }
    } else {
        lowerFeedbackSource = "standalone-native-feedback-presentation-wait";
    }

    const PrStage1SaveUiHostBridgeDirect::SaveUi19148HostTickAttempt tick =
        PrStage1SaveUiHostBridgeDirect::RunSaveUi19148HostTickAttempt(
            ctx,
            effectiveLowerFeedback);
    ObserveSaveUi19148DirectCardEventSubmitAfterTick801C81EC(
        tick);
    if (tick.done) {
        s_saveUi19148CardEventHalState801C81EC = {};
        PrStage1SaveCardHalDirect::ExecuteCardCommunicationTeardown80017574();
    }
    if (outNextRequests) {
        *outNextRequests = tick.lowerFeedbackRequests;
    }
    if (pendingRequests.count > 0 || tick.lowerFeedbackRequests.count > 0 ||
        effectiveLowerFeedback) {
        LogSaveUi19148CardIoLowerObservable801C81EC(
            lowerFeedbackSource,
            pendingRequests,
            effectiveLowerFeedback,
            tick);
    }
    return tick;
}

PrStage1LifecycleExecutorDirect::ActionApplyResult801C81EC
TickBootstrap15590Block(
    PrGameContext& ctx,
    PrStage1LifecycleExecutorDirect::State801C81EC& state,
    PsxVramAtlas* residentAtlas) {
    const PrStage1LifecycleExecutorDirect::HostBlockSnapshot801C81EC block =
        PrStage1LifecycleExecutorDirect::GetHostBlockSnapshot801C81EC(state);
    if (block.kind == PrStage1LifecycleExecutorDirect::HostBlockKind801C81EC::Bootstrap15590 &&
        block.active) PrStage1LoadingDirect::Tick(state.bootstrap15590Loading, ctx.frame);
    PrStage1LifecycleExecutorDirect::Bootstrap15590HostTickInput801C81EC
        input{};
    input.block = block;
    PrStage1LifecycleExecutorDirect::Bootstrap15590HostTickPlan801C81EC plan{};
    using BootstrapCommand =
        PrStage1LifecycleExecutorDirect::
            Bootstrap15590HostTickCommand801C81EC;
    for (;;) {
        plan =
            PrStage1LifecycleExecutorDirect::
                BuildBootstrap15590HostTickPlan801C81EC(input);
        if (!plan.active) {
            return {};
        }
        if (plan.command == BootstrapCommand::ResolvePumpHost &&
            input.pumpKnown) {
            Log::Printf(
                "Scene1 801C81EC bootstrap15590 lower-CD command-select resolve pumpKnown=%d waiting=%d external=%d helperGap=%d limit=%d callback=%u/%d/%d lookup=%u/%d/%d lower=%u/%d/%d record=%u/%d/%d",
                input.pumpKnown ? 1 : 0,
                input.pump.waitingForFeedback ? 1 : 0,
                input.pump.externalProducerRequired ? 1 : 0,
                input.pump.helperGap ? 1 : 0,
                input.pump.reachedStepLimit ? 1 : 0,
                static_cast<unsigned>(input.callbackPumpCount),
                input.callbackAttempted ? 1 : 0,
                input.callbackApplied ? 1 : 0,
                static_cast<unsigned>(input.cdLookupPumpCount),
                input.cdLookupAttempted ? 1 : 0,
                input.cdLookupApplied ? 1 : 0,
                static_cast<unsigned>(input.cdLowerPumpCount),
                input.cdLowerAttempted ? 1 : 0,
                input.cdLowerApplied ? 1 : 0,
                static_cast<unsigned>(input.recordDispatchPumpCount),
                input.recordDispatchAttempted ? 1 : 0,
                input.recordDispatchApplied ? 1 : 0);
        }
        switch (plan.command) {
        case BootstrapCommand::Pump:
            input.pump =
                PrStage1LifecycleExecutorDirect::
                    PumpBootstrap15590LoaderDirect801C81EC(state);
            if (!state.bootstrap15590PendingTimUploads.empty()) {
                const bool projected = PrStageSceneSubmitBackend::
                    ApplyStage1NativeTimUploads8001A8F0(
                        state.bootstrap15590PendingTimUploads, residentAtlas);
                Log::Printf("Scene1 bootstrap15590 native TIM projection: count=%zu committed=%d",
                    state.bootstrap15590PendingTimUploads.size(), projected ? 1 : 0);
                if (!projected) {
                    input.pump.completed = false;
                    input.pump.failed = true;
                } else {
                    state.bootstrap15590PendingTimUploads.clear();
                }
            }
            input.pumpKnown = true;
            input.callbackAttempted = false;
            input.callbackApplied = false;
            input.cdLookupAttempted = false;
            input.cdLookupApplied = false;
            input.cdLowerAttempted = false;
            input.cdLowerApplied = false;
            input.recordDispatchAttempted = false;
            input.recordDispatchApplied = false;
            continue;
        case BootstrapCommand::TryCallback:
            input.callbackAttempted = true;
            ++input.callbackPumpCount;
            input.callbackApplied = false;
            if (input.pump.waitingForFeedback) {
                PrStage1LifecycleExecutorDirect::
                    Bootstrap15590CallbackHostRequest801C81EC request{};
                if (PrStage1LifecycleExecutorDirect::
                        BuildBootstrap15590LoaderCallbackHostRequest801C81EC(
                            state,
                            request)) {
                    PrStage1LifecycleExecutorDirect::
                        Bootstrap15590CallbackHostInput801C81EC callbackInput{};
                    if (request.transitionUpdateRequired) {
                        callbackInput.transitionUpdateAttempted = true;
                        // The callback was ticked once above. A loader pump
                        // may ask repeatedly during one host logic frame.
                        callbackInput.transitionUpdateResult = -1;
                    }
                    if (request.loadingCurtainCallbackRequired) {
                        callbackInput.loadingCurtainCallbackKnown = true;
                        callbackInput.loadingCurtainCallbackFired =
                            state.bootstrap15590Loading.callbacks != 0u;
                    }
                    input.callbackApplied =
                        PrStage1LifecycleExecutorDirect::
                            RunBootstrap15590LoaderCallbackHostFeedback801C81EC(
                                state,
                                callbackInput);
                }
            }
            continue;
        case BootstrapCommand::TryCdLookup:
            input.cdLookupAttempted = true;
            ++input.cdLookupPumpCount;
            input.cdLookupApplied =
                TryPumpBootstrap15590CdLookupIsoHal(ctx, state, input.pump);
            continue;
        case BootstrapCommand::TryCdLower:
            input.cdLowerAttempted = true;
            ++input.cdLowerPumpCount;
            input.cdLowerApplied =
                TryPumpBootstrap15590CdLowerExternalPlan(ctx,
                                                         state,
                                                         input.pump);
            continue;
        case BootstrapCommand::TryRecordDispatch: {
            input.recordDispatchAttempted = true;
            ++input.recordDispatchPumpCount;
            input.recordDispatchApplied = false;
            PrStage1LoaderProducerAdapter::RecordDispatchLiveInput
                recordInput{};
            if (PrStage1LifecycleExecutorDirect::
                    BuildBootstrap15590LoaderRecordDispatchLiveInput801C81EC(
                        state,
                        recordInput)) {
                PrStage1LoaderProducerAdapter::TypedActionFeedback feedback{};
                PrStage1LoaderProducerAdapter::LiveHalFeedbackBuildResult
                    build{};
                input.recordDispatchApplied =
                    PrStage1LifecycleExecutorDirect::
                        RunBootstrap15590LoaderRecordDispatchLiveProducer801C81EC(
                            state,
                            recordInput,
                            true,
                            &feedback,
                            &build);
                Log::Printf(
                    "Scene1 801C81EC bootstrap15590 record-dispatch facts: applied=%d ready=%d record=%u typeKnown=%d type=%d countKnown=%d count=%u sectorsKnown=%d sectors=%u",
                    input.recordDispatchApplied ? 1 : 0,
                    build.produced ? 1 : 0,
                    static_cast<unsigned>(recordInput.recordData.recordIndex),
                    recordInput.recordData.recordTypeKnown ? 1 : 0,
                    static_cast<int>(recordInput.recordData.recordType),
                    recordInput.recordData.recordCountKnown ? 1 : 0,
                    static_cast<unsigned>(recordInput.recordData.recordCount),
                    recordInput.recordData.sectorCountKnown ? 1 : 0,
                    recordInput.recordData.sectorCount);
            } else {
                const PrStage1LifecycleExecutorDirect::
                    Bootstrap15590RecordDispatchProbe801C81EC probe =
                        PrStage1LifecycleExecutorDirect::
                            ProbeBootstrap15590LoaderRecordDispatch801C81EC(
                                state);
                Log::Printf(
                    "Scene1 801C81EC bootstrap15590 record-dispatch facts: skipped active=%d begun=%d owner=%d waiting=%d/%d step=%u record=%u type=%d current=%d/%d/%d addr=%d/%08X size=%d/%u sectors=%d/%u live=%d/%d/%zu history0=%d/%d/%d addr=%d/%08X size=%d/%u sectors=%d/%u live=%d/%d/%zu recordData=%d",
                    probe.active ? 1 : 0,
                    probe.begun ? 1 : 0,
                    probe.loaderOwnerKnown ? 1 : 0,
                    probe.waitingStepKnown ? 1 : 0,
                    probe.waitingStepValid ? 1 : 0,
                    static_cast<unsigned>(probe.stepKind),
                    static_cast<unsigned>(probe.recordIndex),
                    static_cast<int>(probe.recordType),
                    probe.currentPayloadKnown ? 1 : 0,
                    probe.currentPayloadValid ? 1 : 0,
                    static_cast<int>(probe.currentPayloadRecordType),
                    probe.currentPayloadPsxAddressKnown ? 1 : 0,
                    probe.currentPayloadPsxAddress,
                    probe.currentPayloadSizeBytesKnown ? 1 : 0,
                    probe.currentPayloadSizeBytes,
                    probe.currentPayloadSectorCountKnown ? 1 : 0,
                    probe.currentPayloadSectorCount,
                    probe.currentPayloadLiveBytes ? 1 : 0,
                    (probe.currentPayloadLiveDataKnown &&
                     probe.currentPayloadLiveSizeKnown) ? 1 : 0,
                    probe.currentPayloadLiveSize,
                    probe.historyRecord0PayloadKnown ? 1 : 0,
                    probe.historyRecord0PayloadValid ? 1 : 0,
                    static_cast<int>(probe.historyRecord0PayloadRecordType),
                    probe.historyRecord0PayloadPsxAddressKnown ? 1 : 0,
                    probe.historyRecord0PayloadPsxAddress,
                    probe.historyRecord0PayloadSizeBytesKnown ? 1 : 0,
                    probe.historyRecord0PayloadSizeBytes,
                    probe.historyRecord0PayloadSectorCountKnown ? 1 : 0,
                    probe.historyRecord0PayloadSectorCount,
                    probe.historyRecord0PayloadLiveBytes ? 1 : 0,
                    (probe.historyRecord0PayloadLiveDataKnown &&
                     probe.historyRecord0PayloadLiveSizeKnown) ? 1 : 0,
                    probe.historyRecord0PayloadLiveSize,
                    probe.recordDataBuilt ? 1 : 0);
            }
            continue;
        }
        case BootstrapCommand::ResolvePumpHost:
        case BootstrapCommand::None:
            break;
        }
        break;
    }

    const PrStage1LifecycleExecutorDirect::
        Bootstrap15590PumpHostBlockResult801C81EC& pumpHost =
            plan.pumpHost;
    if (pumpHost.completed) {
        if (!PrStage1LoadingDirect::MinimumPresentationComplete(
                state.bootstrap15590Loading, ctx.frame)) {
            return PrStage1LifecycleExecutorDirect::MakeHostBlockResult801C81EC(true);
        }
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 direct loader complete actions=%zu loadingCallbacks=%u visibleTicks=%u firstVisible=%u release=%u",
            pumpHost.poppedActionCount, state.bootstrap15590Loading.callbacks,
            state.bootstrap15590Loading.presentedTicks,
            state.bootstrap15590Loading.firstPresentedTick, ctx.frame);
        PrStage1LoadingDirect::Stop(state.bootstrap15590Loading);
        return PrStage1LifecycleExecutorDirect::
            ApplyBootstrap15590PumpHostBlockResult801C81EC(
                state,
                pumpHost);
    }
    if (pumpHost.failed) {
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 direct loader failed actions=%zu",
            pumpHost.poppedActionCount);
    }
    if (pumpHost.waiting) {
        Log::Printf(
            "Scene1 801C81EC bootstrap15590 direct loader waiting feedback=%d external=%d gap=%d limit=%d",
            pumpHost.waitingForFeedback ? 1 : 0,
            pumpHost.externalProducerRequired ? 1 : 0,
            pumpHost.helperGap ? 1 : 0,
            pumpHost.reachedStepLimit ? 1 : 0);
        if (block.bootstrap15590CdLower.requestPending) {
            Log::Printf(
                "Scene1 801C81EC bootstrap15590 lower-CD pending status=%u reject=%d reason=%u order=%u step=%u cd=%u record=%u attempt=%u lower=%08X final=%08X",
                static_cast<unsigned>(block.bootstrap15590CdLower.status),
                block.bootstrap15590CdLower.lastRejectKnown ? 1 : 0,
                static_cast<unsigned>(
                    block.bootstrap15590CdLower.lastRejectReason),
                block.bootstrap15590CdLower.psxOrder,
                static_cast<unsigned>(block.bootstrap15590CdLower.stepKind),
                static_cast<unsigned>(block.bootstrap15590CdLower.cdActionKind),
                static_cast<unsigned>(block.bootstrap15590CdLower.recordIndex),
                static_cast<unsigned>(block.bootstrap15590CdLower.attemptIndex),
                block.bootstrap15590CdLower.lowerFunction,
                block.bootstrap15590CdLower.finalFunction);
        } else if (block.bootstrap15590CdLower.lastRejectKnown) {
            Log::Printf(
                "Scene1 801C81EC bootstrap15590 lower-CD rejected status=%u reason=%u order=%u step=%u cd=%u record=%u attempt=%u lower=%08X final=%08X",
                static_cast<unsigned>(block.bootstrap15590CdLower.status),
                static_cast<unsigned>(
                    block.bootstrap15590CdLower.lastRejectReason),
                block.bootstrap15590CdLower.psxOrder,
                static_cast<unsigned>(block.bootstrap15590CdLower.stepKind),
                static_cast<unsigned>(block.bootstrap15590CdLower.cdActionKind),
                static_cast<unsigned>(block.bootstrap15590CdLower.recordIndex),
                static_cast<unsigned>(block.bootstrap15590CdLower.attemptIndex),
                block.bootstrap15590CdLower.lowerFunction,
                block.bootstrap15590CdLower.finalFunction);
        }
    }
    return pumpHost.action;
}

} // namespace PrStage1LifecycleHostAdapter801C81EC
