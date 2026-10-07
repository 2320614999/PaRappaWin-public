#pragma once

#include "pr_movie_segment_direct.h"
#include "pr_stage1_loader_direct.h"
#include "pr_stage1_lower_cd_producer_direct.h"

#include <array>
#include <cstdint>
#include <cstddef>
#include <vector>

enum class PrStage1XaCdDirectHalGetlocPSource : uint8_t {
    Unknown = 0,
    AcceptedRingPacket = 1,
    CurrentPhysicalClock = 2,
};

struct PrStage1XaCdDirectRingPacketInput {
    uint32_t sectorIndex = 0u;
    uint8_t file = 0u;
    uint8_t channel = 0u;
    uint8_t coding = 0u;
    bool cdGetlocPResponseKnown = false;
    std::array<uint8_t, 8> cdGetlocPResponse{};
    bool cdDataReadyInterruptKnown = false;
    uint8_t cdDataReadyInterrupt = 0u;
    const uint8_t* header32 = nullptr;
    size_t headerSize = 0u;
    const uint8_t* payload2016 = nullptr;
    size_t payloadSize = 0u;
};

struct PrStage1XaCdDirectRingFrameView {
    bool available = false;
    uint32_t frameHandle = 0u;
    const uint8_t* payload2016 = nullptr;
    size_t payloadSize = 0u;
    const uint8_t* header32 = nullptr;
    size_t headerSize = 0u;
    uint16_t partCount = 0u;
    uint16_t frameIndex = 0u;
};

struct PrStage1XaCdDirectRingPumpResult {
    bool consumed = false;
    bool accepted = false;
    bool frameReady = false;
    uint32_t statusCode = 0u;
    uint32_t writeIndex = 0u;
    uint32_t frameStartIndex = 0u;
    uint32_t expectedPart = 0u;
    uint32_t frameIndex = 0u;
};

struct PrStage1XaCdDirectRingSlot {
    std::array<uint8_t, 32> header{};
    std::array<uint8_t, 2016> payload{};
};

struct PrStage1XaCdDirectState {
    bool streamStarted = false;

    uint32_t dword_800493EC = 0u;
    uint32_t dword_800493FC = 0u;
    uint8_t byte_8004940C = 1u;
    uint8_t byte_8004940D = 1u;
    uint16_t word_8004940E = 0u;
    uint32_t dword_8008A720 = 0u;
    uint32_t dword_8008A724 = 0u;
    bool dword_80092858 = false;
    uint32_t dword_80049410 = 0u;
    uint32_t dword_80049420 = 0u;
    uint32_t dword_80049424 = 0u;
    uint32_t dword_80057504 = 0u;
    uint32_t dword_80091640 = 0u;

    uint32_t dword_801C3868 = 0u;
    uint32_t dword_80095C50 = 0u;
    uint32_t dword_80095C54 = 0u;
    uint32_t dword_80095C58 = 0u;
    uint16_t word_8008ECD4 = 0u;
    uint32_t dword_8008ECA4 = 0u;
    bool dword_8008ECD8 = false;
    uint32_t dword_800917E0 = 0u;
    uint32_t dword_80091720 = 0u;
    uint32_t dword_80091724 = 1u;
    uint32_t dword_8009659C = 0xFFFFFFFFu;
    uint32_t dword_800965A4 = 1u;
    uint32_t dword_800965A8Index = 0u;
    bool dword_80096594 = false;
    uint32_t dword_80092908 = 0u;

    bool dword_8008ECDC = false;
    bool callback80039318Installed = false;
    bool callback80039240Installed = false;
    uint32_t callbackSerial = 0u;
    uint32_t callback80039318Count = 0u;
    uint32_t callback80039240Count = 0u;
    uint32_t lastDmaCallback39318Serial = 0u;
    uint32_t lastPumpCallback39240Serial = 0u;
    bool dword_800570F8Known = false;
    uint32_t dword_800570F8 = 0u;
    bool dword_800570FCKnown = false;
    uint32_t dword_800570FC = 0u;
    uint32_t readyCallbackRegisterSourceFunction = 0u;
    uint32_t readyCallbackRegisterWriteCount = 0u;
    uint32_t readyCallbackRegisterClearCount = 0u;
    uint32_t readyCallbackRegisterInstall39240Count = 0u;
    bool streamClockCallback8001A210Registered = false;
    uint32_t callbackRegisterSourceFunction = 0u;
    bool producerRingAvailable = false;
    bool producerStatusKnown = false;
    uint32_t lastProducerStatusCode = 0u;
    uint8_t lastSetModeByte391AC = 0u;
    uint16_t lastModeWord391AC = 0u;
    uint8_t lastCdCommand = 0u;
    uint32_t commandSerial = 0u;
    uint32_t setloc2Serial = 0u;
    uint32_t setfilter13Serial = 0u;
    uint32_t setmode14Serial = 0u;
    uint32_t readS27Serial = 0u;
    uint32_t command1Serial = 0u;

