#pragma once

#include <array>
#include <cstdint>

namespace PrMovieSegmentDirect {

static constexpr uint32_t kMovieSegmentRecordSize8001A324 = 0x30u;
static constexpr uint32_t kSceneEntryMovieSegmentOffset801C4780 = 0x0Cu;
static constexpr uint32_t kSceneEntryMovieSegmentCount801C4780 = 7u;
static constexpr uint32_t kSub8001A210StreamClockCallback = 0x8001A210u;
static constexpr uint32_t kSub8001A258StreamClockCallbackRegister = 0x8001A258u;
static constexpr uint32_t kSub8001A3C8StreamClockPoll = 0x8001A3C8u;
static constexpr uint32_t kSub8001A750StreamStatusPoll = 0x8001A750u;
static constexpr uint32_t kSub8001A724StreamClockReset = 0x8001A724u;
static constexpr uint32_t kSub8001A7A4StreamClockQuery = 0x8001A7A4u;
static constexpr uint32_t kSub8001A694ClearCdCallback = 0x8001A694u;
static constexpr uint32_t kSub800363A4CdReadyStatus = 0x800363A4u;
static constexpr uint32_t kSub800364D0CdSyncWrapper = 0x800364D0u;
static constexpr uint32_t kSub80036510SetCdCallback = 0x80036510u;
static constexpr uint32_t kSub80035898CheckCallback = 0x80035898u;
static constexpr uint32_t kSub800359B8CdCallbackPendingProducer = 0x800359B8u;
static constexpr uint32_t kSub800359B8SetWord80055F7A = 0x80035A20u;
static constexpr uint32_t kSub800359B8ClearWord80055F7A = 0x80035B6Cu;
static constexpr uint32_t kSub80036AF8CdCallbackEvent = 0x80036AF8u;
static constexpr uint32_t kSub80037070CdSync = 0x80037070u;
static constexpr uint32_t kSub800375BCCdCommand = 0x800375BCu;
static constexpr uint32_t kSub80036A78MsfToLba = 0x80036A78u;
static constexpr uint32_t kSub800154B0OverlayTransferWrapper = 0x800154B0u;
static constexpr uint32_t kSub8001ACF8OverlayTransfer = 0x8001ACF8u;
static constexpr uint32_t kSub8001A89CSeekForOverlayTransfer = 0x8001A89Cu;
static constexpr uint32_t kSub8001A818ReadForOverlayTransfer = 0x8001A818u;
static constexpr uint32_t kSub80036974LbaToMsf = 0x80036974u;
static constexpr uint32_t kSub800367A4SeekSync = 0x800367A4u;
static constexpr uint32_t kSub80038FC0ReadStart = 0x80038FC0u;
static constexpr uint32_t kSub80035560VBlankWait = 0x80035560u;
static constexpr uint32_t kSub800390C8ReadSync = 0x800390C8u;
static constexpr uint32_t kOverlayTransferDest801C3870 = 0x801C3870u;
static constexpr uint32_t kOverlayTransferMaxAttempts8001ACF8 = 4u;
static constexpr int32_t kOverlayTransferSeekSyncMode8001A89C = 2;
static constexpr int32_t kOverlayTransferSeekSyncArg2_8001A89C = 0;
static constexpr int32_t kOverlayTransferReadWaitArg8001A818 = 3;
static constexpr int32_t kOverlayTransferReadSyncArg0_8001A818 = 1;
static constexpr int32_t kOverlayTransferReadSyncArg1_8001A818 = 0;
static constexpr int32_t kOverlayTransferReadSyncSuccess8001A818 = 0;
static constexpr uint32_t kCdCallbackResponseByteCount80036AF8 = 8u;
static constexpr uint32_t kCdCallbackPendingProducerCallbackCount800359B8 =
    11u;

// Gap: Stage-specific path identity/static rows/CD HAL bridges stay outside.

struct MsfBcd80036A78 {
    uint8_t minute = 0;
    uint8_t second = 0;
    uint8_t frame = 0;
};

struct MsfToLbaResult80036A78 {
    bool known = false;
    int32_t lba = 0;
};

enum class StreamClockSource800493F4 : uint8_t {
    Unknown = 0,
    Byte800493F4,
};

struct StreamClockProducerFeedback800493F4 {
    bool known = false;
    StreamClockSource800493F4 source =
        StreamClockSource800493F4::Unknown;
    uint32_t producerAddr = 0x800493F4u;
    bool byte800493F4Known = false;
    MsfBcd80036A78 byte800493F4{};
};

struct StreamClockProducerCarrier800493F4 {
    bool called = false;
    StreamClockProducerFeedback800493F4 feedback{};
    StreamClockSource800493F4 acceptedSource =
        StreamClockSource800493F4::Unknown;
    bool acceptedByte800493F4 = false;
    bool sub80036A78Computed = false;
    MsfToLbaResult80036A78 sub80036A78{};
    bool clockKnown = false;
    int32_t clockLba = 0;
    bool watchdogKnown = false;
    int32_t dword80049404 = 0;
    bool gapMissingByte800493F4ClockProducer = false;
};

struct StreamClockCallbackInput8001A210 {
    uint8_t a1 = 0;
    const uint8_t* resultBytes = nullptr;
    uint32_t resultSize = 0;
};

struct StreamClockCallbackResult8001A210 {
    bool called = false;
    uint32_t sourceFunction = kSub8001A210StreamClockCallback;
    int32_t psxReturn = 2;
    bool acceptedByte800493F4 = false;
    StreamClockProducerFeedback800493F4 feedback{};
    bool gapMissingCallbackResultBytes = false;
};

struct StreamClockPollInput8001A3C8 {
    bool sub800364D0Known = false;
    int32_t sub800364D0Result = 0;
    bool syncBytesKnown = false;
    std::array<uint8_t, 16> syncBytes{};
    bool sub800363A4Known = false;
    int32_t sub800363A4Result = 0;
};

struct StreamStatusPollInput8001A750 {
    bool sub800364D0Known = false;
    int32_t sub800364D0Result = 0;
    bool statusBytesKnown = false;
    std::array<uint8_t, 16> statusBytes{};
    bool commandWrapper80036678Known = false;
    bool commandWrapper80036678Succeeded = false;
};

struct StreamStatusPollResult8001A750 {
    bool called = false;
    uint32_t sourceFunction = kSub8001A750StreamStatusPoll;
    bool resultKnown = false;
    int32_t psxReturn = 0;
    bool requestedCommandWrapper80036678 = false;
    bool commandWrapper80036678Known = false;
    bool commandWrapper80036678Succeeded = false;
    bool gapMissingSub800364D0Feedback = false;
    bool gapMissingCommandWrapper80036678 = false;
};

struct CdReadyStatusFeedback800363A4 {
    bool known = false;
    uint8_t byte80057119 = 0;
};

struct CdReadyStatusResult800363A4 {
    bool called = false;
    uint32_t sourceFunction = kSub800363A4CdReadyStatus;
    bool resultKnown = false;
    int32_t psxReturn = 0;
    bool gapMissingByte80057119 = false;
};

struct CdSyncLowerFeedback80037070 {
    bool known = false;
    bool timedOut = false;
    bool syncResultKnown = false;
    int32_t syncResult = 0;
    bool responseBytesKnown = false;
    uint32_t responseByteCount = 0;
    std::array<uint8_t, 16> responseBytes{};
};

struct CdCallbackEventInput80036AF8 {
    bool interruptKnown = false;
    uint8_t interruptCode = 0;
    bool resultBytesKnown = false;
    uint32_t resultByteCount = 0;
    std::array<uint8_t, 8> resultBytes{};
    bool commandKnown = false;
    uint8_t command = 0;
    bool priorDword80057108Known = false;
    uint32_t priorDword80057108 = 0;
    bool priorDword80057110Known = false;
    uint32_t priorDword80057110 = 0;
    bool priorSyncMaskKnown = false;
    uint8_t priorSyncMask = 0;
};

struct CdCallbackEventRegisterInput80036AF8 {
    bool cdReg3InitialKnown = false;
    uint8_t cdReg3Initial = 0;
    bool cdReg3StableKnown = false;
    uint8_t cdReg3Stable = 0;
    bool resultFifoKnown = false;
    uint32_t resultFifoByteCount = 0;
    std::array<uint8_t, 8> resultFifoBytes{};
    bool commandKnown = false;
    uint8_t command = 0;
    bool priorDword80057108Known = false;
    uint32_t priorDword80057108 = 0;
    bool priorDword80057110Known = false;
    uint32_t priorDword80057110 = 0;
    bool priorSyncMaskKnown = false;
    uint8_t priorSyncMask = 0;
};

struct CdCallbackEventRegisterResult80036AF8 {
    bool called = false;
    uint32_t sourceFunction = kSub80036AF8CdCallbackEvent;
    bool wroteCdReg0Select1 = false;
    bool interruptKnown = false;
    uint8_t initialInterruptCode = 0;
    uint8_t stableInterruptCode = 0;
    bool interruptChangedBeforeStable = false;
    bool resultFifoKnown = false;
    uint32_t resultFifoByteCount = 0;
    std::array<uint8_t, 8> paddedResultFifoBytes{};
    bool zeroPaddedResultFifo = false;
    bool ackCdReg0Select1 = false;
    bool ackCdReg3Write7 = false;
    bool ackCdReg2Write7 = false;
    bool builtEventInput = false;
    CdCallbackEventInput80036AF8 eventInput{};
    bool gapMissingCdReg3Initial = false;
    bool gapMissingCdReg3Stable = false;
    bool gapMissingResultFifo = false;
};

struct CheckCallbackInput80035898 {
    bool word80055F7AKnown = false;
    uint16_t word80055F7A = 0;
};

struct CheckCallbackResult80035898 {
    bool called = false;
    uint32_t sourceFunction = kSub80035898CheckCallback;
    bool pendingKnown = false;
    bool pending = false;
    uint16_t word80055F7A = 0;
    bool gapMissingWord80055F7A = false;
};

struct CdCallbackPendingProducerInput800359B8 {
    bool writeAddressKnown = false;
    uint32_t writeAddress = 0;
    bool word80055F78Known = false;
    uint16_t word80055F78 = 0;
    bool word80055FA8Known = false;
    uint16_t word80055FA8 = 0;
    bool interruptStatusKnown = false;
    uint16_t interruptStatus = 0;
    bool interruptMaskKnown = false;
    uint16_t interruptMask = 0;
    bool watchdogKnown = false;
    uint32_t dword80057010 = 0;
    bool callbackTableKnown = false;
    std::array<bool, kCdCallbackPendingProducerCallbackCount800359B8>
        callbackPresent{};
};

struct CdCallbackPendingProducerResult800359B8 {
    bool called = false;
    uint32_t sourceFunction = kSub800359B8CdCallbackPendingProducer;
    bool writeKnown = false;
    uint32_t writeAddress = 0;
    bool word80055F7AKnown = false;
    uint16_t word80055F7A = 0;
    bool pendingKnown = false;
    bool pending = false;
    bool gapMissingWriteAddress = false;
    bool gapUnknownWriteAddress = false;
    bool fullProducerKnown = false;
    bool returnFromExceptionBeforePending = false;
    bool returnFromExceptionAfterClear = false;
    bool setPendingWrite = false;
    bool clearPendingWrite = false;
    bool interruptAckKnown = false;
    uint32_t interruptAckCount = 0;
    std::array<uint8_t, kCdCallbackPendingProducerCallbackCount800359B8>
        interruptAckBitIndex{};
    std::array<uint16_t, kCdCallbackPendingProducerCallbackCount800359B8>
        interruptAckWriteValue{};
    uint32_t callbackDispatchCount = 0;
    std::array<uint8_t, kCdCallbackPendingProducerCallbackCount800359B8>
        callbackDispatchIndex{};
    bool watchdogKnown = false;
    uint32_t dword80057010 = 0;
    bool watchdogWritten = false;
    uint32_t dword80057010Written = 0;
    bool watchdogTimeout = false;
    bool interruptStatusClearedOnTimeout = false;
    bool watchdogOverflowGap = false;
    bool gapMissingWord80055F78 = false;
    bool gapMissingWord80055FA8 = false;
    bool gapMissingInterruptStatus = false;
    bool gapMissingInterruptMask = false;
    bool gapMissingCallbackTable = false;
};

struct CdCallbackEventResult80036AF8 {
    bool called = false;
    uint32_t sourceFunction = kSub80036AF8CdCallbackEvent;
    bool eventKnown = false;
    int32_t psxReturn = 0;
    uint8_t interruptCode = 0;
    bool resultBytesKnown = false;
    uint32_t resultByteCount = 0;
    std::array<uint8_t, 8> paddedResultBytes{};
    bool dword80057108Known = false;
    uint32_t dword80057108 = 0;
    bool dword8005710CKnown = false;
    uint32_t dword8005710C = 0;
    bool dword80057110Known = false;
    uint32_t dword80057110 = 0;
    bool dword80057110Incremented = false;
    bool cdReg0ClearAfterReady = false;
    bool cdReg3ClearAfterReady = false;
    bool byte800573D4Known = false;
    uint8_t byte800573D4 = 0;
    bool byte800573D5Known = false;
    uint8_t byte800573D5 = 0;
    bool byte800573D6Known = false;
    uint8_t byte800573D6 = 0;
    bool response882F8Known = false;
    std::array<uint8_t, 8> response882F8{};
    bool response88300Known = false;
    std::array<uint8_t, 8> response88300{};
    bool response88308Known = false;
    std::array<uint8_t, 8> response88308{};
    CdSyncLowerFeedback80037070 syncFeedback{};
    bool gapMissingInterruptCode = false;
    bool gapMissingResultBytes = false;
    bool gapMissingCommand = false;
    bool gapMissingPriorDword80057108 = false;
    bool gapMissingPriorDword80057110 = false;
};

struct CdSyncResult80037070 {
    bool called = false;
    uint32_t sourceFunction = kSub80037070CdSync;
    int32_t a0WaitMode = 0;
    bool a1OutputBufferPtrNonNull = false;
    bool syncResultKnown = false;
    int32_t psxReturn = -1;
    bool outputBytesKnown = false;
    uint32_t outputByteCount = 0;
    std::array<uint8_t, 16> outputBytes{};
    bool copiedOutputBytes = false;
    bool timeout = false;
    bool gapMissingCdSyncFeedback = false;
};

struct StreamClockPollResult8001A3C8 {
    bool called = false;
    uint32_t sourceFunction = kSub8001A3C8StreamClockPoll;
    int32_t psxReturn = 0;
    bool acceptedByte800493F4 = false;
    StreamClockProducerFeedback800493F4 feedback{};
    bool dword80049428Known = false;
    int32_t dword80049428 = 0;
    bool gapMissingSub800364D0Feedback = false;
    bool gapMissingSub800363A4Feedback = false;
};

struct StreamClockResetResult8001A724 {
    bool called = false;
    uint32_t sourceFunction = kSub8001A724StreamClockReset;
    int32_t psxReturn = 0;
    bool dword80049404Known = false;
    int32_t dword80049404 = 0;
    bool dword80049408Known = false;
    int32_t dword80049408 = 0;
};

struct MovieSegmentRecord48 {
    bool known = false;
    uint32_t psxAddr = 0;
    uint32_t tableIndex = 0;

