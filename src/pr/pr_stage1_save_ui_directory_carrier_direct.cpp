#include "pr_stage1_save_card_hal_direct.h"

#include <cstring>

namespace PrStage1SaveCardHalDirect {
namespace {

SaveUiDirectoryScanTypedCarrier80019458
    s_saveUiDirectoryScanTypedCarrier80019458{};

constexpr std::size_t kSaveUiDirectoryRawRowCount80019458 = 15u;
constexpr std::size_t kSaveUiDirectoryRawRowBytes80019458 = 40u;
constexpr std::size_t kSaveUiDirectoryRawBankBytes80019458 =
    kSaveUiDirectoryRawRowCount80019458 *
    kSaveUiDirectoryRawRowBytes80019458;
constexpr std::size_t kSaveUiDirectoryRawNameBytes80019458 = 20u;
constexpr std::size_t kSaveUiDirectoryRawSizeOffset80019458 = 24u;
constexpr uint32_t kFn80017B18 = 0x80017B18u;
constexpr uint32_t kFn800178C8 = 0x800178C8u;
constexpr uint32_t kFn80017354 = 0x80017354u;
constexpr uint32_t kSaveUiDirectoryRawBank8007A318 = 0x8007A318u;
constexpr const char kSaveUiDirectoryFilenamePrefix80019458[] =
    "BASCUS-94183";

uint32_t ReadU32LE80019458(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

bool CopyBoundedDirectorySuffix80019458(char (&dst)[32],
                                        const uint8_t* name) {
    dst[0] = '\0';
    constexpr std::size_t kPrefixLen =
        sizeof(kSaveUiDirectoryFilenamePrefix80019458) - 1u;
    if (std::memcmp(name,
                    kSaveUiDirectoryFilenamePrefix80019458,
                    kPrefixLen) != 0) {
        return false;
    }

    std::size_t out = 0u;
    for (std::size_t i = kPrefixLen;
         i < kSaveUiDirectoryRawNameBytes80019458 && out + 1u < sizeof(dst);
         ++i) {
        const uint8_t ch = name[i];
        if (ch == 0) {
            break;
        }
        if (ch < 0x20u || ch >= 0x7Fu) {
            dst[0] = '\0';
            return false;
        }
        dst[out++] = static_cast<char>(ch);
    }
    dst[out] = '\0';
    return true;
}

int32_t BlocksForDirectoryEntry80019458(uint32_t byteCount) {
    if (byteCount == 0u) {
        return 0;
    }
    const uint32_t blocks = (byteCount + 0x1FFFu) >> 13;
    return blocks > kSaveUiDirectoryRawRowCount80019458
               ? static_cast<int32_t>(kSaveUiDirectoryRawRowCount80019458)
               : static_cast<int32_t>(blocks);
}

bool IsExpectedSaveUiDirectoryRequest80019458(
    const PrStage1SaveUi19148LowerFeedbackRequest& request) {
    if (request.kind !=
            PrStage1SaveUi19148LowerFeedbackRequestKind::
                DirectoryRows80019458 ||
        request.psxFunction != kFn80017B18 ||
        request.action.kind !=
            PrStage1SaveUi19148ActionKind::Call80017B18SnapshotDirectory ||
        !((request.stateBefore == 6 && request.stateAfter == 9) ||
          (request.stateBefore == 9 && request.stateAfter == 11)) ||
        static_cast<uint32_t>(request.action.arg0) !=
            kSaveUiDirectoryRawBank8007A318 ||
        static_cast<uint32_t>(request.action.arg1) != kFn800178C8 ||
        static_cast<uint32_t>(request.action.arg2) != kFn80017354 ||
        request.action.arg3 !=
            static_cast<int32_t>(kSaveUiDirectoryRawRowCount80019458)) {
        return false;
    }
    return true;
}

}  // namespace

bool BuildRuntimeSaveUiDirectoryScanFactsFromRawBank80019458(
    const SaveUiDirectoryRawBankFacts80019458& rawFacts,
    SaveUiDirectoryScanRuntimeFacts80019458* out) {
    if (!out) {
        return false;
    }
    *out = {};
    out->factsKnown = rawFacts.factsKnown;
    out->directoryRowsKnown80017B08 =
        rawFacts.directoryRowsKnown80017B08;
    out->snapshotKnown80017B18 = rawFacts.snapshotKnown80017B18;
    out->listRowsBuilt80019458 = rawFacts.rawBankKnown8007A318;
    if (!rawFacts.factsKnown ||
        !rawFacts.directoryRowsKnown80017B08 ||
        !rawFacts.snapshotKnown80017B18 ||
        !rawFacts.rawBankKnown8007A318 ||
        !rawFacts.rawBank8007A318 ||
        rawFacts.rawBankByteCount8007A318 <
            kSaveUiDirectoryRawBankBytes80019458) {
        *out = {};
        return false;
    }

    int32_t usedBlocks = 0;
    int32_t entryCount = 0;
    for (int32_t rowIndex = 0;
         rowIndex < static_cast<int32_t>(kSaveUiDirectoryRawRowCount80019458);
         ++rowIndex) {
        const uint8_t* row =
            rawFacts.rawBank8007A318 +
            static_cast<std::size_t>(rowIndex) *
                kSaveUiDirectoryRawRowBytes80019458;
        if (row[0] == 0) {
            continue;
        }
        usedBlocks += BlocksForDirectoryEntry80019458(
            ReadU32LE80019458(row + kSaveUiDirectoryRawSizeOffset80019458));

        PrStage1SaveUiDirectoryRow80019458 decoded{};
        if (!CopyBoundedDirectorySuffix80019458(decoded.suffix, row)) {
            continue;
        }
        decoded.active = true;
        decoded.freeSlot = false;
        decoded.blockIndexKnown = true;
        decoded.blockIndex = rowIndex;
        decoded.suffixKnown = true;
        out->rows[entryCount++] = decoded;
    }

    if (usedBlocks < 0 ||
        usedBlocks > static_cast<int32_t>(kSaveUiDirectoryRawRowCount80019458)) {
        *out = {};
        return false;
    }

    const int32_t freeSlots =
        static_cast<int32_t>(kSaveUiDirectoryRawRowCount80019458) -
        usedBlocks;
    for (int32_t i = 0; i < freeSlots; ++i) {
        if (entryCount >=
            static_cast<int32_t>(kSaveUiDirectoryRawRowCount80019458)) {
            *out = {};
            return false;
        }
        PrStage1SaveUiDirectoryRow80019458 freeRow{};
        freeRow.active = true;
        freeRow.freeSlot = true;
        freeRow.blockIndexKnown = true;
        freeRow.blockIndex = entryCount;
        freeRow.suffixKnown = true;
        freeRow.suffix[0] = '\0';
        out->rows[entryCount++] = freeRow;
    }

    out->factsKnown = true;
    out->directoryRowsKnown80017B08 = true;
    out->snapshotKnown80017B18 = true;
    out->listRowsBuilt80019458 = true;
    out->entryCountKnown = true;
    out->entryCount = entryCount;
    out->freeSlotsKnown = true;
    out->freeSlots = freeSlots;
    return true;
}

void BuildSaveUiDirectoryScanFactsFromRawBankProducerInput80019458(
    const SaveUiDirectoryRawBankProducerInput80019458& input,
    SaveUiDirectoryRawBankProducerResult80019458* out) {
    if (!out) {
        return;
    }
    *out = {};
    if (input.requestKnown) {
        out->requestUsed = true;
        out->requestMatched =
            IsExpectedSaveUiDirectoryRequest80019458(input.request);
        if (!out->requestMatched) {
            out->incomplete = true;
            return;
        }
        out->directoryRowsKnown80017B08 = true;
        out->snapshotKnown80017B18 = true;
    }
    if (!input.requestKnown || !input.rawBankKnown8007A318 ||
        input.rawBank8007A318 == nullptr ||
        input.rawBankByteCount8007A318 <
            kSaveUiDirectoryRawBankBytes80019458) {
        out->incomplete = true;
        return;
    }

    SaveUiDirectoryRawBankFacts80019458 rawFacts{};
    rawFacts.factsKnown = true;
    rawFacts.directoryRowsKnown80017B08 = true;
    rawFacts.snapshotKnown80017B18 = true;
    rawFacts.rawBankKnown8007A318 = true;
    rawFacts.rawBank8007A318 = input.rawBank8007A318;
    rawFacts.rawBankByteCount8007A318 = input.rawBankByteCount8007A318;
    out->rawBankKnown8007A318 = true;
    out->produced =
        BuildRuntimeSaveUiDirectoryScanFactsFromRawBank80019458(rawFacts,
                                                                &out->facts);
    out->incomplete = !out->produced;
}

bool PublishRuntimeSaveUiDirectoryScanTypedCarrier80019458(
    const SaveUiDirectoryScanRuntimeFacts80019458& facts) {
    SaveUiDirectoryScanTypedCarrier80019458 next{};
    next.known = facts.factsKnown;
    next.producerWired80017B08_80017B18_80019458 =
        facts.factsKnown &&
        facts.directoryRowsKnown80017B08 &&
        facts.snapshotKnown80017B18 &&
        facts.listRowsBuilt80019458;
    next.scanFacts.known = facts.factsKnown;
    next.scanFacts.directoryRowsKnown80017B08 =
        facts.directoryRowsKnown80017B08;
    next.scanFacts.snapshotKnown80017B18 = facts.snapshotKnown80017B18;
    next.scanFacts.listRowsBuilt80019458 = facts.listRowsBuilt80019458;
    next.scanFacts.entryCountKnown = facts.entryCountKnown;
    next.scanFacts.entryCount = facts.entryCount;
    next.scanFacts.freeSlotsKnown = facts.freeSlotsKnown;
    next.scanFacts.freeSlots = facts.freeSlots;
    for (int i = 0; i < 15; ++i) {
        next.scanFacts.rows[i] = facts.rows[i];
    }
    next.typedDirectoryRowsKnown80019458 =
        PrStage1SaveUiDirect::
            BuildSaveUiDirectoryRowsFeedbackFromScanFacts80019458(
                next.scanFacts,
                &next.feedback);
    next.incomplete =
        !next.known ||
        !next.producerWired80017B08_80017B18_80019458 ||
        !next.typedDirectoryRowsKnown80019458;
    if (next.incomplete) {
        s_saveUiDirectoryScanTypedCarrier80019458 = {};
        return false;
    }

    s_saveUiDirectoryScanTypedCarrier80019458 = next;
    return true;
}

bool GetSaveUiDirectoryScanTypedCarrier80019458(
    SaveUiDirectoryScanTypedCarrier80019458* out) {
    if (!out) {
        return false;
    }
    *out = {};
    const SaveUiDirectoryScanTypedCarrier80019458& carrier =
        s_saveUiDirectoryScanTypedCarrier80019458;
    if (!carrier.known ||
        !carrier.producerWired80017B08_80017B18_80019458 ||
        !carrier.typedDirectoryRowsKnown80019458 ||
        carrier.incomplete) {
        return false;
    }
    *out = carrier;
    return true;
}

void ClearSaveUiDirectoryScanTypedCarrier80019458() {
    s_saveUiDirectoryScanTypedCarrier80019458 = {};
}

}  // namespace PrStage1SaveCardHalDirect