    bool byte_800493F4Known = false;
    PrMovieSegmentDirect::MsfBcd80036A78 byte_800493F4{};
    uint32_t byte800493F4ProducerFunction = 0u;
    bool dword_80049404Known = false;
    int32_t dword_80049404 = 0;
    bool dword_80049408Known = false;
    int32_t dword_80049408 = 0;
    bool dword_80049428Known = true;
    int32_t dword_80049428 = 0;
    bool sub8001A7F8Known = false;
    uint32_t sub8001A7F8Result = 0u;
    bool byte_80057119Known = false;
    uint8_t byte_80057119 = 0;
    uint32_t byte80057119ProducerFunction = 0u;
    bool dword_80057108Known = false;
    uint32_t dword_80057108 = 0u;
    bool statusFlags80057108ObservationPcKnown = false;
    uint32_t statusFlags80057108ObservationPc = 0u;
    uint32_t statusFlags80057108ObservationAcceptedCount = 0u;
    uint32_t statusFlags80057108ObservationRejectedCount = 0u;
    uint8_t statusFlags80057108LastReject = 0u;
    bool dword_8005710CKnown = false;
    uint32_t dword_8005710C = 0u;
    bool dword_80057110Known = false;
    uint32_t dword_80057110 = 0u;
    bool statusCounter80057110ObservationPcKnown = false;
    uint32_t statusCounter80057110ObservationPc = 0u;
    uint32_t statusCounter80057110ObservationAcceptedCount = 0u;
    uint32_t statusCounter80057110ObservationRejectedCount = 0u;
    uint8_t statusCounter80057110LastReject = 0u;
    bool cdCommandArgs800375BCKnown = false;
    uint32_t cdCommandArgCount800375BC = 0u;
    std::array<uint8_t, 16> cdCommandArgs800375BC{};
    int32_t cdCommandA3_800375BC = 0;
    bool cdCommandSkipWait800375BC = true;
    bool cdCommandTimeoutDeadline80088310Known = false;
    int32_t cdCommandTimeoutDeadline80088310 = 0;
    bool cdCommandTimeoutSpin80088314Known = false;
    uint32_t cdCommandTimeoutSpin80088314 = 0u;
    bool byte_800573D4Known = false;
    uint8_t byte_800573D4 = 0;
    bool statusRead80036384Known = false;
    uint32_t statusRead80036384 = 0u;
    uint32_t statusByte800573D4ObservationAcceptedCount = 0u;
    uint32_t statusByte800573D4ObservationRejectedCount = 0u;
    uint8_t statusByte800573D4LastReject = 0u;
    bool byte_800573D5Known = false;
    uint8_t byte_800573D5 = 0;
    bool byte_800573D6Known = false;
    uint8_t byte_800573D6 = 0;
    bool cdSyncExplicitStatusKnown = false;
    uint8_t cdSyncExplicitStatus = 0;
    bool cdSyncExplicitResponseBytesKnown = false;
    std::array<uint8_t, 8> cdSyncExplicitResponseBytes{};
    uint32_t halGetlocPFactsApplyCount = 0u;
    PrStage1XaCdDirectHalGetlocPSource lastHalGetlocPSource =
        PrStage1XaCdDirectHalGetlocPSource::Unknown;
    bool lastHalGetlocPSectorIndexKnown = false;
    uint32_t lastHalGetlocPSectorIndex = 0u;
    bool lastHalGetlocPDataReadyInterruptKnown = false;
    uint8_t lastHalGetlocPDataReadyInterrupt = 0u;
    uint32_t cdMmioNaturalAdapterAttemptCount = 0u;
    uint32_t cdMmioNaturalAdapterSourceUnavailableCount = 0u;
    uint32_t cdMmioNaturalAdapterPublishAttemptCount = 0u;
    uint32_t cdMmioNaturalAdapterAcceptedCount = 0u;
    uint8_t cdMmioNaturalAdapterLastReject = 0u;
    bool response_800882F8Known = false;
    std::array<uint8_t, 8> response_800882F8{};
    bool response_80049414Known = false;
    std::array<uint8_t, 8> response_80049414{};
    bool response_80088300Known = false;
    std::array<uint8_t, 8> response_80088300{};
    bool response_80088308Known = false;
    std::array<uint8_t, 8> response_80088308{};
    bool word_80055F78Known = false;
    uint16_t word_80055F78 = 0;
    bool word_80055F7AKnown = false;
    uint16_t word_80055F7A = 0;
    bool callbackPendingWord80055F7AObservationPcKnown = false;
    uint32_t callbackPendingWord80055F7AObservationPc = 0u;
    uint32_t callbackPendingWord80055F7AObservationAcceptedCount = 0u;
    uint32_t callbackPendingWord80055F7AObservationRejectedCount = 0u;
    uint8_t callbackPendingWord80055F7ALastReject = 0u;
    bool word_80055FA8Known = false;
    uint16_t word_80055FA8 = 0;
    bool callbackStateTable80055F7CKnown = false;
    std::array<bool,
               PrMovieSegmentDirect::
                   kCdCallbackPendingProducerCallbackCount800359B8>
        callbackStateTable80055F7CAddressKnown{};
    std::array<uint32_t,
               PrMovieSegmentDirect::
                   kCdCallbackPendingProducerCallbackCount800359B8>
        callbackStateTable80055F7CAddress{};
    uint32_t callbackStateTable80035BA0ApplyCount = 0u;
    uint32_t callbackStateTable80035BA0GapCount = 0u;
    uint32_t callbackStateTable80035BA0SourceFunction = 0u;
    bool dword_80057010Known = false;
    uint32_t dword_80057010 = 0u;
    bool cdCallbackPending80035898Known = false;
    bool cdCallbackPending80035898 = false;
    uint32_t cdCallbackPending80035898GapCount = 0u;
    uint32_t cdCallbackPending800359B8WriteCount = 0u;
    uint32_t cdCallbackPending800359B8GapCount = 0u;
    uint32_t interruptSnapshot800359B8ObservationAcceptedCount = 0u;
    uint32_t interruptSnapshot800359B8ObservationRejectedCount = 0u;
    uint8_t interruptSnapshot800359B8ObservationLastReject = 0u;
    bool interruptSnapshot800359B8LastCallbackStateKnown = false;
    bool interruptSnapshot800359B8LastInitialRegsKnown = false;
    bool interruptSnapshot800359B8LastTerminalRegsKnown = false;
    bool interruptSnapshot800359B8LastWatchdogKnown = false;
    bool interruptSnapshot800359B8LastBundleKnown = false;
    bool cdCallbackPending800359B8InterruptStatusKnown = false;
    uint16_t cdCallbackPending800359B8InterruptStatus = 0;
    bool cdCallbackPending800359B8InterruptMaskKnown = false;
    uint16_t cdCallbackPending800359B8InterruptMask = 0;
    uint32_t cdCallbackPending800359B8AckCount = 0u;
    std::array<uint8_t,
               PrMovieSegmentDirect::
                   kCdCallbackPendingProducerCallbackCount800359B8>
        cdCallbackPending800359B8AckBitIndex{};
    uint32_t cdCallbackPending800359B8CallbackDispatchCount = 0u;
    std::array<uint8_t,
               PrMovieSegmentDirect::
                   kCdCallbackPendingProducerCallbackCount800359B8>
        cdCallbackPending800359B8CallbackDispatchIndex{};
    uint32_t lowerCdSnapshotApplyCount = 0u;
    uint32_t lowerCdSnapshotPendingProducerCount = 0u;
    uint32_t lowerCdSnapshotCallbackEventCount = 0u;
    uint32_t lowerCdSnapshotSyncFeedbackCount = 0u;
    uint32_t lowerCdSnapshotSeamResultCount = 0u;
    uint32_t halGetlocLowerBridgeCount = 0u;
    uint32_t halGetlocLowerBridgeCdSyncCoreCount = 0u;
    uint32_t halGetlocLowerBridgePendingProducerCount = 0u;
    uint32_t halGetlocLowerBridgeCallbackEventCount = 0u;
    bool cdLowerFeedback80036AF8Known = false;
    PrMovieSegmentDirect::CdSyncLowerFeedback80037070
        cdLowerFeedback80036AF8{};
    bool cdLowerFeedback80036AF8FromGetlocP = false;
    bool cdLowerEventPsxReturn80036AF8Known = false;
    int32_t cdLowerEventPsxReturn80036AF8 = 0;
    uint32_t cdLowerEvent80036AF8Serial = 0u;
    uint32_t cdLowerEvent80036AF8DispatchedSerial = 0u;
    uint32_t cdSyncCallback8001A210DispatchCount = 0u;
    uint32_t cdSyncCallback80037070GapCount = 0u;
    uint32_t cdAsyncCallback80037070GapCount = 0u;
    bool cdSync80037070Known = false;
    PrMovieSegmentDirect::CdSyncResult80037070 cdSync80037070{};
    bool rawEvent80036AF8InitialInterruptKnown = false;
    uint8_t rawEvent80036AF8InitialInterrupt = 0;
    uint32_t rawEvent80036AF8InitialInterruptObservationAcceptedCount = 0u;
    uint32_t rawEvent80036AF8InitialInterruptObservationRejectedCount = 0u;
    uint8_t rawEvent80036AF8InitialInterruptObservationLastReject = 0u;
    bool rawEvent80036AF8StableInterruptKnown = false;
    uint8_t rawEvent80036AF8StableInterrupt = 0;
    bool rawEvent80036AF8CdReg0StatusKnown = false;
    uint8_t rawEvent80036AF8CdReg0Status = 0;
    bool rawEvent80036AF8FifoStatusSamplesKnown = false;
    uint32_t rawEvent80036AF8FifoStatusSampleCount = 0u;
    std::array<uint8_t, 9> rawEvent80036AF8FifoStatusSamples{};
    bool rawEvent80036AF8FifoDrainKnown = false;
    uint32_t rawEvent80036AF8FifoDrainCount = 0u;
    bool rawEvent80036AF8FifoDrained = false;
    bool rawEvent80036AF8ResultByteCountKnown = false;
    uint32_t rawEvent80036AF8ResultByteCount = 0u;
    bool rawEvent80036AF8ResultBytesKnown = false;
    std::array<uint8_t, 8> rawEvent80036AF8ResultBytes{};
    bool rawEvent80036AF8AckWritesKnown = false;
    bool rawEvent80036AF8Case1ClearWritesKnown = false;
    uint8_t rawEvent80036AF8Case1ClearCdReg0 = 0;
    uint8_t rawEvent80036AF8Case1ClearCdReg3 = 0;

    std::vector<PrStage1XaCdDirectRingSlot> ringSlots;
};

struct PrStage1XaCdDirectStartInput {
    bool segPresent = false;
    uint32_t cdlFilePosBcd = 0u;
    uint8_t initialChannel = 1u;
    bool mode1Streaming = false;
    bool cdCommandCompletionKnown = false;
    bool cdCommandTimedOut = false;
    bool cdCommandSyncResultKnown = false;
    int32_t cdCommandSyncResult = 0;
};

struct PrStage1XaCdDirectStartResult {
    bool started = false;
    uint8_t file = 0u;
    uint8_t channel = 0u;
    uint16_t modeWord391AC = 0u;
    uint32_t sectorLba = 0u;
};

struct PrStage1XaCdDirectSetFilter13Request {
    bool requestSetFilter = false;
    uint8_t file = 0u;
    uint8_t channel = 0u;
    uint8_t command = 0u;
    uint32_t commandSerial = 0u;
};

struct PrStage1XaCdDirectBgmVolumeRequest8001A4A4 {
    bool requestSetVolume = false;
    int16_t left = 0;
    int16_t right = 0;
    float normalizedVolume = 1.0f;
};

struct PrStage1XaCdDirectCallbackResult {
    bool invoked = false;
    uint32_t callbackSerial = 0u;
    uint32_t callbackCount = 0u;
    bool statusKnown = false;
    uint32_t statusCode = 0u;
};

struct PrStage1XaCdDirectClockCallbackInput8001A210 {
    uint8_t a1 = 0;
    const uint8_t* resultBytes = nullptr;
    uint32_t resultSize = 0;
};

struct PrStage1XaCdDirectCallbackRegisterResult80036510 {
    bool called = false;
    uint32_t sourceFunction = PrMovieSegmentDirect::kSub80036510SetCdCallback;
    int32_t psxReturn = 0;
    bool psxReturnKnown = false;
    bool dword800570F8Known = false;
    uint32_t dword800570F8 = 0u;
    bool streamClockCallback8001A210Registered = false;
    bool callbackCleared = false;
};

struct PrStage1XaCdDirectReadyCallbackRegisterResult80036528 {
    bool called = false;
    uint32_t sourceFunction =
        PrMovieSegmentDirect::kSub80036528SetCdReadyCallback;
    int32_t psxReturn = 0;
    bool psxReturnKnown = false;
    bool dword800570FCKnown = false;
    uint32_t dword800570FC = 0u;
    bool callback80039240Installed = false;
    bool callbackCleared = false;
};

struct PrStage1XaCdDirectCallbackStateTableSetResult80035BA0 {
    bool called = false;
    uint32_t sourceFunction =
        PrMovieSegmentDirect::kSub80035BA0SetCdInterruptCallbackTable;
    int32_t psxReturn = 0;
    uint32_t callbackIndex = 0u;
    bool callbackAddrKnown = false;
    uint32_t callbackAddr = 0u;
    bool oldCallbackKnown = false;
    uint32_t oldCallback = 0u;
    bool applied = false;
    bool noChange = false;
    bool callbackStateTableKnown = false;
    bool word80055FA8Known = false;
    uint16_t word80055FA8 = 0;
    bool gapInvalidCallbackIndex = false;
    bool gapMissingCallbackAddr = false;
    bool gapMissingWord80055F78 = false;
    bool gateClosedWord80055F78 = false;
    bool gapMissingWord80055FA8 = false;
};

