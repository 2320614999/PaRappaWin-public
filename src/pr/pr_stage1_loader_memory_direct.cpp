#include "pr_stage1_loader_memory_direct.h"

#include <algorithm>

namespace {

static int64_t ArithmeticShiftRight3(int64_t value) {
    if (value >= 0) {
        return value >> 3;
    }
    return -(((-value) + 7) >> 3);
}

static bool ValidStackIndex(uint32_t stackIndex) {
    return stackIndex < kPrStage1LoaderMemoryDirectStackTableEntryCount;
}

static bool AddAddress(uint32_t base, uint32_t bytes, uint32_t& out) {
    const uint64_t next = (uint64_t)base + (uint64_t)bytes;
    if (next > 0xFFFFFFFFull) {
        return false;
    }
    out = (uint32_t)next;
    return true;
}

} // namespace

void PrStage1LoaderMemoryDirectReset(
    PrStage1LoaderMemoryDirectState& state) {
    state.gpPlus320LowWater = 0u;
    state.gpPlus324StackDepth = 0u;
    state.gpPlus896HeapBase = kPrStage1LoaderMemoryDirectHeapBase800965B0;
    state.gpPlus900HeapEnd = kPrStage1LoaderMemoryDirectHeapEnd801C35B0;
    state.gpPlus904HeapCursor = kPrStage1LoaderMemoryDirectHeapBase800965B0;
    std::fill(state.stackTable80091858.begin(),
              state.stackTable80091858.end(),
              0u);
}

uint32_t PrStage1LoaderMemoryDirectAlign8Bytes(int32_t sizeBytes) {
    const int64_t adjusted =
        (int64_t)sizeBytes +
        ((sizeBytes < 0) ? kPrStage1LoaderMemoryDirectNegativeRoundAdd
                         : kPrStage1LoaderMemoryDirectPositiveRoundAdd);
    const int64_t units = ArithmeticShiftRight3(adjusted);
    return (uint32_t)(units * kPrStage1LoaderMemoryDirectAlignmentBytes);
}

bool PrStage1LoaderMemoryDirectSectorCountToBytes(uint32_t sectorCount,
                                                  uint8_t sectorToBytesShift,
                                                  int32_t& outSizeBytes) {
    if (sectorToBytesShift >= 31u) {
        return false;
    }
    const uint64_t sizeBytes = (uint64_t)sectorCount << sectorToBytesShift;
    if (sizeBytes > (uint64_t)0x7FFFFFFF) {
        return false;
    }
    outSizeBytes = (int32_t)sizeBytes;
    return true;
}

bool PrStage1LoaderMemoryDirectContainsRange(uint32_t psxAddress,
                                             uint32_t sizeBytes) {
    if (psxAddress < kPrStage1LoaderMemoryDirectHeapBase800965B0 ||
        psxAddress > kPrStage1LoaderMemoryDirectHeapEnd801C35B0) {
        return false;
    }
    uint32_t endAddress = 0u;
    if (!AddAddress(psxAddress, sizeBytes, endAddress)) {
        return false;
    }
    return endAddress <= kPrStage1LoaderMemoryDirectHeapEnd801C35B0;
}

uint8_t* PrStage1LoaderMemoryDirectMutablePtr(
    PrStage1LoaderMemoryDirectState& state,
    uint32_t psxAddress,
    uint32_t sizeBytes) {
    if (!PrStage1LoaderMemoryDirectContainsRange(psxAddress, sizeBytes)) {
        return nullptr;
    }
    return state.heap800965B0.data() +
           (psxAddress - kPrStage1LoaderMemoryDirectHeapBase800965B0);
}

const uint8_t* PrStage1LoaderMemoryDirectConstPtr(
    const PrStage1LoaderMemoryDirectState& state,
    uint32_t psxAddress,
    uint32_t sizeBytes) {
    if (!PrStage1LoaderMemoryDirectContainsRange(psxAddress, sizeBytes)) {
        return nullptr;
    }
    return state.heap800965B0.data() +
           (psxAddress - kPrStage1LoaderMemoryDirectHeapBase800965B0);
}

uint32_t PrStage1LoaderMemoryDirectStackEntryAddress(uint32_t stackIndex) {
    return kPrStage1LoaderMemoryDirectStackTable80091858 + stackIndex * 4u;
}

uint32_t PrStage1LoaderMemoryDirectGetStackEntry(
    const PrStage1LoaderMemoryDirectState& state,
    uint32_t stackIndex) {
    if (!ValidStackIndex(stackIndex)) {
        return 0u;
    }
    return state.stackTable80091858[stackIndex];
}

