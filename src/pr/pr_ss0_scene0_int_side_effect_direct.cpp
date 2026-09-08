#include "pr_ss0_scene0_int_side_effect_direct.h"

#include <cstring>
#include <limits>

namespace PrSS0Scene0IntSideEffectDirect {
namespace {

using PrSS0Scene0IntLoadDirect::BlockMetadata8001A8F0;
using PrSS0Scene0IntLoadDirect::BlockType8001A8F0;
using PrSS0Scene0IntLoadDirect::EntryMetadata8001A8F0;
using PrSS0Scene0IntLoadDirect::ArchiveKind8001AC18;
using PrSS0Scene0IntLoadDirect::Transaction8001AC18;

bool IsExactResetState8001A8F0(
    const PrStage1LoaderMemoryDirectState& state) {
    if (state.gpPlus320LowWater != kScene0PacketArenaLowWater801C4260 ||
        state.gpPlus324StackDepth != 0u ||
        state.gpPlus896HeapBase !=
            kPrStage1LoaderMemoryDirectHeapBase800965B0 ||
        state.gpPlus900HeapEnd !=
            kPrStage1LoaderMemoryDirectHeapEnd801C35B0 ||
        state.gpPlus904HeapCursor !=
            kPrStage1LoaderMemoryDirectHeapBase800965B0) {
        return false;
    }
    for (uint32_t entry : state.stackTable80091858) {
        if (entry != 0u) {
            return false;
        }
    }
    return true;
}

const BlockMetadata8001A8F0* FindBlock8001A8F0(
    const Transaction8001AC18& source,
    uint32_t blockIndex,
    BlockType8001A8F0 type) {
    if (blockIndex >= source.blocks.size()) {
        return nullptr;
    }
    const auto& block = source.blocks[blockIndex];
    return block.blockIndex == blockIndex && block.type == type
               ? &block
               : nullptr;
}

const EntryMetadata8001A8F0* FindEntry8001A8F0(
    const Transaction8001AC18& source,
    const BlockMetadata8001A8F0& block,
    uint32_t entryIndex) {
    const uint64_t flatIndex =
        static_cast<uint64_t>(block.firstEntryIndex) + entryIndex;
    if (entryIndex >= block.files || flatIndex >= source.entries.size()) {
        return nullptr;
    }
    const auto& entry = source.entries[static_cast<std::size_t>(flatIndex)];
    return entry.blockIndex == block.blockIndex &&
                   entry.entryIndex == entryIndex &&
                   entry.type == block.type
               ? &entry
               : nullptr;
}

bool IsArchiveRange8001A8F0(
    const Transaction8001AC18& source,
    uint64_t offset,
    uint64_t byteCount) {
    return offset <= source.archiveBytes.size() &&
           byteCount <= source.archiveBytes.size() - offset;
}

bool TryU32FromU648001A8F0(uint64_t value, uint32_t& out) {
    if (value > (std::numeric_limits<uint32_t>::max)()) {
        return false;
    }
    out = static_cast<uint32_t>(value);
    return true;
}

bool TryI32FromU648001A8F0(uint64_t value, int32_t& out) {
    if (value > static_cast<uint64_t>(
                    (std::numeric_limits<int32_t>::max)())) {
        return false;
    }
    out = static_cast<int32_t>(value);
    return true;
}

bool CopyBlockPayload8001A8F0(
    const Transaction8001AC18& source,
    const BlockMetadata8001A8F0& block,
    PrStage1LoaderMemoryDirectState& destination,
    uint32_t destinationPsxAddress) {
    uint32_t payloadBytes = 0u;
    if (!TryU32FromU648001A8F0(block.payloadBytes, payloadBytes) ||
        !IsArchiveRange8001A8F0(
            source, block.payloadOffset, block.payloadBytes)) {
        return false;
    }
    uint8_t* out = PrStage1LoaderMemoryDirectMutablePtr(
        destination, destinationPsxAddress, payloadBytes);
    if (out == nullptr) {
        return false;
    }
    std::memcpy(out,
                source.archiveBytes.data() + block.payloadOffset,
                payloadBytes);
    return true;
}

bool BuildPayloadRequests8001A8F0(
    const Transaction8001AC18& source,
    const BlockMetadata8001A8F0& block,
    uint32_t destinationPsxAddress,
    std::vector<PayloadRequest8001A8F0>& out) {
    out.clear();
    out.reserve(block.files);
    for (uint32_t index = 0u; index < block.files; ++index) {
        const EntryMetadata8001A8F0* entry =
            FindEntry8001A8F0(source, block, index);
        if (entry == nullptr || entry->dataOffset < block.payloadOffset ||
            !IsArchiveRange8001A8F0(
                source, entry->dataOffset, entry->size)) {
            return false;
        }
        const uint64_t relative = entry->dataOffset - block.payloadOffset;
        const uint64_t address =
            static_cast<uint64_t>(destinationPsxAddress) + relative;
        uint32_t psxAddress = 0u;
        if (!TryU32FromU648001A8F0(address, psxAddress) ||
            !PrStage1LoaderMemoryDirectContainsRange(
                psxAddress, entry->size)) {
            return false;
        }
        PayloadRequest8001A8F0 request{};
        request.blockIndex = block.blockIndex;
        request.entryIndex = index;
        request.type = block.type;
        request.psxAddress = psxAddress;
        request.size = entry->size;
        request.name = entry->name;
        request.archiveDataOffset = entry->dataOffset;
        out.push_back(request);
    }
    return out.size() == block.files;
}

bool EqualPayloadRequest8001A8F0(
    const PayloadRequest8001A8F0& left,
    const PayloadRequest8001A8F0& right) {
    return left.blockIndex == right.blockIndex &&
           left.entryIndex == right.entryIndex && left.type == right.type &&
           left.psxAddress == right.psxAddress && left.size == right.size &&
           left.name == right.name &&
           left.archiveDataOffset == right.archiveDataOffset;
}

bool EqualPayloadRequests8001A8F0(
    const std::vector<PayloadRequest8001A8F0>& left,
    const std::vector<PayloadRequest8001A8F0>& right) {
    if (left.size() != right.size()) {
        return false;
    }
    for (std::size_t index = 0u; index < left.size(); ++index) {
        if (!EqualPayloadRequest8001A8F0(left[index], right[index])) {
            return false;
        }
    }
    return true;
}

bool VerifyVhCandidateBytes8001A8F0(
    const Transaction8001A8F0& transaction,
    const Transaction8001AC18& source,
    const BlockMetadata8001A8F0& vabBlock) {
    const auto* candidate = transaction.loaderMemoryCandidate.get();
    const EntryMetadata8001A8F0* vhEntry =
        FindEntry8001A8F0(source, vabBlock, 0u);
    if (candidate == nullptr || vhEntry == nullptr ||
        !IsArchiveRange8001A8F0(
            source, vhEntry->dataOffset, vhEntry->size)) {
        return false;
    }
    const uint8_t* vhBytes = PrStage1LoaderMemoryDirectConstPtr(
        *candidate, transaction.vabRequest.vhPsxAddress, vhEntry->size);
    return vhBytes != nullptr &&
           std::memcmp(vhBytes,
                       source.archiveBytes.data() + vhEntry->dataOffset,
                       vhEntry->size) == 0;
}

bool VerifyVabBlockCandidateBytes8001A8F0(
    const Transaction8001A8F0& transaction,
    const Transaction8001AC18& source,
    const BlockMetadata8001A8F0& vabBlock) {
    const auto* candidate = transaction.loaderMemoryCandidate.get();
    uint32_t vabPayloadBytes = 0u;
    if (candidate == nullptr ||
        !TryU32FromU648001A8F0(vabBlock.payloadBytes, vabPayloadBytes) ||
        !IsArchiveRange8001A8F0(
            source, vabBlock.payloadOffset, vabBlock.payloadBytes)) {
        return false;
    }
    const uint8_t* vabBytes = PrStage1LoaderMemoryDirectConstPtr(
        *candidate, transaction.vabRequest.vhPsxAddress, vabPayloadBytes);
    return vabBytes != nullptr &&
           std::memcmp(vabBytes,
                       source.archiveBytes.data() + vabBlock.payloadOffset,
                       vabPayloadBytes) == 0;
}

bool VerifyMemCandidateBytes8001A8F0(
    const Transaction8001A8F0& transaction,
    const Transaction8001AC18& source,
    const BlockMetadata8001A8F0& memBlock) {
    const auto* candidate = transaction.loaderMemoryCandidate.get();
    uint32_t memPayloadBytes = 0u;
    if (candidate == nullptr ||
        !TryU32FromU648001A8F0(memBlock.payloadBytes, memPayloadBytes) ||
        !IsArchiveRange8001A8F0(
            source, memBlock.payloadOffset, memBlock.payloadBytes)) {
        return false;
    }
    const uint8_t* memBytes = PrStage1LoaderMemoryDirectConstPtr(
        *candidate, transaction.memBlockPsxAddress, memPayloadBytes);
    return memBytes != nullptr &&
           std::memcmp(memBytes,
                       source.archiveBytes.data() + memBlock.payloadOffset,
                       memPayloadBytes) == 0;
}

void FinalizeAcceptedTransaction8001A8F0(
    Transaction8001A8F0& transaction,
    std::unique_ptr<PrStage1LoaderMemoryDirectState> state) {
    transaction.loaderMemoryCandidateKnown = true;
    transaction.loaderMemoryCandidate = std::move(state);
    transaction.loaderMemoryCommitted = false;
    transaction.timVramCommitted = false;
    transaction.vabSpuCommitted = false;
    transaction.memHandleTableCommitted = false;
    transaction.sideEffectsCommitted = false;
    transaction.final8001AC18ResultKnown = false;
    transaction.final8001AC18Result = 0;
    transaction.final8001AC18ResultAuthority = false;
    transaction.replayValueAuthority = false;
    transaction.oldWinS0Authority = false;
    transaction.hostFilesystemAuthority = false;
    transaction.consumerReadAuthority = false;
    transaction.psxMemoryBackingAuthority = false;
    transaction.stage2PlusAuthority = false;
    transaction.status = Status8001A8F0::Accepted;
    transaction.accepted = true;
    transaction.complete = true;
}

Transaction8001A8F0 BuildArchiveTransaction8001A8F0(
    const Transaction8001AC18& intLoad,
    std::unique_ptr<PrStage1LoaderMemoryDirectState> resetState,
    ArchiveKind8001AC18 expectedArchiveKind,
    bool memBlockRequired) {
    Transaction8001A8F0 transaction{};
    transaction.sourceIntLoadKnown = true;
    transaction.sourceArchiveKind = intLoad.archiveKind;
    bool exactIntLoad = false;
    switch (expectedArchiveKind) {
    case ArchiveKind8001AC18::Scene0Compo00:
        exactIntLoad = PrSS0Scene0IntLoadDirect::
            IsExactAcceptedTransaction8001AC18(intLoad);
        break;
    case ArchiveKind8001AC18::PracticeYCompo:
        exactIntLoad = PrSS0Scene0IntLoadDirect::
            IsExactAcceptedYCompoTransaction80015618(intLoad);
        break;
    case ArchiveKind8001AC18::Scene0Common:
        exactIntLoad = PrSS0Scene0IntLoadDirect::
            IsExactAcceptedStartupCommonTransaction80016B84(intLoad);
        break;
    case ArchiveKind8001AC18::Scene0ZCompo:
        exactIntLoad = PrSS0Scene0IntLoadDirect::
            IsExactAcceptedZCompoTransaction80015590(intLoad);
        break;
    case ArchiveKind8001AC18::Unknown:
        break;
    }
    if (!exactIntLoad || intLoad.archiveKind != expectedArchiveKind) {
        transaction.status = Status8001A8F0::IntLoadRejected;
        return transaction;
    }
    transaction.sourceRequestByteCount = intLoad.requestByteCount;
    if (resetState == nullptr) {
        transaction.status = Status8001A8F0::ResetStateMalformed;
        return transaction;
    }
    transaction.resetStateKnown = true;
    if (!IsExactResetState8001A8F0(*resetState)) {
        transaction.status = Status8001A8F0::ResetStateMalformed;
        return transaction;
    }
    transaction.resetStateExact = true;

    const BlockMetadata8001A8F0* timBlock =
        FindBlock8001A8F0(intLoad, 0u, BlockType8001A8F0::Tim);
    const BlockMetadata8001A8F0* vabBlock =
        FindBlock8001A8F0(intLoad, 1u, BlockType8001A8F0::Vab);
    const BlockMetadata8001A8F0* memBlock = memBlockRequired
        ? FindBlock8001A8F0(intLoad, 2u, BlockType8001A8F0::Mem)
        : nullptr;
    if (timBlock == nullptr || vabBlock == nullptr ||
        (memBlockRequired && memBlock == nullptr)) {
        transaction.status = Status8001A8F0::BlockMetadataMalformed;
        return transaction;
    }

    int32_t timPayloadBytes = 0;
    if (!TryI32FromU648001A8F0(
            timBlock->payloadBytes, timPayloadBytes)) {
        transaction.status = Status8001A8F0::BlockMetadataMalformed;
        return transaction;
    }
    const auto timAllocation =
        PrStage1LoaderMemoryDirectApply80025A70(
            *resetState, timPayloadBytes);
    if (!timAllocation.success || timAllocation.stackIndex != 1u ||
        !CopyBlockPayload8001A8F0(
            intLoad, *timBlock, *resetState, timAllocation.psxAddress) ||
        !BuildPayloadRequests8001A8F0(
            intLoad,
            *timBlock,
            timAllocation.psxAddress,
            transaction.timUploadRequests)) {
        transaction.status = Status8001A8F0::TimRequestMalformed;
        return transaction;
    }
    transaction.timTemporaryAllocationKnown = true;
    transaction.timTemporaryPsxAddress = timAllocation.psxAddress;
    transaction.timTemporaryHandle = timAllocation.stackIndex;
    transaction.timSectorPayloadBytes =
        static_cast<uint32_t>(timPayloadBytes);
    const auto timPop =
        PrStage1LoaderMemoryDirectApply80025AF8(*resetState);
    if (!timPop.popped || timPop.stackIndex != 1u ||
        resetState->gpPlus324StackDepth != 0u ||
        resetState->gpPlus904HeapCursor !=
            kPrStage1LoaderMemoryDirectHeapBase800965B0) {
        transaction.status = Status8001A8F0::TimRequestMalformed;
        return transaction;
    }
    transaction.timTemporaryAllocationPopped = true;

    const EntryMetadata8001A8F0* vhEntry =
        FindEntry8001A8F0(intLoad, *vabBlock, 0u);
    const EntryMetadata8001A8F0* vbEntry =
        FindEntry8001A8F0(intLoad, *vabBlock, 1u);
    if (vhEntry == nullptr || vbEntry == nullptr ||
        vhEntry->size > static_cast<uint32_t>(
                            (std::numeric_limits<int32_t>::max)()) ||
        vbEntry->size > static_cast<uint32_t>(
                            (std::numeric_limits<int32_t>::max)())) {
        transaction.status = Status8001A8F0::VabRequestMalformed;
        return transaction;
    }
    const auto vhAllocation =
        PrStage1LoaderMemoryDirectApply80025A70(
            *resetState, static_cast<int32_t>(vhEntry->size));
    const auto vbAllocation =
        PrStage1LoaderMemoryDirectApply80025A70(
            *resetState, static_cast<int32_t>(vbEntry->size));
    if (!vhAllocation.success || !vbAllocation.success ||
        vhAllocation.stackIndex != kVabVhHandle8001A8F0 ||
        vbAllocation.stackIndex != 2u ||
        vbAllocation.psxAddress != vhAllocation.nextHeapCursor ||
        !CopyBlockPayload8001A8F0(
            intLoad, *vabBlock, *resetState, vhAllocation.psxAddress)) {
        transaction.status = Status8001A8F0::VabRequestMalformed;
        return transaction;
    }
    uint32_t vabPayloadBytes = 0u;
    if (!TryU32FromU648001A8F0(vabBlock->payloadBytes, vabPayloadBytes)) {
        transaction.status = Status8001A8F0::VabRequestMalformed;
        return transaction;
    }
    transaction.vabRequest.known = true;
    transaction.vabRequest.vhPsxAddress = vhAllocation.psxAddress;
    transaction.vabRequest.vhSize = vhEntry->size;
    transaction.vabRequest.vhName = vhEntry->name;
    transaction.vabRequest.vbPsxAddress = vbAllocation.psxAddress;
    transaction.vabRequest.vbSize = vbEntry->size;
    transaction.vabRequest.vbName = vbEntry->name;
    transaction.vabRequest.sectorPayloadBytes = vabPayloadBytes;
    transaction.vabRequest.closeRequested80027120 = true;
    transaction.vabRequest.openRequested80027078 = true;
    transaction.vabRequest.transferRequested800270D4 = true;
    transaction.vabRequest.enableRequested800270FC = true;
    const auto vbPop =
        PrStage1LoaderMemoryDirectApply80025AF8(*resetState);
    if (!vbPop.popped || vbPop.stackIndex != 2u ||
        resetState->gpPlus324StackDepth != kVabVhHandle8001A8F0 ||
        resetState->gpPlus904HeapCursor != vbAllocation.psxAddress ||
        resetState->stackTable80091858[kVabVhHandle8001A8F0] !=
            vhAllocation.psxAddress ||
        resetState->stackTable80091858[2u] != 0u) {
        transaction.status = Status8001A8F0::VabRequestMalformed;
        return transaction;
    }
    transaction.vabVhHandleRetained = true;
    transaction.vabVbHandlePopped = true;

    if (!memBlockRequired) {
        transaction.finalStackDepth = resetState->gpPlus324StackDepth;
        transaction.finalHeapCursor = resetState->gpPlus904HeapCursor;
        if (transaction.finalStackDepth != kVabVhHandle8001A8F0 ||
            transaction.finalHeapCursor != vbAllocation.psxAddress) {
            transaction.status = Status8001A8F0::CandidateShapeMismatch;
            return transaction;
        }
        for (uint32_t index = 2u;
             index < resetState->stackTable80091858.size();
             ++index) {
            if (resetState->stackTable80091858[index] != 0u) {
                transaction.status = Status8001A8F0::CandidateShapeMismatch;
                return transaction;
            }
        }
        FinalizeAcceptedTransaction8001A8F0(transaction, std::move(resetState));
        return transaction;
    }

    int32_t memPayloadBytes = 0;
    if (!TryI32FromU648001A8F0(
            memBlock->payloadBytes, memPayloadBytes)) {
        transaction.status = Status8001A8F0::BlockMetadataMalformed;
        return transaction;
    }
    const auto memAllocation =
        PrStage1LoaderMemoryDirectApply80025A70(
            *resetState, memPayloadBytes);
    if (!memAllocation.success ||
        memAllocation.stackIndex != kMemBaseHandle8001A8F0 ||
        !CopyBlockPayload8001A8F0(
            intLoad, *memBlock, *resetState, memAllocation.psxAddress) ||
        PrStage1LoaderMemoryDirectApply80025BFC(
            *resetState, memAllocation.psxAddress) !=
            static_cast<int32_t>(kMemBaseHandle8001A8F0) ||
        !BuildPayloadRequests8001A8F0(
            intLoad,
            *memBlock,
            memAllocation.psxAddress,
            transaction.memMapRequests)) {
        transaction.status = Status8001A8F0::MemMapFailed;
        return transaction;
    }
    transaction.memBlockAllocationKnown = true;
    transaction.memBlockPsxAddress = memAllocation.psxAddress;
    transaction.memBlockBaseHandle = kMemBaseHandle8001A8F0;
    transaction.memSectorPayloadBytes =
        static_cast<uint32_t>(memPayloadBytes);
    for (uint32_t index = 0u;
         index < transaction.memMapRequests.size();
         ++index) {
        const auto& request = transaction.memMapRequests[index];
        const auto mapped = PrStage1LoaderMemoryDirectApply80025BBC(
            *resetState,
            request.psxAddress,
            static_cast<int32_t>(kMemBaseHandle8001A8F0 + index),
            static_cast<int32_t>(request.size));
        if (!mapped.success) {
            transaction.status = Status8001A8F0::MemMapFailed;
            return transaction;
        }
    }

    transaction.finalStackDepth = resetState->gpPlus324StackDepth;
    transaction.finalHeapCursor = resetState->gpPlus904HeapCursor;
    if (transaction.finalStackDepth != kFinalHandle8001A8F0 ||
        resetState->stackTable80091858[kVabVhHandle8001A8F0] !=
            vhAllocation.psxAddress ||
        resetState->stackTable80091858[kMemBaseHandle8001A8F0] !=
            memAllocation.psxAddress) {
        transaction.status = Status8001A8F0::CandidateShapeMismatch;
        return transaction;
    }
    for (uint32_t index = kFinalHandle8001A8F0 + 1u;
         index < resetState->stackTable80091858.size();
         ++index) {
        if (resetState->stackTable80091858[index] != 0u) {
            transaction.status = Status8001A8F0::CandidateShapeMismatch;
            return transaction;
        }
    }

    FinalizeAcceptedTransaction8001A8F0(transaction, std::move(resetState));
    return transaction;
}

}  // namespace

Transaction8001A8F0 BuildTransaction8001A8F0(
    const Transaction8001AC18& intLoad,
    std::unique_ptr<PrStage1LoaderMemoryDirectState> resetState) {
    return BuildArchiveTransaction8001A8F0(
        intLoad,
        std::move(resetState),
        ArchiveKind8001AC18::Scene0Compo00,
        true);
}

Transaction8001A8F0 BuildYCompoTransaction8001A8F0(
    const Transaction8001AC18& intLoad,
    std::unique_ptr<PrStage1LoaderMemoryDirectState> resetState) {
    return BuildArchiveTransaction8001A8F0(
        intLoad,
        std::move(resetState),
        ArchiveKind8001AC18::PracticeYCompo,
        false);
}

Transaction8001A8F0 BuildStartupCommonTransaction80016B84(
    const Transaction8001AC18& intLoad,
    std::unique_ptr<PrStage1LoaderMemoryDirectState> resetState) {
    return BuildArchiveTransaction8001A8F0(
        intLoad,
        std::move(resetState),
        ArchiveKind8001AC18::Scene0Common,
        false);
}

Transaction8001A8F0 BuildStartupZCompoTransaction80015590(
    const Transaction8001AC18& intLoad,
    std::unique_ptr<PrStage1LoaderMemoryDirectState> resetState) {
    return BuildArchiveTransaction8001A8F0(
        intLoad,
        std::move(resetState),
        ArchiveKind8001AC18::Scene0ZCompo,
        false);
}

static bool IsExactTransactionShape8001A8F0(
    const Transaction8001A8F0& transaction,
    const Transaction8001AC18& intLoad,
    bool expectedLoaderMemoryCommitted,
    bool expectedMemHandleTableCommitted,
    bool expectedFinalResultKnown,
    int32_t expectedFinalResult,
    bool expectedFinalResultAuthority) {
    if (!PrSS0Scene0IntLoadDirect::
            IsExactAcceptedTransaction8001AC18(intLoad) ||
        transaction.sourceArchiveKind !=
            ArchiveKind8001AC18::Scene0Compo00 ||
        transaction.status != Status8001A8F0::Accepted ||
        !transaction.accepted || !transaction.complete ||
        !transaction.sourceIntLoadKnown ||
        transaction.sourceRequestByteCount != intLoad.requestByteCount ||
        !transaction.resetStateKnown || !transaction.resetStateExact ||
        !transaction.loaderMemoryCandidateKnown ||
        transaction.loaderMemoryCandidate == nullptr ||
        !transaction.timTemporaryAllocationKnown ||
        transaction.timTemporaryHandle != 1u ||
        !transaction.timTemporaryAllocationPopped ||
        transaction.timUploadRequests.size() !=
            PrSS0Scene0IntLoadDirect::kExpectedTimEntries8001AC18 ||
        !transaction.vabRequest.known ||
        !transaction.vabVhHandleRetained ||
        !transaction.vabVbHandlePopped ||
        !transaction.memBlockAllocationKnown ||
        transaction.memBlockBaseHandle != kMemBaseHandle8001A8F0 ||
        transaction.memMapRequests.size() !=
            PrSS0Scene0IntLoadDirect::kExpectedMemEntries8001AC18 ||
        transaction.finalStackDepth != kFinalHandle8001A8F0 ||
        transaction.loaderMemoryCommitted !=
            expectedLoaderMemoryCommitted ||
        transaction.timVramCommitted ||
        transaction.vabSpuCommitted ||
        transaction.memHandleTableCommitted !=
            expectedMemHandleTableCommitted ||
        transaction.sideEffectsCommitted ||
        transaction.final8001AC18ResultKnown !=
            expectedFinalResultKnown ||
        transaction.final8001AC18Result != expectedFinalResult ||
        transaction.final8001AC18ResultAuthority !=
            expectedFinalResultAuthority ||
        transaction.replayValueAuthority || transaction.oldWinS0Authority ||
        transaction.hostFilesystemAuthority ||
        transaction.consumerReadAuthority ||
        transaction.psxMemoryBackingAuthority ||
        transaction.stage2PlusAuthority) {
        return false;
    }

    const BlockMetadata8001A8F0* timBlock =
        FindBlock8001A8F0(intLoad, 0u, BlockType8001A8F0::Tim);
    const BlockMetadata8001A8F0* vabBlock =
        FindBlock8001A8F0(intLoad, 1u, BlockType8001A8F0::Vab);
    const BlockMetadata8001A8F0* memBlock =
        FindBlock8001A8F0(intLoad, 2u, BlockType8001A8F0::Mem);
    if (timBlock == nullptr || vabBlock == nullptr || memBlock == nullptr) {
        return false;
    }

    std::vector<PayloadRequest8001A8F0> expectedTim{};
    std::vector<PayloadRequest8001A8F0> expectedMem{};
    if (!BuildPayloadRequests8001A8F0(
            intLoad,
            *timBlock,
            transaction.timTemporaryPsxAddress,
            expectedTim) ||
        !BuildPayloadRequests8001A8F0(
            intLoad,
            *memBlock,
            transaction.memBlockPsxAddress,
            expectedMem) ||
        !EqualPayloadRequests8001A8F0(
            expectedTim, transaction.timUploadRequests) ||
        !EqualPayloadRequests8001A8F0(
            expectedMem, transaction.memMapRequests)) {
        return false;
    }

    const EntryMetadata8001A8F0* vhEntry =
        FindEntry8001A8F0(intLoad, *vabBlock, 0u);
    const EntryMetadata8001A8F0* vbEntry =
        FindEntry8001A8F0(intLoad, *vabBlock, 1u);
    if (vhEntry == nullptr || vbEntry == nullptr ||
        transaction.vabRequest.vhPsxAddress !=
            kPrStage1LoaderMemoryDirectHeapBase800965B0 ||
        transaction.vabRequest.vhSize != vhEntry->size ||
        transaction.vabRequest.vhName != vhEntry->name ||
        transaction.vabRequest.vbPsxAddress !=
            kPrStage1LoaderMemoryDirectHeapBase800965B0 +
                PrStage1LoaderMemoryDirectAlign8Bytes(
                    static_cast<int32_t>(vhEntry->size)) ||
        transaction.vabRequest.vbSize != vbEntry->size ||
        transaction.vabRequest.vbName != vbEntry->name ||
        !transaction.vabRequest.closeRequested80027120 ||
        !transaction.vabRequest.openRequested80027078 ||
        !transaction.vabRequest.transferRequested800270D4 ||
        !transaction.vabRequest.enableRequested800270FC ||
        transaction.timSectorPayloadBytes != timBlock->payloadBytes ||
        transaction.vabRequest.sectorPayloadBytes != vabBlock->payloadBytes ||
        transaction.memSectorPayloadBytes != memBlock->payloadBytes) {
        return false;
    }

    const auto& candidate = *transaction.loaderMemoryCandidate;
    if (candidate.gpPlus320LowWater !=
            kScene0PacketArenaLowWater801C4260 ||
        candidate.gpPlus324StackDepth != kFinalHandle8001A8F0 ||
        candidate.gpPlus904HeapCursor != transaction.finalHeapCursor ||
        candidate.stackTable80091858[0u] != 0u ||
        candidate.stackTable80091858[kVabVhHandle8001A8F0] !=
            transaction.vabRequest.vhPsxAddress) {
        return false;
    }
    for (uint32_t index = 0u; index < transaction.memMapRequests.size();
         ++index) {
        if (candidate.stackTable80091858[kMemBaseHandle8001A8F0 + index] !=
            transaction.memMapRequests[index].psxAddress) {
            return false;
        }
    }
    for (uint32_t index = kFinalHandle8001A8F0 + 1u;
         index < candidate.stackTable80091858.size();
         ++index) {
        if (candidate.stackTable80091858[index] != 0u) {
            return false;
        }
    }
    const auto& finalMem = transaction.memMapRequests.back();
    const uint64_t expectedCursor =
        static_cast<uint64_t>(finalMem.psxAddress) + finalMem.size;
    return expectedCursor == transaction.finalHeapCursor &&
           VerifyVhCandidateBytes8001A8F0(
               transaction, intLoad, *vabBlock) &&
           VerifyMemCandidateBytes8001A8F0(
               transaction, intLoad, *memBlock);
}

bool IsExactAcceptedTransaction8001A8F0(
    const Transaction8001A8F0& transaction,
    const Transaction8001AC18& intLoad) {
    return IsExactTransactionShape8001A8F0(
        transaction, intLoad, false, false, false, 0, false);
}

static bool IsExactYCompoTransactionShape8001A8F0(
    const Transaction8001A8F0& transaction,
    const Transaction8001AC18& intLoad,
    bool expectedLoaderMemoryCommitted) {
    if (!PrSS0Scene0IntLoadDirect::
            IsExactAcceptedYCompoTransaction80015618(intLoad) ||
        transaction.sourceArchiveKind != ArchiveKind8001AC18::PracticeYCompo ||
        transaction.status != Status8001A8F0::Accepted ||
        !transaction.accepted || !transaction.complete ||
        !transaction.sourceIntLoadKnown ||
        transaction.sourceRequestByteCount != intLoad.requestByteCount ||
        !transaction.resetStateKnown || !transaction.resetStateExact ||
        !transaction.loaderMemoryCandidateKnown ||
        transaction.loaderMemoryCandidate == nullptr ||
        !transaction.timTemporaryAllocationKnown ||
        transaction.timTemporaryHandle != 1u ||
        !transaction.timTemporaryAllocationPopped ||
        transaction.timUploadRequests.size() !=
            PrSS0Scene0IntLoadDirect::kExpectedYCompoTimEntries80015618 ||
        !transaction.vabRequest.known ||
        !transaction.vabVhHandleRetained ||
        !transaction.vabVbHandlePopped ||
        transaction.memBlockAllocationKnown ||
        transaction.memBlockPsxAddress != 0u ||
        transaction.memBlockBaseHandle != 0u ||
        transaction.memSectorPayloadBytes != 0u ||
        !transaction.memMapRequests.empty() ||
        transaction.finalStackDepth != kVabVhHandle8001A8F0 ||
        transaction.loaderMemoryCommitted !=
            expectedLoaderMemoryCommitted ||
        transaction.timVramCommitted ||
        transaction.vabSpuCommitted || transaction.memHandleTableCommitted ||
        transaction.sideEffectsCommitted ||
        transaction.final8001AC18ResultKnown ||
        transaction.final8001AC18Result != 0 ||
        transaction.final8001AC18ResultAuthority ||
        transaction.replayValueAuthority || transaction.oldWinS0Authority ||
        transaction.hostFilesystemAuthority ||
        transaction.consumerReadAuthority ||
        transaction.psxMemoryBackingAuthority ||
        transaction.stage2PlusAuthority) {
        return false;
    }

    const BlockMetadata8001A8F0* timBlock =
        FindBlock8001A8F0(intLoad, 0u, BlockType8001A8F0::Tim);
    const BlockMetadata8001A8F0* vabBlock =
        FindBlock8001A8F0(intLoad, 1u, BlockType8001A8F0::Vab);
    if (timBlock == nullptr || vabBlock == nullptr) {
        return false;
    }
    std::vector<PayloadRequest8001A8F0> expectedTim{};
    if (!BuildPayloadRequests8001A8F0(
            intLoad,
            *timBlock,
            transaction.timTemporaryPsxAddress,
            expectedTim) ||
        !EqualPayloadRequests8001A8F0(
            expectedTim, transaction.timUploadRequests)) {
        return false;
    }

    const EntryMetadata8001A8F0* vhEntry =
        FindEntry8001A8F0(intLoad, *vabBlock, 0u);
    const EntryMetadata8001A8F0* vbEntry =
        FindEntry8001A8F0(intLoad, *vabBlock, 1u);
    if (vhEntry == nullptr || vbEntry == nullptr ||
        transaction.vabRequest.vhPsxAddress !=
            kPrStage1LoaderMemoryDirectHeapBase800965B0 ||
        transaction.vabRequest.vhSize != vhEntry->size ||
        transaction.vabRequest.vhName != vhEntry->name ||
        transaction.vabRequest.vbPsxAddress !=
            kPrStage1LoaderMemoryDirectHeapBase800965B0 +
                PrStage1LoaderMemoryDirectAlign8Bytes(
                    static_cast<int32_t>(vhEntry->size)) ||
        transaction.vabRequest.vbSize != vbEntry->size ||
        transaction.vabRequest.vbName != vbEntry->name ||
        !transaction.vabRequest.closeRequested80027120 ||
        !transaction.vabRequest.openRequested80027078 ||
        !transaction.vabRequest.transferRequested800270D4 ||
        !transaction.vabRequest.enableRequested800270FC ||
        transaction.timSectorPayloadBytes != timBlock->payloadBytes ||
        transaction.vabRequest.sectorPayloadBytes != vabBlock->payloadBytes ||
        transaction.finalHeapCursor != transaction.vabRequest.vbPsxAddress) {
        return false;
    }

    const auto& candidate = *transaction.loaderMemoryCandidate;
    if (candidate.gpPlus320LowWater !=
            kScene0PacketArenaLowWater801C4260 ||
        candidate.gpPlus324StackDepth != kVabVhHandle8001A8F0 ||
        candidate.gpPlus904HeapCursor != transaction.finalHeapCursor ||
        candidate.stackTable80091858[0u] != 0u ||
        candidate.stackTable80091858[kVabVhHandle8001A8F0] !=
            transaction.vabRequest.vhPsxAddress) {
        return false;
    }
    for (uint32_t index = 2u;
         index < candidate.stackTable80091858.size();
         ++index) {
        if (candidate.stackTable80091858[index] != 0u) {
            return false;
        }
    }
    return VerifyVabBlockCandidateBytes8001A8F0(
        transaction, intLoad, *vabBlock);
}

bool IsExactAcceptedYCompoTransaction8001A8F0(
    const Transaction8001A8F0& transaction,
    const Transaction8001AC18& intLoad) {
    return IsExactYCompoTransactionShape8001A8F0(
        transaction, intLoad, false);
}

static bool IsExactStartupNoMemTransactionShape8001A8F0(
    const Transaction8001A8F0& transaction,
    const Transaction8001AC18& intLoad,
    ArchiveKind8001AC18 expectedArchiveKind,
    uint32_t expectedTimCount) {
    bool exactIntLoad = false;
    if (expectedArchiveKind == ArchiveKind8001AC18::Scene0Common) {
        exactIntLoad = PrSS0Scene0IntLoadDirect::
            IsExactAcceptedStartupCommonTransaction80016B84(intLoad);
    } else if (expectedArchiveKind == ArchiveKind8001AC18::Scene0ZCompo) {
        exactIntLoad = PrSS0Scene0IntLoadDirect::
            IsExactAcceptedZCompoTransaction80015590(intLoad);
    }
    if (!exactIntLoad || transaction.sourceArchiveKind != expectedArchiveKind ||
        transaction.status != Status8001A8F0::Accepted ||
        !transaction.accepted || !transaction.complete ||
        !transaction.sourceIntLoadKnown ||
        transaction.sourceRequestByteCount != intLoad.requestByteCount ||
        !transaction.resetStateKnown || !transaction.resetStateExact ||
        !transaction.loaderMemoryCandidateKnown ||
        transaction.loaderMemoryCandidate == nullptr ||
        !transaction.timTemporaryAllocationKnown ||
        transaction.timTemporaryHandle != 1u ||
        !transaction.timTemporaryAllocationPopped ||
        transaction.timUploadRequests.size() != expectedTimCount ||
        !transaction.vabRequest.known || !transaction.vabVhHandleRetained ||
        !transaction.vabVbHandlePopped || transaction.memBlockAllocationKnown ||
        transaction.memBlockPsxAddress != 0u ||
        transaction.memBlockBaseHandle != 0u ||
        transaction.memSectorPayloadBytes != 0u ||
        !transaction.memMapRequests.empty() ||
        transaction.finalStackDepth != kVabVhHandle8001A8F0 ||
        transaction.timVramCommitted || transaction.vabSpuCommitted ||
        transaction.memHandleTableCommitted || transaction.sideEffectsCommitted ||
        transaction.final8001AC18ResultKnown ||
        transaction.final8001AC18Result != 0 ||
        transaction.final8001AC18ResultAuthority ||
        transaction.replayValueAuthority || transaction.oldWinS0Authority ||
        transaction.hostFilesystemAuthority ||
        transaction.consumerReadAuthority ||
        transaction.psxMemoryBackingAuthority ||
        transaction.stage2PlusAuthority) {
        return false;
    }

    const BlockMetadata8001A8F0* timBlock =
        FindBlock8001A8F0(intLoad, 0u, BlockType8001A8F0::Tim);
    const BlockMetadata8001A8F0* vabBlock =
        FindBlock8001A8F0(intLoad, 1u, BlockType8001A8F0::Vab);
    if (timBlock == nullptr || vabBlock == nullptr) {
        return false;
    }
    std::vector<PayloadRequest8001A8F0> expectedTim{};
    if (!BuildPayloadRequests8001A8F0(
            intLoad,
            *timBlock,
            transaction.timTemporaryPsxAddress,
            expectedTim) ||
        !EqualPayloadRequests8001A8F0(
            expectedTim, transaction.timUploadRequests)) {
        return false;
    }

    const EntryMetadata8001A8F0* vhEntry =
        FindEntry8001A8F0(intLoad, *vabBlock, 0u);
    const EntryMetadata8001A8F0* vbEntry =
        FindEntry8001A8F0(intLoad, *vabBlock, 1u);
    if (vhEntry == nullptr || vbEntry == nullptr ||
        transaction.vabRequest.vhPsxAddress !=
            kPrStage1LoaderMemoryDirectHeapBase800965B0 ||
        transaction.vabRequest.vhSize != vhEntry->size ||
        transaction.vabRequest.vhName != vhEntry->name ||
        transaction.vabRequest.vbPsxAddress !=
            kPrStage1LoaderMemoryDirectHeapBase800965B0 +
                PrStage1LoaderMemoryDirectAlign8Bytes(
                    static_cast<int32_t>(vhEntry->size)) ||
        transaction.vabRequest.vbSize != vbEntry->size ||
        transaction.vabRequest.vbName != vbEntry->name ||
        !transaction.vabRequest.closeRequested80027120 ||
        !transaction.vabRequest.openRequested80027078 ||
        !transaction.vabRequest.transferRequested800270D4 ||
        !transaction.vabRequest.enableRequested800270FC ||
        transaction.timSectorPayloadBytes != timBlock->payloadBytes ||
        transaction.vabRequest.sectorPayloadBytes != vabBlock->payloadBytes ||
        transaction.finalHeapCursor != transaction.vabRequest.vbPsxAddress) {
        return false;
    }

    const auto& candidate = *transaction.loaderMemoryCandidate;
    if (candidate.gpPlus320LowWater !=
            kScene0PacketArenaLowWater801C4260 ||
        candidate.gpPlus324StackDepth != kVabVhHandle8001A8F0 ||
        candidate.gpPlus904HeapCursor != transaction.finalHeapCursor ||
        candidate.stackTable80091858[0u] != 0u ||
        candidate.stackTable80091858[kVabVhHandle8001A8F0] !=
            transaction.vabRequest.vhPsxAddress) {
        return false;
    }
    for (uint32_t index = 2u;
         index < candidate.stackTable80091858.size();
         ++index) {
        if (candidate.stackTable80091858[index] != 0u) {
            return false;
        }
    }
    return VerifyVabBlockCandidateBytes8001A8F0(
        transaction, intLoad, *vabBlock);
}

bool IsExactAcceptedStartupCommonTransaction80016B84(
    const Transaction8001A8F0& transaction,
    const Transaction8001AC18& intLoad) {
    return IsExactStartupNoMemTransactionShape8001A8F0(
        transaction,
        intLoad,
        ArchiveKind8001AC18::Scene0Common,
        PrSS0Scene0IntLoadDirect::kExpectedCommonTimEntries80016B84);
}

bool IsExactAcceptedStartupZCompoTransaction80015590(
    const Transaction8001A8F0& transaction,
    const Transaction8001AC18& intLoad) {
    return IsExactStartupNoMemTransactionShape8001A8F0(
        transaction,
        intLoad,
        ArchiveKind8001AC18::Scene0ZCompo,
        PrSS0Scene0IntLoadDirect::kExpectedZCompoTimEntries80015590);
}

bool CommitYCompoLoaderMemory8001A8F0(
    Transaction8001A8F0& transaction,
    const Transaction8001AC18& intLoad) {
    if (!IsExactAcceptedYCompoTransaction8001A8F0(
            transaction, intLoad)) {
        return false;
    }
    transaction.loaderMemoryCommitted = true;
    return true;
}

bool IsExactYCompoLoaderMemoryCommittedTransaction8001A8F0(
    const Transaction8001A8F0& transaction,
    const Transaction8001AC18& intLoad) {
    return IsExactYCompoTransactionShape8001A8F0(
        transaction, intLoad, true);
}

bool CommitLoaderMemoryAndHandleTable8001A8F0(
    Transaction8001A8F0& transaction,
    const Transaction8001AC18& intLoad) {
    if (!IsExactAcceptedTransaction8001A8F0(transaction, intLoad)) {
        return false;
    }
    transaction.loaderMemoryCommitted = true;
    transaction.memHandleTableCommitted = true;
    return true;
}

bool IsExactLoaderMemoryCommittedTransaction8001A8F0(
    const Transaction8001A8F0& transaction,
    const Transaction8001AC18& intLoad) {
    return IsExactTransactionShape8001A8F0(
        transaction, intLoad, true, true, false, 0, false);
}

bool CommitFinalResult8001AC18(
    Transaction8001A8F0& transaction,
    const Transaction8001AC18& intLoad,
    const FinalResultSource8001AC18& source) {
    if (!IsExactLoaderMemoryCommittedTransaction8001A8F0(
            transaction, intLoad) ||
        !source.staticEofReturnChainKnown || !source.eofSuccessKnown ||
        !source.eofSuccess || !source.loaderMemoryCommitted ||
        !source.memHandleTableCommitted ||
        !source.timPartialVramCommitted || !source.hostVabBankCommitted ||
        !intLoad.eofPresent || !intLoad.eofHeaderEndsArchive) {
        return false;
    }
    transaction.final8001AC18ResultKnown = true;
    transaction.final8001AC18Result = 1;
    transaction.final8001AC18ResultAuthority = true;
    return true;
}

bool IsExactFinalResultTransaction8001AC18(
    const Transaction8001A8F0& transaction,
    const Transaction8001AC18& intLoad) {
    return IsExactTransactionShape8001A8F0(
        transaction, intLoad, true, true, true, 1, true);
}

}  // namespace PrSS0Scene0IntSideEffectDirect