struct PrStage1XaCdDirectCallbackRegisterResult8001A258 {
    bool called = false;
    uint32_t sourceFunction =
        PrMovieSegmentDirect::kSub8001A258StreamClockCallbackRegister;
    PrStage1XaCdDirectCallbackRegisterResult80036510 setCallback{};
    int32_t psxReturn = 0;
};

struct PrStage1XaCdDirectClearCallbackResult8001A694 {
    bool called = false;
    uint32_t sourceFunction =
        PrMovieSegmentDirect::kSub8001A694ClearCdCallback;
    bool waitForCdSync800367A4 = true;
    PrStage1XaCdDirectCallbackRegisterResult80036510 setCallback{};
    int32_t psxReturn = 0;
};

struct PrStage1XaCdDirectClockPollInput8001A3C8 {
    bool sub800364D0Known = false;
    int32_t sub800364D0Result = 0;
    bool syncBytesKnown = false;
    std::array<uint8_t, 16> syncBytes{};
    bool sub800363A4Known = false;
    int32_t sub800363A4Result = 0;
};

struct PrStage1XaCdDirectCdReadyStatusInput800363A4 {
    bool byte80057119Known = false;
    uint8_t byte80057119 = 0;
};

struct PrStage1XaCdDirectCdSyncInput80037070 {
    int32_t a0WaitMode = 0;
    bool a1OutputBufferPtrNonNull = false;
    PrMovieSegmentDirect::CdSyncLowerFeedback80037070 feedback{};
};

struct PrStage1XaCdDirectCdSyncFromLowerStateInput800364D0 {
    int32_t a0WaitMode = 1;
    bool a1OutputBufferPtrNonNull = true;
    bool allowGetlocPFeedback = true;
};

struct PrStage1XaCdDirectStageRecordTickResult8001A4D0 {
    bool called = false;
    uint32_t sourceFunction = 0x8001A4D0u;
    bool started = false;
    PrStage1XaCdDirectStartResult start{};
    bool waitingForHalFacts = false;
    PrMovieSegmentDirect::CdSyncResult80037070 finalCdSync800364D0{};
    bool finalCommandWrapper80036678Known = false;
    bool finalCommandWrapper80036678Succeeded = false;
    bool resultKnown = false;
    int32_t psxReturn = 0;
    bool gapMissingCdSyncFeedback = false;
};

struct PrStage1XaCdDirectLowerCdProducerSnapshot {
    bool pendingProducer800359B8Known = false;
    PrMovieSegmentDirect::CdCallbackPendingProducerInput800359B8
        pendingProducer800359B8{};

    bool lowerEvent80036AF8Known = false;
    PrMovieSegmentDirect::CdCallbackEventInput80036AF8 lowerEvent80036AF8{};
    bool lowerEventRegisters80036AF8Known = false;
    PrMovieSegmentDirect::CdCallbackEventRegisterInput80036AF8
        lowerEventRegisters80036AF8{};

    bool cdSyncFeedback80037070Known = false;
    PrMovieSegmentDirect::CdSyncLowerFeedback80037070 cdSyncFeedback80037070{};

    bool cdSeamResultKnown = false;
    PrStage1LoaderDirect::CdSeamResult cdSeamResult{};
};

struct PrStage1XaCdDirectLowerCdProducerResult {
    bool called = false;

    bool pendingProducer800359B8Applied = false;
    PrMovieSegmentDirect::CdCallbackPendingProducerResult800359B8
        pendingProducer800359B8{};

    bool checkCallback80035898Applied = false;
    PrMovieSegmentDirect::CheckCallbackResult80035898 checkCallback80035898{};

    bool lowerEvent80036AF8Applied = false;
    PrMovieSegmentDirect::CdCallbackEventResult80036AF8 lowerEvent80036AF8{};
    bool lowerEventRegisters80036AF8Applied = false;
    PrMovieSegmentDirect::CdCallbackEventRegisterResult80036AF8
        lowerEventRegisters80036AF8{};

    bool cdSyncFeedback80037070Applied = false;
    bool readyForCdSync80037070 = false;

    bool cdSeamResultAccepted = false;
    bool cdSeamResultRejected = false;
    PrStage1LoaderDirect::CdSeamResult cdSeamResult{};
};

struct PrStage1XaCdDirectLowerCdSnapshotBridgeInput {
    bool interruptSnapshot800359B8Known = false;
    PrStage1LowerCdProducerDirect::CdInterruptSnapshotInput800359B8
        interruptSnapshot800359B8{};

    bool rawEvent80036AF8Known = false;
    PrStage1LowerCdProducerDirect::RawCdRegTransactionResult80036AF8
        rawEvent80036AF8{};

    bool cdSyncCoreFacts80037070Known = false;
    PrStage1LowerCdProducerDirect::CdSyncLowerFeedbackInput80037070
        cdSyncCoreFacts80037070{};

    bool cdSyncLoopFacts80037070Known = false;
    PrStage1LowerCdProducerDirect::CdSyncLoopFactsInput80037070
        cdSyncLoopFacts80037070{};

    bool cdSyncFeedback80037070Known = false;
    PrMovieSegmentDirect::CdSyncLowerFeedback80037070 cdSyncFeedback80037070{};

    bool lowerCdFactsKnown = false;
    PrStage1LowerCdProducerDirect::LowerCdProducerFacts lowerCdFacts{};
};

struct PrStage1XaCdDirectLowerCdSnapshotBridgeResult {
    bool produced = false;
    bool incomplete = false;
    bool pendingProducer800359B8Bridged = false;
    bool lowerEventRegisters80036AF8Bridged = false;
    bool lowerEventEarlyReturnNoInterrupt = false;
    bool cdSyncCoreFacts80037070Bridged = false;
    bool cdSyncLoopFacts80037070Bridged = false;
    bool cdSyncFeedback80037070Bridged = false;
    bool lowerCdFactsBridged = false;
    PrStage1XaCdDirectLowerCdProducerSnapshot snapshot{};
};

struct PrStage1XaCdDirectCommandInput800375BC {
    uint8_t command = 0;
    bool argsKnown = false;
    uint32_t argCount = 0;
    std::array<uint8_t, 16> args{};
    int32_t a3 = 0;
    bool skipWait = true;
    bool clockKnown = false;
    int32_t clockNow = 0;
    bool waitLoopResultKnown = false;
    int32_t waitLoopPsxReturn = -1;
    PrMovieSegmentDirect::CdSyncLowerFeedback80037070 preSyncFeedback{};
    PrMovieSegmentDirect::CheckCallbackInput80035898 checkCallback{};
    PrMovieSegmentDirect::CdCallbackEventInput80036AF8 callbackEvent{};
};

struct PrStage1XaCdDirectCommandResult800375BC {
    bool called = false;
    uint32_t sourceFunction = PrMovieSegmentDirect::kSub800375BCCdCommand;
    bool incomplete = false;
    bool psxReturnKnown = false;
    int32_t psxReturn = 0;
    uint8_t byte80057119 = 0;
    bool byte80057119Known = false;
    bool preSyncCalled = false;
    PrMovieSegmentDirect::CdSyncResult80037070 preSyncResult{};
    bool waitLoopRequested = false;
    bool waitLoopResultKnown = false;
    int32_t waitLoopPsxReturn = -1;
    PrMovieSegmentDirect::CheckCallbackResult80035898 checkCallbackResult{};
    PrMovieSegmentDirect::CdCallbackEventResult80036AF8 callbackResult{};
};

struct PrStage1XaCdDirectInitResult8001A280 {
    bool called = false;
    uint32_t sourceFunction = 0x8001A280u;
    bool dword80049428Known = false;
    int32_t dword80049428 = 0;
    bool commandIssued = false;
    bool skippedNonZeroWorkBase = false;
    bool gapMissingDword80049428 = false;
    PrStage1XaCdDirectCommandResult800375BC commandResult{};
};

struct PrStage1XaCdDirectStreamClockProbe800493F4 {
    bool inspected = false;
    bool liveByte800493F4ProducerKnown = false;
    bool setlocStartAnchorObserved = false;
    uint32_t setloc2Serial = 0u;
    uint32_t readS27Serial = 0u;
    PrMovieSegmentDirect::StreamClockProducerCarrier800493F4 carrier{};
    bool gapMissingStreamClock800493F4Producer = false;
};

struct PrStage1XaCdDirectHalGetlocPFactsInput {
    PrStage1XaCdDirectHalGetlocPSource source =
        PrStage1XaCdDirectHalGetlocPSource::Unknown;
    bool sectorIndexKnown = false;
    uint32_t sectorIndex = 0u;
    bool cdGetlocPResponseKnown = false;
    std::array<uint8_t, 8> cdGetlocPResponse{};
    bool cdDataReadyInterruptKnown = false;
    uint8_t cdDataReadyInterrupt = 0u;
};

struct PrStage1XaCdDirectHalGetlocPFactsResult {
    bool called = false;
    bool applied = false;
    bool incomplete = false;
};