    bool pathPtrA1Plus00Known = false;
    uint32_t pathPtrA1Plus00 = 0;

    bool opaqueA1Plus04Known = false;
    uint32_t opaqueA1Plus04 = 0;

    bool loadedStateA1Plus0CKnown = false;
    int32_t loadedStateA1Plus0C = 0;

    bool startMsfKnown = false;
    MsfBcd80036A78 startMsf{};

    bool lengthSourceA1Plus20Known = false;
    uint32_t lengthSourceA1Plus20 = 0;

    bool cdlFileNameA1Plus18Known = false;
    std::array<uint8_t, 16> cdlFileNameA1Plus18{};

    bool endBiasA1Plus8Known = false;
    int32_t endBiasA1Plus8 = 0;

    bool timeBaseA1Plus40Known = false;
    int32_t timeBaseA1Plus40 = 0;

    bool endA1Plus44Known = false;
    int32_t endA1Plus44 = 0;
};

struct MovieSegmentRowInitFeedback8001A324 {
    bool known = false;
    bool explicitCdLookupFeedback = false;
    bool lookupRequestKnown = false;
    uint32_t lookupRequestCdlFilePtr = 0;
    bool lookupRequestPathPtrKnown = false;
    uint32_t lookupRequestPathPtr = 0;
    bool lookupResultPtrKnown = false;
    uint32_t lookupResultPtr = 0;
    bool loadedStateA1Plus0CKnown = false;
    int32_t loadedStateA1Plus0C = 0;
    bool cdlFilePosKnown = false;
    MsfBcd80036A78 cdlFilePos{};
    bool cdlFileSizeKnown = false;
    uint32_t cdlFileSize = 0;
    bool cdlFileNameKnown = false;
    std::array<uint8_t, 16> cdlFileName{};
};

struct CdLookupProbeCompletion8001A2B0 {
    bool known = false;
    bool complete = false;
    bool explicitCdLookupFeedback = false;
    bool requestKnown = false;
    uint32_t requestCdlFilePtr = 0;
    bool requestPathPtrKnown = false;
    uint32_t requestPathPtr = 0;
    bool lookupSucceeded = false;
    bool lookupFailed = false;
    uint32_t resultPtr = 0;
    bool cdlFilePosKnown = false;
    MsfBcd80036A78 cdlFilePos{};
    bool cdlFileSizeKnown = false;
    uint32_t cdlFileSize = 0;
    bool cdlFileNameKnown = false;
    std::array<uint8_t, 16> cdlFileName{};
};

struct SceneEntryMovieSegmentRawRow801C4780 {
    bool known = false;
    bool pathPtrA1Plus00Known = false;
    uint32_t pathPtrA1Plus00 = 0;
    bool opaqueA1Plus04Known = false;
    uint32_t opaqueA1Plus04 = 0;
    bool endBiasA1Plus08Known = false;
    int32_t endBiasA1Plus08 = 0;
    bool loadedStateA1Plus0CKnown = false;
    int32_t loadedStateA1Plus0C = 0;
    bool cdlFilePosKnown = false;
    MsfBcd80036A78 cdlFilePos{};
    bool cdlFileSizeKnown = false;
    uint32_t cdlFileSize = 0;
    bool cdlFileNameKnown = false;
    std::array<uint8_t, 16> cdlFileName{};
};

struct SceneEntryMovieSegmentTable801C4780 {
    bool sceneEntryBaseKnown = false;
    uint32_t sceneEntryBase = 0;
    MovieSegmentRecord48 rows[kSceneEntryMovieSegmentCount801C4780]{};
};

struct SegmentInitResult8001A324 {
    MovieSegmentRecord48 record{};
    int32_t result = -1;
    bool lookupRequestKnown = false;
    uint32_t lookupRequestCdlFilePtr = 0;
    bool lookupRequestPathPtrKnown = false;
    uint32_t lookupRequestPathPtr = 0;
    bool skippedPathPtrZero = false;
    bool skippedAlreadyLoaded = false;
    bool cdLookupSucceeded = false;
    bool timeBaseWrittenA1Plus40 = false;
    bool loadedStateWrittenA1Plus0C = false;
    bool endWrittenA1Plus44 = false;
};

struct OverlayTransferAttemptProvenance8001ACF8 {
    bool known = false;
    uint32_t wrapperFunction = kSub800154B0OverlayTransferWrapper;
    uint32_t transferFunction = kSub8001ACF8OverlayTransfer;
    bool rowAddrKnown = false;
    uint32_t rowAddr = 0;
    bool attemptIndexKnown = false;
    uint32_t attemptIndex = 0;
    bool destKnown = false;
    uint32_t dest = 0;
    bool sectorCountKnown = false;
    uint32_t sectorCount = 0;
};

struct OverlayTransferReadFeedback8001A818 {
    bool known = false;
    OverlayTransferAttemptProvenance8001ACF8 provenance{};
    bool dstKnown = false;
    uint32_t dst = 0;
    bool sectorCountKnown = false;
    uint32_t requestedSectorCount = 0;
    bool firstAttemptFlagKnown = false;
    int32_t firstAttemptFlag = 0;
    bool modeFlagKnown = false;
    uint8_t modeFlag = 0;
    uint32_t readStartFunction = kSub80038FC0ReadStart;
    bool readStartSucceeded = false;
    uint32_t readStartRetryCount = 0;
    uint32_t waitFunction = kSub80035560VBlankWait;
    int32_t waitArg = kOverlayTransferReadWaitArg8001A818;
    uint32_t syncFunction = kSub800390C8ReadSync;
    int32_t syncArg0 = kOverlayTransferReadSyncArg0_8001A818;
    int32_t syncArg1 = kOverlayTransferReadSyncArg1_8001A818;
    bool syncResultKnown = false;
    int32_t syncResult = 0;
    uint32_t syncPollCount = 0;
    bool psxReturnKnown = false;
    int32_t psxReturn = 0;
    bool readResultKnown = false;
    int32_t readSectorCount = 0;
};

struct OverlayTransferReadLowerFeedback8001A818 {
    bool known = false;
    bool readStartSucceededKnown = false;
    bool readStartSucceeded = false;
    bool readStartRetryCountKnown = false;
    uint32_t readStartRetryCount = 0;
    bool syncResultKnown = false;
    int32_t syncResult = 0;
    bool syncPollCountKnown = false;
    uint32_t syncPollCount = 0;
};

struct OverlayTransferSeekFeedback8001A89C {
    bool known = false;
    OverlayTransferAttemptProvenance8001ACF8 provenance{};
    bool syncResultKnown = false;
    int32_t syncResult = 0;
    bool psxReturnKnown = false;
    bool psxReturn = false;
};

struct OverlayTransferSeekLowerFeedback8001A89C {
    bool known = false;
    bool syncResultKnown = false;
    int32_t syncResult = 0;
    bool psxReturnKnown = false;
    bool psxReturn = false;
};

struct OverlayTransferSeekResult8001A89C {
    bool called = false;
    uint32_t sourceFunction = kSub8001A89CSeekForOverlayTransfer;
    bool cdlFilePosKnown = false;
    MsfBcd80036A78 cdlFilePos{};
    bool offsetKnown = false;
    int32_t offset = 0;
    bool baseLbaKnown = false;
    int32_t baseLba = 0;
    bool targetLbaKnown = false;
    int32_t targetLba = 0;
    bool targetMsfKnown = false;
    MsfBcd80036A78 targetMsf{};
    uint32_t lbaFunction = kSub80036A78MsfToLba;
    uint32_t lbaToMsfFunction = kSub80036974LbaToMsf;
    uint32_t syncFunction = kSub800367A4SeekSync;
    int32_t syncArg0 = kOverlayTransferSeekSyncMode8001A89C;
    bool syncArg1CdlPtrKnown = false;
    uint32_t syncArg1CdlPtr = 0;
    int32_t syncArg2 = kOverlayTransferSeekSyncArg2_8001A89C;
    bool syncResultKnown = false;
    int32_t syncResult = 0;
    bool psxReturnKnown = false;
    bool psxReturn = false;
    bool gapMissingCdlFilePos = false;
    bool gapMissingSeekFeedback = false;
};

struct OverlayTransferAttempt8001ACF8 {
    bool executed = false;
    uint32_t attemptIndex = 0;
    int32_t firstAttemptFlag = 1;
    SegmentInitResult8001A324 rowInit{};
    bool copiedCdlFileWords = false;
    std::array<uint32_t, 6> cdlFileWords{};
    bool sectorCountKnown = false;
    uint32_t sectorCount = 0;
    bool seekAction = false;
    uint32_t seekFunction = kSub8001A89CSeekForOverlayTransfer;
    OverlayTransferSeekResult8001A89C seek8001A89C{};
    bool readAction = false;
    uint32_t readFunction = kSub8001A818ReadForOverlayTransfer;
    bool destKnown = false;
    uint32_t dest = 0;
    bool readFeedbackKnown = false;
    int32_t readSectorCount = -1;
    bool matchedExpectedSectorCount = false;
    bool gapMissingCdLookupFeedback = false;
    bool gapMissingSeekFeedback = false;
    bool gapMissingReadFeedback = false;
};

struct OverlayTransferResult8001ACF8 {
    bool called = false;
    uint32_t sourceFunction = kSub8001ACF8OverlayTransfer;
    MovieSegmentRecord48 finalRow{};
    bool destKnown = false;
    uint32_t dest = 0;
    uint32_t attemptCount = 0;
    std::array<OverlayTransferAttempt8001ACF8,
               kOverlayTransferMaxAttempts8001ACF8>
        attempts{};
    bool resultKnown = false;
    bool psxReturn = false;
    bool gapMissingCdLookupFeedback = false;
    bool gapMissingSeekFeedback = false;
    bool gapMissingReadFeedback = false;
};

struct OverlayTransferResult800154B0 {
    bool called = false;
    uint32_t sourceFunction = kSub800154B0OverlayTransferWrapper;
    bool rowArgKnown = false;
    uint32_t rowArg = 0;
    bool ignoredArg1Known = false;
    int32_t ignoredArg1 = 0;
    bool destKnown = true;
    uint32_t dest = kOverlayTransferDest801C3870;
    OverlayTransferResult8001ACF8 sub8001ACF8{};
};

struct MovieSegmentScanResult801C4780 {
    bool called = false;
    SceneEntryMovieSegmentTable801C4780 table{};
    SegmentInitResult8001A324 rowInit[kSceneEntryMovieSegmentCount801C4780]{};
    uint32_t feedbackCount = 0;
    std::array<bool, kSceneEntryMovieSegmentCount801C4780> feedbackAppliedMask{};
    uint32_t feedbackAppliedCount = 0;
    std::array<bool, kSceneEntryMovieSegmentCount801C4780> rowNeedsCdLookupMask{};
    uint32_t rowNeedsCdLookupCount = 0;
    std::array<bool, kSceneEntryMovieSegmentCount801C4780>
        rowCdLookupRequestKnownMask{};
    std::array<uint32_t, kSceneEntryMovieSegmentCount801C4780>
        rowCdLookupRequestCdlFilePtr{};
    std::array<bool, kSceneEntryMovieSegmentCount801C4780>
        rowCdLookupRequestPathPtrKnownMask{};
    std::array<uint32_t, kSceneEntryMovieSegmentCount801C4780>
        rowCdLookupRequestPathPtr{};
    uint32_t rowCdLookupRequestKnownCount = 0;
    uint32_t rowCdLookupRequestPathPtrKnownCount = 0;
    std::array<bool, kSceneEntryMovieSegmentCount801C4780>
        rowMissingCdLookupRequestMask{};
    uint32_t rowMissingCdLookupRequestCount = 0;
    std::array<bool, kSceneEntryMovieSegmentCount801C4780>
        rowCdLookupResultPtrKnownMask{};
    std::array<uint32_t, kSceneEntryMovieSegmentCount801C4780>
        rowCdLookupResultPtr{};
    uint32_t rowCdLookupResultPtrKnownCount = 0;
    std::array<bool, kSceneEntryMovieSegmentCount801C4780> rowCdLookupReadyMask{};
    uint32_t rowCdLookupReadyCount = 0;
    std::array<bool, kSceneEntryMovieSegmentCount801C4780>
        rowCdlFileNameReadyMask{};
    uint32_t rowCdlFileNameReadyCount = 0;
    std::array<bool, kSceneEntryMovieSegmentCount801C4780>
        rowMissingCdLookupFeedbackMask{};
    uint32_t rowMissingCdLookupFeedbackCount = 0;
    std::array<bool, kSceneEntryMovieSegmentCount801C4780>
        rowMissingCdlFileNameFeedbackMask{};
    uint32_t rowMissingCdlFileNameFeedbackCount = 0;
    bool gapMissingCdLookupFeedback = false;
    bool gapMissingCdlFileNameFeedback = false;
    bool gapMissingCdLookupRequest = false;
    bool reset80025A00Action = false;
    bool loadCompo8001AC18Action = false;
    uint32_t loadCompoArg0 = 0;
    int32_t loadCompoArg1 = 0;
};

struct SceneEntrySegmentSelection801C4DC4 {
    bool known = false;
    uint32_t sceneEntryBase = 0;
    uint32_t segmentAddr = 0;
    uint32_t segmentOffset = 0;
    uint32_t rowIndex = 0;
    bool rowKnown = false;
    MovieSegmentRecord48 row{};
};

struct MovieStepSegmentFields801C4350 {
    bool segmentTimeBaseA1Plus40Known = false;
    int32_t segmentTimeBaseA1Plus40 = 0;
    bool segmentEndA1Plus44Known = false;
    int32_t segmentEndA1Plus44 = 0;
    bool segmentEndBiasA1Plus8Known = false;
    int32_t segmentEndBiasA1Plus8 = 0;
};

uint32_t ComputeMovieSegmentRowOffset801C4780(uint32_t rowIndex);
bool TryComputeMovieSegmentRowIndex801C4780(uint32_t rowOffset,
                                            uint32_t* rowIndex);
uint32_t ComputeMovieSegmentRowAddr801C4780(uint32_t sceneEntryBase,
                                            uint32_t rowIndex);
int32_t DecodeBcd80036A78(uint8_t value);
MsfToLbaResult80036A78 PsxCall80036A78_MsfToLba(
    const MsfBcd80036A78& msf);
CdReadyStatusResult800363A4 PsxCall800363A4_ReadCdReadyStatus(
    const CdReadyStatusFeedback800363A4& feedback);
CheckCallbackResult80035898 PsxCall80035898_CheckCallback(
    const CheckCallbackInput80035898& input);
CdCallbackPendingProducerResult800359B8
PsxCall800359B8_CdCallbackPendingProducer(
    const CdCallbackPendingProducerInput800359B8& input);
CdCallbackEventResult80036AF8 PsxCall80036AF8_BuildCdLowerEvent(
    const CdCallbackEventInput80036AF8& input);
CdCallbackEventRegisterResult80036AF8 BuildCdCallbackEventInput80036AF8FromCdRegs(
    const CdCallbackEventRegisterInput80036AF8& input);
CdSyncResult80037070 PsxCall80037070_CdSync(
    int32_t a0WaitMode,
    bool a1OutputBufferPtrNonNull,
    const CdSyncLowerFeedback80037070& feedback);
CdSyncResult80037070 PsxCall800364D0_CdSyncWrapper(
    int32_t a0WaitMode,
    bool a1OutputBufferPtrNonNull,
    const CdSyncLowerFeedback80037070& feedback);
StreamClockPollInput8001A3C8
BuildStreamClockPollInput8001A3C8FromCdSync(
    const CdSyncResult80037070& sync,
    const CdReadyStatusResult800363A4& ready);
StreamStatusPollInput8001A750
BuildStreamStatusPollInput8001A750FromCdSync(
    const CdSyncResult80037070& sync);
StreamClockCallbackResult8001A210 PsxCall8001A210_StreamClockCallback(
    const StreamClockCallbackInput8001A210& input);
StreamClockPollResult8001A3C8 PsxCall8001A3C8_StreamClockPoll(
    const StreamClockPollInput8001A3C8& input);
StreamStatusPollResult8001A750 PsxCall8001A750_StreamStatusPoll(
    const StreamStatusPollInput8001A750& input);
StreamClockResetResult8001A724 PsxCall8001A724_ResetStreamClock(
    int32_t a1);
StreamClockProducerCarrier800493F4
BuildStreamClockProducerCarrier800493F4(
    const StreamClockProducerFeedback800493F4& feedback);
SegmentInitResult8001A324 PsxCall8001A324_InitSegmentRecord(
    const MovieSegmentRecord48& source);
OverlayTransferSeekFeedback8001A89C BuildOverlayTransferSeekFeedback8001A89C(
    const OverlayTransferSeekLowerFeedback8001A89C& lower);
OverlayTransferAttemptProvenance8001ACF8
BuildOverlayTransferAttemptProvenance8001ACF8(
    bool rowAddrKnown,
    uint32_t rowAddr,
    bool attemptIndexKnown,
    uint32_t attemptIndex,
    bool destKnown,
    uint32_t dest,
    bool sectorCountKnown,
    uint32_t sectorCount);
void AttachOverlayTransferAttemptProvenance8001ACF8(
    const OverlayTransferAttemptProvenance8001ACF8& provenance,
    OverlayTransferSeekFeedback8001A89C& feedback);
void AttachOverlayTransferAttemptProvenance8001ACF8(
    const OverlayTransferAttemptProvenance8001ACF8& provenance,
    OverlayTransferReadFeedback8001A818& feedback);
OverlayTransferSeekResult8001A89C PsxCall8001A89C_SeekForOverlayTransfer(
    const MovieSegmentRecord48& row,
    int32_t offset,
    bool offsetKnown,
    const OverlayTransferSeekFeedback8001A89C* feedback);
OverlayTransferReadFeedback8001A818 BuildOverlayTransferReadFeedback8001A818(
    uint32_t dst,
    bool dstKnown,
    uint32_t sectorCount,
    bool sectorCountKnown,
    int32_t firstAttemptFlag,
    bool firstAttemptFlagKnown,
    const OverlayTransferReadLowerFeedback8001A818& lower);
OverlayTransferResult8001ACF8 PsxCall8001ACF8_OverlayTransfer(
    const MovieSegmentRecord48& row,
    const MovieSegmentRowInitFeedback8001A324& rowFeedback,
    const OverlayTransferSeekFeedback8001A89C* seekFeedback,
    uint32_t seekFeedbackCount,
    const OverlayTransferReadFeedback8001A818* readFeedback,
    uint32_t readFeedbackCount,
    uint32_t dest,
    bool destKnown);
OverlayTransferResult800154B0 PsxCall800154B0_Row0OverlayTransfer(
    const MovieSegmentRecord48& row,
    const MovieSegmentRowInitFeedback8001A324& rowFeedback,
    const OverlayTransferSeekFeedback8001A89C* seekFeedback,
    uint32_t seekFeedbackCount,
    const OverlayTransferReadFeedback8001A818* readFeedback,
    uint32_t readFeedbackCount);
MovieSegmentRecord48 BuildMovieSegmentRecordFromRawRow801C4780(
    const SceneEntryMovieSegmentRawRow801C4780& raw,
    uint32_t rowIndex);
SceneEntryMovieSegmentTable801C4780 MaterializeSceneEntryMovieSegments801C4780(
    uint32_t sceneEntryBase,
    bool sceneEntryBaseKnown,
    const MovieSegmentRecord48* rows,
    uint32_t rowCount);
SceneEntryMovieSegmentTable801C4780
MaterializeSceneEntryMovieSegmentsFromRawRows801C4780(
    uint32_t sceneEntryBase,
    bool sceneEntryBaseKnown,
    const SceneEntryMovieSegmentRawRow801C4780* rows,
    uint32_t rowCount);
MovieSegmentRecord48 ApplyMovieSegmentRowFeedback8001A324(
    const MovieSegmentRecord48& row,
    const MovieSegmentRowInitFeedback8001A324& feedback);
MovieSegmentRowInitFeedback8001A324
BuildMovieSegmentRowFeedbackFromCdLookup8001A2B0(
    const CdLookupProbeCompletion8001A2B0& feedback,
    bool loadedStateA1Plus0CKnown,
    int32_t loadedStateA1Plus0C);
MovieSegmentScanResult801C4780 PsxCall801C4780_ScanMovieSegments(
    const SceneEntryMovieSegmentTable801C4780& table);
MovieSegmentScanResult801C4780 PsxCall801C4780_ScanMovieSegmentsWithFeedback(
    const SceneEntryMovieSegmentTable801C4780& table,
    const MovieSegmentRowInitFeedback8001A324* feedback,
    uint32_t feedbackCount);
SceneEntrySegmentSelection801C4DC4 PsxSelectMovieSegmentByOffset801C4DC4(
    uint32_t sceneEntryBase,
    bool sceneEntryBaseKnown,
    uint32_t segmentOffset);
SceneEntrySegmentSelection801C4DC4 PsxSelectMovieSegmentByOffsetFromScan801C4DC4(
    const MovieSegmentScanResult801C4780& scan,
    uint32_t segmentOffset);
MovieStepSegmentFields801C4350 BuildMovieStepSegmentFields801C4350(
    const MovieSegmentRecord48& record);

}  // namespace PrMovieSegmentDirect
