#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace PrStage1LoaderCdHal {

constexpr uint32_t kFn8001A2B0 = 0x8001A2B0u;
constexpr uint32_t kFn800381F8 = 0x800381F8u;
constexpr uint32_t kFn8001A89C = 0x8001A89Cu;
constexpr uint32_t kFn80036974 = 0x80036974u;
constexpr uint32_t kFn800367A4 = 0x800367A4u;
constexpr uint32_t kFn8001A818 = 0x8001A818u;
constexpr uint32_t kFn80038FC0 = 0x80038FC0u;
constexpr uint32_t kFn80035560 = 0x80035560u;
constexpr uint32_t kFn800390C8 = 0x800390C8u;
constexpr uint32_t kFn800364F0 = 0x800364F0u;

constexpr uint8_t kProbe8001A2B0RetryCount = 4u;
constexpr int32_t kLbaToMsf80036974BiasSectors = 150;
constexpr int32_t kSeek8001A89CSyncMode = 2;
constexpr int32_t kSeek8001A89CSyncArg2 = 0;
constexpr int32_t kRead8001A818WaitArg = 3;
constexpr int32_t kRead8001A818SyncArg0 = 1;
constexpr int32_t kRead8001A818SyncArg1 = 0;
constexpr int32_t kRead8001A818SuccessSyncResult = 0;
constexpr int32_t kReadStart80038FC0SectorSize = 512;
constexpr int32_t kReadStart80038FC0TransferUnit = 32;
constexpr int32_t kReadSync800390C8TimeoutVblanks = 1200;
constexpr int32_t kReadSync800390C8PumpAfterVblanks = 60;
constexpr uint32_t kIso9660LookupSectorBytes800381F8 = 2352u;
constexpr uint32_t kIso9660LookupUserDataOffset800381F8 = 24u;
constexpr uint32_t kIso9660LookupPvdSector800381F8 = 16u;
constexpr uint32_t kIso9660LookupPvdRootRecordOffset800381F8 = 156u;

struct BcdMsf80036974 {
    uint8_t minute = 0;
    uint8_t second = 0;
    uint8_t frame = 0;
};

enum class ActionKind : uint8_t {
    None = 0,
    Lookup800381F8,
    SeekSync800367A4,
    ReadStart80038FC0,
    VBlankBoundary80035560,
    ReadSync800390C8,
};

struct Action {
    ActionKind kind = ActionKind::None;
    uint32_t psxFunction = 0;
    uint32_t callerFunction = 0;
    uint32_t directHelperFunction = 0;
    uint32_t finalFunction = 0;
    uint8_t sequenceIndex = 0;
    uint8_t retryIndex = 0;

    uint32_t cdlFilePtr = 0;
    uint32_t pathPtr = 0;
    uint32_t msfTargetPtr = 0;
    uint32_t dstPtr = 0;

    int32_t lba = 0;
    int32_t sectorCount = 0;
    int32_t modeFlag = 0;
    int32_t arg0 = 0;
    int32_t arg1 = 0;
    int32_t arg2 = 0;
    int32_t sectorSize = 0;
    int32_t transferUnit = 0;
    int32_t timeoutVblanks = 0;
    int32_t pumpAfterVblanks = 0;
    BcdMsf80036974 msf{};
};

struct Feedback {
    ActionKind kind = ActionKind::None;
    bool handled = false;
    bool success = false;
    int32_t psxReturn = 0;
};

// Request-side facts for the real lower CD seam. These fields describe the
// PSX call request only; they do not prove ISO/bin availability.
struct LowerActionRequestMetadata {
    bool known = false;
    ActionKind actionKind = ActionKind::None;
    uint32_t callerFunction = 0;
    uint32_t directHelperFunction = 0;
    uint32_t lowerFunction = 0;
    uint32_t finalFunction = 0;