PrStage1LoaderMemoryDirectAllocResult
PrStage1LoaderMemoryDirectApply80025B28(
    PrStage1LoaderMemoryDirectState& state,
    int32_t sizeBytes) {
    PrStage1LoaderMemoryDirectAllocResult out{};
    out.alignedBytes = PrStage1LoaderMemoryDirectAlign8Bytes(sizeBytes);
    out.previousHeapCursor = state.gpPlus904HeapCursor;
    out.nextHeapCursor = state.gpPlus904HeapCursor;

    uint32_t requiredCursor = 0u;
    if (!AddAddress(state.gpPlus904HeapCursor,
                    out.alignedBytes,
                    requiredCursor) ||
        requiredCursor >= state.gpPlus900HeapEnd ||
        out.alignedBytes > state.gpPlus900HeapEnd) {
        return out;
    }

    out.success = true;
    out.psxAddress = state.gpPlus900HeapEnd - out.alignedBytes;
    if (out.psxAddress > state.gpPlus320LowWater) {
        state.gpPlus320LowWater = out.psxAddress;
    }
    return out;
}

PrStage1LoaderMemoryDirectAllocResult
PrStage1LoaderMemoryDirectApply80025A70(
    PrStage1LoaderMemoryDirectState& state,
    int32_t sizeBytes) {
    PrStage1LoaderMemoryDirectAllocResult out{};
    out.alignedBytes = PrStage1LoaderMemoryDirectAlign8Bytes(sizeBytes);
    out.previousHeapCursor = state.gpPlus904HeapCursor;
    out.nextHeapCursor = state.gpPlus904HeapCursor;

    if (sizeBytes > (int32_t)kPrStage1LoaderMemoryDirectMaxRequestBytes) {
        return out;
    }

    const uint32_t nextStackIndex = state.gpPlus324StackDepth + 1u;
    if (!ValidStackIndex(nextStackIndex)) {
        return out;
    }

    uint32_t nextCursor = 0u;
    if (!AddAddress(state.gpPlus904HeapCursor,
                    out.alignedBytes,
                    nextCursor) ||
        nextCursor >= state.gpPlus900HeapEnd) {
        return out;
    }

    out.success = true;
    out.psxAddress = state.gpPlus904HeapCursor;
    out.stackIndex = nextStackIndex;
    out.nextHeapCursor = nextCursor;

    state.stackTable80091858[nextStackIndex] = out.psxAddress;
    state.gpPlus324StackDepth = nextStackIndex;
    state.gpPlus904HeapCursor = nextCursor;
    return out;
}

PrStage1LoaderMemoryDirectFreeResult
PrStage1LoaderMemoryDirectApply80025AF8(
    PrStage1LoaderMemoryDirectState& state) {
    PrStage1LoaderMemoryDirectFreeResult out{};
    const uint32_t stackIndex = state.gpPlus324StackDepth;
    if (stackIndex == 0u || !ValidStackIndex(stackIndex)) {
        return out;
    }

    out.popped = true;
    out.stackIndex = stackIndex;
    out.restoredHeapCursor = state.stackTable80091858[stackIndex];
    state.stackTable80091858[stackIndex] = 0u;
    state.gpPlus904HeapCursor = out.restoredHeapCursor;
    state.gpPlus324StackDepth = stackIndex - 1u;
    return out;
}

int32_t PrStage1LoaderMemoryDirectApply80025BFC(
    const PrStage1LoaderMemoryDirectState& state,
    uint32_t psxAddress) {
    const uint32_t depth =
        (std::min)(state.gpPlus324StackDepth,
                   (uint32_t)kPrStage1LoaderMemoryDirectStackTableEntryCount -
                       1u);
    for (uint32_t i = 1u; i <= depth; ++i) {
        if (state.stackTable80091858[i] == psxAddress) {
            return (int32_t)i;
        }
    }
    return 0;
}

PrStage1LoaderMemoryDirectSplitResult
PrStage1LoaderMemoryDirectApply80025BBC(
    PrStage1LoaderMemoryDirectState& state,
    uint32_t psxAddress,
    int32_t stackIndex,
    int32_t sizeBytes) {
    PrStage1LoaderMemoryDirectSplitResult out{};
    if (stackIndex < 0 || sizeBytes < 0) {
        return out;
    }

    out.requestedStackIndex = (uint32_t)stackIndex;
    if (!ValidStackIndex(out.requestedStackIndex)) {
        return out;
    }

    uint32_t nextCursor = 0u;
    if (!AddAddress(psxAddress, (uint32_t)sizeBytes, nextCursor) ||
        nextCursor >= state.gpPlus900HeapEnd) {
        return out;
    }

    state.stackTable80091858[out.requestedStackIndex] = psxAddress;
    state.gpPlus324StackDepth = out.requestedStackIndex;
    state.gpPlus904HeapCursor = nextCursor;

    out.returnValue = 1;
    out.success = true;
    out.nextHeapCursor = nextCursor;
    return out;
}
