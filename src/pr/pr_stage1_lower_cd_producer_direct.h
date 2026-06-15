#pragma once

#include "pr_movie_segment_direct.h"
#include "pr_stage1_loader_direct.h"

#include <cstdint>
#include <cstddef>

namespace PrStage1LowerCdProducerDirect {

static constexpr uint32_t kIStatPtr800359B8 = 0x1F801070u;
static constexpr uint32_t kIMaskPtr800359B8 = 0x1F801074u;
static constexpr uint32_t kCdCallbackTableBase800359B8 = 0x80055F7Cu;
static constexpr uint32_t kCdCallbackCount800359B8 = 11u;
static constexpr uint32_t kCdReg0Ptr80036AF8 = 0x1F801800u;
static constexpr uint32_t kCdReg1Ptr80036AF8 = 0x1F801801u;
static constexpr uint32_t kCdReg2Ptr80036AF8 = 0x1F801802u;
static constexpr uint32_t kCdReg3Ptr80036AF8 = 0x1F801803u;
static constexpr uint32_t kCdInterruptSnapshotMaxSamples800359B8 = 12u;

struct RawCdRegPriorFacts80036AF8 {
    bool dword80057108Known = false;
    uint32_t dword80057108 = 0u;
    bool dword80057110Known = false;
    uint32_t dword80057110 = 0u;
    bool byte80057119Known = false;
    uint8_t byte80057119 = 0;
};

struct CdRegisterPointerSnapshot80036AF8 {
    bool cdReg0PtrKnown = false;
    uint32_t cdReg0Ptr = kCdReg0Ptr80036AF8;
    bool cdReg1PtrKnown = false;
    uint32_t cdReg1Ptr = kCdReg1Ptr80036AF8;
    bool cdReg2PtrKnown = false;
    uint32_t cdReg2Ptr = kCdReg2Ptr80036AF8;
    bool cdReg3PtrKnown = false;
    uint32_t cdReg3Ptr = kCdReg3Ptr80036AF8;
    bool selectorWriteKnown = false;
    uint8_t selectorWriteValue = 1;
};

struct CdRegisterPointerSnapshotResult80036AF8 {
    bool produced = false;
    bool incomplete = false;
    bool pointersMatched = false;
    bool selectorWriteMatched = false;
};

struct RawCdRegTransactionInput80036AF8 {
    CdRegisterPointerSnapshot80036AF8 registerPointers{};
    bool cdReg3InitialInterruptKnown = false;
    uint8_t cdReg3InitialInterrupt = 0;
    bool cdReg3StableInterruptKnown = false;
    uint8_t cdReg3StableInterrupt = 0;
    bool cdReg0StatusKnown = false;
    uint8_t cdReg0Status = 0;
    bool cdReg0FifoStatusSamplesKnown = false;
    uint32_t cdReg0FifoStatusSampleCount = 0u;
    uint8_t cdReg0FifoStatusSamples[9]{};
    bool fifoDrainKnown = false;
    uint32_t fifoDrainCount = 0u;
    bool fifoDrained = false;
    bool resultByteCountKnown = false;
    uint32_t resultByteCount = 0u;
    bool resultBytesKnown = false;
    uint8_t resultBytes[8]{};
    bool ackWritesKnown = false;
    uint8_t ackCdReg0 = 1;
    uint8_t ackCdReg3 = 7;
    uint8_t ackCdReg2 = 7;
    bool case1ClearWritesKnown = false;
    uint8_t case1ClearCdReg0 = 0;
    uint8_t case1ClearCdReg3 = 0;
    RawCdRegPriorFacts80036AF8 priorFacts{};
};

struct RawCdRegTransactionResult80036AF8 {
    bool produced = false;
    bool incomplete = false;
    CdRegisterPointerSnapshotResult80036AF8 registerPointers{};
    uint8_t cdReg3InitialInterrupt = 0;
    uint8_t cdReg3StableInterrupt = 0;
    bool cdReg0StatusKnown = false;
    uint8_t cdReg0Status = 0;
    bool cdReg0FifoStatusSamplesKnown = false;
    uint32_t cdReg0FifoStatusSampleCount = 0u;
    uint8_t cdReg0FifoStatusSamples[9]{};
    bool fifoDrainRequired = false;
    bool fifoDrained = false;
    uint32_t fifoDrainCount = 0u;
    bool fifoStoppedOnNotReady = false;
    bool fifoStoppedAtLimit = false;
    bool resultBytesKnown = false;
    uint32_t resultByteCount = 0u;
    uint8_t resultBytes[8]{};
    bool ackWritesMatched80036AF8 = false;
    bool case1ClearMatched80036AF8 = false;
    bool earlyReturnNoInterrupt = false;
    bool unknownInterrupt = false;
    bool psxReturnKnown = false;
    int32_t psxReturn = 0;
    bool dword80057108WrittenKnown = false;
    uint32_t dword80057108Written = 0u;
    bool dword8005710CWrittenKnown = false;
    uint32_t dword8005710CWritten = 0u;
    bool dword80057110WrittenKnown = false;
    uint32_t dword80057110Written = 0u;
    bool dword80057110Incremented = false;
    bool byte800573D4Known = false;
    uint8_t byte800573D4 = 0;
    bool byte800573D5Known = false;
    uint8_t byte800573D5 = 0;
    bool byte800573D6Known = false;
    uint8_t byte800573D6 = 0;
    bool response882F8Known = false;
    uint8_t response882F8[8]{};
    bool response88300Known = false;
    uint8_t response88300[8]{};
    bool response88308Known = false;
    uint8_t response88308[8]{};
    RawCdRegPriorFacts80036AF8 priorFacts{};
};

struct CdInterruptSnapshotInput800359B8 {
    bool iStatPtrKnown = false;
    uint32_t iStatPtr = kIStatPtr800359B8;
    bool iMaskPtrKnown = false;
    uint32_t iMaskPtr = kIMaskPtr800359B8;
    bool iStatKnown = false;
    uint16_t iStat = 0;
    bool iMaskKnown = false;
    uint16_t iMask = 0;
    bool word80055FA8Known = false;
    uint16_t word80055FA8 = 0;
    bool word80055F78Known = false;
    uint16_t word80055F78 = 0;
    bool word80055F7ABeforeKnown = false;
    uint16_t word80055F7ABefore = 0;
    bool word80055F7ASetWriteKnown = false;
    uint16_t word80055F7ASetWrite = 1;
    bool word80055F7AClearWriteKnown = false;
    uint16_t word80055F7AClearWrite = 0;
    bool watchdogKnown = false;
    uint32_t dword80057010 = 0u;
    bool pendingSampleSequenceKnown = false;
    uint32_t pendingSampleCount = 0u;
    uint16_t iStatSamples[kCdInterruptSnapshotMaxSamples800359B8]{};
    uint16_t iMaskSamples[kCdInterruptSnapshotMaxSamples800359B8]{};
    uint16_t word80055FA8Samples[kCdInterruptSnapshotMaxSamples800359B8]{};
    bool callbackTableKnown = false;
    bool callbackTableBaseKnown = false;
    uint32_t callbackTableBase = kCdCallbackTableBase800359B8;
    bool callbackPresent[kCdCallbackCount800359B8]{};
    bool callbackAddressKnown[kCdCallbackCount800359B8]{};
    uint32_t callbackAddress[kCdCallbackCount800359B8]{};
};

struct CdInterruptSnapshotResult800359B8 {
    bool produced = false;
    bool incomplete = false;
    bool pointerSnapshotMatched = false;
    bool pendingMaskKnown = false;
    uint16_t pendingMask = 0;
    bool word80055F7ASetWriteMatched = false;
    bool word80055F7AClearWriteMatched = false;
    bool returnFromExceptionBeforePending = false;
    uint32_t pendingSampleCount = 0u;
    uint16_t pendingMaskSamples[kCdInterruptSnapshotMaxSamples800359B8]{};
    uint32_t ackCount = 0u;
    uint8_t ackBitIndex[kCdCallbackCount800359B8]{};
    uint16_t ackWriteValue[kCdCallbackCount800359B8]{};
    uint32_t dispatchCount = 0u;
    uint8_t dispatchBitIndex[kCdCallbackCount800359B8]{};
    uint32_t dispatchedCallbackAddress[kCdCallbackCount800359B8]{};
    bool callbackTableComplete = false;
    bool watchdogKnown = false;
    uint32_t dword80057010 = 0u;
    bool watchdogWritten = false;
    uint32_t dword80057010Written = 0u;
    bool watchdogTimeout = false;
    bool interruptStatusClearedOnTimeout = false;
};

struct CdCallbackPendingBridgeResult800359B8 {
    bool produced = false;
    bool incomplete = false;
    PrMovieSegmentDirect::CdCallbackPendingProducerInput800359B8 input{};
};

struct CdCallbackEventRegisterBridgeResult80036AF8 {
    bool produced = false;
    bool incomplete = false;
    bool earlyReturnNoInterrupt = false;
    PrMovieSegmentDirect::CdCallbackEventRegisterInput80036AF8 input{};
};

struct CdSyncLowerFeedbackInput80037070 {
    bool timedOutKnown = false;
    bool timedOut = false;
    bool syncResultKnown = false;
    int32_t syncResult = 0;
    bool responseBytesKnown = false;
    uint32_t responseByteCount = 0u;
    uint8_t responseBytes[8]{};
    bool rawEvent80036AF8Known = false;
    RawCdRegTransactionResult80036AF8 rawEvent80036AF8{};
};

struct CdSyncLoopFactsInput80037070 {
    bool a0WaitModeKnown = false;
    int32_t a0WaitMode = 0;
    bool timeoutKnown = false;
    bool timedOut = false;
    bool checkCallbackKnown = false;
    bool callbackPending = false;
    bool savedCdReg0SelectorKnown = false;
    uint8_t savedCdReg0Selector = 0;
    bool rawPumpSequenceKnown = false;
    uint32_t rawPumpCount = 0u;
    RawCdRegTransactionResult80036AF8 rawPump[16]{};
    int32_t rawPumpPsxReturns[16]{};
    bool callbackPumpDrained = false;
    bool selectorRestoredKnown = false;
    bool selectorRestored = false;
    uint8_t restoredCdReg0Selector = 0;
    bool byte800573D4Known = false;
    uint8_t byte800573D4 = 0;
    bool response882F8Known = false;
    uint32_t response882F8ByteCount = 0u;
    uint8_t response882F8[8]{};
};

struct CdSyncLoopFactsResult80037070 {
    bool produced = false;
    bool incomplete = false;
    bool timedOut = false;
    bool checkCallbackKnown = false;
    bool callbackPending = false;
    bool callbackPumpRequired = false;
    bool callbackPumpDrained = false;
    bool savedCdReg0SelectorKnown = false;
    uint8_t savedCdReg0Selector = 0;
    bool selectorRestored = false;
    uint8_t restoredCdReg0Selector = 0;
    bool noPendingReturnZero = false;
    bool nonblockingReturnZero = false;
    bool coreFactsKnown = false;
    CdSyncLowerFeedbackInput80037070 coreFacts{};
};

struct CdSyncLowerFeedbackResult80037070 {
    bool produced = false;
    bool incomplete = false;
    bool timedOut = false;
    bool rawEvent80036AF8Consumed = false;
    PrMovieSegmentDirect::CdCallbackEventRegisterResult80036AF8
        eventRegister80036AF8{};
    PrMovieSegmentDirect::CdCallbackEventResult80036AF8 event80036AF8{};
    PrMovieSegmentDirect::CdSyncLowerFeedback80037070 feedback{};
};

struct CommandInput800375BC {
    uint8_t command = 0;
    bool argsKnown = false;
    uint32_t argCount = 0u;
    const uint8_t* args = nullptr;
    bool requiredArgCountKnown = false;
    uint32_t requiredArgCount = 0u;
    bool clearsByte800573D5 = false;
    bool preSyncResultKnown = false;
    int32_t preSyncPsxReturn = 0;
    bool skipWait = false;
    bool clockKnown = false;
    int32_t clockNow = 0;
    bool timeoutKnown = false;
    bool timedOut = false;
    bool checkCallbackKnown = false;
    bool callbackPending = false;
    bool savedCdReg0SelectorKnown = false;
    uint8_t savedCdReg0Selector = 0;
    bool callbackPumpKnown = false;
    uint32_t callbackPumpCount = 0u;
    bool callbackPumpDrained = false;
    int32_t lastCallbackPumpReturn = 0;
    bool selectorRestored = false;
    bool rawCallbackTransactionKnown = false;
    RawCdRegTransactionResult80036AF8 rawCallbackTransaction{};
    bool rawCallbackTransactionSequenceKnown = false;
    uint32_t rawCallbackTransactionCount = 0u;
    RawCdRegTransactionResult80036AF8 rawCallbackTransactions[16]{};
    int32_t rawCallbackPsxReturns[16]{};
    bool waitLoopResultKnown = false;
    int32_t waitLoopPsxReturn = -1;
};

struct CommandAttr800375BC {
    bool known = false;
    uint8_t command = 0;
    uint8_t paramCount = 0;
    bool resetReadyByte573D5 = false;
    bool mirrorsSetlocTo57114 = false;
};

struct CommandResult800375BC {
    bool produced = false;
    bool incomplete = false;
    int32_t psxReturn = 0;
    uint8_t command = 0;
    bool byte80057119Known = false;
    uint8_t byte80057119 = 0;
    bool byte800573D4Known = false;
    uint8_t byte800573D4 = 0;
    bool byte800573D5Known = false;
    uint8_t byte800573D5 = 0;
    bool setlocArgsKnown = false;
    uint8_t setlocArgs[4]{};
    bool cdRegisterWritesKnown = false;
    uint32_t cdRegisterArgWriteCount = 0u;
    uint8_t cdRegisterArgWrites[16]{};
    bool preSyncResultKnown = false;
    int32_t preSyncPsxReturn = 0;
    bool waitLoopRequested = false;
    bool timeoutDeadlineKnown = false;
    int32_t timeoutStartClock = 0;
    int32_t timeoutDeadlineClock = 0;
    uint32_t timeoutSpinLimit = 0u;
    bool timedOut = false;
    bool checkCallbackKnown = false;
    bool callbackPending = false;
    bool callbackPumpRequired = false;
    bool savedCdReg0SelectorKnown = false;
    uint8_t savedCdReg0Selector = 0;
    bool callbackPumpKnown = false;
    uint32_t callbackPumpCount = 0u;
    bool callbackPumpDrained = false;
    int32_t lastCallbackPumpReturn = 0;
    bool selectorRestored = false;
    bool rawCallbackTransactionKnown = false;
    RawCdRegTransactionResult80036AF8 rawCallbackTransaction{};
    bool rawCallbackTransactionSequenceKnown = false;
    uint32_t rawCallbackTransactionCount = 0u;
    RawCdRegTransactionResult80036AF8 rawCallbackTransactions[16]{};
    int32_t rawCallbackPsxReturns[16]{};
};

struct CommandWrapperAttempt80036540 {
    bool restartCommand1Known = false;
    CommandResult800375BC restartCommand1{};
    bool preSetlocKnown = false;
    CommandResult800375BC preSetloc{};
    bool commandKnown = false;
    CommandResult800375BC command{};
};

struct CommandWrapperInput80036540 {
    uint8_t command = 0;
    bool argsKnown = false;
    bool argsPresent = false;
    bool status57108Known = false;
    uint8_t status57108 = 0;
    bool commandNeedsSetlocKnown = false;
    bool commandNeedsSetloc = false;
    CommandWrapperAttempt80036540 attempts[4]{};
};

struct CommandWrapperResult80036540 {
    bool produced = false;
    bool incomplete = false;
    int32_t psxReturn = 0;
    uint32_t attemptsUsed = 0u;
    bool dword800570F8Cleared = false;
    bool issuedCommand1 = false;
    bool issuedSetloc = false;
    bool finalSkipWait = false;
    uint32_t clearedCallbackCount = 0u;
};

using CommandWrapperInput80036678 = CommandWrapperInput80036540;
using CommandWrapperResult80036678 = CommandWrapperResult80036540;

struct ReadyWaitInput800372F0 {
    bool a0WaitModeKnown = false;
    int32_t a0WaitMode = 0;
    bool a1OutputBufferPtrKnown = false;
    bool a1OutputBufferPtrNonNull = false;
    bool timeoutCheckKnown = false;
    bool timedOut = false;
    bool callbackCheckKnown = false;
    bool callbackPending = false;
    bool callbackPumpDrainedKnown = false;
    bool callbackPumpDrained = false;
    bool rawCallbackTransactionKnown = false;
    RawCdRegTransactionResult80036AF8 rawCallbackTransaction{};
    bool rawCallbackTransactionSequenceKnown = false;
    uint32_t rawCallbackTransactionCount = 0u;
    RawCdRegTransactionResult80036AF8 rawCallbackTransactions[16]{};
    int32_t rawCallbackPsxReturns[16]{};
    bool callbackMaskKnown = false;
    uint32_t callbackMask = 0u;
    bool readyCallbackRegistered = false;
    bool syncCallbackRegistered = false;
    bool byte800573D6Known = false;
    uint8_t byte800573D6 = 0;
    bool response88308Known = false;
    uint8_t response88308[8]{};
    bool byte800573D5Known = false;
    uint8_t byte800573D5 = 0;
    bool response88300Known = false;
    uint8_t response88300[8]{};
};

struct ReadyWaitResult800372F0 {
    bool produced = false;
    bool incomplete = false;
    int32_t psxReturn = 0;
    bool a0WaitModeKnown = false;
    int32_t a0WaitMode = 0;
    bool a1OutputBufferPtrKnown = false;
    bool a1OutputBufferPtrNonNull = false;
    bool timeoutChecked = false;
    bool timedOut = false;
    bool callbackCheckKnown = false;
    bool callbackPending = false;
    bool callbackPumpRequired = false;
    bool callbackPumpDrained = false;
    bool rawCallbackTransactionKnown = false;
    RawCdRegTransactionResult80036AF8 rawCallbackTransaction{};
    bool rawCallbackTransactionSequenceKnown = false;
    uint32_t rawCallbackTransactionCount = 0u;
    RawCdRegTransactionResult80036AF8 rawCallbackTransactions[16]{};
    int32_t rawCallbackPsxReturns[16]{};
    bool readyCallbackDispatch = false;
    bool syncCallbackDispatch = false;
    bool byte800573D6Consumed = false;
    bool byte800573D5Consumed = false;
    bool outputBufferWritten = false;
    uint8_t outputBuffer[8]{};
};

struct ReadyWaitReadyBankFacts800372F0 {
    bool incomplete = false;
    bool byte800573D6Known = false;
    uint8_t byte800573D6 = 0;
    bool response88308Known = false;
    uint8_t response88308[8]{};
    bool byte800573D5Known = false;
    uint8_t byte800573D5 = 0;
    bool response88300Known = false;
    uint8_t response88300[8]{};
};

struct CallbackSwapInput80036510 {
    bool oldCallbackKnown = false;
    uint32_t oldCallback = 0u;
    uint32_t newCallback = 0u;
};

struct CallbackSwapResult80036510 {
    bool produced = false;
    uint32_t oldCallback = 0u;
    uint32_t newCallback = 0u;
};

using CallbackSwapInput80036528 = CallbackSwapInput80036510;
using CallbackSwapResult80036528 = CallbackSwapResult80036510;

struct StatusReadInput80036384 {
    bool statusKnown = false;
    uint32_t status = 0u;
};

struct StatusReadResult80036384 {
    bool produced = false;
    bool incomplete = false;
    uint32_t status = 0u;
};

struct LocSectorInput80036A78 {
    bool locBytesKnown = false;
    uint8_t locBytes[3]{};
};

struct LocSectorResult80036A78 {
    bool produced = false;
    bool incomplete = false;
    int32_t sector = 0;
};

struct ReadPumpInput80038DE8 {
    bool retry = false;
    CallbackSwapResult80036510 clearSyncCallback{};
    CallbackSwapResult80036528 clearReadyCallback{};
    StatusReadResult80036384 status{};
    bool clockKnown = false;
    int32_t clockNow = 0;
    bool shellOpenCommandResultKnown = false;
    CommandWrapperResult80036678 shellOpenCommandResult{};
    bool modeKnown = false;
    uint32_t modeWord8005741C = 0u;
    bool retryPauseResultKnown = false;
    CommandWrapperResult80036540 retryPauseResult{};
    CommandWrapperResult80036540 retrySetlocResult{};
    bool preReadSetupKnown = false;
    CommandWrapperResult80036540 modeCommandResult{};
    bool startReadResultKnown = false;
    CommandWrapperResult80036678 startReadResult{};
    bool locSectorKnown = false;
    int32_t locSector80057430 = 0;
    bool activeDstKnown = false;
    int32_t activeDst80057414 = 0;
    bool activeSectorCountKnown = false;
    int32_t activeSectorCount80057410 = 0;
};

struct ReadPumpResult80038DE8 {
    bool produced = false;
    bool incomplete = false;
    int32_t psxReturn = 0;
    bool clearedSyncCallback80036510 = false;
    bool clearedReadyCallback80036528 = false;
    bool shellOpen = false;
    bool shellOpenCommandIssued80036678_1 = false;
    bool retryIssued = false;
    bool preReadSetupApplied80036430 = false;
    bool modeCommandIssued = false;
    uint8_t modeCommandArg0 = 0;
    bool callbackInstalled80038BC4 = false;
    bool globalsKnown = false;
    int32_t remaining80057424 = 0;
    int32_t startClock8005742C = 0;
    int32_t lastPumpClock80057428 = 0;
    int32_t activeDst80057418 = 0;
    int32_t activeSectorCount80057410 = 0;
    int32_t expectedSector80057430 = 0;
};

struct Result {
    bool produced = false;
    bool incomplete = false;
    PrStage1LoaderDirect::CdSeamResult cd{};
};

struct ReadStartSetupInput80038FC0 {
    bool sectorCountKnown = false;
    int32_t sectorCount = 0;
    bool dstKnown = false;
    int32_t dst = 0;
    bool modeKnown = false;
    uint32_t mode = 0u;
    CallbackSwapResult80036510 savedSyncCallback{};
    CallbackSwapResult80036528 savedReadyCallback{};
    StatusReadResult80036384 status{};
    bool clockKnown = false;
    int32_t clockNow = 0;
    Result preSeekResult800367A4{};
};

struct ReadStartSetupResult80038FC0 {
    bool produced = false;
    bool incomplete = false;
    uint32_t modeWord8005741C = 0u;
    int32_t sectorSize80057420 = 0;
    int32_t dst80057414 = 0;
    int32_t sectorCount80057410 = 0;
    uint32_t savedSyncCallback80057434 = 0u;
    uint32_t savedReadyCallback80057438 = 0u;
    int32_t startClock8005742C = 0;
    bool preSeekRequested = false;
    bool preSeekApplied = false;
};

struct ReadCallbackInput80038BC4 {
    bool callbackEventKnown = false;
    uint8_t callbackEvent = 0;
    bool callbackResultPtrKnown = false;
    uint32_t callbackResultPtr = 0u;
    bool resultBytesKnown = false;
    uint8_t resultBytes[3]{};
    bool tickNowKnown = false;
    int32_t tickNow = 0;
    bool sectorSizeKnown = false;
    int32_t sectorSize80057420 = 0;
    bool remainingKnown = false;
    int32_t remaining80057424 = 0;
    bool activeDstKnown = false;
    int32_t activeDst80057418 = 0;
    bool expectedSectorKnown = false;
    int32_t expectedSector80057430 = 0;
    bool startClockKnown = false;
    int32_t startClock8005742C = 0;
    bool savedSyncCallbackKnown = false;
    uint32_t savedSyncCallback80057434 = 0u;
    bool savedReadyCallbackKnown = false;
    uint32_t savedReadyCallback80057438 = 0u;
    bool userCallbackKnown = false;
    uint32_t userCallback8005740C = 0u;
    ReadPumpResult80038DE8 retryPumpResult{};
    CommandWrapperResult80036540 pauseCommandResult{};
};

struct ReadCallbackResult80038BC4 {
    bool produced = false;
    bool incomplete = false;
    bool copiedSectorPayload = false;
    bool msfCheckPerformed = false;
    bool msfLbaKnown = false;
    int32_t msfLba = 0;
    bool sectorMismatch = false;
    int32_t remainingAfter = 0;
    int32_t dstAfter = 0;
    int32_t expectedSectorAfter = 0;
    bool retryRequested80038DE8 = false;
    bool retryPumpApplied = false;
    bool timeout1200 = false;
    bool restoredCallbacks = false;
    uint32_t restore36510Arg = 0u;
    uint32_t restore36528Arg = 0u;
    bool pauseCommandIssued80036540_9 = false;
    bool userCallbackInvoked = false;
    uint8_t userCallbackArg0 = 0;
    uint32_t userCallbackArg1 = 0u;
};

struct SeekSyncInput800367A4 {
    PrStage1LoaderCdHal::LowerActionRequestMetadata request{};
    PrStage1LoaderDirect::CdOverlayTransferAttemptAttribution
        overlayTransferAttempt{};
    CommandResult800375BC commandResult{};
    bool halFactsKnown = false;
    PrStage1LoaderCdHal::SeekSyncHalFacts800367A4 halFacts{};
    bool cdSyncResultKnown = false;
    int32_t cdSyncPsxReturn = -1;
};

struct ReadStartInput80038FC0 {
    PrStage1LoaderCdHal::LowerActionRequestMetadata request{};
    PrStage1LoaderDirect::CdOverlayTransferAttemptAttribution
        overlayTransferAttempt{};
    bool halFactsKnown = false;
    PrStage1LoaderCdHal::ReadStartHalFacts80038FC0 halFacts{};
    ReadStartSetupResult80038FC0 setupResult{};
    ReadPumpResult80038DE8 readPumpResult{};
    const uint8_t* payloadData = nullptr;
    std::size_t payloadSize = 0u;
    bool payloadBytesKnown = false;
};

struct ReadSyncInput800390C8 {
    PrStage1LoaderCdHal::LowerActionRequestMetadata request{};
    PrStage1LoaderDirect::CdOverlayTransferAttemptAttribution
        overlayTransferAttempt{};
    bool wait = true;
    bool clockKnown = false;
    int32_t clockNow = 0;
    bool startClockKnown = false;
    int32_t startClock8005742C = 0;
    bool lastPumpClockKnown = false;
    int32_t lastPumpClock80057428 = 0;
    bool remainingKnown = false;
    int32_t remaining80057424 = 0;
    ReadPumpResult80038DE8 retryPumpResult{};
    ReadyWaitResult800372F0 finalReadyResult{};
    const uint8_t* payloadData = nullptr;
    std::size_t payloadSize = 0u;
    bool payloadBytesKnown = false;
};

struct LowerCdProducerFacts {
    PrStage1LoaderCdHal::LowerActionRequestMetadata request{};
    PrStage1LoaderDirect::CdOverlayTransferAttemptAttribution
        overlayTransferAttempt{};

