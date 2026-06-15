#pragma once

#include "pr_mime.h"

#include <array>
#include <cstdint>

namespace PrStage1TodCursorDirect {

struct TodType1TrsState80028054 {
    bool valid = false;
    std::array<int32_t, 3> scale{};
    std::array<int16_t, 3> rotation{};
    std::array<int32_t, 3> translation{};
};

struct TodCursorRuntime {
    bool initialized = false;
    uint16_t handle = 0;
    uint32_t startFrame = 0;
    uint32_t cursorBlockIndex = 0;
    int32_t remainingBlocks = 0;
    uint32_t lastLocalFrame = 0;
    bool coordValid = false;
    TodCoordMatrix coord{};
};

struct TodCommandSideEffects80028054 {
    bool blockTriggered80028504 = false;
    bool descCommandExecuted80028054 = false;
    bool descHeaderCleared80028054 = false;
    bool descAttrMerged80028054 = false;
    bool coordWritten80028054 = false;
    bool type1CoordBackingMissing80028054 = false;
    bool type1CoordWritten80028054 = false;
    bool type4CoordWritten80028054 = false;
    uint16_t commandCount80028504 = 0;
    uint16_t lastCommandObjectId80028054 = 0;
    uint16_t lastCommandType80028054 = 0;
    uint16_t lastCommandFlags80028054 = 0;
    uint32_t oldRawCursorOffset80028504 = 0;
    uint32_t newRawCursorOffset80028504 = 0;
};

void Reset(TodCursorRuntime& runtime);

// Direct sub_8001B000 -> sub_80028504 carrier. It examines only the current
// cursor block and advances it when seq reaches that block trigger. The caller
// owns the PSX count pointer and should decrement it only when outCursorChanged
// is true. Unlike the older object-1 helper, this path follows 80028054 and
// applies desc/COORD commands to the desc passed by the caller without object
// id filtering.
bool AdvanceDescCoordOneBlock8001B000(TodCursorRuntime& runtime,
                                      const TodData& tod,
                                      uint16_t handle,
                                      uint32_t startFrame,
                                      uint32_t seq,
                                      bool rawCursorOffsetKnown,
                                      uint32_t rawCursorOffset,
                                      TodCoordMatrix& outCoord,
                                      bool& outCoordValid,
                                      bool& outCursorChanged,
                                      uint32_t* outNewRawCursorOffset = nullptr,
                                      int32_t* outRemainingBlocks = nullptr,
                                      uint32_t* descAttr = nullptr,
                                      TodType1TrsState80028054*
                                          type1TrsState80028054 = nullptr,
                                      bool* outDescCommandExecuted = nullptr,
                                      TodCommandSideEffects80028054*
                                          outSideEffects = nullptr);

}  // namespace PrStage1TodCursorDirect