    bool seekRequestKnown = false;
    bool seekMsfTargetPtrKnown = false;
    uint32_t seekMsfTargetPtr = 0;
    bool seekLbaKnown = false;
    int32_t seekLba = 0;
    bool seekMsfTargetKnown = false;
    BcdMsf80036974 seekMsfTarget{};
    int32_t seekArg0 = 0;
    int32_t seekArg1 = 0;
    int32_t seekArg2 = 0;

    bool readStartRequestKnown = false;
    bool readStartDstPtrKnown = false;
    uint32_t readStartDstPtr = 0;
    bool readStartSectorCountKnown = false;
    int32_t readStartSectorCount = 0;
    bool readStartModeFlagKnown = false;
    int32_t readStartModeFlag = 0;
    int32_t readStartArg0 = 0;
    int32_t readStartArg1 = 0;

    bool readSyncRequestKnown = false;
    uint32_t readSyncFunction = 0;
    int32_t readSyncArg0 = 0;
    int32_t readSyncArg1 = 0;
};

struct SeekSyncHalInput800367A4 {
    bool requestKnown = false;
    LowerActionRequestMetadata request{};
};

struct SeekSyncHalFacts800367A4 {
    bool known = false;
    bool commandAcceptedKnown = false;
    bool commandAccepted = false;
    bool seekCompletedKnown = false;
    bool seekCompleted = false;
    bool seekFailedKnown = false;
    bool seekFailed = false;
    bool timeoutKnown = false;
    bool timeout = false;
};

struct ReadStartHalInput80038FC0 {
    bool requestKnown = false;
    LowerActionRequestMetadata request{};
    bool commandAcceptedKnown = false;
    bool commandAccepted = false;
    bool readStartedKnown = false;
    bool readStarted = false;
    bool readFailedKnown = false;
    bool readFailed = false;
    bool timeoutKnown = false;
    bool timeout = false;
    bool dstPtrKnown = false;
    uint32_t dstPtr = 0;
    bool sectorCountKnown = false;
    int32_t sectorCount = 0;
};

struct ReadStartHalFacts80038FC0 {
    bool known = false;
    bool commandAcceptedKnown = false;
    bool commandAccepted = false;
    bool readStartedKnown = false;
    bool readStarted = false;
    bool readFailedKnown = false;
    bool readFailed = false;
    bool timeoutKnown = false;
    bool timeout = false;
    bool dstPtrKnown = false;
    uint32_t dstPtr = 0;
    bool sectorCountKnown = false;
    int32_t sectorCount = 0;
};

struct LookupFeedback800381F8 {
    Feedback result{};
    bool requestKnown = false;
    uint32_t requestCdlFilePtr = 0;
    bool requestPathPtrKnown = false;
    uint32_t requestPathPtr = 0;
    bool cdlFilePosKnown = false;
    BcdMsf80036974 cdlFilePos{};
    bool cdlFileSizeKnown = false;
    uint32_t cdlFileSize = 0;
    bool cdlFileNameKnown = false;
    std::array<uint8_t, 16> cdlFileName{};
};

struct ProbeCompletionFeedback8001A2B0 {
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
    BcdMsf80036974 cdlFilePos{};
    bool cdlFileSizeKnown = false;
    uint32_t cdlFileSize = 0;
    bool cdlFileNameKnown = false;
    std::array<uint8_t, 16> cdlFileName{};
};

struct Probe8001A2B0State {
    bool active = false;
    bool complete = false;
    uint32_t originalCdlFilePtr = 0;
    uint32_t currentCdlFilePtr = 0;
    uint32_t pathPtr = 0;
    uint8_t nextRetryIndex = 0;
    uint32_t resultPtr = 0;
};

struct Iso9660LookupInput800381F8 {
    bool valid = false;
    std::filesystem::path binPath;
    const char* psxPath = nullptr;
    uint32_t cdlFilePtr = 0;
    uint32_t pathPtr = 0;
    uint8_t retryIndex = 0;
    uint32_t sectorBytes = kIso9660LookupSectorBytes800381F8;
    uint32_t userDataOffset = kIso9660LookupUserDataOffset800381F8;
};