    CommandResult800375BC commandResult800375BC{};
    bool seekSyncHalFacts800367A4Known = false;
    PrStage1LoaderCdHal::SeekSyncHalFacts800367A4
        seekSyncHalFacts800367A4{};
    bool cdSyncResult80037070Known = false;
    int32_t cdSyncPsxReturn80037070 = -1;
    bool cdSyncLoopFacts80037070Known = false;
    CdSyncLoopFactsInput80037070 cdSyncLoopFacts80037070{};

    bool readStartHalFacts80038FC0Known = false;
    PrStage1LoaderCdHal::ReadStartHalFacts80038FC0
        readStartHalFacts80038FC0{};
    ReadStartSetupResult80038FC0 readStartSetup80038FC0{};
    ReadPumpResult80038DE8 readPump80038DE8{};
    const uint8_t* payloadData = nullptr;
    std::size_t payloadSize = 0u;
    bool payloadBytesKnown = false;

    bool readSyncWait = true;
    bool clockKnown = false;
    int32_t clockNow = 0;
    bool startClockKnown = false;
    int32_t startClock8005742C = 0;
    bool lastPumpClockKnown = false;
    int32_t lastPumpClock80057428 = 0;
    bool remainingKnown = false;
    int32_t remaining80057424 = 0;
    ReadPumpResult80038DE8 retryPump80038DE8{};
    bool finalReadyInput800372F0Known = false;
    ReadyWaitInput800372F0 finalReadyInput800372F0{};
};

CdRegisterPointerSnapshotResult80036AF8
BuildCdRegisterPointerSnapshotResult80036AF8(
    const CdRegisterPointerSnapshot80036AF8& input);
RawCdRegTransactionResult80036AF8 BuildRawCdRegTransactionResult80036AF8(
    const RawCdRegTransactionInput80036AF8& input);
CdInterruptSnapshotResult800359B8 BuildCdInterruptSnapshotResult800359B8(
    const CdInterruptSnapshotInput800359B8& input);
CdCallbackPendingBridgeResult800359B8
BuildCdCallbackPendingBridgeInput800359B8(
    const CdInterruptSnapshotInput800359B8& input);
CdCallbackEventRegisterBridgeResult80036AF8
BuildCdCallbackEventRegisterBridgeInput80036AF8(
    const RawCdRegTransactionResult80036AF8& input);
CdSyncLoopFactsResult80037070 BuildCdSyncLoopFacts80037070(
    const CdSyncLoopFactsInput80037070& input);
CdSyncLowerFeedbackResult80037070
BuildCdSyncLowerFeedback80037070FromCoreFacts(
    const CdSyncLowerFeedbackInput80037070& input);
CommandAttr800375BC ResolveCommandAttr800375BC(uint8_t command);
CommandInput800375BC BuildCommandInput800375BC(
    uint8_t command,
    bool argsKnown,
    uint32_t argCount,
    const uint8_t* args,
    bool preSyncResultKnown,
    int32_t preSyncPsxReturn,
    bool skipWait,
    bool clockKnown,
    int32_t clockNow,
    bool timeoutKnown,
    bool timedOut,
    bool checkCallbackKnown,
    bool callbackPending,
    bool savedCdReg0SelectorKnown,
    uint8_t savedCdReg0Selector,
    bool callbackPumpKnown,
    uint32_t callbackPumpCount,
    bool callbackPumpDrained,
    int32_t lastCallbackPumpReturn,
    bool rawCallbackTransactionSequenceKnown,
    const RawCdRegTransactionResult80036AF8* rawCallbackTransactions,
    const int32_t* rawCallbackPsxReturns,
    bool selectorRestored,
    bool waitLoopResultKnown,
    int32_t waitLoopPsxReturn);
CommandResult800375BC BuildCommandResult800375BC(
    const CommandInput800375BC& input);
CommandWrapperResult80036540 BuildCommandWrapperResult80036540(
    const CommandWrapperInput80036540& input);
CommandWrapperResult80036678 BuildCommandWrapperResult80036678(
    const CommandWrapperInput80036678& input);
ReadyWaitResult800372F0 BuildReadyWaitResult800372F0(
    const ReadyWaitInput800372F0& input);
ReadyWaitReadyBankFacts800372F0 ResolveReadyWaitReadyBankFacts800372F0(
    const ReadyWaitInput800372F0& input);
CallbackSwapResult80036510 BuildCallbackSwapResult80036510(
    const CallbackSwapInput80036510& input);
CallbackSwapResult80036528 BuildCallbackSwapResult80036528(
    const CallbackSwapInput80036528& input);
StatusReadResult80036384 BuildStatusReadResult80036384(
    const StatusReadInput80036384& input);
LocSectorResult80036A78 BuildLocSectorResult80036A78(
    const LocSectorInput80036A78& input);
ReadPumpResult80038DE8 BuildReadPumpResult80038DE8(
    const ReadPumpInput80038DE8& input);
ReadStartSetupResult80038FC0 BuildReadStartSetupResult80038FC0(
    const ReadStartSetupInput80038FC0& input);
ReadCallbackResult80038BC4 BuildReadCallbackResult80038BC4(
    const ReadCallbackInput80038BC4& input);
Result BuildSeekSyncSeam800367A4(const SeekSyncInput800367A4& input);
Result BuildReadStartSeam80038FC0(const ReadStartInput80038FC0& input);
Result BuildReadSyncSeam800390C8(const ReadSyncInput800390C8& input);
Result BuildLowerCdSeamFromFacts(const LowerCdProducerFacts& input);

} // namespace PrStage1LowerCdProducerDirect
