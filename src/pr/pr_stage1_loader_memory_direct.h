#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

constexpr uint32_t kPrStage1LoaderMemoryDirectFn80025B28 = 0x80025B28u;
constexpr uint32_t kPrStage1LoaderMemoryDirectFn80025A34 = 0x80025A34u;
constexpr uint32_t kPrStage1LoaderMemoryDirectFn80025A00 = 0x80025A00u;
constexpr uint32_t kPrStage1LoaderMemoryDirectFn80025A70 = 0x80025A70u;
constexpr uint32_t kPrStage1LoaderMemoryDirectFn80025AF8 = 0x80025AF8u;
constexpr uint32_t kPrStage1LoaderMemoryDirectFn80025BFC = 0x80025BFCu;
constexpr uint32_t kPrStage1LoaderMemoryDirectFn80025BBC = 0x80025BBCu;

constexpr uint32_t kPrStage1LoaderMemoryDirectHeapBase800965B0 = 0x800965B0u;
constexpr uint32_t kPrStage1LoaderMemoryDirectHeapEnd801C35B0 = 0x801C35B0u;
constexpr uint32_t kPrStage1LoaderMemoryDirectStackTable80091858 = 0x80091858u;
constexpr uint32_t kPrStage1LoaderMemoryDirectStackTableFirst8009185C =
    0x8009185Cu;

constexpr uint16_t kPrStage1LoaderMemoryDirectGpOffsetLowWater = 320u;
constexpr uint16_t kPrStage1LoaderMemoryDirectGpOffsetStackDepth = 324u;
constexpr uint16_t kPrStage1LoaderMemoryDirectGpOffsetHeapBase = 896u;
constexpr uint16_t kPrStage1LoaderMemoryDirectGpOffsetHeapEnd = 900u;
constexpr uint16_t kPrStage1LoaderMemoryDirectGpOffsetHeapCursor = 904u;

constexpr uint8_t kPrStage1LoaderMemoryDirectAlignmentBytes = 8u;
constexpr int32_t kPrStage1LoaderMemoryDirectPositiveRoundAdd = 7;
constexpr int32_t kPrStage1LoaderMemoryDirectNegativeRoundAdd = 14;
constexpr uint32_t kPrStage1LoaderMemoryDirectMaxRequestBytes = 0x12CFFFu;
constexpr std::size_t kPrStage1LoaderMemoryDirectStackTableEntryCount = 1024u;
constexpr std::size_t kPrStage1LoaderMemoryDirectHeapSizeBytes =
    kPrStage1LoaderMemoryDirectHeapEnd801C35B0 -
    kPrStage1LoaderMemoryDirectHeapBase800965B0;

struct PrStage1LoaderMemoryDirectState {
    uint32_t gpPlus320LowWater = 0u;
    uint32_t gpPlus324StackDepth = 0u;
    uint32_t gpPlus896HeapBase = kPrStage1LoaderMemoryDirectHeapBase800965B0;
    uint32_t gpPlus900HeapEnd = kPrStage1LoaderMemoryDirectHeapEnd801C35B0;
    uint32_t gpPlus904HeapCursor = kPrStage1LoaderMemoryDirectHeapBase800965B0;
    std::array<uint32_t, kPrStage1LoaderMemoryDirectStackTableEntryCount>
        stackTable80091858{};
    std::array<uint8_t, kPrStage1LoaderMemoryDirectHeapSizeBytes>
        heap800965B0{};
};

struct PrStage1LoaderMemoryDirectAllocResult {
    bool success = false;
    uint32_t psxAddress = 0u;
    uint32_t alignedBytes = 0u;
    uint32_t stackIndex = 0u;
    uint32_t previousHeapCursor = 0u;
    uint32_t nextHeapCursor = 0u;
};

struct PrStage1LoaderMemoryDirectFreeResult {
    bool popped = false;
    uint32_t stackIndex = 0u;
    uint32_t restoredHeapCursor = 0u;
};

struct PrStage1LoaderMemoryDirectSplitResult {
    int32_t returnValue = 0;
    bool success = false;
    uint32_t requestedStackIndex = 0u;
    uint32_t nextHeapCursor = 0u;
};

void PrStage1LoaderMemoryDirectReset(
    PrStage1LoaderMemoryDirectState& state);

uint32_t PrStage1LoaderMemoryDirectAlign8Bytes(int32_t sizeBytes);

bool PrStage1LoaderMemoryDirectSectorCountToBytes(uint32_t sectorCount,
                                                  uint8_t sectorToBytesShift,
                                                  int32_t& outSizeBytes);

bool PrStage1LoaderMemoryDirectContainsRange(uint32_t psxAddress,
                                             uint32_t sizeBytes);

uint8_t* PrStage1LoaderMemoryDirectMutablePtr(
    PrStage1LoaderMemoryDirectState& state,
    uint32_t psxAddress,
    uint32_t sizeBytes);

const uint8_t* PrStage1LoaderMemoryDirectConstPtr(
    const PrStage1LoaderMemoryDirectState& state,
    uint32_t psxAddress,
    uint32_t sizeBytes);

uint32_t PrStage1LoaderMemoryDirectStackEntryAddress(uint32_t stackIndex);

uint32_t PrStage1LoaderMemoryDirectGetStackEntry(
    const PrStage1LoaderMemoryDirectState& state,
    uint32_t stackIndex);

PrStage1LoaderMemoryDirectAllocResult
PrStage1LoaderMemoryDirectApply80025B28(
    PrStage1LoaderMemoryDirectState& state,
    int32_t sizeBytes);

PrStage1LoaderMemoryDirectAllocResult
PrStage1LoaderMemoryDirectApply80025A70(
    PrStage1LoaderMemoryDirectState& state,
    int32_t sizeBytes);

PrStage1LoaderMemoryDirectFreeResult
PrStage1LoaderMemoryDirectApply80025AF8(
    PrStage1LoaderMemoryDirectState& state);

int32_t PrStage1LoaderMemoryDirectApply80025BFC(
    const PrStage1LoaderMemoryDirectState& state,
    uint32_t psxAddress);

PrStage1LoaderMemoryDirectSplitResult
PrStage1LoaderMemoryDirectApply80025BBC(
    PrStage1LoaderMemoryDirectState& state,
    uint32_t psxAddress,
    int32_t stackIndex,
    int32_t sizeBytes);