enum class PrStage1XaCdDirectCallbackSlotsObservationSource : uint8_t {
    Unknown = 0,
    RuntimePsxMemoryObservation = 1,
    RuntimePsxConsumerReadObservation = 2,
};

enum class PrStage1XaCdDirectStatusByteObservationSource : uint8_t {
    Unknown = 0,
    RuntimePsxMemoryObservation = 1,
    RuntimePsxConsumerReadObservation = 2,
};

enum class PrStage1XaCdDirectStatusFlagsObservationSource80057108 : uint8_t {
    Unknown = 0,
    RuntimePsxMemoryObservation = 1,
    RuntimePsxConsumerReadObservation = 2,
};

enum class PrStage1XaCdDirectStatusCounterObservationSource80057110 : uint8_t {
    Unknown = 0,
    RuntimePsxMemoryObservation = 1,
    RuntimePsxConsumerReadObservation = 2,
};

enum class PrStage1XaCdDirectCallbackPendingObservationSource80055F7A
    : uint8_t {
    Unknown = 0,
    RuntimePsxMemoryObservation = 1,
    RuntimePsxConsumerReadObservation = 2,
};

enum class PrStage1XaCdDirectRawEventInitialInterruptObservationSource80036AF8
    : uint8_t {
    Unknown = 0,
    RuntimePsxMemoryObservation = 1,
    RuntimePsxConsumerReadObservation = 2,
};

enum class PrStage1XaCdDirectCdMmioSnapshotObservationSource : uint8_t {
    Unknown = 0,
    RuntimeCdMmioSampleProducer = 1,
};

enum class PrStage1XaCdDirectInterruptSnapshotObservationSource800359B8
    : uint8_t {
    Unknown = 0,
    RuntimePsxMemoryObservation = 1,
    RuntimePsxConsumerReadObservation = 2,
};

