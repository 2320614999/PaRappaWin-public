#include "pr_stage1_loader_cd_hal.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace PrStage1LoaderCdHal {
namespace {

uint8_t ToBcd(int32_t value) {
    return static_cast<uint8_t>(((value / 10) << 4) | (value % 10));
}

void CompleteProbe(Probe8001A2B0State& state, uint32_t resultPtr) {
    state.active = false;
    state.complete = true;
    state.resultPtr = resultPtr;
}

uint32_t ReadLe32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

bool ReadMode2UserDataSector(std::ifstream& bin,
                             uint32_t lba,
                             uint32_t sectorBytes,
                             uint32_t userDataOffset,
                             std::vector<uint8_t>& out) {
    if (sectorBytes < 2048u || userDataOffset > sectorBytes - 2048u) {
        return false;
    }
    out.assign(2048u, 0u);
    const std::streamoff offset =
        static_cast<std::streamoff>(lba) * sectorBytes + userDataOffset;
    bin.seekg(offset, std::ios::beg);
    if (!bin.good()) {
        return false;
    }
    bin.read(reinterpret_cast<char*>(out.data()),
             static_cast<std::streamsize>(out.size()));
    return bin.gcount() == static_cast<std::streamsize>(out.size());
}

bool ReadMode2UserDataRange(std::ifstream& bin,
                            uint32_t lba,
                            uint32_t size,
                            uint32_t sectorBytes,
                            uint32_t userDataOffset,
                            std::vector<uint8_t>& out) {
    out.assign(size, 0u);
    uint32_t copied = 0;
    std::vector<uint8_t> sector{};
    while (copied < size) {
        if (!ReadMode2UserDataSector(bin,
                                     lba + copied / 2048u,
                                     sectorBytes,
                                     userDataOffset,
                                     sector)) {
            return false;
        }
        const uint32_t inSector = copied % 2048u;
        const uint32_t chunk =
            (std::min)(2048u - inSector, size - copied);
        std::memcpy(out.data() + copied, sector.data() + inSector, chunk);
        copied += chunk;
    }
    return true;
}

std::string NormalizeIsoPath800381F8(const char* psxPath) {
    std::string path = psxPath != nullptr ? psxPath : "";
    std::replace(path.begin(), path.end(), '/', '\\');
    while (!path.empty() && path.front() == '\\') {
        path.erase(path.begin());
    }
    std::transform(path.begin(), path.end(), path.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return path;
}

std::vector<std::string> SplitIsoPath800381F8(const std::string& path) {
    std::vector<std::string> out{};
    std::size_t pos = 0;
    while (pos < path.size()) {
        const std::size_t next = path.find('\\', pos);
        const std::size_t end = next == std::string::npos ? path.size() : next;
        if (end > pos) {
            out.push_back(path.substr(pos, end - pos));
        }
        if (next == std::string::npos) {
            break;
        }
        pos = next + 1u;
    }
    return out;
}

bool IsoNameMatches800381F8(const std::string& recordName,
                            const std::string& wanted) {
    if (recordName == wanted) {
        return true;
    }
    const std::size_t semicolon = recordName.find(';');
    if (semicolon != std::string::npos &&
        recordName.substr(0, semicolon) == wanted) {
        return true;
    }
    const std::size_t wantedSemicolon = wanted.find(';');
    if (wantedSemicolon != std::string::npos &&
        wanted.substr(0, wantedSemicolon) == recordName) {
        return true;
    }
    return false;
}

void FillCdlFileName800381F8(const std::string& isoName,
                             std::array<uint8_t, 16>& out) {
    out = {};
    const std::size_t count = (std::min)(isoName.size(), out.size());
    for (std::size_t i = 0; i < count; ++i) {
        out[i] = static_cast<uint8_t>(isoName[i]);
    }
}

struct IsoDirectoryRecord800381F8 {
    bool valid = false;
    bool directory = false;
    uint32_t extentLba = 0;
    uint32_t size = 0;
    std::string name{};
};

bool FindInDirectory800381F8(std::ifstream& bin,
                             uint32_t dirLba,
                             uint32_t dirSize,
                             uint32_t sectorBytes,
                             uint32_t userDataOffset,
                             const std::string& wanted,
                             IsoDirectoryRecord800381F8& out) {
    out = {};
    std::vector<uint8_t> dir{};
    if (!ReadMode2UserDataRange(bin,
                                dirLba,
                                dirSize,
                                sectorBytes,
                                userDataOffset,
                                dir)) {
        return false;
    }
    std::size_t pos = 0;
    while (pos < dir.size()) {
        const uint8_t len = dir[pos];
        if (len == 0u) {
            pos = ((pos / 2048u) + 1u) * 2048u;
            continue;
        }
        if (pos + len > dir.size() || len < 34u) {
            return false;
        }
        const uint8_t nameLen = dir[pos + 32u];
        if (33u + nameLen > len) {
            return false;
        }
        if (nameLen == 1u &&
            (dir[pos + 33u] == 0u || dir[pos + 33u] == 1u)) {
            pos += len;
            continue;
        }
        std::string name(reinterpret_cast<const char*>(dir.data() + pos + 33u),
                         nameLen);
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });
        if (IsoNameMatches800381F8(name, wanted)) {
            out.valid = true;
            out.extentLba = ReadLe32(dir.data() + pos + 2u);
            out.size = ReadLe32(dir.data() + pos + 10u);
            out.directory = (dir[pos + 25u] & 0x02u) != 0u;
            out.name = name;
            return true;
        }
        pos += len;
    }
    return false;
}

} // namespace