struct Iso9660LookupResult800381F8 {
    bool attempted = false;
    bool success = false;
    bool binPathKnown = false;
    std::filesystem::path binPath;
    bool psxPathKnown = false;
    std::string psxPath{};
    uint32_t cdlFilePtr = 0;
    uint32_t pathPtr = 0;
    uint8_t retryIndex = 0;
    bool sectorLayoutKnown = false;
    uint32_t sectorBytes = 0;
    uint32_t userDataOffset = 0;
    uint32_t pvdSector = 0;
    uint32_t pvdRootRecordOffset = 0;
    bool pvdKnown = false;
    uint32_t rootExtentLba = 0;
    uint32_t rootSize = 0;
    uint32_t matchedExtentLba = 0;
    uint32_t matchedSize = 0;
    std::array<uint8_t, 16> matchedCdlFileName{};
    LookupFeedback800381F8 feedback{};
};

struct Iso9660UserDataReadInput8001A818 {
    bool valid = false;
    std::filesystem::path binPath;
    int32_t lba = 0;
    uint32_t byteCount = 0;
    uint32_t sectorBytes = kIso9660LookupSectorBytes800381F8;
    uint32_t userDataOffset = kIso9660LookupUserDataOffset800381F8;
};

struct Iso9660UserDataReadResult8001A818 {
    bool attempted = false;
    bool success = false;
    bool binPathKnown = false;
    std::filesystem::path binPath;
    bool lbaKnown = false;
    int32_t lba = 0;
    bool byteCountKnown = false;
    uint32_t byteCount = 0;
    bool sectorLayoutKnown = false;
    uint32_t sectorBytes = 0;
    uint32_t userDataOffset = 0;
    std::vector<uint8_t> bytes;
};

BcdMsf80036974 LbaToBcdMsf80036974(int32_t lba);
uint8_t BuildReadModeFlag8001A818(int32_t parserFlag);
bool IsReadSyncPending8001A818(int32_t syncResult);
int32_t ResolveReadResult8001A818(int32_t sectorCount, int32_t syncResult);
Iso9660LookupResult800381F8 BuildIso9660LookupFeedback800381F8(
    const Iso9660LookupInput800381F8& input);
Iso9660UserDataReadResult8001A818 ReadIso9660UserDataBytes8001A818(
    const Iso9660UserDataReadInput8001A818& input);

Probe8001A2B0State BeginProbe8001A2B0(uint32_t cdlFilePtr,
                                      uint32_t pathPtr);
bool BuildNextLookupAction8001A2B0(const Probe8001A2B0State& state,
                                   Action& out);
void ApplyLookupFeedback8001A2B0(Probe8001A2B0State& state,
                                 const Feedback& feedback);
void ApplyLookupFeedback8001A2B0(Probe8001A2B0State& state,
                                 const LookupFeedback800381F8& feedback);
ProbeCompletionFeedback8001A2B0 BuildProbeCompletionFeedback8001A2B0(
    const Probe8001A2B0State& state,
    const LookupFeedback800381F8& feedback);

Action MakeLookupAction800381F8(uint32_t cdlFilePtr,
                                uint32_t pathPtr,
                                uint8_t retryIndex);
Action MakeSeekSyncAction800367A4(uint32_t msfTargetPtr, int32_t lba);
Action MakeReadStartAction80038FC0(int32_t sectorCount,
                                   uint32_t dstPtr,
                                   int32_t modeFlag);
Action MakeVBlankBoundaryAction80035560(int32_t waitArg);
Action MakeReadSyncAction800390C8();
LowerActionRequestMetadata BuildLowerActionRequestMetadata(
    const Action& action);
SeekSyncHalFacts800367A4 BuildSeekSyncHalFacts800367A4(
    const SeekSyncHalInput800367A4& input);
ReadStartHalFacts80038FC0 BuildReadStartHalFacts80038FC0(
    const ReadStartHalInput80038FC0& input);

} // namespace PrStage1LoaderCdHal
