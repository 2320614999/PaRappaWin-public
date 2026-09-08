#include "pr_ss0_title_transform_direct.h"

#include <algorithm>
#include <limits>

namespace PrSS0TitleTransformDirect {
namespace {

constexpr std::size_t kPaLoc2RawBytes = 144u;
constexpr std::size_t kPaLoc2PayloadBytes = 140u;
constexpr uint32_t kPaLoc2BlockRawBytes = 132u;
constexpr uint32_t kPaLoc2RawBlockCount = 1u;
constexpr uint16_t kPaLoc2BlockHeader = 0x21u;
constexpr uint16_t kPaLoc2CommandCount = 9u;
constexpr uint32_t kPaLoc2CoordCommandHeader = 0x09040001u;
constexpr uint8_t kMode25 = 0x25u;
constexpr uint8_t kMode25Ilen = 6u;
constexpr uint8_t kMode25Olen = 7u;
constexpr uint32_t kMode25PacketBytes = 28u;

static constexpr int16_t kPsxSqrtTable8005C9D4[] = {
    4096, 4127, 4159, 4190, 4222, 4252, 4283, 4314,
    4344, 4374, 4404, 4434, 4463, 4492, 4521, 4550,
    4579, 4608, 4636, 4664, 4692, 4720, 4748, 4775,
    4802, 4830, 4857, 4884, 4910, 4937, 4964, 4990,
    5016, 5042, 5068, 5094, 5120, 5145, 5170, 5196,
    5221, 5246, 5271, 5296, 5320, 5345, 5369, 5394,
    5418, 5442, 5466, 5490, 5514, 5538, 5561, 5585,
    5608, 5632, 5655, 5678, 5701, 5724, 5747, 5769,
    5792, 5815, 5837, 5860, 5882, 5904, 5926, 5948,
    5970, 5992, 6014, 6036, 6058, 6079, 6101, 6122,
    6144, 6165, 6186, 6207, 6228, 6249, 6270, 6291,
    6312, 6333, 6353, 6374, 6394, 6415, 6435, 6456,
    6476, 6496, 6516, 6536, 6556, 6576, 6596, 6616,
    6636, 6656, 6675, 6695, 6714, 6734, 6753, 6773,
    6792, 6811, 6830, 6850, 6869, 6888, 6907, 6926,
    6945, 6963, 6982, 7001, 7020, 7038, 7057, 7075,
    7094, 7112, 7131, 7149, 7168, 7186, 7204, 7222,
    7240, 7258, 7276, 7294, 7312, 7330, 7348, 7366,
    7384, 7401, 7419, 7437, 7454, 7472, 7489, 7507,
    7524, 7542, 7559, 7576, 7594, 7611, 7628, 7645,
    7662, 7680, 7697, 7714, 7731, 7747, 7764, 7781,
    7798, 7815, 7832, 7848, 7865, 7882, 7898, 7915,
    7931, 7948, 7964, 7981, 7997, 8014, 8030, 8046,
    8062, 8079, 8095, 8111, 8127, 8143, 8159, 8175,
};

BaseCarrierResult FailBase(Failure failure)
{
    BaseCarrierResult result{};
    result.failure = failure;
    return result;
}

DeformationResult FailDeformation(Failure failure)
{
    DeformationResult result{};
    result.failure = failure;
    return result;
}

Rtpt3InputResult FailRtpt3(Failure failure)
{
    Rtpt3InputResult result{};
    result.failure = failure;
    return result;
}

Rtpt3ExecutionResult800428B0 FailRtpt3Execution(Failure failure)
{
    Rtpt3ExecutionResult800428B0 result{};
    result.failure = failure;
    return result;
}

TitleRuntimeMatrixResult801C5E60 FailRuntimeMatrix(Failure failure)
{
    TitleRuntimeMatrixResult801C5E60 result{};
    result.failure = failure;
    return result;
}

TitleTodAdvanceResult8001B000 FailTitleTodAdvance(
    Failure failure,
    const TitleTodCursorState8001B000& state)
{
    TitleTodAdvanceResult8001B000 result{};
    result.failure = failure;
    result.nextState = state;
    return result;
}

TitlePanelCoordResult801C5D8C FailTitlePanelCoord(Failure failure)
{
    TitlePanelCoordResult801C5D8C result{};
    result.failure = failure;
    return result;
}

TitleCoordWithCameraResult80041A68 FailTitleCoordWithCamera(
    Failure failure)
{
    TitleCoordWithCameraResult80041A68 result{};
    result.failure = failure;
    return result;
}

TitlePanelWithCameraResult80041A68 FailTitlePanelWithCamera(
    Failure failure)
{
    TitlePanelWithCameraResult80041A68 result{};
    result.failure = failure;
    return result;
}

TitleCameraMatrixResult80041D3C FailTitleCameraMatrix(Failure failure)
{
    TitleCameraMatrixResult80041D3C result{};
    result.failure = failure;
    return result;
}

TitleCameraAdvanceResult801C6410 FailTitleCameraAdvance(
    Failure failure,
    const TitleCameraCursorState801CB59C& state)
{
    TitleCameraAdvanceResult801C6410 result{};
    result.failure = failure;
    result.nextState = state;
    return result;
}

int64_t ArithmeticShiftRight(int64_t value, unsigned bits)
{
    if (bits == 0u) {
        return value;
    }
    const int64_t divisor = int64_t{1} << bits;
    if (value >= 0) {
        return value / divisor;
    }
    return -(((-value) + divisor - 1) / divisor);
}

int64_t ArithmeticShiftRight12(int64_t value)
{
    return ArithmeticShiftRight(value, 12u);
}

int32_t WrapSigned32(int64_t value)
{
    const uint32_t bits = static_cast<uint32_t>(value);
    return (bits & 0x80000000u) == 0u
               ? static_cast<int32_t>(bits)
               : static_cast<int32_t>(
                     static_cast<int64_t>(bits) - 0x100000000ll);
}

int32_t ShiftRightSigned32(int32_t value, unsigned bits)
{
    return static_cast<int32_t>(ArithmeticShiftRight(value, bits));
}

int LeadingZeroCount32(uint32_t value)
{
    if (value == 0u) {
        return 32;
    }
    int count = 0;
    for (uint32_t bit = 0x80000000u; (value & bit) == 0u; bit >>= 1u) {
        ++count;
    }
    return count;
}

int32_t ReadSigned32LittleEndian(const uint8_t* bytes)
{
    const uint32_t value = static_cast<uint32_t>(bytes[0]) |
                           (static_cast<uint32_t>(bytes[1]) << 8u) |
                           (static_cast<uint32_t>(bytes[2]) << 16u) |
                           (static_cast<uint32_t>(bytes[3]) << 24u);
    return WrapSigned32(value);
}

bool SameTitleCameraView(const TitleCameraView801C6FB4& view,
                         const std::array<int32_t, 3>& position,
                         const std::array<int32_t, 3>& target)
{
    return view.known && view.position == position && view.target == target &&
           view.twist == 0 && view.superAddress == 0u;
}

bool SquareRoot0_80041548(uint32_t value, int32_t& out)
{
    out = 0;
    if (value == 0u || value > 0x7FFFFFFFu) {
        return value == 0u;
    }

    const int leading = LeadingZeroCount32(value);
    const int evenLeading = leading & ~1;
    const uint32_t tableValue = evenLeading < 24
                                    ? value >> (24 - evenLeading)
                                    : value << (evenLeading - 24);
    if (tableValue < 64u) {
        return false;
    }
    const std::size_t tableIndex = tableValue - 64u;
    if (tableIndex >=
        sizeof(kPsxSqrtTable8005C9D4) /
            sizeof(kPsxSqrtTable8005C9D4[0])) {
        return false;
    }

    const int shift = (31 - evenLeading) >> 1;
    const uint32_t scaled =
        static_cast<uint32_t>(kPsxSqrtTable8005C9D4[tableIndex]) << shift;
    out = static_cast<int32_t>(scaled >> 12u);
    return true;
}

void CordicSquareRootStep8004234C(int32_t& positive,
                                 int32_t& residual,
                                 unsigned shift)
{
    const int32_t oldPositive = positive;
    const int32_t oldResidual = residual;
    if (oldResidual >= 0) {
        positive = WrapSigned32(
            static_cast<int64_t>(oldPositive) -
            ShiftRightSigned32(oldResidual, shift));
        residual = WrapSigned32(
            static_cast<int64_t>(oldResidual) -
            ShiftRightSigned32(oldPositive, shift));
    } else {
        positive = WrapSigned32(
            static_cast<int64_t>(oldPositive) +
            ShiftRightSigned32(oldResidual, shift));
        residual = WrapSigned32(
            static_cast<int64_t>(oldResidual) +
            ShiftRightSigned32(oldPositive, shift));
    }
}

int32_t CordicSquareRootCore8004234C(uint32_t value)
{
    constexpr int32_t kCordicBias = 0x005D50AD;
    int32_t positive = WrapSigned32(
        static_cast<int64_t>(value) + kCordicBias);
    int32_t residual = WrapSigned32(
        static_cast<int64_t>(value) - kCordicBias);
    for (unsigned shift = 1u; shift <= 6u; ++shift) {
        CordicSquareRootStep8004234C(positive, residual, shift);
        if (shift == 4u) {
            CordicSquareRootStep8004234C(positive, residual, shift);
        }
    }
    return positive;
}

bool SquareRoot12_800424A0(uint32_t value, int32_t& out)
{
    out = 0;
    if (value == 0u) {
        return true;
    }
    if (value > 0x7FFFFFFFu) {
        return false;
    }

    const int scale = 8 - LeadingZeroCount32(value);
    int halfScale = ShiftRightSigned32(scale, 1u);
    uint32_t normalized = 0u;
    if (scale < 0) {
        ++halfScale;
        const int leftShift = -2 * halfScale;
        if (leftShift < 0 || leftShift >= 32) {
            return false;
        }
        normalized = value << leftShift;
    } else {
        const int rightShift = 2 * halfScale;
        if (rightShift < 0 || rightShift >= 32) {
            return false;
        }
        normalized = value >> rightShift;
    }

    const int resultScale = halfScale - 6;
    int32_t result = CordicSquareRootCore8004234C(normalized);
    if (resultScale >= 0) {
        if (resultScale >= 32) {
            return false;
        }
        result = WrapSigned32(
            static_cast<uint32_t>(result) << resultScale);
    } else {
        if (-resultScale >= 32) {
            return false;
        }
        result = ShiftRightSigned32(result,
                                    static_cast<unsigned>(-resultScale));
    }
    out = result;
    return result >= 0;
}

bool BuildSignedUnitComponent80041D3C(int32_t delta,
                                     uint32_t denominatorSquared,
                                     bool negate,
                                     int16_t& out)
{
    out = 0;
    const uint64_t magnitude = delta < 0
                                   ? static_cast<uint64_t>(
                                         -static_cast<int64_t>(delta))
                                   : static_cast<uint64_t>(delta);
    const uint64_t square64 = magnitude * magnitude;
    if (square64 > 0x7FFFFFFFu || denominatorSquared == 0u ||
        denominatorSquared > 0x7FFFFFFFu) {
        return false;
    }
    const uint32_t square = static_cast<uint32_t>(square64);
    if (square == 0u) {
        return true;
    }

    const int shift = 12 - LeadingZeroCount32(square);
    int32_t componentMagnitude = 0;
    if (shift >= 0) {
        const unsigned leftShift = static_cast<unsigned>(12 - shift);
        const uint32_t numerator = square << leftShift;
        const uint32_t denominator =
            denominatorSquared >> static_cast<unsigned>(shift);
        if (denominator == 0u ||
            !SquareRoot12_800424A0(
                numerator / denominator, componentMagnitude)) {
            return false;
        }
    } else {
        int32_t denominatorRoot = 0;
        if (!SquareRoot0_80041548(
                denominatorSquared, denominatorRoot) ||
            denominatorRoot == 0) {
            return false;
        }
        componentMagnitude = static_cast<int32_t>(
            (magnitude * 4096u) /
            static_cast<uint32_t>(denominatorRoot));
    }

    int64_t signedValue = delta < 0 ? -componentMagnitude
                                    : componentMagnitude;
    if (negate) {
        signedValue = -signedValue;
    }
    if (signedValue < (std::numeric_limits<int16_t>::min)() ||
        signedValue > (std::numeric_limits<int16_t>::max)()) {
        return false;
    }
    out = static_cast<int16_t>(signedValue);
    return true;
}

bool BuildPositiveUnitComponent80041D3C(uint32_t numeratorSquared,
                                       uint32_t denominatorSquared,
                                       int16_t& out)
{
    out = 0;
    if (numeratorSquared == 0u) {
        return true;
    }
    if (numeratorSquared > 0x7FFFFFFFu || denominatorSquared == 0u ||
        denominatorSquared > 0x7FFFFFFFu) {
        return false;
    }

    const int leading = LeadingZeroCount32(numeratorSquared);
    const int shift = 12 - leading;
    int32_t component = 0;
    if (shift >= 0) {
        const uint32_t numerator = numeratorSquared << leading;
        const uint32_t denominator =
            denominatorSquared >> static_cast<unsigned>(shift);
        if (denominator == 0u ||
            !SquareRoot12_800424A0(
                numerator / denominator, component)) {
            return false;
        }
    } else {
        int32_t numeratorRoot = 0;
        int32_t denominatorRoot = 0;
        if (!SquareRoot0_80041548(numeratorSquared, numeratorRoot) ||
            !SquareRoot0_80041548(
                denominatorSquared, denominatorRoot) ||
            denominatorRoot == 0) {
            return false;
        }
        component = static_cast<int32_t>(
            (static_cast<int64_t>(numeratorRoot) * 4096) /
            denominatorRoot);
    }
    if (component < 0 || component > (std::numeric_limits<int16_t>::max)()) {
        return false;
    }
    out = static_cast<int16_t>(component);
    return true;
}

bool MultiplyTitleCameraRotation80040884(
    const int16_t (&left)[3][3],
    const int16_t (&right)[3][3],
    int16_t (&out)[3][3])
{
    int16_t candidate[3][3] = {};
    for (std::size_t row = 0u; row < 3u; ++row) {
        for (std::size_t column = 0u; column < 3u; ++column) {
            int64_t sum = 0;
            for (std::size_t lane = 0u; lane < 3u; ++lane) {
                sum += static_cast<int64_t>(left[row][lane]) *
                       right[lane][column];
            }
            const int64_t value = ArithmeticShiftRight12(sum);
            if (value < (std::numeric_limits<int16_t>::min)() ||
                value > (std::numeric_limits<int16_t>::max)()) {
                return false;
            }
            candidate[row][column] = static_cast<int16_t>(value);
        }
    }
    for (std::size_t row = 0u; row < 3u; ++row) {
        for (std::size_t column = 0u; column < 3u; ++column) {
            out[row][column] = candidate[row][column];
        }
    }
    return true;
}

int16_t AddS16Wrap(int16_t base, int64_t delta)
{
    const uint32_t sum =
        static_cast<uint32_t>(static_cast<uint16_t>(base)) +
        static_cast<uint32_t>(static_cast<uint16_t>(delta));
    const uint32_t wrapped = sum & 0xFFFFu;
    const int32_t signedValue = wrapped >= 0x8000u
                                    ? static_cast<int32_t>(wrapped) - 0x10000
                                    : static_cast<int32_t>(wrapped);
    return static_cast<int16_t>(signedValue);
}

int32_t SignedLow16(uint32_t value)
{
    return static_cast<int16_t>(value & 0xFFFFu);
}

int32_t SignedHigh16(uint32_t value)
{
    return static_cast<int16_t>(value >> 16u);
}

void UnpackMatrixRotation(const PrPsxGteDirect::Matrix3x4& packed,
                          int32_t (&rotation)[3][3])
{
    rotation[0][0] = SignedLow16(packed.words[0]);
    rotation[0][1] = SignedHigh16(packed.words[0]);
    rotation[0][2] = SignedLow16(packed.words[1]);
    rotation[1][0] = SignedHigh16(packed.words[1]);
    rotation[1][1] = SignedLow16(packed.words[2]);
    rotation[1][2] = SignedHigh16(packed.words[2]);
    rotation[2][0] = SignedLow16(packed.words[3]);
    rotation[2][1] = SignedHigh16(packed.words[3]);
    rotation[2][2] = SignedLow16(packed.words[4]);
}

uint32_t PackSigned16Pair(int32_t low, int32_t high)
{
    return static_cast<uint32_t>(static_cast<uint16_t>(low)) |
           (static_cast<uint32_t>(static_cast<uint16_t>(high)) << 16u);
}

bool IsSupportedTitleTodCommandType(uint16_t type)
{
    switch (type) {
    case 0u:
    case 2u:
    case 3u:
    case 4u:
    case 7u:
    case 8u:
    case 9u:
        return true;
    default:
        return false;
    }
}

bool IsStructurallyCompleteTitleTod(const TodData& tod)
{
    if (tod.rawBlockCount == 0u || tod.rawBlockCount != tod.blockCount ||
        tod.blocks.size() != tod.blockCount) {
        return false;
    }

    uint32_t previousTrigger = 0u;
    bool firstBlock = true;
    for (const TodBlock& block : tod.blocks) {
        if (block.cmdCount != block.commands.size() ||
            (!firstBlock && block.triggerTime < previousTrigger)) {
            return false;
        }
        firstBlock = false;
        previousTrigger = block.triggerTime;
        for (const TodCommand& command : block.commands) {
            uint32_t declaredDwords = (command.header >> 24u) & 0xFFu;
            if (declaredDwords == 0u) {
                declaredDwords = 1u;
            }
            if (declaredDwords != command.data.size() + 1u) {
                return false;
            }
        }
    }
    return true;
}

bool ApplyTitleTodType4Coord80028054(const TodCommand& command,
                                     TodCoordMatrix& coord)
{
    if (command.data.size() != 8u) {
        return false;
    }
    const std::vector<uint32_t>& data = command.data;
    // 80028054 type 4 stores the nine source halfwords by columns into the
    // row-major PSX MATRIX.  Reading the payload straight across as rows
    // transposes every animated rotation (most visibly PA_DANCE's jump arc).
    coord.m[0][0] = static_cast<int16_t>(data[0] & 0xFFFFu);
    coord.m[1][0] = static_cast<int16_t>(data[0] >> 16u);
    coord.m[2][0] = static_cast<int16_t>(data[1] & 0xFFFFu);
    coord.m[0][1] = static_cast<int16_t>(data[1] >> 16u);
    coord.m[1][1] = static_cast<int16_t>(data[2] & 0xFFFFu);
    coord.m[2][1] = static_cast<int16_t>(data[2] >> 16u);
    coord.m[0][2] = static_cast<int16_t>(data[3] & 0xFFFFu);
    coord.m[1][2] = static_cast<int16_t>(data[3] >> 16u);
    coord.m[2][2] = static_cast<int16_t>(data[4] & 0xFFFFu);
    coord.t[0] = static_cast<int32_t>(data[5]);
    coord.t[1] = static_cast<int32_t>(data[6]);
    coord.t[2] = static_cast<int32_t>(data[7]);
    return true;
}

PrPsxGteDirect::Matrix3x4 PackTitleTodCoord800417A4(
    const TodCoordMatrix& coord)
{
    PrPsxGteDirect::Matrix3x4 matrix{};
    matrix.words[0] = PackSigned16Pair(coord.m[0][0], coord.m[0][1]);
    matrix.words[1] = PackSigned16Pair(coord.m[0][2], coord.m[1][0]);
    matrix.words[2] = PackSigned16Pair(coord.m[1][1], coord.m[1][2]);
    matrix.words[3] = PackSigned16Pair(coord.m[2][0], coord.m[2][1]);
    matrix.words[4] = PackSigned16Pair(coord.m[2][2], 0);
    matrix.words[5] = static_cast<uint32_t>(coord.t[0]);
    matrix.words[6] = static_cast<uint32_t>(coord.t[1]);
    matrix.words[7] = static_cast<uint32_t>(coord.t[2]);
    return matrix;
}

bool IsSigned16(int64_t value)
{
    return value >= (std::numeric_limits<int16_t>::min)() &&
           value <= (std::numeric_limits<int16_t>::max)();
}

bool IsSigned32(int64_t value)
{
    return value >= (std::numeric_limits<int32_t>::min)() &&
           value <= (std::numeric_limits<int32_t>::max)();
}

bool ComposePackedAffineMatrix(const PrPsxGteDirect::Matrix3x4& lhs,
                               const PrPsxGteDirect::Matrix3x4& rhs,
                               PrPsxGteDirect::Matrix3x4& out)
{
    int32_t lhsRotation[3][3] = {};
    int32_t rhsRotation[3][3] = {};
    int32_t resultRotation[3][3] = {};
    UnpackMatrixRotation(lhs, lhsRotation);
    UnpackMatrixRotation(rhs, rhsRotation);

    for (std::size_t row = 0u; row < 3u; ++row) {
        for (std::size_t column = 0u; column < 3u; ++column) {
            int64_t sum = 0;
            for (std::size_t lane = 0u; lane < 3u; ++lane) {
                sum += static_cast<int64_t>(lhsRotation[row][lane]) *
                       rhsRotation[lane][column];
            }
            const int64_t value = ArithmeticShiftRight12(sum);
            if (!IsSigned16(value)) {
                return false;
            }
            resultRotation[row][column] = static_cast<int32_t>(value);
        }
    }

    int32_t resultTranslation[3] = {};
    for (std::size_t row = 0u; row < 3u; ++row) {
        int64_t sum = 0;
        for (std::size_t lane = 0u; lane < 3u; ++lane) {
            sum += static_cast<int64_t>(lhsRotation[row][lane]) *
                   static_cast<int32_t>(rhs.words[5u + lane]);
        }
        const int64_t value =
            ArithmeticShiftRight12(sum) +
            static_cast<int32_t>(lhs.words[5u + row]);
        if (!IsSigned32(value)) {
            return false;
        }
        resultTranslation[row] = static_cast<int32_t>(value);
    }

    out = {};
    out.words[0] =
        PackSigned16Pair(resultRotation[0][0], resultRotation[0][1]);
    out.words[1] =
        PackSigned16Pair(resultRotation[0][2], resultRotation[1][0]);
    out.words[2] =
        PackSigned16Pair(resultRotation[1][1], resultRotation[1][2]);
    out.words[3] =
        PackSigned16Pair(resultRotation[2][0], resultRotation[2][1]);
    out.words[4] = PackSigned16Pair(resultRotation[2][2], 0);
    for (std::size_t lane = 0u; lane < 3u; ++lane) {
        out.words[5u + lane] =
            static_cast<uint32_t>(resultTranslation[lane]);
    }
    return true;
}

bool HasSupportedMode25Shape(const TmdPrimitive& primitive)
{
    return primitive.mode == kMode25 && primitive.ilen == kMode25Ilen &&
           primitive.olen == kMode25Olen && primitive.textured &&
           !primitive.quad &&
           primitive.rawPacketByteSize == kMode25PacketBytes;
}

bool IsSupportedModelPath(ModelPath path)
{
    switch (path) {
    case ModelPath::Pa:
    case ModelPath::PaKage:
    case ModelPath::Lo:
    case ModelPath::Hp:
        return true;
    }
    return false;
}

PrPsxGteDirect::VertexS16 ToGteVertex(const TmdVertex& vertex)
{
    PrPsxGteDirect::VertexS16 out{};
    out.x = vertex.x;
    out.y = vertex.y;
    out.z = vertex.z;
    out.pad = vertex.pad;
    return out;
}

} // namespace

bool IsExactTitleGteControl801C609C(
    const PrPsxGteDirect::GteControlState& control)
{
    return control.geomScreenKnown && control.geomScreen == 440u &&
           control.geomOffsetKnown &&
           control.geomOffsetX == 0 && control.geomOffsetY == 0 &&
           control.depthCueKnown && control.depthCueA == -4194 &&
           control.depthCueB == 0x01400000 &&
           control.zScaleFactorKnown && control.zScaleFactor3 == 341 &&
           control.zScaleFactor4 == 256;
}

bool ParseExactTitleCameraTable801C6FB4(
    const uint8_t* comod0,
    std::size_t byteCount,
    TitleCameraTable801C6FB4& outTable)
{
    outTable = {};
    if (comod0 == nullptr || byteCount != kScene0Comod0ExpectedBytes ||
        kTitleCameraTableEnd801C6FB4 > byteCount) {
        return false;
    }

    TitleCameraTable801C6FB4 candidate{};
    for (std::size_t index = 0u;
         index < kTitleCameraRecordCount801C6FB4;
         ++index) {
        const uint8_t* record =
            comod0 + kTitleCameraTableOffset801C6FB4 +
            index * kTitleCameraRecordStride801C6FB4;
        TitleCameraView801C6FB4 view{};
        view.known = true;
        for (std::size_t lane = 0u; lane < 3u; ++lane) {
            view.position[lane] =
                ReadSigned32LittleEndian(record + lane * 4u);
            view.target[lane] =
                ReadSigned32LittleEndian(record + (lane + 3u) * 4u);
        }
        view.twist = ReadSigned32LittleEndian(record + 0x18u);
        view.superAddress = static_cast<uint32_t>(
            ReadSigned32LittleEndian(record + 0x1Cu));
        if (view.twist != 0 || view.superAddress != 0u) {
            return false;
        }
        candidate.views[index] = view;
    }

    if (!SameTitleCameraView(
            candidate.views[0],
            {{78, -1437, -9869}},
            {{0, -1400, 0}}) ||
        !SameTitleCameraView(
            candidate.views[1],
            {{156, -1474, -9768}},
            {{0, -1400, 0}}) ||
        !SameTitleCameraView(
            candidate.views[200],
            {{1784, -3545, -6791}},
            {{-8, -1613, 69}}) ||
        !SameTitleCameraView(
            candidate.views[299],
            {{0, -1600, -8790}},
            {{0, -1600, 0}})) {
        return false;
    }

    candidate.known = true;
    outTable = candidate;
    return true;
}

TitleCameraMatrixResult80041D3C BuildTitleCameraMatrix80041D3C(
    const TitleCameraView801C6FB4& view)
{
    if (!view.known) {
        return FailTitleCameraMatrix(Failure::TitleCameraSourceUnknown);
    }
    if (view.twist != 0) {
        return FailTitleCameraMatrix(Failure::TitleCameraTwistUnsupported);
    }
    if (view.superAddress != 0u) {
        return FailTitleCameraMatrix(Failure::TitleCameraSuperUnsupported);
    }

    int32_t delta[3] = {};
    for (std::size_t lane = 0u; lane < 3u; ++lane) {
        const int64_t value =
            static_cast<int64_t>(view.target[lane]) -
            view.position[lane];
        if (!IsSigned32(value)) {
            return FailTitleCameraMatrix(Failure::TitleCameraVectorOverflow);
        }
        delta[lane] = static_cast<int32_t>(value);
    }

    uint64_t totalSquared64 = 0u;
    for (int32_t value : delta) {
        totalSquared64 += static_cast<uint64_t>(
            static_cast<int64_t>(value) * value);
    }
    const uint64_t horizontalSquared64 =
        static_cast<uint64_t>(
            static_cast<int64_t>(delta[0]) * delta[0]) +
        static_cast<uint64_t>(
            static_cast<int64_t>(delta[2]) * delta[2]);
    if (totalSquared64 == 0u) {
        return FailTitleCameraMatrix(Failure::TitleCameraDegenerateView);
    }
    if (totalSquared64 > 0x7FFFFFFFu ||
        horizontalSquared64 > 0x7FFFFFFFu) {
        return FailTitleCameraMatrix(Failure::TitleCameraVectorOverflow);
    }
    const uint32_t totalSquared =
        static_cast<uint32_t>(totalSquared64);
    const uint32_t horizontalSquared =
        static_cast<uint32_t>(horizontalSquared64);

    int16_t pitchSin = 0;
    int16_t pitchCos = 0;
    if (!BuildSignedUnitComponent80041D3C(
            delta[1], totalSquared, false, pitchSin) ||
        !BuildPositiveUnitComponent80041D3C(
            horizontalSquared, totalSquared, pitchCos)) {
        return FailTitleCameraMatrix(Failure::TitleCameraVectorOverflow);
    }

    int16_t rotation[3][3] = {
        {4096, 0, 0},
        {0, 4096, 0},
        {0, 0, 4096},
    };
    const int16_t rotateX[3][3] = {
        {4096, 0, 0},
        {0, pitchCos, static_cast<int16_t>(-pitchSin)},
        {0, pitchSin, pitchCos},
    };
    int16_t nextRotation[3][3] = {};
    if (!MultiplyTitleCameraRotation80040884(
            rotation, rotateX, nextRotation)) {
        return FailTitleCameraMatrix(Failure::TitleCameraVectorOverflow);
    }
    for (std::size_t row = 0u; row < 3u; ++row) {
        for (std::size_t column = 0u; column < 3u; ++column) {
            rotation[row][column] = nextRotation[row][column];
        }
    }

    if (horizontalSquared != 0u) {
        int16_t yawSin = 0;
        int16_t yawCos = 0;
        if (!BuildSignedUnitComponent80041D3C(
                delta[0], horizontalSquared, true, yawSin) ||
            !BuildSignedUnitComponent80041D3C(
                delta[2], horizontalSquared, false, yawCos)) {
            return FailTitleCameraMatrix(Failure::TitleCameraVectorOverflow);
        }
        const int16_t rotateY[3][3] = {
            {yawCos, 0, yawSin},
            {0, 4096, 0},
            {static_cast<int16_t>(-yawSin), 0, yawCos},
        };
        if (!MultiplyTitleCameraRotation80040884(
                rotation, rotateY, nextRotation)) {
            return FailTitleCameraMatrix(Failure::TitleCameraVectorOverflow);
        }
        for (std::size_t row = 0u; row < 3u; ++row) {
            for (std::size_t column = 0u; column < 3u; ++column) {
                rotation[row][column] = nextRotation[row][column];
            }
        }
    }

    int32_t translation[3] = {};
    for (std::size_t row = 0u; row < 3u; ++row) {
        int64_t sum = 0;
        for (std::size_t lane = 0u; lane < 3u; ++lane) {
            sum += static_cast<int64_t>(rotation[row][lane]) *
                   -static_cast<int64_t>(view.position[lane]);
        }
        const int64_t value = ArithmeticShiftRight12(sum);
        if (!IsSigned32(value)) {
            return FailTitleCameraMatrix(Failure::TitleCameraVectorOverflow);
        }
        translation[row] = static_cast<int32_t>(value);
    }

    TitleCameraMatrixResult80041D3C result{};
    result.cameraMatrix80092880.words[0] =
        PackSigned16Pair(rotation[0][0], rotation[0][1]);
    result.cameraMatrix80092880.words[1] =
        PackSigned16Pair(rotation[0][2], rotation[1][0]);
    result.cameraMatrix80092880.words[2] =
        PackSigned16Pair(rotation[1][1], rotation[1][2]);
    result.cameraMatrix80092880.words[3] =
        PackSigned16Pair(rotation[2][0], rotation[2][1]);
    result.cameraMatrix80092880.words[4] =
        PackSigned16Pair(rotation[2][2], 0);
    for (std::size_t lane = 0u; lane < 3u; ++lane) {
        result.cameraMatrix80092880.words[5u + lane] =
            static_cast<uint32_t>(translation[lane]);
    }
    result.cameraMatrixKnown80092880 = true;
    return result;
}

TitleCameraAdvanceResult801C6410 AdvanceTitleCamera801C6410(
    const TitleCameraCursorState801CB59C& state,
    const TitleCameraTable801C6FB4* table)
{
    if (!state.cursorKnown) {
        return FailTitleCameraAdvance(
            Failure::TitleCameraCursorUnknown, state);
    }
    if (table == nullptr || !table->known) {
        return FailTitleCameraAdvance(
            Failure::TitleCameraSourceUnknown, state);
    }
    if (state.cursor > table->views.size()) {
        return FailTitleCameraAdvance(
            Failure::TitleCameraCursorOutOfRange, state);
    }
    if (state.cursor == table->views.size()) {
        TitleCameraAdvanceResult801C6410 result{};
        result.sourceExhausted = true;
        result.nextState = state;
        return result;
    }

    const TitleCameraView801C6FB4& view =
        table->views[state.cursor];
    const TitleCameraMatrixResult80041D3C matrix =
        BuildTitleCameraMatrix80041D3C(view);
    if (!matrix.cameraMatrixKnown80092880) {
        return FailTitleCameraAdvance(matrix.failure, state);
    }

    TitleCameraAdvanceResult801C6410 result{};
    result.accepted = true;
    result.sampledIndex = state.cursor;
    result.nextState = state;
    ++result.nextState.cursor;
    result.cameraMatrixKnown80092880 = true;
    result.cameraMatrix80092880 = matrix.cameraMatrix80092880;
    return result;
}

bool ExtractPaLoc2Coord80028054(const TodData& tod,
                               TodCoordMatrix& outCoord)
{
    outCoord = {};
    if (tod.rawBytes.size() != kPaLoc2RawBytes ||
        tod.rawBlockCount != kPaLoc2RawBlockCount ||
        tod.blockCount != kPaLoc2RawBlockCount || tod.blocks.size() != 1u) {
        return false;
    }
    for (std::size_t index = kPaLoc2PayloadBytes;
         index < tod.rawBytes.size(); ++index) {
        if (tod.rawBytes[index] != 0u) {
            return false;
        }
    }

    const TodBlock& block = tod.blocks[0];
    if (block.rawOffset != 8u || block.rawSize != kPaLoc2BlockRawBytes ||
        block.rawOffset + block.rawSize != kPaLoc2PayloadBytes ||
        block.unk0 != kPaLoc2BlockHeader ||
        block.cmdCount != kPaLoc2CommandCount || block.triggerTime != 0u ||
        block.commands.size() != kPaLoc2CommandCount) {
        return false;
    }

    const TodCommand* coordCommand = nullptr;
    for (const TodCommand& command : block.commands) {
        const uint16_t type =
            static_cast<uint16_t>((command.header >> 16u) & 0xFu);
        const uint16_t objectId =
            static_cast<uint16_t>(command.header & 0xFFFFu);
        if (type != 4u) {
            continue;
        }
        if (objectId != 1u) {
            return false;
        }
        if (coordCommand != nullptr ||
            command.header != kPaLoc2CoordCommandHeader ||
            command.data.size() != 8u) {
            return false;
        }
        coordCommand = &command;
    }
    if (coordCommand == nullptr) {
        return false;
    }

    const std::vector<uint32_t>& data = coordCommand->data;
    // Keep the static resource path identical to 80028054's type-4 case:
    // the command's nine halfwords are written to MATRIX by columns
    // (destination halfwords 0/3/6, 1/4/7, 2/5/8), not read as rows.
    outCoord.m[0][0] = static_cast<int16_t>(data[0] & 0xFFFFu);
    outCoord.m[1][0] = static_cast<int16_t>(data[0] >> 16u);
    outCoord.m[2][0] = static_cast<int16_t>(data[1] & 0xFFFFu);
    outCoord.m[0][1] = static_cast<int16_t>(data[1] >> 16u);
    outCoord.m[1][1] = static_cast<int16_t>(data[2] & 0xFFFFu);
    outCoord.m[2][1] = static_cast<int16_t>(data[2] >> 16u);
    outCoord.m[0][2] = static_cast<int16_t>(data[3] & 0xFFFFu);
    outCoord.m[1][2] = static_cast<int16_t>(data[3] >> 16u);
    outCoord.m[2][2] = static_cast<int16_t>(data[4] & 0xFFFFu);
    outCoord.t[0] = static_cast<int32_t>(data[5]);
    outCoord.t[1] = static_cast<int32_t>(data[6]);
    outCoord.t[2] = static_cast<int32_t>(data[7]);

    const TodCoordMatrix expected = [] {
        TodCoordMatrix matrix{};
        matrix.m[0][0] = 4096;
        matrix.m[1][1] = 4096;
        matrix.m[2][2] = 4096;
        matrix.t[2] = 800;
        return matrix;
    }();
    for (std::size_t row = 0; row < 3u; ++row) {
        for (std::size_t col = 0; col < 3u; ++col) {
            if (outCoord.m[row][col] != expected.m[row][col]) {
                outCoord = {};
                return false;
            }
        }
        if (outCoord.t[row] != expected.t[row]) {
            outCoord = {};
            return false;
        }
    }
    return true;
}

TitleTodAdvanceResult8001B000 AdvanceTitleTodDrawCoord8001B000(
    const TitleTodCursorState8001B000& state,
    const TodData* tod)
{
    if (!state.bindingKnown || state.resourceHandle == 0u) {
        return FailTitleTodAdvance(Failure::TitleTodBindingUnknown, state);
    }
    if (!state.sampleSeqKnown) {
        return FailTitleTodAdvance(Failure::TitleTodCursorUnknown, state);
    }
    if (tod == nullptr) {
        return FailTitleTodAdvance(Failure::TitleTodSourceUnknown, state);
    }
    if (!IsStructurallyCompleteTitleTod(*tod)) {
        return FailTitleTodAdvance(Failure::TitleTodSourceIncomplete, state);
    }
    if (state.blockIndex > tod->blocks.size()) {
        return FailTitleTodAdvance(Failure::TitleTodCursorOutOfRange, state);
    }
    if (state.sampleSeq == (std::numeric_limits<uint32_t>::max)()) {
        return FailTitleTodAdvance(Failure::TitleTodCursorOverflow, state);
    }

    TitleTodCursorState8001B000 next = state;
    bool blockAdvanced = false;
    if (next.blockIndex < tod->blocks.size()) {
        const TodBlock& block = tod->blocks[next.blockIndex];
        if (next.sampleSeq >= block.triggerTime) {
            TodCoordMatrix nextCoord = next.drawCoord;
            bool nextCoordKnown = next.drawCoordKnown;
            for (const TodCommand& command : block.commands) {
                const uint16_t type = static_cast<uint16_t>(
                    (command.header >> 16u) & 0xFu);
                if (!IsSupportedTitleTodCommandType(type)) {
                    return FailTitleTodAdvance(
                        Failure::TitleTodCommandUnsupported, state);
                }
                if (type == 4u) {
                    if (!ApplyTitleTodType4Coord80028054(
                            command, nextCoord)) {
                        return FailTitleTodAdvance(
                            Failure::TitleTodCommandMalformed, state);
                    }
                    nextCoordKnown = true;
                }
            }
            next.drawCoord = nextCoord;
            next.drawCoordKnown = nextCoordKnown;
            ++next.blockIndex;
            blockAdvanced = true;
        }
    }
    ++next.sampleSeq;

    TitleTodAdvanceResult8001B000 result{};
    result.accepted = true;
    result.blockAdvanced = blockAdvanced;
    result.sourceExhausted = next.blockIndex == tod->blocks.size();
    result.nextState = next;
    result.drawCoordWorldKnown800417A4 = next.drawCoordKnown;
    if (next.drawCoordKnown) {
        result.drawCoordWorld800417A4 =
            PackTitleTodCoord800417A4(next.drawCoord);
    }
    return result;
}

TitlePanelCoordResult801C5D8C BuildTitlePanelCoord801C5D8C(
    const TitlePanelCoordInput801C5D8C& input)
{
    if (!input.known) {
        return FailTitlePanelCoord(Failure::TitlePanelCoordInputUnknown);
    }
    if (input.totalFrames <= 0) {
        return FailTitlePanelCoord(
            Failure::TitlePanelCoordFrameCountInvalid);
    }

    const int64_t xShear =
        (-4096ll * static_cast<int64_t>(input.deltaX)) /
        input.totalFrames;
    const int64_t yShear =
        (-4096ll * static_cast<int64_t>(input.deltaY)) /
        input.totalFrames;
    if (!IsSigned16(xShear) || !IsSigned16(yShear)) {
        return FailTitlePanelCoord(Failure::TitlePanelCoordOverflow);
    }

    TitlePanelCoordResult801C5D8C result{};
    result.panelCoord801CD7BC.words[0] =
        PackSigned16Pair(4096, static_cast<int32_t>(xShear));
    result.panelCoord801CD7BC.words[1] = PackSigned16Pair(0, 0);
    result.panelCoord801CD7BC.words[2] = PackSigned16Pair(0, 0);
    result.panelCoord801CD7BC.words[3] =
        PackSigned16Pair(0, static_cast<int32_t>(yShear));
    result.panelCoord801CD7BC.words[4] = PackSigned16Pair(4096, 0);
    result.panelCoordKnown801CD7BC = true;
    return result;
}

TitleCoordWithCameraResult80041A68 BuildTitleCoordWithCamera80041A68(
    const TitleCoordWithCameraInput80041A68& input)
{
    if (!input.drawCoordWorldKnown800417A4) {
        return FailTitleCoordWithCamera(Failure::DrawCoordWorldUnknown);
    }
    if (!input.cameraMatrixKnown80092880) {
        return FailTitleCoordWithCamera(Failure::TitleCameraMatrixUnknown);
    }

    TitleCoordWithCameraResult80041A68 result{};
    if (!ComposePackedAffineMatrix(input.cameraMatrix80092880,
                                   input.drawCoordWorld800417A4,
                                   result.coordWithCamera80041A68)) {
        return FailTitleCoordWithCamera(Failure::RuntimeMatrixOverflow);
    }
    result.coordWithCameraKnown80041A68 = true;
    return result;
}

TitlePanelWithCameraResult80041A68 BuildTitlePanelWithCamera80041A68(
    const TitlePanelWithCameraInput80041A68& input)
{
    if (!input.panelCoordKnown801CD7BC) {
        return FailTitlePanelWithCamera(Failure::TitlePanelCoordUnknown);
    }

    TitleCoordWithCameraInput80041A68 coordInput{};
    coordInput.drawCoordWorldKnown800417A4 = true;
    coordInput.drawCoordWorld800417A4 = input.panelCoord801CD7BC;
    coordInput.cameraMatrixKnown80092880 = input.cameraMatrixKnown80092880;
    coordInput.cameraMatrix80092880 = input.cameraMatrix80092880;
    const TitleCoordWithCameraResult80041A68 coordResult =
        BuildTitleCoordWithCamera80041A68(coordInput);
    if (!coordResult.coordWithCameraKnown80041A68) {
        return FailTitlePanelWithCamera(coordResult.failure);
    }

    TitlePanelWithCameraResult80041A68 result{};
    result.panelWithCamera80041A68 = coordResult.coordWithCamera80041A68;
    result.panelWithCameraKnown80041A68 = true;
    return result;
}

BaseCarrierResult BuildMode25BaseTriangleCarrier801C5E60(
    const PrPsxGteDirect::GteControlState* control,
    const TmdObject* object,
    uint32_t objectIndex,
    std::size_t primitiveIndex,
    bool sourceParseComplete,
    ModelPath path)
{
    if (control == nullptr) {
        return FailBase(Failure::GteControlUnknown);
    }
    if (!IsExactTitleGteControl801C609C(*control)) {
        return FailBase(Failure::GteControlMismatch);
    }
    if (object == nullptr) {
        return FailBase(Failure::NullObject);
    }
    if (!sourceParseComplete) {
        return FailBase(Failure::SourceParseIncomplete);
    }
    if (primitiveIndex >= object->primitives.size()) {
        return FailBase(Failure::PrimitiveIndexOutOfRange);
    }
    if (object->vertices.size() >
        (std::numeric_limits<uint32_t>::max)()) {
        return FailBase(Failure::VdfShapeMismatch);
    }

    const TmdPrimitive& primitive = object->primitives[primitiveIndex];
    if (!primitive.rawPacketKnown) {
        return FailBase(Failure::RawPacketUnknown);
    }
    if (!HasSupportedMode25Shape(primitive)) {
        return FailBase(Failure::UnsupportedPrimitiveShape);
    }

    const std::array<uint16_t, 3> indices = {{
        primitive.v0_idx,
        primitive.v1_idx,
        primitive.v2_idx,
    }};
    for (uint16_t index : indices) {
        if (index >= object->vertices.size()) {
            return FailBase(Failure::VertexIndexOutOfRange);
        }
    }

    if (!IsSupportedModelPath(path)) {
        return FailBase(Failure::UnsupportedModelPath);
    }

    BaseCarrierResult result{};
    result.carrier.known = true;
    result.carrier.control = *control;
    result.carrier.vertexIndices = indices;
    result.carrier.objectIndex = objectIndex;
    result.carrier.objectVertexCount =
        static_cast<uint32_t>(object->vertices.size());
    result.carrier.primitiveIndex = primitiveIndex;
    result.carrier.path = path;
    for (std::size_t index = 0; index < indices.size(); ++index) {
        result.carrier.baseVertices[index] =
            ToGteVertex(object->vertices[indices[index]]);
    }
    result.carrier.runtimeMimeBindingKnown = false;
    result.carrier.runtimeMimeCursorKnown = false;
    result.carrier.runtimeVertexDeformationKnown = false;
    result.carrier.runtimeMatrixKnown = false;
    result.carrier.rtpt3InputKnown = false;
    result.baseCarrierReady = true;
    return result;
}

DeformationResult ApplyMode25Mime80013EA8(
    const Mode25BaseTriangleCarrier& base,
    const MimeRuntimeInput80013EA8& runtime)
{
    if (!base.known) {
        return FailDeformation(Failure::BaseTriangleUnknown);
    }
    if (!IsSupportedModelPath(base.path)) {
        return FailDeformation(Failure::UnsupportedModelPath);
    }
    if (!runtime.bindingKnown) {
        return FailDeformation(Failure::RuntimeMimeBindingUnknown);
    }
    if (runtime.vdf == nullptr) {
        return FailDeformation(Failure::VdfSourceUnknown);
    }
    if (runtime.dat == nullptr) {
        return FailDeformation(Failure::DatSourceUnknown);
    }
    if (!runtime.sampleFrameKnown) {
        return FailDeformation(Failure::RuntimeMimeCursorUnknown);
    }
    if (!runtime.loopFlagKnown) {
        return FailDeformation(Failure::RuntimeMimeLoopFlagUnknown);
    }
    if (runtime.loopFlag > 1u) {
        return FailDeformation(Failure::UnsupportedMimeLoopFlag);
    }

    const VdfData& vdf = *runtime.vdf;
    const DatData& dat = *runtime.dat;
    if (vdf.keys != dat.keys) {
        return FailDeformation(Failure::MimeKeyCountMismatch);
    }
    if (vdf.keys == 0u || vdf.keyList.size() != vdf.keys) {
        return FailDeformation(Failure::VdfShapeMismatch);
    }
    if (dat.keys == 0u || dat.keyList.size() != dat.keys) {
        return FailDeformation(Failure::DatShapeMismatch);
    }

    for (const VdfKey& key : vdf.keyList) {
        if (key.deltas.size() != key.nVert) {
            return FailDeformation(Failure::VdfShapeMismatch);
        }
    }
    uint16_t maxFrames = 0u;
    for (const DatKey& key : dat.keyList) {
        if (key.frames == 0u || key.influence.size() != key.frames) {
            return FailDeformation(Failure::DatShapeMismatch);
        }
        maxFrames = (std::max)(maxFrames, key.frames);
    }
    if (dat.maxFrames != maxFrames) {
        return FailDeformation(Failure::DatShapeMismatch);
    }
    if (runtime.objectIndex != base.objectIndex) {
        return FailDeformation(Failure::MimeObjectSourceMissing);
    }

    uint16_t sampleFrame = runtime.sampleFrame;
    if (runtime.loopFlag == 1u) {
        sampleFrame = static_cast<uint16_t>(sampleFrame % maxFrames);
    }

    Mode25DeformedTriangleCarrier carrier{};
    carrier.known = true;
    carrier.control = base.control;
    carrier.vertices = base.baseVertices;
    carrier.vertexIndices = base.vertexIndices;
    carrier.primitiveIndex = base.primitiveIndex;
    carrier.path = base.path;
    carrier.sampleFrame = sampleFrame;

    bool objectSourceFound = false;
    for (std::size_t keyIndex = 0u;
         keyIndex < vdf.keyList.size(); ++keyIndex) {
        const VdfKey& vdfKey = vdf.keyList[keyIndex];
        if (vdfKey.obj != runtime.objectIndex) {
            continue;
        }
        objectSourceFound = true;

        const uint64_t rangeEnd =
            static_cast<uint64_t>(vdfKey.vertTop) + vdfKey.nVert;
        if (rangeEnd > base.objectVertexCount) {
            return FailDeformation(Failure::VdfShapeMismatch);
        }

        const DatKey& datKey = dat.keyList[keyIndex];
        const uint16_t keySample = sampleFrame < datKey.frames
                                       ? sampleFrame
                                       : static_cast<uint16_t>(datKey.frames - 1u);
        const int16_t weight = datKey.influence[keySample];
        for (std::size_t lane = 0u;
             lane < carrier.vertexIndices.size(); ++lane) {
            const uint32_t vertexIndex = carrier.vertexIndices[lane];
            if (vertexIndex < vdfKey.vertTop || vertexIndex >= rangeEnd) {
                continue;
            }

            const VdfVertexDelta& delta =
                vdfKey.deltas[vertexIndex - vdfKey.vertTop];
            PrPsxGteDirect::VertexS16& vertex = carrier.vertices[lane];
            vertex.x = AddS16Wrap(
                vertex.x,
                ArithmeticShiftRight12(
                    static_cast<int64_t>(delta.x) * weight));
            vertex.y = AddS16Wrap(
                vertex.y,
                ArithmeticShiftRight12(
                    static_cast<int64_t>(delta.y) * weight));
            vertex.z = AddS16Wrap(
                vertex.z,
                ArithmeticShiftRight12(
                    static_cast<int64_t>(delta.z) * weight));
        }
    }
    if (!objectSourceFound) {
        return FailDeformation(Failure::MimeObjectSourceMissing);
    }

    DeformationResult result{};
    result.carrier = carrier;
    result.deformedTriangleReady = true;
    return result;
}

TitleRuntimeMatrixResult801C5E60 BuildTitleRuntimeMatrix801C5E60(
    const TitleRuntimeMatrixInput801C5E60& input)
{
    if (!input.drawCoordWorldKnown800417A4) {
        return FailRuntimeMatrix(Failure::DrawCoordWorldUnknown);
    }
    if (!input.panelWithCameraKnown80041A68) {
        return FailRuntimeMatrix(Failure::PanelWithCameraUnknown);
    }

    TitleRuntimeMatrixResult801C5E60 result{};
    if (!ComposePackedAffineMatrix(input.panelWithCamera80041A68,
                                   input.drawCoordWorld800417A4,
                                   result.runtimeMatrix800406D8)) {
        return FailRuntimeMatrix(Failure::RuntimeMatrixOverflow);
    }
    result.runtimeMatrixKnown = true;
    return result;
}

Rtpt3InputResult BuildMode25Rtpt3Input801C5E60(
    const Mode25DeformedTriangleCarrier& deformed,
    const PrPsxGteDirect::Matrix3x4* runtimeMatrix,
    bool runtimeMatrixKnown)
{
    if (!deformed.known) {
        return FailRtpt3(Failure::DeformedTriangleUnknown);
    }
    if (!IsSupportedModelPath(deformed.path)) {
        return FailRtpt3(Failure::UnsupportedModelPath);
    }
    if (!runtimeMatrixKnown || runtimeMatrix == nullptr) {
        return FailRtpt3(Failure::RuntimeMatrixUnknown);
    }

    Rtpt3InputResult result{};
    result.input.known = true;
    result.input.matrix = *runtimeMatrix;
    result.input.control = deformed.control;
    result.input.vertices = deformed.vertices;
    result.input.vertexIndices = deformed.vertexIndices;
    result.input.primitiveIndex = deformed.primitiveIndex;
    result.input.path = deformed.path;
    result.input.sampleFrame = deformed.sampleFrame;
    result.ready = true;
    return result;
}

Rtpt3ExecutionResult800428B0 ExecuteMode25Rtpt3At800428B0(
    const Mode25Rtpt3Input& input)
{
    if (!input.known) {
        return FailRtpt3Execution(Failure::DeformedTriangleUnknown);
    }
    if (!IsSupportedModelPath(input.path)) {
        return FailRtpt3Execution(Failure::UnsupportedModelPath);
    }

    PrPsxGteDirect::Rtpt3ExactInput280030 exact{};
    exact.matrixKnown = true;
    exact.matrix = input.matrix;
    exact.controlKnown = true;
    exact.control = input.control;
    exact.verticesKnown = true;
    exact.vertices = input.vertices;
    const PrPsxGteDirect::Rtpt3ExactOutput280030 output =
        PrPsxGteDirect::ExecuteRtpt3Exact280030(exact);
    const bool complete =
        output.known && output.flagAfterRtptKnown && output.ir0Known &&
        output.sxy[0].known && output.sxy[1].known && output.sxy[2].known &&
        !output.szAfterRtpt.elementKnown[0] &&
        output.szAfterRtpt.elementKnown[1] &&
        output.szAfterRtpt.elementKnown[2] &&
        output.szAfterRtpt.elementKnown[3];
    if (!complete) {
        return FailRtpt3Execution(Failure::GteControlMismatch);
    }

    PrPsxGteDirect::Mode25TriangleGeometryInput geometryInput{};
    geometryInput.sxy = output.sxy;
    geometryInput.szAfterRtpt = output.szAfterRtpt;
    geometryInput.zScaleFactor3Known = input.control.zScaleFactorKnown;
    geometryInput.zScaleFactor3 = input.control.zScaleFactor3;
    PrPsxGteDirect::Mode25TriangleGeometryTrace geometry =
        PrPsxGteDirect::TraceMode25TriangleGeometry(geometryInput);
    const bool geometryComplete =
        geometry.projectedSxyKnown && geometry.nclip.known &&
        geometry.visibilityKnown && geometry.avsz3.known && geometry.otzKnown;
    if (!geometryComplete) {
        return FailRtpt3Execution(Failure::GteControlUnknown);
    }
    if (static_cast<int32_t>(output.flagAfterRtpt) < 0) {
        geometry.visible = false;
    }

    Rtpt3ExecutionResult800428B0 result{};
    result.input = input;
    result.output = output;
    result.geometry = geometry;
    result.geometryReady = true;
    result.ready = true;
    return result;
}

} // namespace PrSS0TitleTransformDirect