BcdMsf80036974 LbaToBcdMsf80036974(int32_t lba) {
    const int32_t biased = lba + kLbaToMsf80036974BiasSectors;
    const int32_t totalSeconds = biased / 75;

    BcdMsf80036974 out{};
    out.minute = ToBcd(totalSeconds / 60);
    out.second = ToBcd(totalSeconds % 60);
    out.frame = ToBcd(biased % 75);
    return out;
}

uint8_t BuildReadModeFlag8001A818(int32_t parserFlag) {
    return static_cast<uint8_t>((parserFlag == 1 ? 1 : 0) << 7);
}

bool IsReadSyncPending8001A818(int32_t syncResult) {
    return syncResult > 0;
}

int32_t ResolveReadResult8001A818(int32_t sectorCount, int32_t syncResult) {
    return syncResult == kRead8001A818SuccessSyncResult ? sectorCount : 0;
}

Iso9660LookupResult800381F8 BuildIso9660LookupFeedback800381F8(
    const Iso9660LookupInput800381F8& input) {
    Iso9660LookupResult800381F8 out{};
    out.attempted = true;
    out.binPathKnown = !input.binPath.empty();
    out.binPath = input.binPath;
    out.psxPathKnown = input.psxPath != nullptr;
    out.psxPath = input.psxPath != nullptr ? input.psxPath : "";
    out.cdlFilePtr = input.cdlFilePtr;
    out.pathPtr = input.pathPtr;
    out.retryIndex = input.retryIndex;
    out.sectorBytes = input.sectorBytes;
    out.userDataOffset = input.userDataOffset;
    out.pvdSector = kIso9660LookupPvdSector800381F8;
    out.pvdRootRecordOffset = kIso9660LookupPvdRootRecordOffset800381F8;
    out.sectorLayoutKnown =
        input.sectorBytes >= 2048u &&
        input.userDataOffset <= input.sectorBytes - 2048u;
    out.feedback.result.kind = ActionKind::Lookup800381F8;
    out.feedback.result.handled = true;
    out.feedback.result.success = false;
    out.feedback.result.psxReturn = 0;
    out.feedback.requestKnown = input.cdlFilePtr != 0u;
    out.feedback.requestCdlFilePtr = input.cdlFilePtr;
    out.feedback.requestPathPtrKnown = input.pathPtr != 0u;
    out.feedback.requestPathPtr = input.pathPtr;
    if (!input.valid || !out.psxPathKnown || input.binPath.empty() ||
        !out.sectorLayoutKnown) {
        return out;
    }

    std::ifstream bin(input.binPath, std::ios::binary);
    if (!bin) {
        return out;
    }

    std::vector<uint8_t> pvd{};
    if (!ReadMode2UserDataSector(bin,
                                 kIso9660LookupPvdSector800381F8,
                                 input.sectorBytes,
                                 input.userDataOffset,
                                 pvd)) {
        return out;
    }
    if (pvd.size() < 2048u ||
        pvd[0] != 1u ||
        std::memcmp(pvd.data() + 1u, "CD001", 5u) != 0) {
        return out;
    }
    out.pvdKnown = true;
    const uint8_t* root =
        pvd.data() + kIso9660LookupPvdRootRecordOffset800381F8;
    out.rootExtentLba = ReadLe32(root + 2u);
    out.rootSize = ReadLe32(root + 10u);

    const std::vector<std::string> parts =
        SplitIsoPath800381F8(NormalizeIsoPath800381F8(input.psxPath));
    if (parts.empty()) {
        return out;
    }

    uint32_t dirLba = out.rootExtentLba;
    uint32_t dirSize = out.rootSize;
    IsoDirectoryRecord800381F8 record{};
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (!FindInDirectory800381F8(bin,
                                    dirLba,
                                    dirSize,
                                    input.sectorBytes,
                                    input.userDataOffset,
                                    parts[i],
                                    record)) {
            return out;
        }
        const bool last = i + 1u == parts.size();
        if (!last) {
            if (!record.directory) {
                return out;
            }
            dirLba = record.extentLba;
            dirSize = record.size;
            continue;
        }
    }

    if (!record.valid || record.directory) {
        return out;
    }
    out.success = true;
    out.matchedExtentLba = record.extentLba;
    out.matchedSize = record.size;
    FillCdlFileName800381F8(record.name, out.matchedCdlFileName);
    out.feedback.result.success = true;
    out.feedback.result.psxReturn = 1;
    out.feedback.cdlFilePosKnown = true;
    out.feedback.cdlFilePos =
        LbaToBcdMsf80036974(static_cast<int32_t>(record.extentLba));
    out.feedback.cdlFileSizeKnown = true;
    out.feedback.cdlFileSize = record.size;
    out.feedback.cdlFileNameKnown = true;
    out.feedback.cdlFileName = out.matchedCdlFileName;
    return out;
}