struct PrStage1XaCdDirectCallbackSlotsObservation800570F8FC {
    PrStage1XaCdDirectCallbackSlotsObservationSource source =
        PrStage1XaCdDirectCallbackSlotsObservationSource::Unknown;
    uint32_t psxAddress = 0u;
    uint32_t byteSize = 0u;
    bool valueKnown = false;
    const uint8_t* bytes = nullptr;
    size_t bytesSize = 0u;
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

enum class PrStage1XaCdDirectCallbackSlotsObservationRejectReason : uint8_t {
    None = 0,
    NonRuntimePsxMemoryObservation = 1,
    WrongPsxAddress = 2,
    WrongByteSize = 3,
    UnknownValue = 4,
    MissingBytes = 5,
};

struct PrStage1XaCdDirectCallbackSlotsObservationResult {
    bool accepted = false;
    PrStage1XaCdDirectCallbackSlotsObservationRejectReason rejectReason =
        PrStage1XaCdDirectCallbackSlotsObservationRejectReason::None;
};

struct PrStage1XaCdDirectStatusByteObservation800573D4 {
    PrStage1XaCdDirectStatusByteObservationSource source =
        PrStage1XaCdDirectStatusByteObservationSource::Unknown;
    uint32_t psxAddress = 0u;
    uint32_t byteSize = 0u;
    bool valueKnown = false;
    uint8_t value = 0u;
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

enum class PrStage1XaCdDirectStatusByteObservationRejectReason : uint8_t {
    None = 0,
    NonRuntimePsxMemoryObservation = 1,
    WrongPsxAddress = 2,
    WrongByteSize = 3,
    UnknownValue = 4,
};

struct PrStage1XaCdDirectStatusByteObservationResult {
    bool accepted = false;
    PrStage1XaCdDirectStatusByteObservationRejectReason rejectReason =
        PrStage1XaCdDirectStatusByteObservationRejectReason::None;
};

struct PrStage1XaCdDirectStatusFlagsObservation80057108 {
    PrStage1XaCdDirectStatusFlagsObservationSource80057108 source =
        PrStage1XaCdDirectStatusFlagsObservationSource80057108::Unknown;
    uint32_t psxAddress = 0u;
    uint32_t byteSize = 0u;
    bool valueKnown = false;
    uint32_t value = 0u;
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

enum class PrStage1XaCdDirectStatusFlagsObservationRejectReason80057108
    : uint8_t {
    None = 0,
    NonRuntimePsxMemoryObservation = 1,
    WrongPsxAddress = 2,
    WrongByteSize = 3,
    UnknownValue = 4,
};

struct PrStage1XaCdDirectStatusFlagsObservationResult80057108 {
    bool accepted = false;
    PrStage1XaCdDirectStatusFlagsObservationRejectReason80057108
        rejectReason =
            PrStage1XaCdDirectStatusFlagsObservationRejectReason80057108::None;
};

struct PrStage1XaCdDirectStatusCounterObservation80057110 {
    PrStage1XaCdDirectStatusCounterObservationSource80057110 source =
        PrStage1XaCdDirectStatusCounterObservationSource80057110::Unknown;
    uint32_t psxAddress = 0u;
    uint32_t byteSize = 0u;
    bool valueKnown = false;
    uint32_t value = 0u;
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

enum class PrStage1XaCdDirectStatusCounterObservationRejectReason80057110
    : uint8_t {
    None = 0,
    NonRuntimePsxMemoryObservation = 1,
    WrongPsxAddress = 2,
    WrongByteSize = 3,
    UnknownValue = 4,
};

struct PrStage1XaCdDirectStatusCounterObservationResult80057110 {
    bool accepted = false;
    PrStage1XaCdDirectStatusCounterObservationRejectReason80057110
        rejectReason =
            PrStage1XaCdDirectStatusCounterObservationRejectReason80057110::
                None;
};

struct PrStage1XaCdDirectCallbackPendingObservation80055F7A {
    PrStage1XaCdDirectCallbackPendingObservationSource80055F7A source =
        PrStage1XaCdDirectCallbackPendingObservationSource80055F7A::Unknown;
    uint32_t psxAddress = 0u;
    uint32_t byteSize = 0u;
    bool valueKnown = false;
    uint16_t value = 0u;
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

enum class PrStage1XaCdDirectCallbackPendingObservationRejectReason80055F7A
    : uint8_t {
    None = 0,
    NonRuntimePsxMemoryObservation = 1,
    WrongPsxAddress = 2,
    WrongByteSize = 3,
    UnknownValue = 4,
};

struct PrStage1XaCdDirectCallbackPendingObservationResult80055F7A {
    bool accepted = false;
    PrStage1XaCdDirectCallbackPendingObservationRejectReason80055F7A
        rejectReason =
            PrStage1XaCdDirectCallbackPendingObservationRejectReason80055F7A::
                None;
};

struct PrStage1XaCdDirectRawEventInitialInterruptObservation80036AF8 {
    PrStage1XaCdDirectRawEventInitialInterruptObservationSource80036AF8
        source =
            PrStage1XaCdDirectRawEventInitialInterruptObservationSource80036AF8::
                Unknown;
    uint32_t psxAddress = 0u;
    uint32_t byteSize = 0u;
    bool valueKnown = false;
    uint8_t value = 0u;
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

enum class PrStage1XaCdDirectRawEventInitialInterruptObservationRejectReason80036AF8
    : uint8_t {
    None = 0,
    NonRuntimePsxMemoryObservation = 1,
    WrongPsxAddress = 2,
    WrongByteSize = 3,
    UnknownValue = 4,
};

struct PrStage1XaCdDirectRawEventInitialInterruptObservationResult80036AF8 {
    bool accepted = false;
    PrStage1XaCdDirectRawEventInitialInterruptObservationRejectReason80036AF8
        rejectReason =
            PrStage1XaCdDirectRawEventInitialInterruptObservationRejectReason80036AF8::
                None;
};

struct PrStage1XaCdDirectCdMmioSnapshotObservation {
    PrStage1XaCdDirectCdMmioSnapshotObservationSource source =
        PrStage1XaCdDirectCdMmioSnapshotObservationSource::Unknown;
    uint32_t psxAddress = 0u;
    uint32_t byteSize = 0u;
    bool valueKnown = false;
    uint8_t value = 0u;
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

enum class PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason : uint8_t {
    None = 0,
    NonRuntimeCdMmioSampleProducer = 1,
    UnsupportedPsxAddress = 2,
    WrongByteSize = 3,
    UnknownValue = 4,
    RuntimeProviderUnavailable = 5,
};

struct PrStage1XaCdDirectCdMmioSnapshotObservationResult {
    bool accepted = false;
    PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason rejectReason =
        PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None;
};

enum class PrStage1XaCdDirectStatusFlagsRuntimeSourceBlocker80057108
    : uint8_t {
    None = 0,
    SourceUnavailable = 1,
    ValueUnknown = 2,
    ObservationRejected = 3,
};

enum class PrStage1XaCdDirectStatusCounterRuntimeSourceBlocker80057110
    : uint8_t {
    None = 0,
    SourceUnavailable = 1,
    ValueUnknown = 2,
    ObservationRejected = 3,
};

enum class PrStage1XaCdDirectCallbackPendingRuntimeSourceBlocker80055F7A
    : uint8_t {
    None = 0,
    SourceUnavailable = 1,
    ValueUnknown = 2,
    ObservationRejected = 3,
};

enum class PrStage1XaCdDirectCommandWaitLoopRuntimeSourceBlocker800375BC
    : uint8_t {
    None = 0,
    SourceUnavailable = 1,
    ResultUnknown = 2,
};

struct PrStage1XaCdDirectStatusFlagsRuntimeSource80057108 {
    bool sourceAvailable = false;
    bool valueKnown = false;
    PrStage1XaCdDirectStatusFlagsObservation80057108 observation{};
};

struct PrStage1XaCdDirectStatusCounterRuntimeSource80057110 {
    bool sourceAvailable = false;
    bool valueKnown = false;
    PrStage1XaCdDirectStatusCounterObservation80057110 observation{};
};

struct PrStage1XaCdDirectCallbackPendingRuntimeSource80055F7A {
    bool sourceAvailable = false;
    bool valueKnown = false;
    PrStage1XaCdDirectCallbackPendingObservation80055F7A observation{};
};

struct PrStage1XaCdDirectCommandWaitLoopRuntimeSource800375BC {
    bool sourceAvailable = false;
    bool resultKnown = false;
    int32_t psxReturn = -1;
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

struct PrStage1XaCdDirectCommandWaitLoopRuntimeSourceWindow800375BC {
    bool providerInstalled = false;
    bool deadlineReadAttempted = false;
    bool deadlineReadable = false;
    uint32_t deadlinePsxAddress = 0u;
    uint32_t deadlineByteSize = 0u;
    uint32_t deadlineClock = 0u;
    bool spinReadAttempted = false;
    bool spinReadable = false;
    uint32_t spinPsxAddress = 0u;
    uint32_t spinByteSize = 0u;
    uint32_t spinCount = 0u;
    bool clockKnown = false;
    int32_t clockNow = 0;
    bool resultKnown = false;
    int32_t psxReturn = -1;
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

using PrStage1XaCdDirectRuntimePsxMemoryReadFn =
    bool (*)(void* userData,
             uint32_t psxAddress,
             uint32_t byteSize,
             uint8_t* outBytes,
             size_t outSize);

struct PrStage1XaCdDirectRuntimePsxMemoryProvider {
    bool installed = false;
    PrStage1XaCdDirectRuntimePsxMemoryReadFn read = nullptr;
    void* userData = nullptr;
    bool loaderHeapSourceInstalled = false;
    bool xaCdKnownStateRelayInstalled = false;
    bool cdMmioSourceInstalled = false;
    PrStage1XaCdDirectRuntimePsxMemoryReadFn cdMmioRead = nullptr;
    void* cdMmioUserData = nullptr;
    bool exactCdSourceInstalled = false;
    PrStage1XaCdDirectRuntimePsxMemoryReadFn exactCdRead = nullptr;
    void* exactCdUserData = nullptr;
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

struct PrStage1XaCdDirectCdMmioSnapshotRuntimeSource {
    bool producerIngressInstalled = false;
    uint32_t producerObservationCallCount = 0u;
    bool cdReg3InitialKnown = false;
    uint8_t cdReg3Initial = 0u;
    bool cdReg0StatusKnown = false;
    uint8_t cdReg0Status = 0u;
    uint32_t observationAcceptedCount = 0u;
    uint32_t observationRejectedCount = 0u;
    PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason lastRejectReason =
        PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None;
};

struct PrStage1XaCdDirectCdMmioSnapshotRuntimeSourceAudit {
    bool sourceInstalled = false;
    bool readFnInstalled = false;
    bool userDataInstalled = false;
    bool snapshotSource = false;
    bool producerIngressInstalled = false;
    bool providerSubmitAvailable = false;
    uint32_t producerObservationCallCount = 0u;
    bool cdReg3InitialKnown = false;
    uint8_t cdReg3Initial = 0u;
    bool cdReg0StatusKnown = false;
    uint8_t cdReg0Status = 0u;
    bool canFeedCdReg3Initial = false;
    bool canFeedCdReg0Status = false;
    bool samplePairAvailable = false;
    uint32_t observationAcceptedCount = 0u;
    uint32_t observationRejectedCount = 0u;
    PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason lastRejectReason =
        PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None;
};

struct PrStage1XaCdDirectCdMmioRuntimeProducerSample {
    bool sourceAvailable = false;
    uint32_t psxAddress = 0u;
    uint32_t byteSize = 0u;
    bool valueKnown = false;
    uint8_t value = 0u;
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

struct PrStage1XaCdDirectCdMmioRuntimeProducerResult {
    bool attempted = false;
    bool sourceAvailable = false;
    bool valueKnown = false;
    bool exactEnvelope = false;
    bool publishAttempted = false;
    bool accepted = false;
    PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason rejectReason =
        PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None;
};

struct PrStage1XaCdDirectCdMmioRuntimeProducerPairResult {
    bool attempted = false;
    bool sourceAvailable = false;
    bool cdReg3ExactEnvelope = false;
    bool cdReg0ExactEnvelope = false;
    bool publishAttempted = false;
    bool accepted = false;
    PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason
        cdReg3RejectReason =
            PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None;
    PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason
        cdReg0RejectReason =
            PrStage1XaCdDirectCdMmioSnapshotObservationRejectReason::None;
};

bool PrStage1XaCdDirectReadKnownStateRuntimePsxMemory(
    void* userData,
    uint32_t psxAddress,
    uint32_t byteSize,
    uint8_t* outBytes,
    size_t outSize);

bool PrStage1XaCdDirectReadCdMmioSnapshotRuntimePsxMemory(
    void* userData,
    uint32_t psxAddress,
    uint32_t byteSize,
    uint8_t* outBytes,
    size_t outSize);

PrStage1XaCdDirectCdMmioSnapshotRuntimeSourceAudit
PrStage1XaCdDirectAuditCdMmioSnapshotRuntimeSource(
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider);

struct PrStage1XaCdDirectStatusFlagsRuntimeSourceWindow80057108 {
    bool providerInstalled = false;
    bool readAttempted = false;
    bool windowReadable = false;
    uint32_t psxAddress = 0u;
    uint32_t byteSize = 0u;
    bool valueKnown = false;
    uint32_t value = 0u;
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

struct PrStage1XaCdDirectStatusCounterRuntimeSourceWindow80057110 {
    bool providerInstalled = false;
    bool readAttempted = false;
    bool windowReadable = false;
    uint32_t psxAddress = 0u;
    uint32_t byteSize = 0u;
    bool valueKnown = false;
    uint32_t value = 0u;
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

struct PrStage1XaCdDirectCallbackPendingRuntimeSourceWindow80055F7A {
    bool providerInstalled = false;
    bool readAttempted = false;
    bool windowReadable = false;
    uint32_t psxAddress = 0u;
    uint32_t byteSize = 0u;
    bool valueKnown = false;
    uint16_t value = 0u;
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

struct PrStage1XaCdDirectStatusFlagsRuntimeSourceResult80057108 {
    bool sourceAvailable = false;
    bool valueKnown = false;
    bool publishAttempted = false;
    PrStage1XaCdDirectStatusFlagsRuntimeSourceBlocker80057108 blocker =
        PrStage1XaCdDirectStatusFlagsRuntimeSourceBlocker80057108::None;
    PrStage1XaCdDirectStatusFlagsObservationResult80057108 observation{};
};

struct PrStage1XaCdDirectStatusCounterRuntimeSourceResult80057110 {
    bool sourceAvailable = false;
    bool valueKnown = false;
    bool publishAttempted = false;
    PrStage1XaCdDirectStatusCounterRuntimeSourceBlocker80057110 blocker =
        PrStage1XaCdDirectStatusCounterRuntimeSourceBlocker80057110::None;
    PrStage1XaCdDirectStatusCounterObservationResult80057110 observation{};
};

struct PrStage1XaCdDirectCallbackPendingRuntimeSourceResult80055F7A {
    bool sourceAvailable = false;
    bool valueKnown = false;
    bool publishAttempted = false;
    PrStage1XaCdDirectCallbackPendingRuntimeSourceBlocker80055F7A blocker =
        PrStage1XaCdDirectCallbackPendingRuntimeSourceBlocker80055F7A::None;
    PrStage1XaCdDirectCallbackPendingObservationResult80055F7A observation{};
};

struct PrStage1XaCdDirectCommandWaitLoopRuntimeSourceResult800375BC {
    bool sourceAvailable = false;
    bool resultKnown = false;
    bool feedsCommand = false;
    int32_t psxReturn = -1;
    PrStage1XaCdDirectCommandWaitLoopRuntimeSourceBlocker800375BC blocker =
        PrStage1XaCdDirectCommandWaitLoopRuntimeSourceBlocker800375BC::None;
};

enum class PrStage1XaCdDirectRawEventRuntimeSourceBlocker80036AF8
    : uint8_t {
    None = 0,
    SourceUnavailable = 1,
    TransactionUnknown = 2,
    BridgeRejected = 3,
};

enum class PrStage1XaCdDirectRawEventInitialInterruptRuntimeSourceBlocker80036AF8
    : uint8_t {
    None = 0,
    SourceUnavailable = 1,
    ValueUnknown = 2,
    ObservationRejected = 3,
};

struct PrStage1XaCdDirectRawEventInitialInterruptRuntimeSource80036AF8 {
    bool sourceAvailable = false;
    bool valueKnown = false;
    PrStage1XaCdDirectRawEventInitialInterruptObservation80036AF8
        observation{};
};

struct PrStage1XaCdDirectRawEventInitialInterruptRuntimeSourceResult80036AF8 {
    bool sourceAvailable = false;
    bool valueKnown = false;
    bool publishAttempted = false;
    PrStage1XaCdDirectRawEventInitialInterruptRuntimeSourceBlocker80036AF8
        blocker =
            PrStage1XaCdDirectRawEventInitialInterruptRuntimeSourceBlocker80036AF8::
                None;
    PrStage1XaCdDirectRawEventInitialInterruptObservationResult80036AF8
        observation{};
};

struct PrStage1XaCdDirectRawEventRuntimeSource80036AF8 {
    bool sourceAvailable = false;
    bool transactionKnown = false;
    PrStage1LowerCdProducerDirect::RawCdRegTransactionResult80036AF8
        transaction{};
};

struct PrStage1XaCdDirectRawEventRuntimeSourceResult80036AF8 {
    bool sourceAvailable = false;
    bool transactionKnown = false;
    bool publishAttempted = false;
    bool cdMmioSubmitAttempted = false;
    bool cdMmioSubmitAccepted = false;
    bool accepted = false;
    bool lowerEventApplied = false;
    bool earlyReturnNoInterrupt = false;
    PrStage1XaCdDirectRawEventRuntimeSourceBlocker80036AF8 blocker =
        PrStage1XaCdDirectRawEventRuntimeSourceBlocker80036AF8::None;
    PrStage1XaCdDirectLowerCdSnapshotBridgeResult bridge{};
    PrStage1XaCdDirectLowerCdProducerResult applied{};
};

struct PrStage1XaCdDirectRawEventRuntimeSourceWindow80036AF8 {
    bool providerInstalled = false;
    bool cdReg3InitialReadAttempted = false;
    bool cdReg3InitialReadable = false;
    uint32_t cdReg3InitialPsxAddress = 0u;
    uint32_t cdReg3InitialByteSize = 0u;
    bool cdReg3InitialValueKnown = false;
    uint8_t cdReg3InitialValue = 0u;
    bool cdReg0StatusReadAttempted = false;
    bool cdReg0StatusReadable = false;
    uint32_t cdReg0StatusPsxAddress = 0u;
    uint32_t cdReg0StatusByteSize = 0u;
    bool cdReg0StatusValueKnown = false;
    uint8_t cdReg0StatusValue = 0u;
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

static constexpr uint32_t
    kPrStage1XaCdDirectTypedSourceGapRegisterPointers80036AF8 = 1u << 0;
static constexpr uint32_t
    kPrStage1XaCdDirectTypedSourceGapInitialInterrupt80036AF8 = 1u << 1;
static constexpr uint32_t
    kPrStage1XaCdDirectTypedSourceGapStableInterrupt80036AF8 = 1u << 2;
static constexpr uint32_t
    kPrStage1XaCdDirectTypedSourceGapCdReg0Status80036AF8 = 1u << 3;
static constexpr uint32_t
    kPrStage1XaCdDirectTypedSourceGapFifoStatusSamples80036AF8 = 1u << 4;
static constexpr uint32_t
    kPrStage1XaCdDirectTypedSourceGapResultByteCount80036AF8 = 1u << 5;
static constexpr uint32_t
    kPrStage1XaCdDirectTypedSourceGapResultBytes80036AF8 = 1u << 6;
static constexpr uint32_t
    kPrStage1XaCdDirectTypedSourceGapAckWrites80036AF8 = 1u << 7;
static constexpr uint32_t
    kPrStage1XaCdDirectTypedSourceGapPriorDword80057108 = 1u << 8;
static constexpr uint32_t
    kPrStage1XaCdDirectTypedSourceGapPriorDword80057110 = 1u << 9;
static constexpr uint32_t
    kPrStage1XaCdDirectTypedSourceGapPriorByte80057119 = 1u << 10;

enum class PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8
    : uint8_t {
    None = 0,
    RegisterPointers = 1,
    InitialInterrupt = 2,
    StableInterrupt = 3,
    CdReg0Status = 4,
    FifoStatusSamples = 5,
    ResultByteCount = 6,
    ResultBytes = 7,
    AckWrites = 8,
    PriorDword80057108 = 9,
    PriorDword80057110 = 10,
    PriorByte80057119 = 11,
};

struct PrStage1XaCdDirectRawEventTypedSourceAudit80036AF8 {
    bool inspected = false;
    bool registerPointersKnown = false;
    bool initialInterruptKnown = false;
    bool stableInterruptKnown = false;
    bool cdReg0StatusKnown = false;
    bool fifoStatusSamplesKnown = false;
    bool resultByteCountKnown = false;
    bool resultBytesKnown = false;
    bool ackWritesKnown = false;
    bool priorDword80057108Known = false;
    bool priorDword80057110Known = false;
    bool priorByte80057119Known = false;
    uint32_t missingMask = 0u;
    PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8 firstMissing =
        PrStage1XaCdDirectRawEventTypedSourceFirstMissing80036AF8::None;
    bool typedCanReconstructTransaction = false;
    bool canFeedRuntimeSource = false;
};

struct PrStage1XaCdDirectInterruptSnapshotObservation800359B8 {
    PrStage1XaCdDirectInterruptSnapshotObservationSource800359B8 source =
        PrStage1XaCdDirectInterruptSnapshotObservationSource800359B8::Unknown;
    uint32_t psxAddress = 0u;
    uint32_t byteSize = 0u;
    bool valueKnown = false;
    const uint8_t* bytes = nullptr;
    size_t bytesSize = 0u;
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

struct PrStage1XaCdDirectInterruptSnapshotObservationBundle800359B8 {
    PrStage1XaCdDirectInterruptSnapshotObservation800359B8
        callbackState80055F78{};
    PrStage1XaCdDirectInterruptSnapshotObservation800359B8
        initialInterruptRegs1F801070{};
    PrStage1XaCdDirectInterruptSnapshotObservation800359B8
        terminalInterruptRegs1F801070{};
    PrStage1XaCdDirectInterruptSnapshotObservation800359B8 watchdog80057010{};
};

struct PrStage1XaCdDirectInterruptSnapshotRuntimeSource800359B8 {
    bool sourceAvailable = false;
    bool bundleKnown = false;
    PrStage1XaCdDirectInterruptSnapshotObservationBundle800359B8 bundle{};
};

struct PrStage1XaCdDirectInterruptSnapshotRuntimeSourceWindow800359B8 {
    bool providerInstalled = false;
    bool callbackStateReadAttempted = false;
    bool callbackStateReadable = false;
    bool initialRegsReadAttempted = false;
    bool initialRegsReadable = false;
    bool terminalRegsReadAttempted = false;
    bool terminalRegsReadable = false;
    bool watchdogReadAttempted = false;
    bool watchdogReadable = false;
    bool bundleReadable = false;
    std::array<uint8_t, 0x34> callbackState80055F78Bytes{};
    std::array<uint8_t, 8> initialInterruptRegs1F801070Bytes{};
    std::array<uint8_t, 8> terminalInterruptRegs1F801070Bytes{};
    std::array<uint8_t, 4> watchdog80057010Bytes{};
    bool frameKnown = false;
    uint32_t frame = 0u;
    bool pcKnown = false;
    uint32_t pc = 0u;
};

enum class PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8
    : uint8_t {
    None = 0,
    NonRuntimePsxMemoryObservation = 1,
    WrongPsxAddress = 2,
    WrongByteSize = 3,
    UnknownValue = 4,
    MissingBytes = 5,
    IncompleteSnapshot = 6,
};

struct PrStage1XaCdDirectInterruptSnapshotObservationResult800359B8 {
    bool accepted = false;
    PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8
        rejectReason =
            PrStage1XaCdDirectInterruptSnapshotObservationRejectReason800359B8::
                None;
    PrStage1XaCdDirectLowerCdSnapshotBridgeResult bridge{};
};

struct PrStage1XaCdDirectInterruptSnapshotRuntimeSourceResult800359B8 {
    bool sourceAvailable = false;
    bool bundleKnown = false;
    bool publishAttempted = false;
    PrStage1XaCdDirectInterruptSnapshotObservationResult800359B8 observation{};
};

static constexpr uint32_t
    kPrStage1XaCdDirectTypedSourceGapCallbackStateTable800359B8 = 1u << 0;
static constexpr uint32_t
    kPrStage1XaCdDirectTypedSourceGapInitialRegs800359B8 = 1u << 1;
static constexpr uint32_t
    kPrStage1XaCdDirectTypedSourceGapTerminalRegs800359B8 = 1u << 2;
static constexpr uint32_t
    kPrStage1XaCdDirectTypedSourceGapWatchdog800359B8 = 1u << 3;

struct PrStage1XaCdDirectInterruptSnapshotTypedSourceAudit800359B8 {
    bool inspected = false;
    uint32_t callbackStatePsxAddress = 0u;
    uint32_t callbackStateByteSize = 0u;
    uint32_t callbackSlotBasePsxAddress = 0u;
    uint32_t callbackSlotByteSize = 0u;
    bool callbackSlotsKnown = false;
    bool callbackSlotsAreCallbackStateTable = false;
    bool callbackStateWordsKnown = false;
    bool callbackStateTableKnown = false;
    bool initialInterruptRegsKnown = false;
    bool terminalInterruptRegsKnown = false;
    bool watchdogKnown = false;
    uint32_t missingMask = 0u;
    bool typedCanReconstructBundle = false;
    bool canFeedRuntimeObservation = false;
};

void PrStage1XaCdDirectReset(PrStage1XaCdDirectState& state);

PrStage1XaCdDirectCallbackSlotsObservationResult
PrStage1XaCdDirectApplyCallbackSlotsRuntimeObservation800570F8FC(
    const PrStage1XaCdDirectCallbackSlotsObservation800570F8FC& observation,
    PrStage1XaCdDirectState& state);

PrStage1XaCdDirectCallbackSlotsObservationResult
PrStage1XaCdDirectPublishCallbackSlotsDiscFullbootStartupObservation800570F8FC(
    PrStage1XaCdDirectState& state);

PrStage1XaCdDirectStatusByteObservationResult
PrStage1XaCdDirectApplyStatusByteRuntimeObservation800573D4(
    const PrStage1XaCdDirectStatusByteObservation800573D4& observation,
    PrStage1XaCdDirectState& state);

PrStage1XaCdDirectStatusFlagsObservationResult80057108
PrStage1XaCdDirectApplyStatusFlagsRuntimeObservation80057108(
    const PrStage1XaCdDirectStatusFlagsObservation80057108& observation,
    PrStage1XaCdDirectState& state);

PrStage1XaCdDirectStatusCounterObservationResult80057110
PrStage1XaCdDirectApplyStatusCounterRuntimeObservation80057110(
    const PrStage1XaCdDirectStatusCounterObservation80057110& observation,
    PrStage1XaCdDirectState& state);

PrStage1XaCdDirectCallbackPendingObservationResult80055F7A
PrStage1XaCdDirectApplyCallbackPendingRuntimeObservation80055F7A(
    const PrStage1XaCdDirectCallbackPendingObservation80055F7A& observation,
    PrStage1XaCdDirectState& state);

PrStage1XaCdDirectRawEventInitialInterruptObservationResult80036AF8
PrStage1XaCdDirectApplyRawEventInitialInterruptRuntimeObservation80036AF8(
    const PrStage1XaCdDirectRawEventInitialInterruptObservation80036AF8&
        observation,
    PrStage1XaCdDirectState& state);

PrStage1XaCdDirectCdMmioSnapshotObservationResult
PrStage1XaCdDirectApplyCdMmioSnapshotRuntimeObservation(
    const PrStage1XaCdDirectCdMmioSnapshotObservation& observation,
    PrStage1XaCdDirectCdMmioSnapshotRuntimeSource& snapshot);

PrStage1XaCdDirectCdMmioSnapshotObservationResult
PrStage1XaCdDirectApplyCdMmioSnapshotRuntimeProviderObservation(
    const PrStage1XaCdDirectCdMmioSnapshotObservation& observation,
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider);

PrStage1XaCdDirectCdMmioRuntimeProducerResult
PrStage1XaCdDirectSubmitCdMmioRuntimeProducerSample(
    const PrStage1XaCdDirectCdMmioRuntimeProducerSample& sample,
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider);

PrStage1XaCdDirectCdMmioRuntimeProducerPairResult
PrStage1XaCdDirectSubmitCdMmioRuntimeProducerSamplePair(
    const PrStage1XaCdDirectCdMmioRuntimeProducerSample& cdReg3Sample,
    const PrStage1XaCdDirectCdMmioRuntimeProducerSample& cdReg0Sample,
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider);

bool PrStage1XaCdDirectSubmitCdMmioSnapshotSamplesFromRawEvent80036AF8(
    const PrStage1LowerCdProducerDirect::RawCdRegTransactionResult80036AF8&
        transaction,
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider);

PrStage1XaCdDirectStatusFlagsRuntimeSource80057108
PrStage1XaCdDirectBuildStatusFlagsRuntimeSource80057108(
    const PrStage1XaCdDirectStatusFlagsRuntimeSourceWindow80057108& window);

PrStage1XaCdDirectStatusCounterRuntimeSource80057110
PrStage1XaCdDirectBuildStatusCounterRuntimeSource80057110(
    const PrStage1XaCdDirectStatusCounterRuntimeSourceWindow80057110& window);

PrStage1XaCdDirectCallbackPendingRuntimeSource80055F7A
PrStage1XaCdDirectBuildCallbackPendingRuntimeSource80055F7A(
    const PrStage1XaCdDirectCallbackPendingRuntimeSourceWindow80055F7A& window);

PrStage1XaCdDirectStatusFlagsRuntimeSourceWindow80057108
PrStage1XaCdDirectReadStatusFlagsRuntimeSourceWindow80057108(
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider);

PrStage1XaCdDirectStatusCounterRuntimeSourceWindow80057110
PrStage1XaCdDirectReadStatusCounterRuntimeSourceWindow80057110(
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider);

PrStage1XaCdDirectCallbackPendingRuntimeSourceWindow80055F7A
PrStage1XaCdDirectReadCallbackPendingRuntimeSourceWindow80055F7A(
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider);

PrStage1XaCdDirectRawEventRuntimeSourceWindow80036AF8
PrStage1XaCdDirectReadRawEventRuntimeSourceWindow80036AF8(
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider);

PrStage1XaCdDirectRawEventInitialInterruptRuntimeSource80036AF8
PrStage1XaCdDirectBuildRawEventInitialInterruptRuntimeSource80036AF8(
    const PrStage1XaCdDirectRawEventRuntimeSourceWindow80036AF8& window);

PrStage1XaCdDirectStatusFlagsRuntimeSourceResult80057108
PrStage1XaCdDirectPublishStatusFlagsRuntimeSource80057108(
    const PrStage1XaCdDirectStatusFlagsRuntimeSource80057108& source,
    PrStage1XaCdDirectState& state);

PrStage1XaCdDirectStatusCounterRuntimeSourceResult80057110
PrStage1XaCdDirectPublishStatusCounterRuntimeSource80057110(
    const PrStage1XaCdDirectStatusCounterRuntimeSource80057110& source,
    PrStage1XaCdDirectState& state);

PrStage1XaCdDirectCallbackPendingRuntimeSourceResult80055F7A
PrStage1XaCdDirectPublishCallbackPendingRuntimeSource80055F7A(
    const PrStage1XaCdDirectCallbackPendingRuntimeSource80055F7A& source,
    PrStage1XaCdDirectState& state);

PrStage1XaCdDirectCommandWaitLoopRuntimeSourceResult800375BC
PrStage1XaCdDirectResolveCommandWaitLoopRuntimeSource800375BC(
    const PrStage1XaCdDirectCommandWaitLoopRuntimeSource800375BC& source);

PrStage1XaCdDirectCommandWaitLoopRuntimeSourceWindow800375BC
PrStage1XaCdDirectReadCommandWaitLoopRuntimeSourceWindow800375BC(
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider,
    bool clockKnown,
    int32_t clockNow);

PrStage1XaCdDirectCommandWaitLoopRuntimeSource800375BC
PrStage1XaCdDirectBuildCommandWaitLoopRuntimeSource800375BC(
    const PrStage1XaCdDirectCommandWaitLoopRuntimeSourceWindow800375BC& window);

PrStage1XaCdDirectRawEventInitialInterruptRuntimeSourceResult80036AF8
PrStage1XaCdDirectPublishRawEventInitialInterruptRuntimeSource80036AF8(
    const PrStage1XaCdDirectRawEventInitialInterruptRuntimeSource80036AF8&
        source,
    PrStage1XaCdDirectState& state);

PrStage1XaCdDirectRawEventRuntimeSourceResult80036AF8
PrStage1XaCdDirectPublishRawEventRuntimeSource80036AF8(
    const PrStage1XaCdDirectRawEventRuntimeSource80036AF8& source,
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectRuntimePsxMemoryProvider* provider = nullptr);

PrStage1XaCdDirectRawEventRuntimeSource80036AF8
PrStage1XaCdDirectBuildRawEventRuntimeSource80036AF8(
    const PrStage1XaCdDirectState& state);

PrStage1XaCdDirectRawEventTypedSourceAudit80036AF8
PrStage1XaCdDirectAuditRawEventTypedSource80036AF8(
    const PrStage1XaCdDirectState& state);

PrStage1XaCdDirectInterruptSnapshotObservationResult800359B8
PrStage1XaCdDirectApplyInterruptSnapshotRuntimeObservation800359B8(
    const PrStage1XaCdDirectInterruptSnapshotObservationBundle800359B8&
        observation,
    PrStage1XaCdDirectState& state);

PrStage1XaCdDirectInterruptSnapshotRuntimeSource800359B8
PrStage1XaCdDirectBuildInterruptSnapshotRuntimeSource800359B8(
    const PrStage1XaCdDirectInterruptSnapshotRuntimeSourceWindow800359B8&
        window);

PrStage1XaCdDirectInterruptSnapshotRuntimeSourceWindow800359B8
PrStage1XaCdDirectReadInterruptSnapshotRuntimeSourceWindow800359B8(
    const PrStage1XaCdDirectRuntimePsxMemoryProvider& provider);

PrStage1XaCdDirectInterruptSnapshotRuntimeSourceResult800359B8
PrStage1XaCdDirectPublishInterruptSnapshotRuntimeSource800359B8(
    const PrStage1XaCdDirectInterruptSnapshotRuntimeSource800359B8& source,
    PrStage1XaCdDirectState& state);

PrStage1XaCdDirectInterruptSnapshotTypedSourceAudit800359B8
PrStage1XaCdDirectAuditInterruptSnapshotTypedSource800359B8(
    const PrStage1XaCdDirectState& state);

PrStage1XaCdDirectStartResult PrStage1XaCdDirectStartStageStream(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectStartInput& input);

PrStage1XaCdDirectSetFilter13Request PrStage1XaCdDirectApplySub8001A654(
    PrStage1XaCdDirectState& state,
    uint8_t a1);

PrStage1XaCdDirectBgmVolumeRequest8001A4A4
PrStage1XaCdDirectApplySub8001A478(int16_t a1);

PrStage1XaCdDirectBgmVolumeRequest8001A4A4
PrStage1XaCdDirectApplySub8001A4A4(int a1);

PrStage1XaCdDirectCallbackResult PrStage1XaCdDirectApplySub80039240PumpCallback(
    PrStage1XaCdDirectState& state);

PrStage1XaCdDirectCallbackResult PrStage1XaCdDirectApplySub80039318DmaCallback(
    PrStage1XaCdDirectState& state);

PrMovieSegmentDirect::StreamClockCallbackResult8001A210
PrStage1XaCdDirectApplySub8001A210ClockCallback(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectClockCallbackInput8001A210& input);

PrStage1XaCdDirectCallbackRegisterResult80036510
PrStage1XaCdDirectApplySub80036510SetCdCallback(
    PrStage1XaCdDirectState& state,
    uint32_t callbackAddr,
    uint32_t sourceFunction = PrMovieSegmentDirect::kSub80036510SetCdCallback);

PrStage1XaCdDirectReadyCallbackRegisterResult80036528
PrStage1XaCdDirectApplySub80036528SetCdReadyCallback(
    PrStage1XaCdDirectState& state,
    uint32_t callbackAddr,
    uint32_t sourceFunction =
        PrMovieSegmentDirect::kSub80036528SetCdReadyCallback);

PrStage1XaCdDirectCallbackStateTableSetResult80035BA0
PrStage1XaCdDirectApplySub80035BA0SetCdInterruptCallbackTable(
    PrStage1XaCdDirectState& state,
    uint32_t callbackIndex,
    bool callbackAddrKnown,
    uint32_t callbackAddr,
    uint32_t sourceFunction =
        PrMovieSegmentDirect::kSub80035BA0SetCdInterruptCallbackTable);

PrStage1XaCdDirectCallbackRegisterResult8001A258
PrStage1XaCdDirectApplySub8001A258StreamClockCallbackRegister(
    PrStage1XaCdDirectState& state);

PrStage1XaCdDirectClearCallbackResult8001A694
PrStage1XaCdDirectApplySub8001A694ClearCdCallback(
    PrStage1XaCdDirectState& state);

// Suspending 8001A694 -> 800367A4(8,0,80049414). The lower owner must
// execute each requested call. Missing feedback is never completion.
enum class PrStage1CdStopPhase8001A694 : uint8_t {
    Idle, BeginAttempt, StatusCommand, StopCommand, Sync, Complete
};
struct PrStage1CdStopRequest8001A694 {
    uint64_t serial = 0;
    uint32_t function = 0;
    std::array<uint32_t, 4> args{};
};
struct PrStage1CdStopFeedback8001A694 {
    bool known = false;
    PrStage1CdStopRequest8001A694 request{};
    int32_t result = 0;
};
struct PrStage1CdStopRuntime8001A694 {
    PrStage1CdStopPhase8001A694 phase = PrStage1CdStopPhase8001A694::Idle;
    uint32_t savedCallback800570F8 = 0;
    uint32_t attempts800367A4 = 0;
    uint32_t calls800367A4 = 0;
    uint64_t requestSerial = 0;
    PrStage1CdStopRequest8001A694 request{};
    bool returnKnown = false;
    int32_t result = 0;
};
struct PrStage1CdStopStep8001A694 {
    bool requestIssued = false;
    bool feedbackConsumed = false;
    bool feedbackRejected = false;
    bool waitingForSource = false;
    bool complete = false;
};
PrStage1CdStopStep8001A694 PrStage1XaCdDirectAdvanceStop8001A694(
    PrStage1CdStopRuntime8001A694& runtime,
    PrStage1XaCdDirectState& state,
    const PrStage1CdStopFeedback8001A694& feedback = {});

void PrStage1XaCdDirectDispatchCommandCallbacks800375BC(PrStage1XaCdDirectState& state);
PrMovieSegmentDirect::CdSyncResult80037070 PrStage1XaCdDirectApplySub80037070(
    PrStage1XaCdDirectState& state, const PrStage1XaCdDirectCdSyncInput80037070& input);

PrMovieSegmentDirect::StreamClockPollResult8001A3C8
PrStage1XaCdDirectApplySub8001A3C8ClockPoll(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectClockPollInput8001A3C8& input);

PrMovieSegmentDirect::StreamClockResetResult8001A724
PrStage1XaCdDirectApplySub8001A724ClockReset(
    PrStage1XaCdDirectState& state,
    int32_t a1);

PrMovieSegmentDirect::CdReadyStatusResult800363A4
PrStage1XaCdDirectApplySub800363A4CdReadyStatus(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectCdReadyStatusInput800363A4& input);

PrMovieSegmentDirect::CdSyncResult80037070
PrStage1XaCdDirectApplySub800364D0CdSync(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectCdSyncInput80037070& input);

PrMovieSegmentDirect::CdSyncResult80037070
PrStage1XaCdDirectApplySub800364D0CdSyncFromLowerState(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectCdSyncFromLowerStateInput800364D0& input);

PrStage1XaCdDirectStageRecordTickResult8001A4D0
PrStage1XaCdDirectApplySub8001A4D0StageRecordTick(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectStartInput& input);

PrMovieSegmentDirect::CdCallbackEventResult80036AF8
PrStage1XaCdDirectApplySub80036AF8CdLowerEvent(
    PrStage1XaCdDirectState& state,
    const PrMovieSegmentDirect::CdCallbackEventInput80036AF8& input);

PrMovieSegmentDirect::CheckCallbackResult80035898
PrStage1XaCdDirectApplySub80035898CheckCallback(
    PrStage1XaCdDirectState& state,
    const PrMovieSegmentDirect::CheckCallbackInput80035898& input);
PrMovieSegmentDirect::CdCallbackPendingProducerResult800359B8
PrStage1XaCdDirectApplySub800359B8CdCallbackPendingProducer(
    PrStage1XaCdDirectState& state,
    const PrMovieSegmentDirect::CdCallbackPendingProducerInput800359B8& input);

PrStage1XaCdDirectLowerCdProducerResult
PrStage1XaCdDirectApplyLowerCdProducerSnapshot(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectLowerCdProducerSnapshot& snapshot);
PrStage1XaCdDirectLowerCdSnapshotBridgeResult
PrStage1XaCdDirectBuildLowerCdProducerSnapshot(
    const PrStage1XaCdDirectLowerCdSnapshotBridgeInput& input);

PrStage1XaCdDirectCommandResult800375BC
PrStage1XaCdDirectApplySub800375BCCommand(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectCommandInput800375BC& input);

void PrStage1XaCdDirectPrimeSub800375BCTimeoutState(
    PrStage1XaCdDirectState& state,
    int32_t clockNow);

PrStage1XaCdDirectInitResult8001A280
PrStage1XaCdDirectApplySub8001A280WorkBaseCommand(
    PrStage1XaCdDirectState& state);

PrMovieSegmentDirect::StreamClockPollResult8001A3C8
PrStage1XaCdDirectApplySub8001A3C8ClockPollFromLowerState(
    PrStage1XaCdDirectState& state);

PrMovieSegmentDirect::StreamStatusPollResult8001A750
PrStage1XaCdDirectApplySub8001A750StatusPollFromLowerState(
    PrStage1XaCdDirectState& state);

PrStage1XaCdDirectStreamClockProbe800493F4
PrStage1XaCdDirectProbeStreamClockProducer800493F4(
    const PrStage1XaCdDirectState& state);

PrStage1XaCdDirectHalGetlocPFactsResult
PrStage1XaCdDirectApplyHalGetlocPFacts(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectHalGetlocPFactsInput& input);

void PrStage1XaCdDirectClearRing(PrStage1XaCdDirectState& state,
                                 uint32_t slotCount = 32u);

bool PrStage1XaCdDirectIsRingPacketCandidate(
    const PrStage1XaCdDirectRingPacketInput& packet);

PrStage1XaCdDirectRingPumpResult PrStage1XaCdDirectApplySub80039670Packet(
    PrStage1XaCdDirectState& state,
    const PrStage1XaCdDirectRingPacketInput& packet);

bool PrStage1XaCdDirectApplySub8003958CAcquireFrame(
    PrStage1XaCdDirectState& state,
    PrStage1XaCdDirectRingFrameView& out);

bool PrStage1XaCdDirectApplySub80039490ReleaseFrame(
    PrStage1XaCdDirectState& state,
    uint32_t frameHandle);
