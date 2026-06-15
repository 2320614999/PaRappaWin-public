#include "pr_stage1_tod_cursor_direct.h"

#include <algorithm>

namespace PrStage1TodCursorDirect {
namespace {

constexpr int32_t kTodFixedOne = 4096;

#include "pr_psx_rotmatrix_trig_table_800581c0.inc"

static int32_t FixedMul(int32_t a, int32_t b) {
    int32_t productLo =
        static_cast<int32_t>(static_cast<uint32_t>(
            static_cast<int64_t>(a) * static_cast<int64_t>(b)));
    if (productLo < 0) {
        productLo += 0xFFF;
    }
    return productLo >> 12;
}

struct PsxTodMatrix3x48003F790 {
    std::array<uint32_t, 8> words{};
};

static int32_t SignExtendLow16(uint32_t value) {
    return static_cast<int16_t>(value & 0xFFFFu);
}

static uint32_t PackSignedLowHigh16(int32_t low, int32_t high) {
    return static_cast<uint32_t>(static_cast<uint16_t>(low)) |
           (static_cast<uint32_t>(static_cast<uint16_t>(high)) << 16);
}

static void SetMatrixHalfword8003F790(PsxTodMatrix3x48003F790& matrix,
                                      uint32_t halfwordIndex,
                                      int32_t value) {
    const uint32_t wordIndex = halfwordIndex / 2u;
    const uint32_t shift = (halfwordIndex & 1u) != 0u ? 16u : 0u;
    const uint32_t mask = 0xFFFFu << shift;
    matrix.words[wordIndex] =
        (matrix.words[wordIndex] & ~mask) |
        (static_cast<uint32_t>(static_cast<uint16_t>(value)) << shift);
}

static int16_t MatrixHalfword80028054(const PsxTodMatrix3x48003F790& matrix,
                                      uint32_t halfwordIndex) {
    const uint32_t word = matrix.words[halfwordIndex / 2u];
    const uint32_t shift = (halfwordIndex & 1u) != 0u ? 16u : 0u;
    return static_cast<int16_t>(word >> shift);
}

static int32_t TrigSinForSignedAngle8003F790(int32_t angle,
                                             uint32_t* packedOut) {
    const uint32_t index =
        static_cast<uint32_t>(angle < 0 ? -angle : angle) & 0xFFFu;
    const uint32_t packed = kRotMatrixTrigTable800581C0[index];
    if (packedOut != nullptr) {
        *packedOut = packed;
    }
    const int32_t sinValue = static_cast<int16_t>(packed & 0xFFFFu);
    return angle < 0 ? -sinValue : sinValue;
}

static PsxTodMatrix3x48003F790 PsxCall8003F790_RotMatrix(int16_t x,
                                                          int16_t y,
                                                          int16_t z) {
    PsxTodMatrix3x48003F790 matrix{};

    uint32_t packedX = 0u;
    const int32_t sinX = TrigSinForSignedAngle8003F790(x, &packedX);
    const int32_t cosX = static_cast<int32_t>(packedX) >> 16;

    uint32_t packedY = 0u;
    const int32_t rawSinY = TrigSinForSignedAngle8003F790(y, &packedY);
    const int32_t cosY = static_cast<int32_t>(packedY) >> 16;
    const int32_t negSinY = -rawSinY;

    SetMatrixHalfword8003F790(matrix, 2u, rawSinY);
    SetMatrixHalfword8003F790(matrix, 5u, -FixedMul(cosY, sinX));
    SetMatrixHalfword8003F790(matrix, 8u, FixedMul(cosY, cosX));

    uint32_t packedZ = 0u;
    const int32_t sinZ = TrigSinForSignedAngle8003F790(z, &packedZ);
    const int32_t cosZ = static_cast<int32_t>(packedZ) >> 16;
    const int32_t zCosY = FixedMul(cosZ, cosY);
    const int32_t zSinY = FixedMul(cosZ, negSinY);
    const int32_t sinZSinY = FixedMul(sinZ, negSinY);

    SetMatrixHalfword8003F790(matrix, 0u, zCosY);
    SetMatrixHalfword8003F790(matrix, 1u, -FixedMul(sinZ, cosY));
    SetMatrixHalfword8003F790(matrix,
                              3u,
                              FixedMul(sinZ, cosX) -
                                  FixedMul(zSinY, sinX));
    SetMatrixHalfword8003F790(matrix,
                              6u,
                              FixedMul(sinZ, sinX) +
                                  FixedMul(zSinY, cosX));
    SetMatrixHalfword8003F790(matrix,
                              4u,
                              FixedMul(cosZ, cosX) +
                                  FixedMul(sinZSinY, sinX));
    SetMatrixHalfword8003F790(matrix,
                              7u,
                              FixedMul(cosZ, sinX) -
                                  FixedMul(sinZSinY, cosX));
    return matrix;
}

static PsxTodMatrix3x48003F790 PsxCall8003B0FC_ScaleMatrix(
    PsxTodMatrix3x48003F790 matrix,
    const std::array<int32_t, 3>& scale) {
    const uint32_t word0 = matrix.words[0];
    const uint32_t word1 = matrix.words[1];
    const uint32_t word2 = matrix.words[2];
    const uint32_t word3 = matrix.words[3];
    const uint32_t word4 = matrix.words[4];

    matrix.words[0] =
        PackSignedLowHigh16(FixedMul(SignExtendLow16(word0), scale[0]),
                            FixedMul(static_cast<int16_t>(word0 >> 16),
                                     scale[1]));
    matrix.words[1] =
        PackSignedLowHigh16(FixedMul(SignExtendLow16(word1), scale[2]),
                            FixedMul(static_cast<int16_t>(word1 >> 16),
                                     scale[0]));
    matrix.words[2] =
        PackSignedLowHigh16(FixedMul(SignExtendLow16(word2), scale[1]),
                            FixedMul(static_cast<int16_t>(word2 >> 16),
                                     scale[2]));
    matrix.words[3] =
        PackSignedLowHigh16(FixedMul(SignExtendLow16(word3), scale[0]),
                            FixedMul(static_cast<int16_t>(word3 >> 16),
                                     scale[1]));
    matrix.words[4] =
        static_cast<uint32_t>(FixedMul(SignExtendLow16(word4), scale[2]));
    return matrix;
}

static PsxTodMatrix3x48003F790 MakeIdentityMatrix80028054() {
    PsxTodMatrix3x48003F790 matrix{};
    SetMatrixHalfword8003F790(matrix, 0u, kTodFixedOne);
    SetMatrixHalfword8003F790(matrix, 4u, kTodFixedOne);
    SetMatrixHalfword8003F790(matrix, 8u, kTodFixedOne);
    return matrix;
}

static bool ReadS16AtShortOffset(const std::vector<uint32_t>& data,
                                 size_t shortOffset,
                                 int16_t& out) {
    const size_t wordIndex = shortOffset / 2u;
    if (wordIndex >= data.size()) {
        return false;
    }
    const uint32_t word = data[wordIndex];
    out = (shortOffset & 1u) != 0u
              ? static_cast<int16_t>(word >> 16)
              : static_cast<int16_t>(word & 0xFFFFu);
    return true;
}

static bool ReadS32AtShortOffset(const std::vector<uint32_t>& data,
                                 size_t shortOffset,
                                 int32_t& out) {
    if ((shortOffset & 1u) != 0u) {
        return false;
    }
    const size_t wordIndex = shortOffset / 2u;
    if (wordIndex >= data.size()) {
        return false;
    }
    out = static_cast<int32_t>(data[wordIndex]);
    return true;
}

static bool ReadU16AtByteOffset(const std::vector<uint8_t>& data,
                                size_t offset,
                                uint16_t& out) {
    if (offset + 2u > data.size()) {
        return false;
    }
    out = static_cast<uint16_t>(data[offset]) |
          static_cast<uint16_t>(static_cast<uint16_t>(data[offset + 1u]) << 8);
    return true;
}

static bool ReadU32AtByteOffset(const std::vector<uint8_t>& data,
                                size_t offset,
                                uint32_t& out) {
    if (offset + 4u > data.size()) {
        return false;
    }
    out = static_cast<uint32_t>(data[offset]) |
          (static_cast<uint32_t>(data[offset + 1u]) << 8) |
          (static_cast<uint32_t>(data[offset + 2u]) << 16) |
          (static_cast<uint32_t>(data[offset + 3u]) << 24);
    return true;
}

static void InitType1State(TodType1TrsState80028054& state) {
    state.valid = true;
    state.scale = {kTodFixedOne, kTodFixedOne, kTodFixedOne};
    state.rotation = {0, 0, 0};
    state.translation = {0, 0, 0};
}

static void RebuildCoordFromType1State(
    const TodType1TrsState80028054& state,
    TodCursorRuntime& runtime) {
    const PsxTodMatrix3x48003F790 matrix =
        PsxCall8003B0FC_ScaleMatrix(
            PsxCall8003F790_RotMatrix(state.rotation[0],
                                      state.rotation[1],
                                      state.rotation[2]),
            state.scale);
    runtime.coord.m[0][0] = MatrixHalfword80028054(matrix, 0u);
    runtime.coord.m[0][1] = MatrixHalfword80028054(matrix, 1u);
    runtime.coord.m[0][2] = MatrixHalfword80028054(matrix, 2u);
    runtime.coord.m[1][0] = MatrixHalfword80028054(matrix, 3u);
    runtime.coord.m[1][1] = MatrixHalfword80028054(matrix, 4u);
    runtime.coord.m[1][2] = MatrixHalfword80028054(matrix, 5u);
    runtime.coord.m[2][0] = MatrixHalfword80028054(matrix, 6u);
    runtime.coord.m[2][1] = MatrixHalfword80028054(matrix, 7u);
    runtime.coord.m[2][2] = MatrixHalfword80028054(matrix, 8u);
    for (int i = 0; i < 3; ++i) {
        runtime.coord.t[i] = state.translation[i];
    }
    runtime.coordValid = true;
}

static void StoreMatrix80028054(const PsxTodMatrix3x48003F790& matrix,
                                TodCursorRuntime& runtime) {
    runtime.coord.m[0][0] = MatrixHalfword80028054(matrix, 0u);
    runtime.coord.m[0][1] = MatrixHalfword80028054(matrix, 1u);
    runtime.coord.m[0][2] = MatrixHalfword80028054(matrix, 2u);
    runtime.coord.m[1][0] = MatrixHalfword80028054(matrix, 3u);
    runtime.coord.m[1][1] = MatrixHalfword80028054(matrix, 4u);
    runtime.coord.m[1][2] = MatrixHalfword80028054(matrix, 5u);
    runtime.coord.m[2][0] = MatrixHalfword80028054(matrix, 6u);
    runtime.coord.m[2][1] = MatrixHalfword80028054(matrix, 7u);
    runtime.coord.m[2][2] = MatrixHalfword80028054(matrix, 8u);
    runtime.coordValid = true;
}

static void StoreTranslation80028054(
    const TodType1TrsState80028054& state,
    TodCursorRuntime& runtime) {
    for (int i = 0; i < 3; ++i) {
        runtime.coord.t[i] = state.translation[i];
    }
    runtime.coordValid = true;
}

static bool ApplyType1DescCoord80028054(const TodCommand& cmd,
                                        TodCursorRuntime& runtime,
                                        TodType1TrsState80028054*
                                            type1TrsState80028054) {
    const uint16_t modeFlags =
        static_cast<uint16_t>((cmd.header >> 20) & 0xFu);
    if (type1TrsState80028054 == nullptr) {
        return false;
    }

    if (modeFlags == 0u) {
        StoreMatrix80028054(MakeIdentityMatrix80028054(), runtime);
        return true;
    }

    TodType1TrsState80028054& state = *type1TrsState80028054;
    if (!state.valid) {
        InitType1State(state);
    }

    const bool additive = (modeFlags & 1u) != 0u;
    size_t shortOffset = 0;
    PsxTodMatrix3x48003F790 localMatrix = MakeIdentityMatrix80028054();
    if ((modeFlags & 2u) != 0u) {
        int32_t value = 0;
        if (!ReadS32AtShortOffset(cmd.data, shortOffset, value)) {
            return false;
        }
        const int16_t rx = static_cast<int16_t>(value / 360);
        if (!ReadS32AtShortOffset(cmd.data, shortOffset + 2u, value)) {
            return false;
        }
        const int16_t ry = static_cast<int16_t>(value / 360);
        if (!ReadS32AtShortOffset(cmd.data, shortOffset + 4u, value)) {
            return false;
        }
        const int16_t rz = static_cast<int16_t>(value / 360);
        if (additive) {
            state.rotation[0] =
                static_cast<int16_t>(state.rotation[0] + rx);
            state.rotation[1] =
                static_cast<int16_t>(state.rotation[1] + ry);
            state.rotation[2] =
                static_cast<int16_t>(state.rotation[2] + rz);
        } else {
            state.rotation = {rx, ry, rz};
            localMatrix = PsxCall8003F790_RotMatrix(state.rotation[0],
                                                    state.rotation[1],
                                                    state.rotation[2]);
        }
        shortOffset += 6u;
    }

    if ((modeFlags & 4u) != 0u) {
        int16_t sx = 0;
        int16_t sy = 0;
        int16_t sz = 0;
        if (!ReadS16AtShortOffset(cmd.data, shortOffset, sx) ||
            !ReadS16AtShortOffset(cmd.data, shortOffset + 1u, sy) ||
            !ReadS16AtShortOffset(cmd.data, shortOffset + 2u, sz)) {
            return false;
        }
        if (additive) {
            state.scale[0] = FixedMul(state.scale[0], sx);
            state.scale[1] = FixedMul(state.scale[1], sy);
            state.scale[2] = FixedMul(state.scale[2], sz);
        } else {
            state.scale = {sx, sy, sz};
            if ((modeFlags & 2u) == 0u) {
                localMatrix =
                    PsxCall8003F790_RotMatrix(state.rotation[0],
                                              state.rotation[1],
                                              state.rotation[2]);
            }
            localMatrix =
                PsxCall8003B0FC_ScaleMatrix(localMatrix, state.scale);
        }
        shortOffset += 4u;
    }

    if ((modeFlags & 8u) != 0u) {
        int32_t tx = 0;
        int32_t ty = 0;
        int32_t tz = 0;
        if (!ReadS32AtShortOffset(cmd.data, shortOffset, tx) ||
            !ReadS32AtShortOffset(cmd.data, shortOffset + 2u, ty) ||
            !ReadS32AtShortOffset(cmd.data, shortOffset + 4u, tz)) {
            return false;
        }
        if (additive) {
            state.translation[0] += tx;
            state.translation[1] += ty;
            state.translation[2] += tz;
        } else {
            state.translation = {tx, ty, tz};
        }
    }

    if (additive) {
        RebuildCoordFromType1State(state, runtime);
    } else {
        StoreMatrix80028054(localMatrix, runtime);
        if ((modeFlags & 8u) != 0u) {
            StoreTranslation80028054(state, runtime);
        }
    }
    return true;
}

static bool ApplyType0DescAttrMerge(const TodCommand& cmd,
                                    uint32_t* descAttr) {
    if (descAttr == nullptr || cmd.data.size() < 2u) {
        return false;
    }
    *descAttr = (*descAttr & cmd.data[0]) | cmd.data[1];
    return true;
}

}  // namespace

void Reset(TodCursorRuntime& runtime) {
    runtime = TodCursorRuntime{};
}

static void InitFromTod(TodCursorRuntime& runtime,
                        const TodData& tod,
                        uint16_t handle,
                        uint32_t startFrame) {
    runtime = TodCursorRuntime{};
    runtime.initialized = true;
    runtime.handle = handle;
    runtime.startFrame = startFrame;
    runtime.cursorBlockIndex = 0;
    runtime.remainingBlocks = static_cast<int32_t>(tod.rawBlockCount);
    runtime.lastLocalFrame = 0;
}

static void ApplyType4DescCoord80028054(const std::vector<uint32_t>& data,
                                        TodCoordMatrix& out) {
    if (data.size() < 8u) {
        return;
    }

    out.m[0][0] = static_cast<int16_t>(data[0] & 0xFFFFu);
    out.m[1][0] = static_cast<int16_t>(data[0] >> 16);
    out.m[2][0] = static_cast<int16_t>(data[1] & 0xFFFFu);
    out.m[0][1] = static_cast<int16_t>(data[1] >> 16);
    out.m[1][1] = static_cast<int16_t>(data[2] & 0xFFFFu);
    out.m[2][1] = static_cast<int16_t>(data[2] >> 16);
    out.m[0][2] = static_cast<int16_t>(data[3] & 0xFFFFu);
    out.m[1][2] = static_cast<int16_t>(data[3] >> 16);
    out.m[2][2] = static_cast<int16_t>(data[4] & 0xFFFFu);
    out.t[0] = static_cast<int32_t>(data[5]);
    out.t[1] = static_cast<int32_t>(data[6]);
    out.t[2] = static_cast<int32_t>(data[7]);
}

static bool ReadRawCommand80028054(const TodData& tod,
                                   uint32_t commandOffset,
                                   TodCommand& out,
                                   uint32_t& outNextOffset) {
    uint32_t header = 0;
    if (!ReadU32AtByteOffset(tod.rawBytes, commandOffset, header)) {
        return false;
    }
    uint32_t lenDwords = (header >> 24) & 0xFFu;
    if (lenDwords == 0u) {
        lenDwords = 1u;
    }
    const uint64_t byteLen = static_cast<uint64_t>(lenDwords) * 4u;
    if (byteLen > UINT32_MAX ||
        static_cast<uint64_t>(commandOffset) + byteLen > tod.rawBytes.size()) {
        return false;
    }

    out = TodCommand{};
    out.header = header;
    out.data.resize(lenDwords - 1u);
    for (uint32_t i = 0; i + 1u < lenDwords; ++i) {
        uint32_t word = 0;
        if (!ReadU32AtByteOffset(tod.rawBytes,
                                 commandOffset + 4u + i * 4u,
                                 word)) {
            return false;
        }
        out.data[i] = word;
    }
    outNextOffset = commandOffset + static_cast<uint32_t>(byteLen);
    return true;
}

static bool ApplyRawBlockDescCoord80028504(
    const TodData& tod,
    uint32_t rawCursorOffset,
    uint32_t seq,
    TodCursorRuntime& runtime,
    uint32_t* descAttr,
    TodType1TrsState80028054* type1TrsState80028054,
    TodCommandSideEffects80028054* outSideEffects,
    uint32_t& outNewRawCursorOffset) {
    outNewRawCursorOffset = rawCursorOffset;
    uint16_t commandCount = 0;
    uint32_t triggerTime = 0;
    if (!ReadU16AtByteOffset(tod.rawBytes, rawCursorOffset + 2u, commandCount) ||
        !ReadU32AtByteOffset(tod.rawBytes, rawCursorOffset + 4u, triggerTime)) {
        return false;
    }
    if (outSideEffects != nullptr) {
        outSideEffects->oldRawCursorOffset80028504 = rawCursorOffset;
        outSideEffects->newRawCursorOffset80028504 = rawCursorOffset;
    }
    if (seq < triggerTime) {
        return false;
    }

    bool commandExecuted = false;
    uint32_t commandOffset = rawCursorOffset + 8u;
    if (outSideEffects != nullptr) {
        outSideEffects->blockTriggered80028504 = true;
        outSideEffects->commandCount80028504 = commandCount;
    }
    for (uint16_t commandIndex = 0; commandIndex < commandCount;
         ++commandIndex) {
        TodCommand cmd{};
        uint32_t nextCommandOffset = commandOffset;
        if (!ReadRawCommand80028054(tod,
                                    commandOffset,
                                    cmd,
                                    nextCommandOffset)) {
            break;
        }
        commandExecuted = true;
        const uint16_t cmdType =
            static_cast<uint16_t>((cmd.header >> 16) & 0xFu);
        const uint16_t cmdFlags =
            static_cast<uint16_t>((cmd.header >> 20) & 0xFu);
        if (outSideEffects != nullptr) {
            outSideEffects->descCommandExecuted80028054 = true;
            outSideEffects->descHeaderCleared80028054 = true;
            outSideEffects->lastCommandObjectId80028054 =
                static_cast<uint16_t>(cmd.header & 0xFFFFu);
            outSideEffects->lastCommandType80028054 = cmdType;
            outSideEffects->lastCommandFlags80028054 = cmdFlags;
        }
        if (cmdType == 0u) {
            const bool attrMerged = ApplyType0DescAttrMerge(cmd, descAttr);
            if (outSideEffects != nullptr) {
                outSideEffects->descAttrMerged80028054 =
                    outSideEffects->descAttrMerged80028054 || attrMerged;
            }
        } else if (cmdType == 1u) {
            const bool coordWritten =
                ApplyType1DescCoord80028054(cmd,
                                            runtime,
                                            type1TrsState80028054);
            if (outSideEffects != nullptr) {
                outSideEffects->type1CoordBackingMissing80028054 =
                    outSideEffects->type1CoordBackingMissing80028054 ||
                    (type1TrsState80028054 == nullptr);
                outSideEffects->type1CoordWritten80028054 =
                    outSideEffects->type1CoordWritten80028054 || coordWritten;
                outSideEffects->coordWritten80028054 =
                    outSideEffects->coordWritten80028054 || coordWritten;
            }
        } else if (cmdType == 4u && cmd.data.size() >= 8u) {
            ApplyType4DescCoord80028054(cmd.data, runtime.coord);
            runtime.coordValid = true;
            if (outSideEffects != nullptr) {
                outSideEffects->type4CoordWritten80028054 = true;
                outSideEffects->coordWritten80028054 = true;
            }
        }
        commandOffset = nextCommandOffset;
    }
    outNewRawCursorOffset = commandOffset;
    if (outSideEffects != nullptr) {
        outSideEffects->newRawCursorOffset80028504 = commandOffset;
    }
    return commandExecuted;
}

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
                                      uint32_t* outNewRawCursorOffset,
                                      int32_t* outRemainingBlocks,
                                      uint32_t* descAttr,
                                      TodType1TrsState80028054*
                                          type1TrsState80028054,
                                      bool* outDescCommandExecuted,
                                      TodCommandSideEffects80028054*
                                          outSideEffects) {
    outCoord = TodCoordMatrix{};
    outCoordValid = false;
    outCursorChanged = false;
    if (outDescCommandExecuted != nullptr) {
        *outDescCommandExecuted = false;
    }
    if (outSideEffects != nullptr) {
        *outSideEffects = TodCommandSideEffects80028054{};
    }
    if (outNewRawCursorOffset != nullptr) {
        *outNewRawCursorOffset = rawCursorOffset;
    }

    if (handle == 0u || tod.blocks.empty()) {
        Reset(runtime);
        if (outRemainingBlocks) {
            *outRemainingBlocks = 0;
        }
        return false;
    }

    if (!runtime.initialized ||
        runtime.handle != handle ||
        runtime.startFrame != startFrame ||
        seq < runtime.lastLocalFrame) {
        InitFromTod(runtime, tod, handle, startFrame);
    }
    const uint32_t parsedBlockCount =
        static_cast<uint32_t>(tod.blocks.size());
    if (rawCursorOffsetKnown) {
        uint32_t rawCursorBlockIndex = parsedBlockCount;
        for (uint32_t i = 0; i < parsedBlockCount; ++i) {
            if (tod.blocks[i].rawOffset == rawCursorOffset) {
                rawCursorBlockIndex = i;
                break;
            }
        }
        const uint32_t clampedBlockIndex =
            (std::min)(rawCursorBlockIndex, parsedBlockCount);
        runtime.cursorBlockIndex = clampedBlockIndex;
        runtime.remainingBlocks =
            static_cast<int32_t>(tod.rawBlockCount - clampedBlockIndex);
    }
    runtime.lastLocalFrame = seq;

    const bool wasLive =
        runtime.remainingBlocks > 0 && rawCursorOffsetKnown;
    if (wasLive && rawCursorOffset < tod.rawBytes.size()) {
        uint32_t newRawCursorOffset = rawCursorOffset;
        const bool descCommandExecuted =
            ApplyRawBlockDescCoord80028504(tod,
                                           rawCursorOffset,
                                           seq,
                                           runtime,
                                           descAttr,
                                           type1TrsState80028054,
                                           outSideEffects,
                                           newRawCursorOffset);
        if (outDescCommandExecuted != nullptr) {
            *outDescCommandExecuted = descCommandExecuted;
        }
        outCursorChanged = newRawCursorOffset != rawCursorOffset;
        if (outNewRawCursorOffset != nullptr) {
            *outNewRawCursorOffset = newRawCursorOffset;
        }
        if (outCursorChanged) {
            ++runtime.cursorBlockIndex;
            --runtime.remainingBlocks;
        }
    }

    if (runtime.coordValid) {
        outCoord = runtime.coord;
        outCoordValid = true;
    }
    if (outRemainingBlocks) {
        *outRemainingBlocks = runtime.remainingBlocks;
    }
    return wasLive;
}

}  // namespace PrStage1TodCursorDirect