Iso9660UserDataReadResult8001A818 ReadIso9660UserDataBytes8001A818(
    const Iso9660UserDataReadInput8001A818& input) {
    Iso9660UserDataReadResult8001A818 out{};
    out.attempted = true;
    out.binPathKnown = !input.binPath.empty();
    out.binPath = input.binPath;
    out.lbaKnown = input.lba >= 0;
    out.lba = input.lba;
    out.byteCountKnown = input.byteCount != 0u;
    out.byteCount = input.byteCount;
    out.sectorBytes = input.sectorBytes;
    out.userDataOffset = input.userDataOffset;
    out.sectorLayoutKnown =
        input.sectorBytes >= 2048u &&
        input.userDataOffset <= input.sectorBytes - 2048u;
    if (!input.valid || input.binPath.empty() || input.lba < 0 ||
        input.byteCount == 0u || !out.sectorLayoutKnown) {
        return out;
    }

    std::ifstream bin(input.binPath, std::ios::binary);
    if (!bin) {
        return out;
    }

    out.success = ReadMode2UserDataRange(
        bin,
        static_cast<uint32_t>(input.lba),
        input.byteCount,
        input.sectorBytes,
        input.userDataOffset,
        out.bytes);
    if (!out.success) {
        out.bytes.clear();
    }
    return out;
}

