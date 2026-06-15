#pragma once

#include "pr_movie_segment_direct.h"
#include "pr_stage1_loader_direct.h"
#include "pr_stage1_lower_cd_producer_direct.h"

#include <array>
#include <cstdint>
#include <cstddef>
#include <vector>

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
    bool dword_8005710CKnown = false;
    uint32_t dword_8005710C = 0u;
    bool dword_80057110Known = false;
    uint32_t dword_80057110 = 0u;
    bool cdCommandArgs800375BCKnown = false;
    uint32_t cdCommandArgCount800375BC = 0u;
    std::array<uint8_t, 16> cdCommandArgs800375BC{};
    int32_t cdCommandA3_800375BC = 0;
    bool cdCommandSkipWait800375BC = true;
    bool byte_800573D4Known = false;
    uint8_t byte_800573D4 = 0;
    bool byte_800573D5Known = false;
    uint8_t byte_800573D5 = 0;
    bool byte_800573D6Known = false;
    uint8_t byte_800573D6 = 0;
    bool cdSyncExplicitStatusKnown = false;
    uint8_t cdSyncExplicitStatus = 0;
    bool cdSyncExplicitResponseBytesKnown = false;
    std::array<uint8_t, 8> cdSyncExplicitResponseBytes{};
    uint32_t halGetlocPFactsApplyCount = 0u;
    bool lastHalGetlocPSectorIndexKnown = false;
    uint32_t lastHalGetlocPSectorIndex = 0u;
    bool response_800882F8Known = false;
    std::array<uint8_t, 8> response_800882F8{};
    bool response_80088300Known = false;
    std::array<uint8_t, 8> response_80088300{};
    bool response_80088308Known = false;
    std::array<uint8_t, 8> response_80088308{};
    bool word_80055F78Known = false;
    uint16_t word_80055F78 = 0;
    bool word_80055F7AKnown = false;
    uint16_t word_80055F7A = 0;
    bool word_80055FA8Known = false;
    uint16_t word_80055FA8 = 0;
    bool dword_80057010Known = false;
    uint32_t dword_80057010 = 0u;
    bool cdCallbackPending80035898Known = false;
    bool cdCallbackPending80035898 = false;
    uint32_t cdCallbackPending80035898GapCount = 0u;
    uint32_t cdCallbackPending800359B8WriteCount = 0u;
    uint32_t cdCallbackPending800359B8GapCount = 0u;
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
    bool dword800570F8Known = false;
    uint32_t dword800570F8 = 0u;
    bool streamClockCallback8001A210Registered = false;
    bool callbackCleared = false;
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
    PrMovieSegmentDirect::CdSyncLowerFeedback80037070 preSyncFeedback{};
    PrMovieSegmentDirect::CheckCallbackInput80035898 checkCallback{};
    PrMovieSegmentDirect::CdCallbackEventInput80036AF8 callbackEvent{};
};

struct PrStage1XaCdDirectCommandResult800375BC {
    bool called = false;
    uint32_t sourceFunction = PrMovieSegmentDirect::kSub800375BCCdCommand;
    int32_t psxReturn = 0;
    uint8_t byte80057119 = 0;
    bool byte80057119Known = false;
    bool preSyncCalled = false;
    PrMovieSegmentDirect::CdSyncResult80037070 preSyncResult{};
    bool waitLoopRequested = false;
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

void PrStage1XaCdDirectReset(PrStage1XaCdDirectState& state);

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

PrStage1XaCdDirectCallbackRegisterResult8001A258
PrStage1XaCdDirectApplySub8001A258StreamClockCallbackRegister(
    PrStage1XaCdDirectState& state);

PrStage1XaCdDirectClearCallbackResult8001A694
PrStage1XaCdDirectApplySub8001A694ClearCdCallback(
    PrStage1XaCdDirectState& state);

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