Probe8001A2B0State BeginProbe8001A2B0(uint32_t cdlFilePtr,
                                      uint32_t pathPtr) {
    Probe8001A2B0State out{};
    out.originalCdlFilePtr = cdlFilePtr;
    out.currentCdlFilePtr = cdlFilePtr;
    out.pathPtr = pathPtr;
    if (pathPtr == 0u) {
        CompleteProbe(out, 0u);
        return out;
    }

    out.active = true;
    return out;
}

bool BuildNextLookupAction8001A2B0(const Probe8001A2B0State& state,
                                   Action& out) {
    out = Action{};
    if (!state.active || state.complete ||
        state.nextRetryIndex >= kProbe8001A2B0RetryCount) {
        return false;
    }

    out = MakeLookupAction800381F8(state.currentCdlFilePtr,
                                   state.pathPtr,
                                   state.nextRetryIndex);
    return true;
}

void ApplyLookupFeedback8001A2B0(Probe8001A2B0State& state,
                                 const Feedback& feedback) {
    if (!state.active || state.complete ||
        feedback.kind != ActionKind::Lookup800381F8 || !feedback.handled) {
        return;
    }

    if (feedback.success || feedback.psxReturn != 0) {
        CompleteProbe(state, state.originalCdlFilePtr);
        return;
    }

    ++state.nextRetryIndex;
    state.currentCdlFilePtr = state.originalCdlFilePtr;
    if (state.nextRetryIndex >= kProbe8001A2B0RetryCount) {
        CompleteProbe(state, 0u);
    }
}

void ApplyLookupFeedback8001A2B0(
    Probe8001A2B0State& state,
    const LookupFeedback800381F8& feedback) {
    ApplyLookupFeedback8001A2B0(state, feedback.result);
}

ProbeCompletionFeedback8001A2B0 BuildProbeCompletionFeedback8001A2B0(
    const Probe8001A2B0State& state,
    const LookupFeedback800381F8& feedback) {
    ProbeCompletionFeedback8001A2B0 out{};
    if (!state.complete) {
        return out;
    }

    out.known = true;
    out.complete = true;
    out.explicitCdLookupFeedback =
        feedback.result.handled &&
        feedback.result.kind == ActionKind::Lookup800381F8;
    out.requestKnown = state.originalCdlFilePtr != 0u;
    out.requestCdlFilePtr = state.originalCdlFilePtr;
    out.requestPathPtrKnown = state.pathPtr != 0u;
    out.requestPathPtr = state.pathPtr;
    out.resultPtr = state.resultPtr;
    out.lookupSucceeded =
        state.resultPtr != 0u && feedback.result.handled &&
        feedback.result.kind == ActionKind::Lookup800381F8 &&
        (feedback.result.success || feedback.result.psxReturn != 0);
    out.lookupFailed = !out.lookupSucceeded;

    if (out.lookupSucceeded) {
        out.cdlFilePosKnown = feedback.cdlFilePosKnown;
        out.cdlFilePos = feedback.cdlFilePos;
        out.cdlFileSizeKnown = feedback.cdlFileSizeKnown;
        out.cdlFileSize = feedback.cdlFileSize;
        out.cdlFileNameKnown = feedback.cdlFileNameKnown;
        out.cdlFileName = feedback.cdlFileName;
    }
    return out;
}

Action MakeLookupAction800381F8(uint32_t cdlFilePtr,
                                uint32_t pathPtr,
                                uint8_t retryIndex) {
    Action out{};
    out.kind = ActionKind::Lookup800381F8;
    out.psxFunction = kFn800381F8;
    out.callerFunction = kFn8001A2B0;
    out.retryIndex = retryIndex;
    out.cdlFilePtr = cdlFilePtr;
    out.pathPtr = pathPtr;
    return out;
}

Action MakeSeekSyncAction800367A4(uint32_t msfTargetPtr, int32_t lba) {
    Action out{};
    out.kind = ActionKind::SeekSync800367A4;
    out.psxFunction = kFn800367A4;
    out.callerFunction = kFn8001A89C;
    out.directHelperFunction = kFn80036974;
    out.msfTargetPtr = msfTargetPtr;
    out.lba = lba;
    out.arg0 = kSeek8001A89CSyncMode;
    out.arg1 = static_cast<int32_t>(msfTargetPtr);
    out.arg2 = kSeek8001A89CSyncArg2;
    out.msf = LbaToBcdMsf80036974(lba);
    return out;
}

Action MakeReadStartAction80038FC0(int32_t sectorCount,
                                   uint32_t dstPtr,
                                   int32_t modeFlag) {
    Action out{};
    out.kind = ActionKind::ReadStart80038FC0;
    out.psxFunction = kFn80038FC0;
    out.callerFunction = kFn8001A818;
    out.finalFunction = kFn800390C8;
    out.sectorCount = sectorCount;
    out.dstPtr = dstPtr;
    out.modeFlag = modeFlag;
    out.arg0 = sectorCount;
    out.arg1 = static_cast<int32_t>(dstPtr);
    out.sectorSize = kReadStart80038FC0SectorSize;
    out.transferUnit = kReadStart80038FC0TransferUnit;
    return out;
}

Action MakeVBlankBoundaryAction80035560(int32_t waitArg) {
    Action out{};
    out.kind = ActionKind::VBlankBoundary80035560;
    out.psxFunction = kFn80035560;
    out.callerFunction = kFn8001A818;
    out.arg0 = waitArg;
    return out;
}

Action MakeReadSyncAction800390C8() {
    Action out{};
    out.kind = ActionKind::ReadSync800390C8;
    out.psxFunction = kFn800390C8;
    out.callerFunction = kFn8001A818;
    out.arg0 = kRead8001A818SyncArg0;
    out.arg1 = kRead8001A818SyncArg1;
    out.timeoutVblanks = kReadSync800390C8TimeoutVblanks;
    out.pumpAfterVblanks = kReadSync800390C8PumpAfterVblanks;
    out.finalFunction = kFn800364F0;
    return out;
}

LowerActionRequestMetadata BuildLowerActionRequestMetadata(
    const Action& action) {
    LowerActionRequestMetadata out{};
    if (action.kind == ActionKind::None) {
        return out;
    }

    out.known = true;
    out.actionKind = action.kind;
    out.callerFunction = action.callerFunction;
    out.directHelperFunction = action.directHelperFunction;
    out.lowerFunction = action.psxFunction;
    out.finalFunction = action.finalFunction;

    switch (action.kind) {
    case ActionKind::SeekSync800367A4:
        out.seekRequestKnown = true;
        out.seekMsfTargetPtrKnown = action.msfTargetPtr != 0u;
        out.seekMsfTargetPtr = action.msfTargetPtr;
        out.seekLbaKnown = true;
        out.seekLba = action.lba;
        out.seekMsfTargetKnown = true;
        out.seekMsfTarget = action.msf;
        out.seekArg0 = action.arg0;
        out.seekArg1 = action.arg1;
        out.seekArg2 = action.arg2;
        break;
    case ActionKind::ReadStart80038FC0:
        out.readStartRequestKnown = true;
        out.readStartDstPtrKnown = action.dstPtr != 0u;
        out.readStartDstPtr = action.dstPtr;
        out.readStartSectorCountKnown = true;
        out.readStartSectorCount = action.sectorCount;
        out.readStartModeFlagKnown = true;
        out.readStartModeFlag = action.modeFlag;
        out.readStartArg0 = action.arg0;
        out.readStartArg1 = action.arg1;
        out.readSyncRequestKnown = true;
        out.readSyncFunction = kFn800390C8;
        out.readSyncArg0 = kRead8001A818SyncArg0;
        out.readSyncArg1 = kRead8001A818SyncArg1;
        break;
    case ActionKind::ReadSync800390C8:
        out.readSyncRequestKnown = true;
        out.readSyncFunction = action.psxFunction;
        out.readSyncArg0 = action.arg0;
        out.readSyncArg1 = action.arg1;
        break;
    default:
        break;
    }
    return out;
}

SeekSyncHalFacts800367A4 BuildSeekSyncHalFacts800367A4(
    const SeekSyncHalInput800367A4& input) {
    SeekSyncHalFacts800367A4 out{};
    out.known = true;
    out.commandAcceptedKnown = true;
    out.seekCompletedKnown = true;
    out.seekFailedKnown = true;
    out.timeoutKnown = true;

    const LowerActionRequestMetadata& request = input.request;
    const bool requestShapeOk =
        input.requestKnown &&
        request.known &&
        request.actionKind == ActionKind::SeekSync800367A4 &&
        request.callerFunction == kFn8001A89C &&
        request.directHelperFunction == kFn80036974 &&
        request.lowerFunction == kFn800367A4 &&
        request.finalFunction == 0u &&
        request.seekRequestKnown &&
        request.seekArg0 == kSeek8001A89CSyncMode &&
        (!request.seekMsfTargetPtrKnown ||
         request.seekArg1 == static_cast<int32_t>(request.seekMsfTargetPtr)) &&
        request.seekArg2 == kSeek8001A89CSyncArg2;

    if (!requestShapeOk) {
        out.commandAccepted = false;
        out.seekCompleted = false;
        out.seekFailed = true;
        out.timeout = false;
        return out;
    }

    out.commandAccepted = true;
    out.seekCompleted = true;
    out.seekFailed = false;
    out.timeout = false;
    return out;
}

ReadStartHalFacts80038FC0 BuildReadStartHalFacts80038FC0(
    const ReadStartHalInput80038FC0& input) {
    ReadStartHalFacts80038FC0 out{};

    const LowerActionRequestMetadata& request = input.request;
    const bool requestShapeOk =
        input.requestKnown &&
        request.known &&
        request.actionKind == ActionKind::ReadStart80038FC0 &&
        request.callerFunction == kFn8001A818 &&
        request.lowerFunction == kFn80038FC0 &&
        request.finalFunction == kFn800390C8 &&
        request.readStartRequestKnown &&
        request.readStartSectorCountKnown &&
        request.readStartModeFlagKnown &&
        request.readStartArg0 == request.readStartSectorCount &&
        request.readStartArg1 == static_cast<int32_t>(request.readStartDstPtr) &&
        request.readSyncRequestKnown &&
        request.readSyncFunction == kFn800390C8 &&
        request.readSyncArg0 == kRead8001A818SyncArg0 &&
        request.readSyncArg1 == kRead8001A818SyncArg1;

    if (!requestShapeOk ||
        !input.commandAcceptedKnown ||
        !input.readStartedKnown ||
        !input.readFailedKnown ||
        !input.timeoutKnown ||
        !input.sectorCountKnown ||
        input.sectorCount != request.readStartSectorCount) {
        return out;
    }
    if (request.readStartDstPtrKnown &&
        (!input.dstPtrKnown || input.dstPtr != request.readStartDstPtr)) {
        return out;
    }

    out.known = true;
    out.commandAcceptedKnown = true;
    out.commandAccepted = input.commandAccepted;
    out.readStartedKnown = true;
    out.readStarted = input.readStarted;
    out.readFailedKnown = true;
    out.readFailed = input.readFailed;
    out.timeoutKnown = true;
    out.timeout = input.timeout;
    out.dstPtrKnown = input.dstPtrKnown;
    out.dstPtr = input.dstPtr;
    out.sectorCountKnown = true;
    out.sectorCount = input.sectorCount;
    return out;
}

} // namespace PrStage1LoaderCdHal
