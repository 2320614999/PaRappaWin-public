#include "pr_stage_scene_submit_draw_private.h"
#include "pr_stage_scene_submit_runtime_private.h"
#include "pr_mime.h"
#include "pr_psx_event_frame_direct.h"
#include "pr_psx_graph_owner_direct.h"
#include "pr_scn1.h"
#include "pr_stage1_compact_rail_80024744_direct.h"
#include "pr_stage1_hud_presentation_direct.h"
#include "pr_stage1_scene1_movie1_direct.h"
#include "pr_stage1_movie_text_direct.h"
#include "pr_stage1_script_box_direct.h"
#include "pr_stage1_camera_motion_direct.h"
#include "pr_stage1_tod_cursor_direct.h"
#include "pr_tmd.h"

#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <cmath>
#include <map>
#include <utility>
#include <vector>

namespace PrStageSceneSubmitDirect {

constexpr std::size_t kStage1PsxWorkListCount801CBFDC = 7;
constexpr uint32_t kFn8003EE84_GsSortBoxFill = 0x8003EE84u;
constexpr uint32_t kFn8001B718_BoxFillSubmitCallsite = 0x8001B718u;
constexpr uint32_t kFn8003EF5C_LinkPacket = 0x8003EF5Cu;
constexpr uint32_t kPsxPacketAddrMask8003EF5C = 0x00FFFFFFu;
constexpr uint32_t kBoxFillPacketPayloadWords8003EE84 = 4u;
constexpr uint32_t kBoxFillPacketTotalWords8003EE84 =
    kBoxFillPacketPayloadWords8003EE84 + 1u;
constexpr uint32_t kBoxFillPacketAdvanceBytes8003EE84 =
    kBoxFillPacketTotalWords8003EE84 * 4u;
using PsxViewMatrixRows428B0 = std::array<int32_t, 12>;

struct PsxProjectionParams428B0 {
    int32_t projectionDistance = 256;
    int32_t screenCenterX = 0;
    int32_t screenCenterY = 0;
};

struct PsxCameraState801CBFDC {
    bool valid = false;
    std::array<float, 3> pos{};
    std::array<float, 3> target{};
    int32_t twist = 0;
    int32_t projectionDistance = 440;
    int32_t screenCenterX = 160;
    int32_t screenCenterY = 120;
};

struct PsxBackdropCamera801CAC34 {
    std::array<float, 3> pos{};
    std::array<float, 3> target{};
};

struct PsxTmdVertex428B0 {
    int16_t x = 0;
    int16_t y = 0;
    int16_t z = 0;
};

struct PsxTmdPrimitive428B0 {
    bool rawPacketKnown = false;
    uint32_t rawPrimitiveIndex = 0;
    uint32_t rawPacketOffset = 0;
    uint32_t rawPacketByteSize = 0;
    bool quad = false;
    std::array<uint16_t, 4> vertexIndices{};
    std::array<uint8_t, 4> u{};
    std::array<uint8_t, 4> v{};
    bool textured = false;
    bool semiTransparent = false;
    uint16_t tpage = 0;
    uint16_t clut = 0;
    uint8_t abr = 0;
    uint8_t r = 0x80;
    uint8_t g = 0x80;
    uint8_t b = 0x80;
    std::array<uint32_t, 4> colorWords{};
    uint32_t psxRgbCodeWord = 0;
    uint8_t psxMode = 0;
    uint8_t psxFlag = 0;
    uint8_t psxWordStride8004274C = 0;
    uint16_t psxGroupCount8004274C = 0;
};

struct PsxTmdRawPrimitivePacket428B0 {
    uint32_t rawPrimitiveIndex = 0;
    uint32_t rawPacketOffset = 0;
    uint32_t rawPacketByteSize = 0;
    uint8_t olen = 0;
    uint8_t ilen = 0;
    uint8_t flag = 0;
    uint8_t mode = 0;
    bool parsed = false;
    uint32_t parsedPrimitiveIndex = 0;
};

struct PsxTmdPrimitiveGroup8004274C {
    uint32_t primitiveStartIndex = 0;
    uint16_t primitiveCount = 0;
    uint8_t psxMode = 0;
    uint8_t psxFlag = 0;
    uint8_t psxWordStride = 0;
};

struct PsxTmdObjectView428B0 {
    std::vector<PsxTmdVertex428B0> vertices;
    std::vector<PsxTmdRawPrimitivePacket428B0> rawPrimitivePackets;
    std::vector<PsxTmdPrimitive428B0> primitives;
    std::vector<PsxTmdPrimitiveGroup8004274C> primitiveGroups8004274C;
};

struct PsxTmdModelView428B0 {
    std::vector<PsxTmdObjectView428B0> objects;
};

enum class PsxMimeActor801CB190 : uint8_t {
    Onion,
    Parappa,
};

struct PsxMimeSampleState80090240 {
    bool valid = false;
    uint32_t frame = 0;
};

struct PsxMimeResourceBinding801CBFDC {
    bool valid = false;
    uint8_t datHandle = 0;
    uint8_t vdfHandle = 0;
};

constexpr std::size_t kPsxPrimitiveMaxVertexCount428B0 = 4;
constexpr std::size_t kPsxTriangleVertexCount428B0 = 3;
constexpr std::size_t kPsxQuadTriangleCount428B0 = 2;
constexpr std::array<
    std::array<uint8_t, kPsxTriangleVertexCount428B0>,
    kPsxQuadTriangleCount428B0>
    kPsxTriangleSlots428B0 = {{
        {0u, 1u, 2u},
        {1u, 3u, 2u},
    }};
constexpr std::size_t kPsxViewMatrixRowCount428B0 = 3;
constexpr std::size_t kPsxViewMatrixColumnCount428B0 = 4;
constexpr uint8_t kPsxDrawBlendAlpha428B0 = 0u;
constexpr uint8_t kPsxDrawBlendAdditive428B0 = 1u;
constexpr uint8_t kPsxDrawBlendSubtractive428B0 = 2u;
constexpr int32_t kPsxGteZsf3_80040D6C = 341;
constexpr int32_t kPsxGteZsf4_80040D6C = 256;
constexpr int16_t kStage1InitialCameraBezId801CB6BC = 200;
constexpr int32_t kStage1ProjectionH801CB6BC = 440;
constexpr int32_t kStage1CameraBezPeriod801CBFDC = 11;
constexpr uint32_t kOneShotCtxFlags801CBFDC =
    0x00800000u | // UIList seed block C.
    0x00080000u | // row0 TOD 8001AFD8 reload.
    0x00040000u | // performance TOD 8001AFD8 reload.
    0x00020000u | // ON MIMe 80014050 init.
    0x00010000u | // PA MIMe 800140E0 init.
    0x00002000u | // dynamic scene row 8001AFD8 reload.
    0x00000010u | // PA MIMe ranged 800140E0 init.
    0x00000400u;  // camera BEZ 800127F0 init.

struct PsxFixedMatrix3x480040FA0 {
    int32_t m[3][3] = {};
    int32_t t[3] = {};
};

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

static int32_t PsxSar801CBFDC(int64_t value, int bits) {
    if (bits <= 0) {
        return static_cast<int32_t>(value);
    }
    if (value >= 0) {
        return static_cast<int32_t>(value >> bits);
    }
    const int64_t divisor = 1ll << bits;
    return static_cast<int32_t>(-(((-value) + divisor - 1) >> bits));
}

static int32_t PsxDivTrunc801CBFDC(int64_t numer, int32_t denom) {
    return denom != 0 ? static_cast<int32_t>(numer / denom) : 0;
}

static int PsxLeadingZeroCount80041548(uint32_t value) {
    if (value == 0u) {
        return 32;
    }
    int count = 0;
    for (uint32_t bit = 0x80000000u; (value & bit) == 0u; bit >>= 1) {
        ++count;
    }
    return count;
}

static int32_t PsxCall80041548_SquareRoot0(int64_t value) {
    if (value <= 0) {
        return 0;
    }

    const uint32_t u =
        value > 0x7FFFFFFFll ? 0x7FFFFFFFu : static_cast<uint32_t>(value);
    const int lz = PsxLeadingZeroCount80041548(u);
    if (lz == 32) {
        return 0;
    }

    const int evenLz = lz & ~1;
    const uint32_t tableValue =
        evenLz < 24 ? (u >> (24 - evenLz)) : (u << (evenLz - 24));
    int index = static_cast<int>(tableValue) - 64;
    if (index < 0) {
        index = 0;
    } else if (index >= static_cast<int>(
                            sizeof(kPsxSqrtTable8005C9D4) /
                            sizeof(kPsxSqrtTable8005C9D4[0]))) {
        index = static_cast<int>(
                    sizeof(kPsxSqrtTable8005C9D4) /
                    sizeof(kPsxSqrtTable8005C9D4[0])) -
                1;
    }

    const int shift = (31 - evenLz) >> 1;
    return static_cast<int32_t>(
        (static_cast<int64_t>(kPsxSqrtTable8005C9D4[index]) << shift) >> 12);
}

static int16_t PsxS16AddWrap801CBFDC(int16_t lhs, int32_t rhs) {
    return static_cast<int16_t>(
        static_cast<uint16_t>(lhs) + static_cast<uint16_t>(rhs));
}

static int32_t PsxRoundFloatToI32_80040FA0(float value) {
    return static_cast<int32_t>(std::lround(value));
}

static int PsxCall80041464_MaxAbsComponent(
    const std::array<int32_t, 6>& values) {
    int32_t maxAbs = 0;
    for (int32_t value : values) {
        const int32_t absValue =
            value == INT32_MIN ? INT32_MAX : (value < 0 ? -value : value);
        if (maxAbs < absValue) {
            maxAbs = absValue;
        }
    }
    return maxAbs;
}

static int PsxCall8004152C_BitCount(int value) {
    int result = 0;
    for (; value > 0; ++result) {
        value >>= 1;
    }
    return result;
}

static void PsxCall80041374_ScaleGsRView2(
    const std::array<int32_t, 8>& src,
    std::array<int32_t, 6>& dst) {
    const std::array<int32_t, 6> firstSix = {{
        src[0],
        src[1],
        src[2],
        src[3],
        src[4],
        src[5],
    }};
    const int v5 =
        PsxCall8004152C_BitCount(PsxCall80041464_MaxAbsComponent(firstSix));
    const int shift = v5 - 15;
    if (v5 < 16) {
        for (size_t i = 0; i < dst.size(); ++i) {
            dst[i] = src[i];
        }
        return;
    }
    for (size_t i = 0; i < dst.size(); ++i) {
        dst[i] = PsxSar801CBFDC(src[i], shift);
    }
}

static void PsxFixedMatrixIdentity800416E0(
    PsxFixedMatrix3x480040FA0& out) {
    out = PsxFixedMatrix3x480040FA0{};
    out.m[0][0] = 4096;
    out.m[1][1] = 4096;
    out.m[2][2] = 4096;
}

static void PsxFixedMatrixMul3x380040FA0(
    const PsxFixedMatrix3x480040FA0& a,
    const PsxFixedMatrix3x480040FA0& b,
    PsxFixedMatrix3x480040FA0& out) {
    PsxFixedMatrix3x480040FA0 result{};
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            result.m[r][c] = PsxSar801CBFDC(
                static_cast<int64_t>(a.m[r][0]) * b.m[0][c] +
                    static_cast<int64_t>(a.m[r][1]) * b.m[1][c] +
                    static_cast<int64_t>(a.m[r][2]) * b.m[2][c],
                12);
        }
    }
    out = result;
}

static void PsxCall800416E0_RotMatrixByAxis(
    PsxFixedMatrix3x480040FA0& out,
    int16_t sinValue,
    int16_t cosValue,
    char axis) {
    PsxFixedMatrixIdentity800416E0(out);
    switch (axis) {
        case 'X':
        case 'x':
            out.m[1][1] = cosValue;
            out.m[1][2] = -sinValue;
            out.m[2][1] = sinValue;
            out.m[2][2] = cosValue;
            break;
        case 'Y':
        case 'y':
            out.m[0][0] = cosValue;
            out.m[0][2] = sinValue;
            out.m[2][0] = -sinValue;
            out.m[2][2] = cosValue;
            break;
        case 'Z':
        case 'z':
            out.m[0][0] = cosValue;
            out.m[0][1] = -sinValue;
            out.m[1][0] = sinValue;
            out.m[1][1] = cosValue;
            break;
        default:
            break;
    }
}

static void PsxCall8003ABA4_ApplyMatrixLv(
    const PsxFixedMatrix3x480040FA0& matrix,
    const std::array<int32_t, 3>& vec,
    std::array<int32_t, 3>& out) {
    for (int r = 0; r < 3; ++r) {
        out[r] = PsxSar801CBFDC(
            static_cast<int64_t>(matrix.m[r][0]) * vec[0] +
                static_cast<int64_t>(matrix.m[r][1]) * vec[1] +
                static_cast<int64_t>(matrix.m[r][2]) * vec[2],
            12);
    }
}

static int Stage1PsxRatan2_8003B70C_801CAC34(int y, int x) {
    if (x == 0 && y == 0) {
        return 0;
    }
    constexpr double kTau = 6.28318530717958647692;
    return static_cast<int>(std::lround(
        std::atan2(static_cast<double>(y), static_cast<double>(x)) *
        (4096.0 / kTau)));
}

enum class PsxWorkListId : uint8_t {
    W7B1C,
    W80A8,
    W8080,
    W8058,
    W526C,
    W3244,
    W5AF4,
};

struct PsxSubmitRange {
    PsxWorkListId work = PsxWorkListId::W7B1C;
    uint8_t depth = 0;
    bool headInsert = false;
    uint32_t psxWorkBaseAddr = 0;
    uint8_t psxWorkSlotIndex8006EDA8 = 0;
    uint32_t psxWorkAddr = 0;
    uint32_t descAddr = 0;
    uint8_t descCount = 0;
};

struct PsxWorkListState801CBFDC {
    PsxWorkListId work = PsxWorkListId::W7B1C;
    uint8_t slotIndex8006EDA8 = 0;
    uint32_t psxWorkBaseAddr = 0;
    uint32_t psxWorkAddr = 0;
    uint32_t otOrder = 14;
    uint32_t otBaseAddr = 0;
    uint16_t x = 0;
    uint16_t y = 0;
    uint32_t otLastAddr = 0;
    bool clearOtagCalled = false;
    uint32_t clearOtagRHeadAddr = 0;
    uint32_t clearOtagRLength = 0;
};

struct PsxDrawDescState {
    uint32_t descAddr = 0;
    uint32_t attr = 0;
    uint32_t coordNodeAddr = 0;
    uint32_t objectDescToken8004274C = 0;
    PsxDrawModelRef modelRef{};
    uint16_t objectIndex8004274C = 0;
    bool coordValid = false;
    TodCoordMatrix coord{};
};

struct PsxCoordNode {
    uint32_t nodeAddr = 0;
    uint32_t header = 0;
    uint32_t parentNodeAddr = 0;
    uint32_t childNodeAddr = 0;
    TodCoordMatrix local{};
    bool coord44BackingSlotKnown80028054 = false;
    uint32_t coord44BackingSlotAddr80028054 = 0;
    bool coord44BackingZeroInit80028054 = false;
    PrStage1TodCursorDirect::TodType1TrsState80028054 type1Trs80028054{};
};

struct Stage1SceneSubmitTodCacheEntry801CBFDC {
    const uint8_t* bytes = nullptr;
    std::size_t byteSize = 0;
    uint32_t psxSourceBaseToken8001AFD8 = 0;
    TodData tod{};
};

using DescBankActiveGroups =
    std::array<bool, PrStageSceneSubmitConstants::kDescBankGroupCount>;
using DescBankGroupFrames =
    std::array<uint32_t, PrStageSceneSubmitConstants::kDescBankGroupCount>;

struct PsxTodCursorSource8001B000 {
    uint32_t cursorAddr = 0;
    uint8_t handle = 0;
    uint32_t startFrame = 0;
    const TodData* tod = nullptr;
    uint32_t psxSourceBaseToken8001AFD8 = 0;
    uint32_t cursorValue8001B000 = 0;
    PrStage1TodCursorDirect::TodCursorRuntime runtime{};
};

struct PsxDrawDescAttrState428B0 {
    uint8_t code = 0;
    uint8_t abr = 0;
    bool semiTransparent = false;
    bool textureRaw = false;
    uint8_t tge = 0;
    bool highPriority = false;
};

enum class PsxGsTmdFastHandler428B0 : uint8_t {
    Unsupported,
    F3NL,
    NF3,
    TF3NL,
    TNF3,
    F4NL,
    NF4,
    TF4NL,
    TNF4,
    G3NL,
    NG3,
    TG3NL,
    TNG3,
    G4NL,
    NG4,
    TG4NL,
    TNG4,
};

constexpr std::size_t kPsxGsTmdFastTableSlotCount8001C1E8 = 64u;
constexpr uint32_t kPsxGsTmdFastTableBase8001C1E8 = 0x8008EDD8u;
constexpr uint32_t kPsxGsTmdFastFlag4TableBase8008EED8 = 0x8008EED8u;
constexpr uint32_t kPsxGsTmdFastFlag4TableBase8008EEE4 = 0x8008EEE4u;

constexpr std::array<
    PsxGsTmdFastHandler428B0,
    kPsxGsTmdFastTableSlotCount8001C1E8>
    kPsxGsTmdFastTable8001C1E8 = {{
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::F3NL,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::NF3,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::G3NL,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::NG3,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::TF3NL,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::TNF3,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::TG3NL,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::TNG3,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::F4NL,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::NF4,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::G4NL,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::NG4,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::TF4NL,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::TNF4,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::TG4NL,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::Unsupported,
        PsxGsTmdFastHandler428B0::TNG4,
        PsxGsTmdFastHandler428B0::Unsupported,
    }};

struct PsxProjectedVertex428B0 {
    int16_t x = 0;
    int16_t y = 0;
    int32_t z = 0;
    float screenX = 0.0f;
    float screenY = 0.0f;
    float screenZ = 1.0f;
};

struct PsxGpuPacket428B0 {
    PsxGsTmdFastHandler428B0 handler =
        PsxGsTmdFastHandler428B0::Unsupported;
    uint8_t primitiveCode = 0;
    uint8_t primitiveFlag = 0;
    bool primitiveFlag4Path = false;
    uint8_t handlerTableSlot8001C1E8 = 0;
    uint32_t handlerTableEntryAddress8001C1E8 = 0;
    uint16_t primitiveCursorAdvanceHalfwords800428B0 = 0;
    uint8_t wordCount = 0;
    std::array<uint32_t, 13> words{};
    uint16_t otz = 0;
};

struct PsxGpuPacketGap428B0 {
    const PsxTmdPrimitive428B0* primitive = nullptr;
    PsxGpuPacketDispatchGap428B0 gap =
        PsxGpuPacketDispatchGap428B0::None;
    uint32_t specialTableBase800428B0 = 0;
};

struct PsxProjectedPrimitiveWorkEntry428B0 {
    const PsxTmdPrimitive428B0* primitive = nullptr;
    PsxGpuPacket428B0 packet{};
    PsxDrawDescAttrState428B0 attr{};
    bool sourceModelRefKnown = false;
    PsxDrawModelRef sourceModelRef{};
    bool projectedDepthKnown = false;
    bool projectedScreenKnown = false;
    uint8_t projectedVertexCount = 0;
    std::array<int32_t, kPsxPrimitiveMaxVertexCount428B0> projectedDepth{};
    std::array<float, kPsxPrimitiveMaxVertexCount428B0> projectedScreenX{};
    std::array<float, kPsxPrimitiveMaxVertexCount428B0> projectedScreenY{};
    std::array<float, kPsxPrimitiveMaxVertexCount428B0> projectedScreenZ{};
};

using PsxOtWorkLists428B0 = std::array<
    std::map<
        int,
        std::vector<PsxProjectedPrimitiveWorkEntry428B0>,
        std::greater<int>>,
    kStage1PsxWorkListCount801CBFDC>;

static uint8_t PsxPrimitiveWordStride8004274C(uint8_t mode, uint8_t flag);

static void FillPacketCommandPrimitiveMetadata428B0(
    PsxGpuPacketCommand428B0& command,
    const PsxTmdPrimitive428B0& primitive) {
    command.primitiveCode = static_cast<uint8_t>(primitive.psxMode & 0xFDu);
    command.primitiveFlag = primitive.psxFlag;
    command.primitiveFlag4Path = (primitive.psxFlag & 4u) != 0u;
    command.rawPacketKnown800428B0 = primitive.rawPacketKnown;
    command.rawPrimitiveIndex800428B0 = primitive.rawPrimitiveIndex;
    command.rawPacketOffset800428B0 = primitive.rawPacketOffset;
    command.rawPacketByteSize800428B0 = primitive.rawPacketByteSize;
    command.primitiveCursorAdvanceHalfwords800428B0 =
        static_cast<uint16_t>(
            PsxPrimitiveWordStride8004274C(primitive.psxMode,
                                           primitive.psxFlag) *
            2u);
}

static PsxGpuPacketHandler428B0 ToPublicGpuPacketHandler428B0(
    PsxGsTmdFastHandler428B0 handler) {
    switch (handler) {
        case PsxGsTmdFastHandler428B0::F3NL:
            return PsxGpuPacketHandler428B0::F3NL;
        case PsxGsTmdFastHandler428B0::NF3:
            return PsxGpuPacketHandler428B0::NF3;
        case PsxGsTmdFastHandler428B0::TF3NL:
            return PsxGpuPacketHandler428B0::TF3NL;
        case PsxGsTmdFastHandler428B0::TNF3:
            return PsxGpuPacketHandler428B0::TNF3;
        case PsxGsTmdFastHandler428B0::F4NL:
            return PsxGpuPacketHandler428B0::F4NL;
        case PsxGsTmdFastHandler428B0::NF4:
            return PsxGpuPacketHandler428B0::NF4;
        case PsxGsTmdFastHandler428B0::TF4NL:
            return PsxGpuPacketHandler428B0::TF4NL;
        case PsxGsTmdFastHandler428B0::TNF4:
            return PsxGpuPacketHandler428B0::TNF4;
        case PsxGsTmdFastHandler428B0::G3NL:
            return PsxGpuPacketHandler428B0::G3NL;
        case PsxGsTmdFastHandler428B0::NG3:
            return PsxGpuPacketHandler428B0::NG3;
        case PsxGsTmdFastHandler428B0::TG3NL:
            return PsxGpuPacketHandler428B0::TG3NL;
        case PsxGsTmdFastHandler428B0::TNG3:
            return PsxGpuPacketHandler428B0::TNG3;
        case PsxGsTmdFastHandler428B0::G4NL:
            return PsxGpuPacketHandler428B0::G4NL;
        case PsxGsTmdFastHandler428B0::NG4:
            return PsxGpuPacketHandler428B0::NG4;
        case PsxGsTmdFastHandler428B0::TG4NL:
            return PsxGpuPacketHandler428B0::TG4NL;
        case PsxGsTmdFastHandler428B0::TNG4:
            return PsxGpuPacketHandler428B0::TNG4;
        case PsxGsTmdFastHandler428B0::Unsupported:
        default:
            return PsxGpuPacketHandler428B0::Unsupported;
    }
}

static PsxGsTmdFastHandler428B0 ToInternalGpuPacketHandler428B0(
    PsxGpuPacketHandler428B0 handler) {
    switch (handler) {
        case PsxGpuPacketHandler428B0::F3NL:
            return PsxGsTmdFastHandler428B0::F3NL;
        case PsxGpuPacketHandler428B0::NF3:
            return PsxGsTmdFastHandler428B0::NF3;
        case PsxGpuPacketHandler428B0::TF3NL:
            return PsxGsTmdFastHandler428B0::TF3NL;
        case PsxGpuPacketHandler428B0::TNF3:
            return PsxGsTmdFastHandler428B0::TNF3;
        case PsxGpuPacketHandler428B0::F4NL:
            return PsxGsTmdFastHandler428B0::F4NL;
        case PsxGpuPacketHandler428B0::NF4:
            return PsxGsTmdFastHandler428B0::NF4;
        case PsxGpuPacketHandler428B0::TF4NL:
            return PsxGsTmdFastHandler428B0::TF4NL;
        case PsxGpuPacketHandler428B0::TNF4:
            return PsxGsTmdFastHandler428B0::TNF4;
        case PsxGpuPacketHandler428B0::G3NL:
            return PsxGsTmdFastHandler428B0::G3NL;
        case PsxGpuPacketHandler428B0::NG3:
            return PsxGsTmdFastHandler428B0::NG3;
        case PsxGpuPacketHandler428B0::TG3NL:
            return PsxGsTmdFastHandler428B0::TG3NL;
        case PsxGpuPacketHandler428B0::TNG3:
            return PsxGsTmdFastHandler428B0::TNG3;
        case PsxGpuPacketHandler428B0::G4NL:
            return PsxGsTmdFastHandler428B0::G4NL;
        case PsxGpuPacketHandler428B0::NG4:
            return PsxGsTmdFastHandler428B0::NG4;
        case PsxGpuPacketHandler428B0::TG4NL:
            return PsxGsTmdFastHandler428B0::TG4NL;
        case PsxGpuPacketHandler428B0::TNG4:
            return PsxGsTmdFastHandler428B0::TNG4;
        case PsxGpuPacketHandler428B0::Unsupported:
        default:
            return PsxGsTmdFastHandler428B0::Unsupported;
    }
}

struct PsxMatrix3x4Fixed428B0 {
    int32_t m[3][3] = {};
    int32_t t[3] = {};
};

struct PsxMatrix3x4Float428B0 {
    float m[3][3] = {};
    float t[3] = {};
};

struct PsxPrStageRunnerCtx801CBFDC {
    bool valid = false;
    uint32_t flags_00 = 0;
    int modeArg = 0;

    int16_t uiRenderBlock_4E = 0;
    uint16_t tailGate54 = 0;
    uint16_t tailGate64 = 0;
    int16_t tailArg68 = 0;
    int16_t ctx72_72 = 0;
    int16_t uiListSeedKind_7C = 0;

    std::array<std::array<uint16_t, 4>, 3> shortLists_AC{};
    uint32_t paDatHandle_DC = 0;
    uint32_t onDatHandle_E0 = 0;
    uint32_t paVdfHandle_E8 = 0;
    uint32_t onVdfHandle_EC = 0;
    uint32_t listSlotF4 = 0;
    uint32_t listSlotF8 = 0;
    uint32_t tamaSelector_FC = 0;
    uint32_t cameraBezHandle104 = 0;
    uint32_t cameraSourceEventPsxAddr = 0;
    const char* tailTextPtr10C = nullptr;
    int16_t todStart_11C = 0;
    int16_t todEnd_120 = 0;
    std::array<uint32_t, PrStageSceneSubmitConstants::kDynamicRowSlotCount>
        uiListHandles_134{};

    uint32_t queryFrame = 0;
    uint32_t uiListHandles134StartFrame = 0;

    bool uiListHandles134Valid = false;
    uint32_t listSlotF4StartFrame = 0;
    uint32_t listSlotF8StartFrame = 0;

    bool uiListSeedSourcesValid = false;
    DescBankActiveGroups uiListSeedActiveGroups{};
    DescBankGroupFrames uiListSeedStartFrames{};

    bool compactInputCaptured80024744 = false;
    bool compactResetWobbleBanks80024744 = false;
    bool compactTeacherResetPulse80024308 = false;
    bool compactStudentResetPulse80024390 = false;
    int16_t compactPainterGate7A = 0;
    int16_t compactRowCount8A = 0;
    std::array<PsxCompactRailRow80024744, kCompactRailMaxRows80024744>
        compactRows80024744{};
};

struct Stage1SceneSubmitRuntime::Impl {
    std::vector<PsxDrawDescState> localDescStates;
    std::vector<PsxCoordNode> coordNodes;
    std::array<Stage1SceneSubmitTodCacheEntry801CBFDC, 256> todCacheByHandle{};
    bool advanceFrameValid = false;
    uint32_t lastAdvanceQueryFrame = 0;
    PsxViewMatrixRows428B0 viewRows{};
    PsxProjectionParams428B0 projection{};
    PsxBackdropCamera801CAC34 backdropCamera{};
    PrStage1CameraMotionDirect::CameraBezPlayer cameraBezPlayer801CBFDC{};
    PsxCameraState801CBFDC cameraAuthorityState801CBFDC{};
    PsxCameraState801CBFDC cameraPresentationState801CBFDC{};
    int16_t cameraBezId801CBFDC = 0;
    uint32_t cameraSourceEventPsxAddr801CBFDC = 0;
    bool cameraUsingNeutral801CBFDC = true;
    bool cameraAdvanceFrameValid801CBFDC = false;
    uint32_t lastCameraQueryFrame801CBFDC = 0;
    PsxBackdropGradientCommand801CAC34 backdropGradient801CAC34{};
    std::vector<PsxBackdropSpriteCommand801CAC34> backdropSprites801CAC34;
    PsxDrawEnvClearColorCommand80040060 drawEnvClearColor80040060{};
    bool backdropExtra4CBC = false;
    uint8_t psxWorkSlotIndex8006EDA8 = 0;
    uint32_t currentPacketAllocator800901C8 = 0;
    PrPsxGraphOwnerDirect::PsxGraphState graphOwner801CBFDC{};
    PrPsxGraphOwnerDirect::PsxGraphFlipResult80040370
        lastGraphFlip80040370{};
    std::array<
        PsxWorkListState801CBFDC,
        kStage1PsxWorkListCount801CBFDC * 2>
        psxWorkLists80040CC8{};
    std::vector<PsxTimUploadCommand801CBFDC> timUploadCommands801CBFDC;
    std::vector<PsxScriptBoxCommand80023E10> scriptBoxCommands80023E10;
    std::vector<PsxNumericStatusCommand8001E2E4>
        numericStatusCommands8001E2E4;
    std::vector<PsxCompactRailCommand80024744>
        compactRailCommands80024744;
    std::vector<PsxCompactRailSpriteCommand80024744>
        compactRailSpriteCommands80024744;
    PsxTodSideEffectStats80028054 todSideEffectStats80028054{};
    PrStage1CompactRail80024744Direct::PsxCompactRailRuntime80024744
        compactRailRuntime80024744{};
    PsxPreventTransitionBoundary801CBFDC preventTransition801CBFDC{};
    PrStage1Scene1Movie1Direct::Movie1TransitionTileMaskRuntimeState
        preventTransitionTileMask801CBFDC{};
    uint32_t preventTransitionGp196801CBFDC = 0;
    bool preventTransitionTileMaskInitialized801CBFDC = false;
    PsxPreventTransitionSetup801CBFDC preventTransitionTileMaskSetup801CBFDC =
        PsxPreventTransitionSetup801CBFDC::None;
    bool coordBankInitialized801CB6BC = false;
    bool descStaticBindingsInitialized801CB6BC = false;
    uint32_t nextPsxTodSourceBaseToken8001AFD8 = 0x60000000u;
    std::vector<PsxTodCursorSource8001B000> psxTodCursorSources8001B000;
    std::vector<PsxSubmitRange> submitRanges801CAC34;
    std::vector<PsxSubmitRange> submitRanges801CB190;
    std::array<PsxMimeSampleState80090240, 2> mimeSampleState80090240{};
    PsxMimeResourceBinding801CBFDC onMimeBinding801CBFDC{};
    PsxMimeResourceBinding801CBFDC paMimeBinding801CBFDC{};
    bool primaryMimeCursorValid800141D8 = false;
    int32_t primaryMimeCursor20 = 0;
    int32_t primaryMimeEnd24 = 0;
    bool oneShotPulseConsumed801CBFDCValid = false;
    uint32_t oneShotPulseConsumed801CBFDCQueryFrame = 0;
    uint32_t oneShotPulseConsumed801CBFDCFlags = 0;
    struct PsxStageRunnerGlobals801CBFDC {
        uint32_t dword_801D7F40 = 0;
        uint32_t dword_801D7F44 = 0;
        uint32_t dword_801D7F48 = 0;
        uint32_t dword_801D7F4C = 0;
        uint32_t dword_801D7F50 = 0;
        int32_t g_PrStageRunner_UIList_InitCounter = 0;
        bool g_PrStageRunner_UIList_Block = false;
        bool g_PrStageRunner_UIRender_Block = false;
        bool g_PrStageRunner_UIRender_Block2 = false;
        bool g_PrStageRunner_UIRender_RefreshRequest = false;
        bool g_PrStageRunner_UIListA_Active = false;
        bool g_PrStageRunner_UIListB_Active = false;
        bool g_PrStageRunner_UIListC_Active = false;
        bool g_PrStageRunner_UIListD_Active = false;
        int32_t g_PrStageRunner_UIListA_ItemCount = 0;
        int32_t g_PrStageRunner_UIListB_ItemCount = 0;
        int32_t g_PrStageRunner_UIListC_ItemCount = 0;
        int32_t g_PrStageRunner_UIListD_ItemCount = 0;
        uint32_t g_PrStageRunner_UIListA_Seq = 0;
        uint32_t g_PrStageRunner_UIListB_Seq = 0;
        uint32_t g_PrStageRunner_UIListC_Seq = 0;
        uint32_t g_PrStageRunner_UIListD_Seq = 0;
        uint32_t dword_801D3014 = 0;
        uint32_t dword_801D3018 = 0;
        uint32_t dword_801D301C = 0;
        uint32_t dword_801D3020 = 0;
        uint32_t dword_801D3024 = 0;
        uint32_t dword_801D302C = 0;
        uint32_t dword_801D3034 = 0;
        uint32_t dword_801D3028 = 0;
        int32_t dword_801DB2D4 = 0;
        int32_t dword_801DB338 = 0;
        uint32_t dword_801D7D48 = 0;
        uint32_t dword_801D7D4C = 0;
        uint32_t dword_801D7D50 = 0;
        uint32_t dword_801D7D54 = 0;
        uint32_t dword_801D7D58 = 0;
        uint32_t dword_801D7D5C = 0;
        uint32_t dword_801D7D60 = 0;
        uint32_t dword_801D7D64 = 0;
        uint32_t dword_801D7D68 = 0;
        uint32_t dword_801D7D6C = 0;
        int32_t dword_801D7D74 = 0;
        int32_t dword_801D7D78 = 0;
        int32_t dword_801D7D80 = 0;
        int32_t dword_801D7D84 = 0;
        int32_t dword_801D7D88 = 0;
        int32_t dword_801D7D8C = 0;
        int32_t dword_801D7D90 = 0;
        int32_t dword_801D7D94 = 0;
        uint32_t dword_801D7E38 = 0;
        uint32_t dword_801D7E3C = 0;
        uint32_t dword_801D7E40 = 0;
        uint32_t dword_801D7E44 = 0;
        int32_t dword_801D7E4C = 0;
        uint32_t dword_801D5AE4 = 0;
        uint32_t dword_801D5AE8 = 0;
        int32_t dword_801D5AF0 = 0;
    } psx801CBFDC{};
};

Stage1SceneSubmitRuntime::Stage1SceneSubmitRuntime()
    : impl_(std::make_unique<Impl>()) {
    PrPsxGraphOwnerDirect::PsxInitializeGraphState8003FB9C(
        impl_->graphOwner801CBFDC,
        320u,
        240u);
    PrPsxGraphOwnerDirect::PsxCall80040B84_ApplyScreenCenterAndDrawOffset(
        impl_->graphOwner801CBFDC);
}

Stage1SceneSubmitRuntime::~Stage1SceneSubmitRuntime() = default;

Stage1SceneSubmitRuntime::Stage1SceneSubmitRuntime(
    Stage1SceneSubmitRuntime&&) noexcept = default;

Stage1SceneSubmitRuntime& Stage1SceneSubmitRuntime::operator=(
    Stage1SceneSubmitRuntime&&) noexcept = default;

Stage1SceneSubmitRuntime::Stage1SceneSubmitRuntime(
    const Stage1SceneSubmitRuntime& other)
    : impl_(other.impl_ ? std::make_unique<Impl>(*other.impl_)
                        : std::make_unique<Impl>()) {}

Stage1SceneSubmitRuntime& Stage1SceneSubmitRuntime::operator=(
    const Stage1SceneSubmitRuntime& other) {
    if (this != &other) {
        if (other.impl_) {
            if (!impl_) {
                impl_ = std::make_unique<Impl>();
            }
            *impl_ = *other.impl_;
        } else {
            impl_ = std::make_unique<Impl>();
        }
    }
    return *this;
}

const Stage1SceneSubmitRuntime::Impl&
Stage1SceneSubmitRuntimePrivateAccess::ConstImpl(
    const Stage1SceneSubmitRuntime& runtime) {
    return *runtime.impl_;
}

Stage1SceneSubmitRuntime::Impl&
Stage1SceneSubmitRuntimePrivateAccess::MutableImpl(
    Stage1SceneSubmitRuntime& runtime) {
    return *runtime.impl_;
}

static Stage1SceneSubmitRuntime&
OwnedStage1SceneSubmitRuntime801CBFDC190() {
    static Stage1SceneSubmitRuntime runtime;
    return runtime;
}

namespace {

using Work = PsxWorkListId;

static uint32_t PsxWorkBaseAddr8001B084(PsxWorkListId work) {
    switch (work) {
        case PsxWorkListId::W7B1C:
            return 0x801D7B1Cu;
        case PsxWorkListId::W80A8:
            return 0x801D80A8u;
        case PsxWorkListId::W8080:
            return 0x801D8080u;
        case PsxWorkListId::W8058:
            return 0x801D8058u;
        case PsxWorkListId::W526C:
            return 0x801D526Cu;
        case PsxWorkListId::W3244:
            return 0x801D3244u;
        case PsxWorkListId::W5AF4:
            return 0x801D5AF4u;
        default:
            return 0u;
    }
}

static size_t PsxWorkListOrdinal80040CC8(PsxWorkListId work) {
    switch (work) {
        case PsxWorkListId::W7B1C:
            return 0u;
        case PsxWorkListId::W3244:
            return 1u;
        case PsxWorkListId::W8058:
            return 2u;
        case PsxWorkListId::W8080:
            return 3u;
        case PsxWorkListId::W80A8:
            return 4u;
        case PsxWorkListId::W526C:
            return 5u;
        case PsxWorkListId::W5AF4:
            return 6u;
        default:
            return 0u;
    }
}

static uint32_t PsxWorkListOtBaseAddr80040CC8(PsxWorkListId work,
                                              uint8_t slotIndex8006EDA8) {
    const uint32_t workBaseAddr = PsxWorkBaseAddr8001B084(work);
    if (workBaseAddr == 0u) {
        return 0u;
    }
    return workBaseAddr + 0x1000u +
           static_cast<uint32_t>(slotIndex8006EDA8) * (1u << 14u) * 4u;
}

static uint32_t PsxPacketAllocatorBase8006ED50(uint8_t slotIndex8006EDA8) {
    switch (slotIndex8006EDA8 & 1u) {
        case 0u:
            return 0x801A73B0u;
        case 1u:
            return 0x801B54B0u;
        default:
            return 0u;
    }
}

static PsxWorkListState801CBFDC& PsxWorkListState80040CC8(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    PsxWorkListId work) {
    const size_t ordinal = PsxWorkListOrdinal80040CC8(work);
    const size_t slot =
        static_cast<size_t>(runtimeState.psxWorkSlotIndex8006EDA8 & 1u);
    return runtimeState
        .psxWorkLists80040CC8[ordinal * 2u + slot];
}

static void PsxClearWorkList80040CC8(PsxWorkListState801CBFDC& state,
                                      uint16_t x,
                                      uint16_t y) {
    PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8 work{};
    work.order_00 = state.otOrder;
    work.headAddr_04 = state.otBaseAddr;
    work.x_08 = state.x;
    work.y_0C = state.y;
    work.lastAddr_10 = state.otLastAddr;
    PrPsxGraphOwnerDirect::PsxCall80040CC8_ClearWorkList(work, x, y);
    state.x = static_cast<uint16_t>(work.x_08);
    state.y = static_cast<uint16_t>(work.y_0C);
    state.otLastAddr = work.lastAddr_10;
    state.clearOtagCalled = work.clearOtagRCalled;
    state.clearOtagRHeadAddr = work.clearOtagRHeadAddr;
    state.clearOtagRLength = work.clearOtagRLength;
}

static PsxSubmitRange MakePsxSubmitRange8001B084(
    PsxWorkListId work,
    uint8_t depth,
    bool headInsert,
    uint32_t descAddr,
    uint8_t descCount,
    uint8_t workSlotIndex8006EDA8) {
    PsxSubmitRange range{};
    range.work = work;
    range.depth = depth;
    range.headInsert = headInsert;
    range.psxWorkBaseAddr = PsxWorkBaseAddr8001B084(work);
    range.psxWorkSlotIndex8006EDA8 = workSlotIndex8006EDA8;
    range.psxWorkAddr =
        range.psxWorkBaseAddr +
        static_cast<uint32_t>(workSlotIndex8006EDA8) * 20u;
    range.descAddr = descAddr;
    range.descCount = descCount;
    return range;
}

static uint8_t PsxCall8004019C_801CBFDC(
    const Stage1SceneSubmitRuntime::Impl& runtimeState) {
    return static_cast<uint8_t>(
        PrPsxGraphOwnerDirect::PsxCall8004019C_GetDrawBuffer(
            runtimeState.graphOwner801CBFDC) &
        1u);
}

static void PsxCall80040C74_801CB6BC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t geomScreen) {
    PrPsxGraphOwnerDirect::PsxCall80040C74_GsSetProjection(
        runtimeState.graphOwner801CBFDC,
        geomScreen);
}

static bool PsxWorkListFlushOrder8001B084(const PsxSubmitRange& range,
                                          uint8_t& outFlushOrder) {
    const uint32_t slotOffset =
        static_cast<uint32_t>(range.psxWorkSlotIndex8006EDA8) * 20u;
    if (range.psxWorkAddr < slotOffset) {
        outFlushOrder = 0u;
        return false;
    }
    const uint32_t psxWorkBaseAddr = range.psxWorkAddr - slotOffset;
    if (range.psxWorkBaseAddr != 0u &&
        range.psxWorkBaseAddr != psxWorkBaseAddr) {
        outFlushOrder = 0u;
        return false;
    }
    switch (psxWorkBaseAddr) {
        case 0x801D7B1Cu:
            outFlushOrder = 0u;
            return true;
        case 0x801D80A8u:
            outFlushOrder = 1u;
            return true;
        case 0x801D8080u:
            outFlushOrder = 2u;
            return true;
        case 0x801D8058u:
            outFlushOrder = 3u;
            return true;
        case 0x801D526Cu:
            outFlushOrder = 4u;
            return true;
        case 0x801D3244u:
            outFlushOrder = 5u;
            return true;
        case 0x801D5AF4u:
            outFlushOrder = 6u;
            return true;
        default:
            outFlushOrder = 0u;
            return false;
    }
}

using DescBankSourceHandleTable801CBFDC = std::array<
    std::array<uint8_t, PrStageSceneSubmitConstants::kDescBankLifecycleSourceCount>,
    PrStageSceneSubmitConstants::kDescBankGroupCount>;
using DescBankSourceAddrTable801CBFDC = std::array<
    std::array<uint32_t, PrStageSceneSubmitConstants::kDescBankLifecycleSourceCount>,
    PrStageSceneSubmitConstants::kDescBankGroupCount>;

constexpr DescBankSourceHandleTable801CBFDC kStage1DescBankLifecycleHandles801CBFDC = {{
    {{0x0Bu, 0x00u}},  // selector 1/C: NIN_TTM.TOD count path
    {{0x0Au, 0x00u}},  // selector 2/D: NIN_TEN.TOD count path
    {{0x05u, 0x06u}},  // selector 3/B: MADO_L/R shared count path
    {{0x07u, 0x08u}},  // selector 4/A: NIN_L/R shared count path
}};

constexpr DescBankSourceHandleTable801CBFDC kStage1DescBankVisibleHandles801CBFDC = {{
    {{0x16u, 0x00u}},  // selector 1/C: TATAMI visible submit
    {{0x00u, 0x00u}},  // selector 2/D: 7CF8 is the count path and submit desc
    {{0x09u, 0x00u}},  // selector 3/B: NIN_SHJ visible submit
    {{0x07u, 0x08u}},  // selector 4/A: NIN_L/R visible submit
}};

constexpr uint32_t kDescNinTen7CF8 = 0x801D7CF8u;
constexpr uint32_t kDescNinTtm7D08 = 0x801D7D08u;
constexpr uint32_t kDescNinShj7D18 = 0x801D7D18u;
constexpr uint32_t kDescNinL7D28 = 0x801D7D28u;
constexpr uint32_t kDescNinR7D38 = 0x801D7D38u;
constexpr uint32_t kDescWakuL7EF0 = 0x801D7EF0u;
constexpr uint32_t kDescWakuR7F00 = 0x801D7F00u;
constexpr uint32_t kDescDondeL7F10 = 0x801D7F10u;
constexpr uint32_t kDescDondeR7F20 = 0x801D7F20u;
constexpr uint32_t kDescTourou7F68 = 0x801D7F68u;
constexpr uint32_t kDescTennjou7F78 = 0x801D7F78u;
constexpr uint32_t kDescKabeR7F88 = 0x801D7F88u;
constexpr uint32_t kDescKabeL7F98 = 0x801D7F98u;
constexpr uint32_t kDescKabeB7FA8 = 0x801D7FA8u;
constexpr uint32_t kDescKamonL7FB8 = 0x801D7FB8u;
constexpr uint32_t kDescKamonR7FC8 = 0x801D7FC8u;
constexpr uint32_t kDescMadoL7FD8 = 0x801D7FD8u;
constexpr uint32_t kDescMadoR7FE8 = 0x801D7FE8u;
constexpr uint32_t kDescAux7FF8 = 0x801D7FF8u;
constexpr uint32_t kDescAux8008 = 0x801D8008u;
constexpr uint32_t kDescAux8018 = 0x801D8018u;
constexpr uint32_t kDescFloor8038 = 0x801D8038u;
constexpr uint32_t kDescKage8028 = 0x801D8028u;
constexpr uint32_t kDescTama98D0 = 0x801D98D0u;
constexpr uint32_t kDescOn98F0 = 0x801D98F0u;
constexpr uint32_t kDescPaB2D8 = 0x801DB2D8u;
constexpr uint32_t kCoordKabeB31F4 = 0x801D31F4u;
constexpr uint32_t kCoordMadoL7D98 = 0x801D7D98u;
constexpr uint32_t kCoordMadoR7DE8 = 0x801D7DE8u;
constexpr uint32_t kCoordKamonLeft7E50 = 0x801D7E50u;
constexpr uint32_t kCoordKamonRight7EA0 = 0x801D7EA0u;
constexpr uint32_t kCoordNinL7C44 = 0x801D7C44u;
constexpr uint32_t kCoordNinR7C94 = 0x801D7C94u;
constexpr uint32_t kGlobalUiListEnable7F40 = 0x801D7F40u;
constexpr uint32_t kGlobalUiListEnable7F44 = 0x801D7F44u;
constexpr uint32_t kGlobalUiListEnable7F48 = 0x801D7F48u;
constexpr uint32_t kGlobalUiListEnable7F4C = 0x801D7F4Cu;
constexpr uint32_t kGlobalUiListEnable7F50 = 0x801D7F50u;
constexpr uint32_t kGlobalUiListInitCounter7F64 = 0x801D7F64u;
constexpr uint32_t kGlobalUiListDSource7D48 = 0x801D7D48u;
constexpr uint32_t kGlobalUiListDItemCount7D70 = 0x801D7D70u;
constexpr uint32_t kGlobalUiListCSource7D4C = 0x801D7D4Cu;
constexpr uint32_t kGlobalUiListCCount7D74 = 0x801D7D74u;
constexpr uint32_t kGlobalUiListCVisibleSource5AE4 = 0x801D5AE4u;
constexpr uint32_t kGlobalUiListCItemCount5AEC = 0x801D5AECu;
constexpr uint32_t kGlobalUiListCVisibleCount7D88 = 0x801D7D88u;
constexpr uint32_t kGlobalUiListBSourceMadoL7E38 = 0x801D7E38u;
constexpr uint32_t kGlobalUiListBSourceMadoR7E3C = 0x801D7E3Cu;
constexpr uint32_t kGlobalUiListBItemCount7E48 = 0x801D7E48u;
constexpr uint32_t kGlobalUiListBItemCount7E4C = 0x801D7E4Cu;
constexpr uint32_t kGlobalUiListBVisibleSource7D50 = 0x801D7D50u;
constexpr uint32_t kGlobalUiListBVisibleCount7D78 = 0x801D7D78u;
constexpr uint32_t kGlobalUiListASourceKamonL7D54 = 0x801D7D54u;
constexpr uint32_t kGlobalUiListASourceKamonR7D58 = 0x801D7D58u;
constexpr uint32_t kGlobalUiListAItemCount7D7C = 0x801D7D7Cu;
constexpr uint32_t kGlobalUiListAVisibleCount7D80 = 0x801D7D80u;
constexpr uint32_t kGlobalOnCursor301C = 0x801D301Cu;
constexpr uint32_t kGlobalOnCountB2D4 = 0x801DB2D4u;
constexpr uint32_t kGlobalPaCursor3024 = 0x801D3024u;
constexpr uint32_t kGlobalPaCountB338 = 0x801DB338u;
constexpr uint32_t kGlobalOnMimeTimelineB0C4 = 0x801DB0C4u;
constexpr uint8_t kStage1InitialOnTodHandle801CB6BC = 12u;
constexpr uint8_t kStage1InitialPaTodHandle801CB6BC = 18u;
constexpr uint8_t kStage1InitialOnMimeVdfHandle801CB6BC = 54u;
constexpr uint8_t kStage1InitialOnMimeDatHandle801CB6BC = 106u;
constexpr uint8_t kStage1InitialPaMimeVdfHandle801CB6BC = 98u;
constexpr uint8_t kStage1InitialPaMimeDatHandle801CB6BC = 150u;
constexpr std::array<uint16_t, 21> kStage1InitialFaceTimHandles801D2FC0 = {{
    220u, 223u, 225u, 227u, 231u, 235u, 237u,
    239u, 241u, 243u, 245u, 247u, 252u, 260u,
    268u, 271u, 274u, 277u, 282u, 284u, 280u,
}};
constexpr std::array<uint32_t, PrStageSceneSubmitConstants::kDynamicRowSlotCount>
    kDynamicRowCursorAddrs801CBFDC = {{
    kGlobalUiListEnable7F40,
    kGlobalUiListEnable7F44,
    kGlobalUiListEnable7F48,
    kGlobalUiListEnable7F4C,
    kGlobalUiListEnable7F50,
}};
constexpr DescBankSourceAddrTable801CBFDC
    kStage1DescBankLifecycleCursorAddrs801CBFDC = {{
    {{kGlobalUiListCSource7D4C, 0u}},
    {{kGlobalUiListDSource7D48, 0u}},
    {{kGlobalUiListBSourceMadoL7E38, kGlobalUiListBSourceMadoR7E3C}},
    {{kGlobalUiListASourceKamonL7D54, kGlobalUiListASourceKamonR7D58}},
}};
constexpr DescBankSourceAddrTable801CBFDC
    kStage1DescBankVisibleCursorAddrs801CBFDC = {{
    {{kGlobalUiListCVisibleSource5AE4, 0u}},
    {{0u, 0u}},
    {{kGlobalUiListBVisibleSource7D50, 0u}},
    {{kGlobalUiListASourceKamonL7D54, kGlobalUiListASourceKamonR7D58}},
}};
using ModelSet = PsxDrawModelRef::Set;
constexpr uint16_t kStage1OnionModelIndex801CB190 = 3u;
constexpr uint16_t kStage1ParappaModelIndex801CB190 = 4u;

struct Stage1DrawDescInit801CB6BC {
    uint32_t resourceSlotAddr = 0;
    uint32_t descAddr = 0;
    uint32_t coordNodeAddr = 0;
    ModelSet modelSet = ModelSet::Stage1Main;
    uint16_t modelIndex = 0;
    uint16_t objectCount8001AF1C = 1;
};

constexpr std::size_t kStage1DrawDescInitCount801CB6BC = 26u;

constexpr std::array<
    Stage1DrawDescInit801CB6BC,
    kStage1DrawDescInitCount801CB6BC>
    kStage1DrawDescInits801CB6BC = {{
    {0x80091900u, kDescOn98F0, 0x801D9900u, ModelSet::Stage1Main, 3u},
    {0x80091904u, kDescPaB2D8, 0x801DB2E8u, ModelSet::Stage1Main, 4u},
    {0x80091908u, kDescTama98D0, 0x801D3064u, ModelSet::Stage1Main, 2u, 2u},
    {0x80091914u, kDescTourou7F68, 0x801D30B4u, ModelSet::Stage1SceneMap, 7u},
    {0x800918CCu, kDescKabeB7FA8, 0x801D31F4u, ModelSet::Stage1SceneMap, 11u},
    {0x800918DCu, kDescKamonL7FB8, 0x801D7E50u, ModelSet::Stage1SceneMap, 5u},
    {0x800918E0u, kDescKamonR7FC8, 0x801D7EA0u, ModelSet::Stage1SceneMap, 6u},
    {0x800918E4u, kDescMadoL7FD8, 0x801D7D98u, ModelSet::Stage1SceneMap, 3u},
    {0x800918E8u, kDescMadoR7FE8, 0x801D7DE8u, ModelSet::Stage1SceneMap, 4u},
    {0x800918C8u, kDescAux7FF8, 0x801D3064u, ModelSet::Stage1SceneMap, 2u},
    {0x800918C4u, kDescAux8018, 0x801D3064u, ModelSet::Stage1SceneMap, 1u},
    {0x80091910u, kDescTennjou7F78, 0x801D3104u, ModelSet::Stage1SceneMap, 8u},
    {0x800918D4u, kDescKabeR7F88, 0x801D3154u, ModelSet::Stage1SceneMap, 9u},
    {0x800918D0u, kDescKabeL7F98, 0x801D31A4u, ModelSet::Stage1SceneMap, 10u},
    {0x80091920u, kDescAux8008, 0x801D3064u, ModelSet::Stage1SceneMap, 0u},
    {0x800918D8u, kDescKage8028, 0x801D3064u, ModelSet::Stage1Main, 1u},
    {0x800918F8u, kDescNinTen7CF8, 0x801D7B54u, ModelSet::Stage1SceneMap, 12u},
    {0x8009190Cu, kDescFloor8038, 0x801D5A94u, ModelSet::Stage1Main, 0u},
    {0x800918FCu, kDescNinTtm7D08, 0x801D7BA4u, ModelSet::Stage1SceneMap, 13u},
    {0x800918F4u, kDescNinShj7D18, 0x801D7BF4u, ModelSet::Stage1SceneMap, 14u},
    {0x800918ECu, kDescNinL7D28, 0x801D7C44u, ModelSet::Stage1SceneMap, 19u},
    {0x800918F0u, kDescNinR7D38, 0x801D7C94u, ModelSet::Stage1SceneMap, 20u},
    {0x80091918u, kDescWakuL7EF0, 0x801D7E50u, ModelSet::Stage1SceneMap, 15u},
    {0x8009191Cu, kDescWakuR7F00, 0x801D7EA0u, ModelSet::Stage1SceneMap, 16u},
    {0x800918BCu, kDescDondeL7F10, 0x801D7E50u, ModelSet::Stage1SceneMap, 17u},
    {0x800918C0u, kDescDondeR7F20, 0x801D7EA0u, ModelSet::Stage1SceneMap, 18u},
}};

TodCoordMatrix MakeIdentityCoordMatrix801CB6BC() {
    TodCoordMatrix coord{};
    coord.m[0][0] = 4096;
    coord.m[1][1] = 4096;
    coord.m[2][2] = 4096;
    return coord;
}

const PsxCoordNode* FindCoordNodePtr801CB6BC(
    const std::vector<PsxCoordNode>& nodes,
    uint32_t nodeAddr) {
    const auto nodeIt =
        std::find_if(nodes.begin(),
                     nodes.end(),
                     [nodeAddr](const PsxCoordNode& candidate) {
                         return candidate.nodeAddr == nodeAddr;
                     });
    return (nodeIt != nodes.end()) ? &*nodeIt : nullptr;
}

PsxDrawDescState& EnsureDescState801CB6BC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t descAddr) {
    for (PsxDrawDescState& state : runtimeState.localDescStates) {
        if (state.descAddr == descAddr) {
            return state;
        }
    }

    PsxDrawDescState next{};
    next.descAddr = descAddr;
    runtimeState.localDescStates.push_back(next);
    return runtimeState.localDescStates.back();
}

PsxCoordNode& EnsureCoordNode801CB6BC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t nodeAddr) {
    for (PsxCoordNode& node : runtimeState.coordNodes) {
        if (node.nodeAddr == nodeAddr) {
            return node;
        }
    }

    PsxCoordNode next{};
    next.nodeAddr = nodeAddr;
    next.local = MakeIdentityCoordMatrix801CB6BC();
    runtimeState.coordNodes.push_back(next);
    return runtimeState.coordNodes.back();
}

PsxCoordNode* FindCoordNode801CB6BC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t nodeAddr) {
    for (PsxCoordNode& node : runtimeState.coordNodes) {
        if (node.nodeAddr == nodeAddr) {
            return &node;
        }
    }
    return nullptr;
}

PrStage1TodCursorDirect::TodType1TrsState80028054*
ResolveDescType1TrsState80028054(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    PsxDrawDescState& descState) {
    if (descState.coordNodeAddr != 0u) {
        PsxCoordNode* node =
            FindCoordNode801CB6BC(runtimeState, descState.coordNodeAddr);
        if (node != nullptr && node->coord44BackingSlotKnown80028054) {
            return &node->type1Trs80028054;
        }
    }
    return nullptr;
}

void SeedTodCursorCoordTarget80028054(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    PsxDrawDescState& descState,
    PrStage1TodCursorDirect::TodCursorRuntime& cursorRuntime) {
    if (descState.coordNodeAddr == 0u) {
        return;
    }

    const PsxCoordNode* node =
        FindCoordNode801CB6BC(runtimeState, descState.coordNodeAddr);
    if (node == nullptr) {
        return;
    }
    cursorRuntime.coord = node->local;
    cursorRuntime.coordValid = true;
}

void PsxCall8004049C_801CB6BC(Stage1SceneSubmitRuntime::Impl& runtimeState,
                              uint32_t parentArg,
                              uint32_t nodeAddr) {
    PsxCoordNode& node = EnsureCoordNode801CB6BC(runtimeState, nodeAddr);
    node.header = 0u;
    node.local = MakeIdentityCoordMatrix801CB6BC();
    node.coord44BackingSlotKnown80028054 = true;
    node.coord44BackingSlotAddr80028054 = nodeAddr + 0x44u;
    node.coord44BackingZeroInit80028054 = true;
    node.parentNodeAddr =
        (parentArg != 0u && parentArg != 1u) ? parentArg : 0u;
    if (node.parentNodeAddr != 0u) {
        PsxCoordNode& parent =
            EnsureCoordNode801CB6BC(runtimeState, node.parentNodeAddr);
        parent.childNodeAddr = nodeAddr;
    }
    for (PsxDrawDescState& state : runtimeState.localDescStates) {
        if (state.coordNodeAddr == nodeAddr) {
            state.coordValid = false;
            state.coord = TodCoordMatrix{};
        }
    }
}

bool ResolveStage1ModelRefFromResourceSlot801CB6BC(
    uint32_t resourceSlotAddr,
    PsxDrawModelRef& outModelRef) {
    for (const Stage1DrawDescInit801CB6BC& init :
         kStage1DrawDescInits801CB6BC) {
        if (init.resourceSlotAddr == resourceSlotAddr) {
            outModelRef.set = init.modelSet;
            outModelRef.index = init.modelIndex;
            return true;
        }
    }
    return false;
}

uint32_t MakeStage1ObjectDescToken8004274C(uint32_t resourceSlotAddr,
                                           uint16_t objectIndex) {
    return resourceSlotAddr + 0x0Cu +
           static_cast<uint32_t>(objectIndex) * 0x1Cu;
}

void PsxCall8001AF1C_801CB6BC(Stage1SceneSubmitRuntime::Impl& runtimeState,
                              uint32_t resourceSlotAddr,
                              uint32_t descAddr,
                              uint32_t coordNodeAddr,
                              uint16_t objectCount8001AF1C) {
    for (uint16_t objectIndex = 0u;
         objectIndex < objectCount8001AF1C;
         ++objectIndex) {
        PsxDrawDescState& state =
            EnsureDescState801CB6BC(
                runtimeState,
                descAddr + static_cast<uint32_t>(objectIndex) * 0x10u);
        state.attr = 0u;
        state.coordNodeAddr = coordNodeAddr;
        state.objectDescToken8004274C =
            MakeStage1ObjectDescToken8004274C(resourceSlotAddr, objectIndex);
        state.objectIndex8004274C = objectIndex;
        state.coordValid = false;
        ResolveStage1ModelRefFromResourceSlot801CB6BC(resourceSlotAddr,
                                                      state.modelRef);
    }
}

void StoreDescCoord8001B000(Stage1SceneSubmitRuntime::Impl& runtimeState,
                            uint32_t descAddr,
                            const TodCoordMatrix& coord) {
    PsxDrawDescState& state =
        EnsureDescState801CB6BC(runtimeState, descAddr);
    state.coordValid = true;
    state.coord = coord;
    if (state.coordNodeAddr != 0u) {
        PsxCoordNode& node =
            EnsureCoordNode801CB6BC(runtimeState, state.coordNodeAddr);
        node.local = coord;
    }
}

void ClearDescCoordNodeHeader80028054(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t descAddr) {
    PsxDrawDescState& state =
        EnsureDescState801CB6BC(runtimeState, descAddr);
    if (state.coordNodeAddr != 0u) {
        PsxCoordNode& node =
            EnsureCoordNode801CB6BC(runtimeState, state.coordNodeAddr);
        node.header = 0u;
    }
}

uint32_t* ResolvePsxCursorCellAddr801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t cursorAddr) {
    auto& g = runtimeState.psx801CBFDC;
    switch (cursorAddr) {
        case kGlobalUiListEnable7F40:
            return &g.dword_801D7F40;
        case kGlobalUiListEnable7F44:
            return &g.dword_801D7F44;
        case kGlobalUiListEnable7F48:
            return &g.dword_801D7F48;
        case kGlobalUiListEnable7F4C:
            return &g.dword_801D7F4C;
        case kGlobalUiListEnable7F50:
            return &g.dword_801D7F50;
        case kGlobalOnCursor301C:
            return &g.dword_801D301C;
        case kGlobalPaCursor3024:
            return &g.dword_801D3024;
        case kGlobalUiListDSource7D48:
            return &g.dword_801D7D48;
        case kGlobalUiListCSource7D4C:
            return &g.dword_801D7D4C;
        case kGlobalUiListBVisibleSource7D50:
            return &g.dword_801D7D50;
        case kGlobalUiListASourceKamonL7D54:
            return &g.dword_801D7D54;
        case kGlobalUiListASourceKamonR7D58:
            return &g.dword_801D7D58;
        case kGlobalUiListBSourceMadoL7E38:
            return &g.dword_801D7E38;
        case kGlobalUiListBSourceMadoR7E3C:
            return &g.dword_801D7E3C;
        case kGlobalUiListCVisibleSource5AE4:
            return &g.dword_801D5AE4;
        default:
            return nullptr;
    }
}

uint32_t LoadPsxCursorCell801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t cursorAddr) {
    uint32_t* cursorCell =
        ResolvePsxCursorCellAddr801CBFDC(runtimeState, cursorAddr);
    return (cursorCell != nullptr) ? *cursorCell : 0u;
}

void StorePsxCursorCell801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t cursorAddr,
    uint32_t cursorValue) {
    uint32_t* cursorCell =
        ResolvePsxCursorCellAddr801CBFDC(runtimeState, cursorAddr);
    if (cursorCell != nullptr) {
        *cursorCell = cursorValue;
    }
}

uint32_t AlignPsxTodSourceByteSize8001AFD8(std::size_t byteSize) {
    const uint32_t size = static_cast<uint32_t>(
        std::min<std::size_t>(byteSize, 0x000FFFFFu));
    return (std::max<uint32_t>(size, 8u) + 0xFFu) & ~0xFFu;
}

uint32_t AllocatePsxTodSourceBaseToken8001AFD8(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    std::size_t byteSize) {
    constexpr uint32_t kSourceBase = 0x60000000u;
    if (runtimeState.nextPsxTodSourceBaseToken8001AFD8 == 0u) {
        runtimeState.nextPsxTodSourceBaseToken8001AFD8 = kSourceBase;
    }
    const uint32_t base =
        runtimeState.nextPsxTodSourceBaseToken8001AFD8;
    runtimeState.nextPsxTodSourceBaseToken8001AFD8 +=
        AlignPsxTodSourceByteSize8001AFD8(byteSize);
    return base;
}

uint32_t ResolvePsxTodSourceBaseToken8001AFD8(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint8_t handle,
    const TodData* tod) {
    if (handle == 0u || tod == nullptr) {
        return 0u;
    }
    const Stage1SceneSubmitTodCacheEntry801CBFDC& cache =
        runtimeState.todCacheByHandle[handle];
    if (&cache.tod == tod && cache.psxSourceBaseToken8001AFD8 != 0u) {
        return cache.psxSourceBaseToken8001AFD8;
    }
    return 0u;
}

uint32_t TodRawEndOffset801CBFDC(const TodData* tod) {
    if (tod == nullptr || tod->blocks.empty()) {
        return 8u;
    }
    const TodBlock& last = tod->blocks.back();
    return last.rawOffset + last.rawSize;
}

uint32_t MakePsxTodCursorValue8001B000(
    const PsxTodCursorSource8001B000& source) {
    if (source.psxSourceBaseToken8001AFD8 == 0u) {
        return 0u;
    }
    if (source.tod == nullptr || source.tod->blocks.empty()) {
        return source.psxSourceBaseToken8001AFD8 + 8u;
    }
    const uint32_t blockIndex = source.runtime.cursorBlockIndex;
    if (blockIndex < source.tod->blocks.size()) {
        return source.psxSourceBaseToken8001AFD8 +
               source.tod->blocks[blockIndex].rawOffset;
    }
    return source.psxSourceBaseToken8001AFD8 +
           TodRawEndOffset801CBFDC(source.tod);
}

bool ResolvePsxTodCursorBlockIndex8001B000(
    const PsxTodCursorSource8001B000& source,
    uint32_t cursorValue,
    uint32_t& outBlockIndex) {
    outBlockIndex = 0;
    if (source.psxSourceBaseToken8001AFD8 == 0u ||
        source.tod == nullptr) {
        return false;
    }
    const uint32_t sourceBase = source.psxSourceBaseToken8001AFD8;
    const uint32_t rawEnd = sourceBase + TodRawEndOffset801CBFDC(source.tod);
    if (cursorValue >= rawEnd) {
        outBlockIndex = static_cast<uint32_t>(source.tod->blocks.size());
        return true;
    }
    for (uint32_t i = 0; i < source.tod->blocks.size(); ++i) {
        if (cursorValue == sourceBase + source.tod->blocks[i].rawOffset) {
            outBlockIndex = i;
            return true;
        }
    }
    return false;
}

bool ResolvePsxTodCursorOffset8001B000(
    const PsxTodCursorSource8001B000& source,
    uint32_t cursorValue,
    uint32_t& outRawOffset) {
    outRawOffset = 0;
    if (source.psxSourceBaseToken8001AFD8 == 0u ||
        source.tod == nullptr ||
        cursorValue < source.psxSourceBaseToken8001AFD8) {
        return false;
    }
    const uint32_t rawOffset =
        cursorValue - source.psxSourceBaseToken8001AFD8;
    if (source.tod->rawBytes.empty() ||
        rawOffset > source.tod->rawBytes.size()) {
        return false;
    }
    outRawOffset = rawOffset;
    return true;
}

PsxTodCursorSource8001B000& EnsurePsxTodCursorSource8001B000(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t cursorAddr) {
    for (PsxTodCursorSource8001B000& source :
         runtimeState.psxTodCursorSources8001B000) {
        if (source.cursorAddr == cursorAddr) {
            return source;
        }
    }

    PsxTodCursorSource8001B000 source{};
    source.cursorAddr = cursorAddr;
    runtimeState.psxTodCursorSources8001B000.push_back(source);
    return runtimeState.psxTodCursorSources8001B000.back();
}

void ClearPsxTodCursorSource8001B000(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t cursorAddr,
    bool clearCursorCell) {
    if (cursorAddr == 0u) {
        return;
    }

    for (PsxTodCursorSource8001B000& source :
         runtimeState.psxTodCursorSources8001B000) {
        if (source.cursorAddr == cursorAddr) {
            source = PsxTodCursorSource8001B000{};
            source.cursorAddr = cursorAddr;
            break;
        }
    }

    if (clearCursorCell) {
        StorePsxCursorCell801CBFDC(runtimeState, cursorAddr, 0u);
    }
}

void ClearStage1SceneInitCursorSources801CB6BC(
    Stage1SceneSubmitRuntime::Impl& runtimeState) {
    ClearPsxTodCursorSource8001B000(
        runtimeState, kGlobalOnCursor301C, false);
    ClearPsxTodCursorSource8001B000(
        runtimeState, kGlobalPaCursor3024, false);
    ClearPsxTodCursorSource8001B000(
        runtimeState, kGlobalUiListDSource7D48, false);
    ClearPsxTodCursorSource8001B000(
        runtimeState, kGlobalUiListCSource7D4C, false);
    ClearPsxTodCursorSource8001B000(
        runtimeState, kGlobalUiListBVisibleSource7D50, false);
    ClearPsxTodCursorSource8001B000(
        runtimeState, kGlobalUiListASourceKamonL7D54, false);
    ClearPsxTodCursorSource8001B000(
        runtimeState, kGlobalUiListASourceKamonR7D58, false);
    ClearPsxTodCursorSource8001B000(
        runtimeState, kGlobalUiListCVisibleSource5AE4, false);
    ClearPsxTodCursorSource8001B000(
        runtimeState, kGlobalUiListBSourceMadoL7E38, false);
    ClearPsxTodCursorSource8001B000(
        runtimeState, kGlobalUiListBSourceMadoR7E3C, false);
    for (uint32_t cursorAddr : kDynamicRowCursorAddrs801CBFDC) {
        ClearPsxTodCursorSource8001B000(runtimeState, cursorAddr, true);
    }
}

bool RegisterPsxTodCursorSource8001B000(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t cursorAddr,
    uint8_t handle,
    uint32_t startFrame,
    const TodData* tod) {
    if (cursorAddr == 0u || handle == 0u || tod == nullptr ||
        tod->blocks.empty()) {
        ClearPsxTodCursorSource8001B000(runtimeState, cursorAddr, true);
        return false;
    }

    PsxTodCursorSource8001B000& source =
        EnsurePsxTodCursorSource8001B000(runtimeState, cursorAddr);
    const bool changed =
        source.handle != handle ||
        source.startFrame != startFrame ||
        source.tod != tod;
    if (source.handle != handle ||
        source.startFrame != startFrame ||
        source.tod != tod) {
        source.runtime = PrStage1TodCursorDirect::TodCursorRuntime{};
        source.handle = handle;
        source.startFrame = startFrame;
        source.tod = tod;
        source.psxSourceBaseToken8001AFD8 =
            ResolvePsxTodSourceBaseToken8001AFD8(runtimeState,
                                                 handle,
                                                 tod);
        source.cursorValue8001B000 =
            MakePsxTodCursorValue8001B000(source);
        StorePsxCursorCell801CBFDC(runtimeState,
                                   cursorAddr,
                                   source.cursorValue8001B000);
    }
    source.handle = handle;
    source.startFrame = startFrame;
    source.tod = tod;
    return changed;
}

PsxTodCursorSource8001B000* FindRegisteredPsxTodCursorSource8001B000(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t cursorAddr) {
    for (PsxTodCursorSource8001B000& source :
         runtimeState.psxTodCursorSources8001B000) {
        if (source.cursorAddr == cursorAddr && source.tod != nullptr) {
            return &source;
        }
    }
    return nullptr;
}

TodCoordMatrix ComposeCoordWorld80041A68(const TodCoordMatrix& parent,
                                         const TodCoordMatrix& local) {
    TodCoordMatrix world{};
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            int64_t value = 0;
            for (int k = 0; k < 3; ++k) {
                value += static_cast<int64_t>(parent.m[r][k]) *
                         static_cast<int64_t>(local.m[k][c]);
            }
            world.m[r][c] = static_cast<int16_t>(
                (std::clamp<int32_t>)(static_cast<int32_t>(value / 4096),
                                      -32768,
                                      32767));
        }
    }
    for (int r = 0; r < 3; ++r) {
        int64_t value = parent.t[r];
        for (int k = 0; k < 3; ++k) {
            value += static_cast<int64_t>(parent.m[r][k]) *
                     static_cast<int64_t>(local.t[k]) / 4096;
        }
        world.t[r] = static_cast<int32_t>(
            (std::clamp<int64_t>)(value, INT32_MIN, INT32_MAX));
    }
    return world;
}

bool PsxCall80041A68_8001B084(const std::vector<PsxCoordNode>& nodes,
                              uint32_t nodeAddr,
                              TodCoordMatrix& outWorld) {
    const PsxCoordNode* node = FindCoordNodePtr801CB6BC(nodes, nodeAddr);
    if (node == nullptr) {
        return false;
    }
    if (node->parentNodeAddr == 0u) {
        outWorld = node->local;
        return true;
    }

    TodCoordMatrix parentWorld{};
    if (!PsxCall80041A68_8001B084(nodes, node->parentNodeAddr, parentWorld)) {
        return false;
    }
    outWorld = ComposeCoordWorld80041A68(parentWorld, node->local);
    return true;
}

PsxMatrix3x4Fixed428B0 PsxCall80040544_8001B084(
    const PsxMatrix3x4Fixed428B0& view,
    const TodCoordMatrix& coordWorld) {
    PsxMatrix3x4Fixed428B0 combined{};
    for (size_t r = 0; r < 3u; ++r) {
        for (size_t c = 0; c < 3u; ++c) {
            combined.m[r][c] = PsxSar801CBFDC(
                static_cast<int64_t>(view.m[r][0]) * coordWorld.m[0][c] +
                    static_cast<int64_t>(view.m[r][1]) *
                        coordWorld.m[1][c] +
                    static_cast<int64_t>(view.m[r][2]) *
                        coordWorld.m[2][c],
                12);
        }
        combined.t[r] = PsxSar801CBFDC(
            static_cast<int64_t>(view.m[r][0]) * coordWorld.t[0] +
                static_cast<int64_t>(view.m[r][1]) * coordWorld.t[1] +
                static_cast<int64_t>(view.m[r][2]) * coordWorld.t[2],
            12) + view.t[r];
    }
    return combined;
}

static void PsxFloatMatrixIdentity428B0(PsxMatrix3x4Float428B0& out) {
    out = PsxMatrix3x4Float428B0{};
    out.m[0][0] = 1.0f;
    out.m[1][1] = 1.0f;
    out.m[2][2] = 1.0f;
}

static void PsxFloatMatrixMul3x3428B0(
    const PsxMatrix3x4Float428B0& a,
    const PsxMatrix3x4Float428B0& b,
    PsxMatrix3x4Float428B0& out) {
    PsxMatrix3x4Float428B0 result{};
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            result.m[r][c] = a.m[r][0] * b.m[0][c] +
                             a.m[r][1] * b.m[1][c] +
                             a.m[r][2] * b.m[2][c];
        }
    }
    out = result;
}

static PsxMatrix3x4Float428B0 PsxFloatRotMatrixByAxis428B0(
    float sinValue,
    float cosValue,
    char axis) {
    PsxMatrix3x4Float428B0 out{};
    PsxFloatMatrixIdentity428B0(out);
    switch (axis) {
        case 'x':
            out.m[1][1] = cosValue;
            out.m[1][2] = -sinValue;
            out.m[2][1] = sinValue;
            out.m[2][2] = cosValue;
            break;
        case 'y':
            out.m[0][0] = cosValue;
            out.m[0][2] = sinValue;
            out.m[2][0] = -sinValue;
            out.m[2][2] = cosValue;
            break;
        case 'z':
            out.m[0][0] = cosValue;
            out.m[0][1] = -sinValue;
            out.m[1][0] = sinValue;
            out.m[1][1] = cosValue;
            break;
        default:
            break;
    }
    return out;
}

static bool BuildPresentationViewMatrix428B0(
    const PsxCameraState801CBFDC& camera,
    PsxMatrix3x4Float428B0& out) {
    if (!camera.valid) {
        return false;
    }

    PsxFloatMatrixIdentity428B0(out);
    const float dx = camera.target[0] - camera.pos[0];
    const float dy = camera.target[1] - camera.pos[1];
    const float dz = camera.target[2] - camera.pos[2];
    const float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (dist <= 0.0001f) {
        return false;
    }

    if (camera.twist != 0) {
        constexpr double kTau = 6.28318530717958647692;
        const int angle = -camera.twist / 360;
        const double rad = static_cast<double>(angle & 0xFFF) *
                           (kTau / 4096.0);
        const PsxMatrix3x4Float428B0 rotZ =
            PsxFloatRotMatrixByAxis428B0(
                static_cast<float>(std::sin(rad)),
                static_cast<float>(std::cos(rad)),
                'z');
        PsxFloatMatrixMul3x3428B0(out, rotZ, out);
    }

    const float horizontal = std::sqrt(dx * dx + dz * dz);
    const PsxMatrix3x4Float428B0 rotX =
        PsxFloatRotMatrixByAxis428B0(
            dy / dist,
            horizontal / dist,
            'x');
    PsxFloatMatrixMul3x3428B0(out, rotX, out);

    if (horizontal > 0.0001f) {
        const PsxMatrix3x4Float428B0 rotY =
            PsxFloatRotMatrixByAxis428B0(
                -dx / horizontal,
                dz / horizontal,
                'y');
        PsxFloatMatrixMul3x3428B0(out, rotY, out);
    }

    for (int r = 0; r < 3; ++r) {
        out.t[r] = -(out.m[r][0] * camera.pos[0] +
                     out.m[r][1] * camera.pos[1] +
                     out.m[r][2] * camera.pos[2]);
    }
    return true;
}

static PsxMatrix3x4Float428B0 PsxCall80040544Presentation428B0(
    const PsxMatrix3x4Float428B0& view,
    const TodCoordMatrix& coordWorld) {
    PsxMatrix3x4Float428B0 combined{};
    for (size_t r = 0; r < 3u; ++r) {
        for (size_t c = 0; c < 3u; ++c) {
            combined.m[r][c] =
                view.m[r][0] *
                    (static_cast<float>(coordWorld.m[0][c]) / 4096.0f) +
                view.m[r][1] *
                    (static_cast<float>(coordWorld.m[1][c]) / 4096.0f) +
                view.m[r][2] *
                    (static_cast<float>(coordWorld.m[2][c]) / 4096.0f);
        }
        combined.t[r] = view.m[r][0] * static_cast<float>(coordWorld.t[0]) +
                        view.m[r][1] * static_cast<float>(coordWorld.t[1]) +
                        view.m[r][2] * static_cast<float>(coordWorld.t[2]) +
                        view.t[r];
    }
    return combined;
}

} // namespace

static void PsxCall8001B084_801CB190(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t descAddr,
    uint8_t descCount,
    Work work,
    uint8_t depth) {
    runtimeState.submitRanges801CB190.push_back(
        MakePsxSubmitRange8001B084(work,
                                   depth,
                                   true,
                                   descAddr,
                                   descCount,
                                   runtimeState.psxWorkSlotIndex8006EDA8));
}

static void PsxCall8001B084_801CAC34(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t descAddr,
    uint8_t descCount,
    Work work,
    uint8_t depth) {
    runtimeState.submitRanges801CAC34.push_back(
        MakePsxSubmitRange8001B084(work,
                                   depth,
                                   true,
                                   descAddr,
                                   descCount,
                                   runtimeState.psxWorkSlotIndex8006EDA8));
}

static void PsxCall80013EA8_801CB190(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t timelineAddr,
    uint32_t frame,
    uint8_t group) {
    (void)timelineAddr;
    if (group < runtimeState.mimeSampleState80090240.size()) {
        runtimeState.mimeSampleState80090240[group] = {true, frame};
    }
}

static void PsxCall800141D8_801CB190(
    Stage1SceneSubmitRuntime::Impl& runtimeState) {
    if (!runtimeState.primaryMimeCursorValid800141D8 ||
        runtimeState.primaryMimeCursor20 >= runtimeState.primaryMimeEnd24) {
        return;
    }

    const int32_t sampleFrame = runtimeState.primaryMimeCursor20;
    runtimeState.primaryMimeCursor20 = sampleFrame + 1;

    runtimeState.mimeSampleState80090240[0] =
        PsxMimeSampleState80090240{true, static_cast<uint32_t>(sampleFrame)};
}

struct PsxDatTimelineState80013D10 {
    const DatData* dat = nullptr;
    uint16_t keyCount_00 = 0;
    uint16_t loopFlag_08 = 0;
    uint16_t maxFrames_0A = 0;
    uint16_t sampleFrame_0C = 0;
};

static int PsxCall80013D10_801CB190(PsxDatTimelineState80013D10& state,
                                    const DatData* dat,
                                    uint16_t loopFlag) {
    state = PsxDatTimelineState80013D10{};
    state.dat = dat;
    if (dat == nullptr) {
        return -1;
    }
    state.keyCount_00 = dat->keys;
    state.loopFlag_08 = loopFlag;
    uint16_t maxFrames = 0;
    for (const DatKey& key : dat->keyList) {
        maxFrames = (std::max)(maxFrames, key.frames);
    }
    state.maxFrames_0A = maxFrames;
    return maxFrames != 0u ? static_cast<int>(state.keyCount_00) : -1;
}

static int PsxCall80013E04_801CB190(PsxDatTimelineState80013D10& state,
                                    uint16_t frame) {
    uint16_t sample = frame;
    if (state.loopFlag_08 == 1u && state.maxFrames_0A != 0u) {
        sample = static_cast<uint16_t>(frame % state.maxFrames_0A);
    }
    state.sampleFrame_0C = sample;
    return frame;
}

static int16_t PsxCall80013DB8_801CB190(
    const PsxDatTimelineState80013D10& state,
    uint16_t keyIndex) {
    if (state.dat == nullptr || keyIndex >= state.dat->keyList.size()) {
        return 0;
    }
    const DatKey& key = state.dat->keyList[keyIndex];
    if (key.influence.empty()) {
        return 0;
    }
    uint16_t sample = state.sampleFrame_0C;
    if (sample >= key.frames) {
        sample = static_cast<uint16_t>(key.frames - 1u);
    }
    if (sample >= key.influence.size()) {
        sample = static_cast<uint16_t>(key.influence.size() - 1u);
    }
    return key.influence[sample];
}

static void PsxGteMIMefunc8003A5DC_801CB190(
    std::vector<TmdVertex>& vertices,
    uint32_t vertexTop,
    const std::vector<VdfVertexDelta>& deltas,
    uint32_t nVert,
    int16_t weight) {
    const uint32_t maxCount =
        (std::min<uint32_t>)(nVert, static_cast<uint32_t>(deltas.size()));
    for (uint32_t i = 0u; i < maxCount; ++i) {
        const uint32_t vertexIndex = vertexTop + i;
        if (vertexIndex >= vertices.size()) {
            break;
        }
        const VdfVertexDelta& delta = deltas[i];
        TmdVertex& vertex = vertices[vertexIndex];
        vertex.x = PsxS16AddWrap801CBFDC(
            vertex.x,
            (static_cast<int32_t>(delta.x) * weight) >> 12);
        vertex.y = PsxS16AddWrap801CBFDC(
            vertex.y,
            (static_cast<int32_t>(delta.y) * weight) >> 12);
        vertex.z = PsxS16AddWrap801CBFDC(
            vertex.z,
            (static_cast<int32_t>(delta.z) * weight) >> 12);
    }
}

static bool PsxCall80013AA8_801CB190(
    const PsxDatTimelineState80013D10& datState,
    const VdfData* vdf,
    uint32_t objectIndex,
    std::vector<TmdVertex>& vertices) {
    if (vdf == nullptr || datState.dat == nullptr) {
        return false;
    }

    const uint32_t keyCount =
        (std::min<uint32_t>)(vdf->keys, datState.keyCount_00);
    bool wrote = false;
    for (uint32_t keyIndex = 0u; keyIndex < keyCount; ++keyIndex) {
        if (keyIndex >= vdf->keyList.size()) {
            break;
        }
        const VdfKey& key = vdf->keyList[keyIndex];
        if (key.obj != objectIndex) {
            continue;
        }
        const int16_t weight =
            PsxCall80013DB8_801CB190(
                datState, static_cast<uint16_t>(keyIndex));
        if (weight == 0) {
            continue;
        }
        PsxGteMIMefunc8003A5DC_801CB190(vertices,
                                        key.vertTop,
                                        key.deltas,
                                        key.nVert,
                                        weight);
        wrote = true;
    }
    return wrote;
}

static int32_t* ResolvePsxCountAddr801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t countAddr) {
    auto& g = runtimeState.psx801CBFDC;
    switch (countAddr) {
        case kGlobalUiListInitCounter7F64:
            return &g.g_PrStageRunner_UIList_InitCounter;
        case kGlobalUiListDItemCount7D70:
            return &g.g_PrStageRunner_UIListD_ItemCount;
        case kGlobalUiListCCount7D74:
            return &g.dword_801D7D74;
        case kGlobalUiListCItemCount5AEC:
            return &g.g_PrStageRunner_UIListC_ItemCount;
        case kGlobalUiListBItemCount7E48:
            return &g.g_PrStageRunner_UIListB_ItemCount;
        case kGlobalUiListBVisibleCount7D78:
            return &g.dword_801D7D78;
        case kGlobalUiListAItemCount7D7C:
            return &g.g_PrStageRunner_UIListA_ItemCount;
        case kGlobalUiListAVisibleCount7D80:
            return &g.dword_801D7D80;
        case kGlobalOnCountB2D4:
            return &g.dword_801DB2D4;
        case kGlobalPaCountB338:
            return &g.dword_801DB338;
        default:
            return nullptr;
    }
}

static void RecordTodSideEffects80028054(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t seq,
    uint32_t countAddr,
    uint32_t cursorAddr,
    uint32_t descAddr,
    const PrStage1TodCursorDirect::TodCommandSideEffects80028054&
        sideEffects) {
    PsxTodSideEffectStats80028054& stats =
        runtimeState.todSideEffectStats80028054;
    ++stats.observedCallCount8001B000;
    stats.lastSeq8001B000 = seq;
    stats.lastCountAddr8001B000 = countAddr;
    stats.lastCursorAddr8001B000 = cursorAddr;
    stats.lastDescAddr8001B000 = descAddr;
    stats.lastCommandCount80028504 = sideEffects.commandCount80028504;
    stats.lastCommandObjectId80028054 =
        sideEffects.lastCommandObjectId80028054;
    stats.lastCommandType80028054 = sideEffects.lastCommandType80028054;
    stats.lastCommandFlags80028054 = sideEffects.lastCommandFlags80028054;
    stats.lastBlockTriggered80028504 =
        sideEffects.blockTriggered80028504;
    stats.lastDescCommandExecuted80028054 =
        sideEffects.descCommandExecuted80028054;
    stats.lastDescHeaderCleared80028054 =
        sideEffects.descHeaderCleared80028054;
    stats.lastDescAttrMerged80028054 =
        sideEffects.descAttrMerged80028054;
    stats.lastCoordWritten80028054 = sideEffects.coordWritten80028054;
    stats.lastType1CoordBackingMissing80028054 =
        sideEffects.type1CoordBackingMissing80028054;
    stats.lastType1CoordWritten80028054 =
        sideEffects.type1CoordWritten80028054;
    stats.lastType4CoordWritten80028054 =
        sideEffects.type4CoordWritten80028054;
}

static int PsxCall8001B000_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t seq,
    int32_t* localCount,
    uint32_t countAddr,
    uint32_t cursorAddr,
    uint32_t descAddr) {
    PsxTodCursorSource8001B000* cursorSource =
        FindRegisteredPsxTodCursorSource8001B000(runtimeState, cursorAddr);
    int32_t* count = localCount;
    if (count == nullptr) {
        count = ResolvePsxCountAddr801CBFDC(runtimeState, countAddr);
    }
    uint32_t* cursorCell =
        ResolvePsxCursorCellAddr801CBFDC(runtimeState, cursorAddr);
    const uint32_t oldCursorValue =
        (cursorCell != nullptr)
            ? *cursorCell
            : ((cursorSource != nullptr)
                   ? cursorSource->cursorValue8001B000
                   : 0u);
    if (count != nullptr &&
        (oldCursorValue == 0u || *count <= 0)) {
        return -1;
    }

    if (cursorSource != nullptr && count != nullptr) {
        uint32_t rawCursorOffset = 0;
        const bool rawCursorOffsetKnown =
            ResolvePsxTodCursorOffset8001B000(*cursorSource,
                                              oldCursorValue,
                                              rawCursorOffset);
        PsxDrawDescState& descState =
            EnsureDescState801CB6BC(runtimeState, descAddr);
        PrStage1TodCursorDirect::TodType1TrsState80028054* type1TrsState =
            ResolveDescType1TrsState80028054(runtimeState, descState);
        SeedTodCursorCoordTarget80028054(runtimeState,
                                         descState,
                                         cursorSource->runtime);
        TodCoordMatrix coord{};
        bool coordValid = false;
        bool cursorChanged = false;
        uint32_t newRawCursorOffset = rawCursorOffset;
        PrStage1TodCursorDirect::TodCommandSideEffects80028054
            sideEffects80028054{};
        PrStage1TodCursorDirect::AdvanceDescCoordOneBlock8001B000(
            cursorSource->runtime,
            *cursorSource->tod,
            cursorSource->handle,
            cursorSource->startFrame,
            seq,
            rawCursorOffsetKnown,
            rawCursorOffset,
            coord,
            coordValid,
            cursorChanged,
            &newRawCursorOffset,
            nullptr,
            &descState.attr,
            type1TrsState,
            nullptr,
            &sideEffects80028054);
        RecordTodSideEffects80028054(runtimeState,
                                     seq,
                                     countAddr,
                                     cursorAddr,
                                     descAddr,
                                     sideEffects80028054);
        uint32_t newCursorValue = oldCursorValue;
        if (cursorChanged) {
            newCursorValue =
                cursorSource->psxSourceBaseToken8001AFD8 + newRawCursorOffset;
        }
        cursorSource->cursorValue8001B000 = newCursorValue;
        StorePsxCursorCell801CBFDC(runtimeState,
                                   cursorAddr,
                                   newCursorValue);
        if (sideEffects80028054.descHeaderCleared80028054) {
            ClearDescCoordNodeHeader80028054(runtimeState, descAddr);
        }
        if (oldCursorValue != newCursorValue) {
            --*count;
        }
        if (sideEffects80028054.coordWritten80028054 && coordValid &&
            descState.coordNodeAddr != 0u) {
            StoreDescCoord8001B000(runtimeState, descAddr, coord);
        }
        return *count;
    }

    (void)descAddr;
    return -1;
}

static int PsxCall8001B000_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t seq,
    uint32_t countAddr,
    uint32_t cursorAddr,
    uint32_t descAddr) {
    return PsxCall8001B000_801CBFDC(runtimeState,
                                    seq,
                                    nullptr,
                                    countAddr,
                                    cursorAddr,
                                    descAddr);
}

static PsxCameraState801CBFDC MakeStage1CameraDefaults801CB6BC() {
    PsxCameraState801CBFDC camera{};
    camera.valid = true;
    camera.pos = {0.0f, -900.0f, -2800.0f};
    camera.target = {0.0f, -700.0f, 0.0f};
    camera.twist = 0;
    camera.projectionDistance = kStage1ProjectionH801CB6BC;
    camera.screenCenterX = 160;
    camera.screenCenterY = 120;
    return camera;
}

static PsxCameraState801CBFDC MakeCameraFromBezPlayer801CBFDC(
    const PrStage1CameraMotionDirect::CameraBezPlayer& player) {
    PsxCameraState801CBFDC camera = MakeStage1CameraDefaults801CB6BC();
    camera.pos[0] = static_cast<float>(player.outPosX);
    camera.pos[1] = static_cast<float>(player.outPosY);
    camera.pos[2] = static_cast<float>(player.outPosZ);
    camera.target[0] = static_cast<float>(player.outTgtX);
    camera.target[1] = static_cast<float>(player.outTgtY);
    camera.target[2] = static_cast<float>(player.outTgtZ);
    return camera;
}

static PsxCameraState801CBFDC MakeCameraFromBezFloatSample801CBFDC(
    const PrStage1CameraMotionDirect::CameraBezFloatSample& sample) {
    PsxCameraState801CBFDC camera = MakeStage1CameraDefaults801CB6BC();
    camera.pos[0] = sample.posX;
    camera.pos[1] = sample.posY;
    camera.pos[2] = sample.posZ;
    camera.target[0] = sample.tgtX;
    camera.target[1] = sample.tgtY;
    camera.target[2] = sample.tgtZ;
    return camera;
}

static PsxCameraState801CBFDC MakeCameraFromBezStartSample801CBFDC(
    const PrStage1CameraMotionDirect::CameraBezPlayer& player) {
    PsxCameraState801CBFDC camera{};
    if (!player.loaded || player.totalEntries <= 0) {
        return camera;
    }

    camera = MakeStage1CameraDefaults801CB6BC();
    camera.pos[0] = static_cast<float>(player.posX[0]);
    camera.pos[1] = static_cast<float>(player.posY[0]);
    camera.pos[2] = static_cast<float>(player.posZ[0]);
    camera.target[0] = static_cast<float>(player.tgtX[0]);
    camera.target[1] = static_cast<float>(player.tgtY[0]);
    camera.target[2] = static_cast<float>(player.tgtZ[0]);
    return camera;
}

static bool PsxCall800127F0_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    int16_t bezId,
    uint32_t sourceEventPsxAddr,
    bool resetLike800128DC,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData) {
    if (resolveResource == nullptr) {
        runtimeState.cameraUsingNeutral801CBFDC = true;
        runtimeState.cameraAuthorityState801CBFDC =
            MakeStage1CameraDefaults801CB6BC();
        runtimeState.cameraPresentationState801CBFDC =
            runtimeState.cameraAuthorityState801CBFDC;
        return false;
    }

    const uint8_t* bytes = nullptr;
    std::size_t byteSize = 0;
    if (bezId <= 0 ||
        !resolveResource(static_cast<uint16_t>(bezId),
                         bytes,
                         byteSize,
                         resourceUserData) ||
        bytes == nullptr ||
        byteSize == 0u) {
        runtimeState.cameraUsingNeutral801CBFDC = true;
        runtimeState.cameraAuthorityState801CBFDC =
            MakeStage1CameraDefaults801CB6BC();
        runtimeState.cameraPresentationState801CBFDC =
            runtimeState.cameraAuthorityState801CBFDC;
        return false;
    }

    if (!runtimeState.cameraBezPlayer801CBFDC.LoadBez(
            bytes,
            byteSize,
            kStage1CameraBezPeriod801CBFDC,
            true)) {
        runtimeState.cameraUsingNeutral801CBFDC = true;
        runtimeState.cameraAuthorityState801CBFDC =
            MakeStage1CameraDefaults801CB6BC();
        runtimeState.cameraPresentationState801CBFDC =
            runtimeState.cameraAuthorityState801CBFDC;
        return false;
    }
    runtimeState.cameraBezPlayer801CBFDC.Start();
    if (resetLike800128DC) {
        runtimeState.cameraBezPlayer801CBFDC.ResetLike800128DC();
    }
    runtimeState.cameraBezId801CBFDC = bezId;
    runtimeState.cameraSourceEventPsxAddr801CBFDC = sourceEventPsxAddr;
    runtimeState.cameraUsingNeutral801CBFDC = false;
    runtimeState.cameraAuthorityState801CBFDC =
        MakeCameraFromBezPlayer801CBFDC(runtimeState.cameraBezPlayer801CBFDC);
    runtimeState.cameraPresentationState801CBFDC =
        runtimeState.cameraAuthorityState801CBFDC;
    return true;
}

static void PsxCall80012960_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    int32_t tick,
    uint8_t renderSubFrame8) {
    if (runtimeState.cameraUsingNeutral801CBFDC ||
        !runtimeState.cameraBezPlayer801CBFDC.loaded) {
        runtimeState.cameraAuthorityState801CBFDC =
            MakeStage1CameraDefaults801CB6BC();
        runtimeState.cameraPresentationState801CBFDC =
            runtimeState.cameraAuthorityState801CBFDC;
        return;
    }

    if (renderSubFrame8 == 0u) {
        runtimeState.cameraBezPlayer801CBFDC.Tick(tick);
        runtimeState.cameraAuthorityState801CBFDC =
            MakeCameraFromBezPlayer801CBFDC(
                runtimeState.cameraBezPlayer801CBFDC);
        runtimeState.cameraPresentationState801CBFDC =
            runtimeState.cameraAuthorityState801CBFDC;
        return;
    }

    PrStage1CameraMotionDirect::CameraBezFloatSample sample{};
    const float renderT =
        std::clamp(static_cast<float>(renderSubFrame8) / 255.0f,
                   0.0f,
                   1.0f);
    if (runtimeState.cameraBezPlayer801CBFDC.SampleFloatAtTick(
            static_cast<float>(tick) + renderT,
            sample)) {
        runtimeState.cameraPresentationState801CBFDC =
            MakeCameraFromBezFloatSample801CBFDC(sample);
        return;
    }
    runtimeState.cameraPresentationState801CBFDC =
        runtimeState.cameraAuthorityState801CBFDC;
}

static int PrStageRunner_UIList_Init_801CA754(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    int a1) {
    auto& g = runtimeState.psx801CBFDC;
    int result = g.dword_801D7F40 != 0u ? 1 : 0;
    const int v2 = a1;
    if (g.dword_801D7F40 != 0u) {
        int v3 = 0;
        if (g.g_PrStageRunner_UIList_InitCounter > 0) {
            constexpr std::array<uint32_t, 4> kCursorAddrs = {{
                kGlobalUiListEnable7F40,
                kGlobalUiListEnable7F44,
                kGlobalUiListEnable7F48,
                kGlobalUiListEnable7F4C,
            }};
            constexpr std::array<uint32_t, 4> kDescAddrs = {{
                kDescTourou7F68,
                kDescTennjou7F78,
                kDescKabeR7F88,
                kDescKabeL7F98,
            }};
            do {
                int32_t v8 = g.g_PrStageRunner_UIList_InitCounter;
                PsxCall8001B000_801CBFDC(runtimeState,
                                         static_cast<uint32_t>(a1),
                                         &v8,
                                         0u,
                                         kCursorAddrs[v3],
                                         kDescAddrs[v3]);
                ++v3;
                a1 = v2;
            } while (v3 < 4);
            return PsxCall8001B000_801CBFDC(
                runtimeState,
                static_cast<uint32_t>(v2),
                kGlobalUiListInitCounter7F64,
                kGlobalUiListEnable7F50,
                kDescKabeB7FA8);
        }

        result = 1;
        g.g_PrStageRunner_UIList_Block = true;
        g.g_PrStageRunner_UIRender_Block2 = true;
    }
    return result;
}

static int PrStageRunner_UIList_UpdateC_801CA838(
    Stage1SceneSubmitRuntime::Impl& runtimeState) {
    auto& g = runtimeState.psx801CBFDC;
    int result = g.g_PrStageRunner_UIListC_ItemCount;
    if (g.g_PrStageRunner_UIListC_ItemCount > 0) {
        if (g.g_PrStageRunner_UIListC_Seq == 16u) {
            int v1 = 0;
            do {
                const uint32_t oldSeq = g.g_PrStageRunner_UIListC_Seq;
                ++v1;
                PsxCall8001B000_801CBFDC(runtimeState,
                                         oldSeq,
                                         kGlobalUiListCCount7D74,
                                         kGlobalUiListCSource7D4C,
                                         kDescNinTtm7D08);
                PsxCall8001B000_801CBFDC(runtimeState,
                                         oldSeq,
                                         kGlobalUiListCItemCount5AEC,
                                         kGlobalUiListCVisibleSource5AE4,
                                         kDescFloor8038);
                g.g_PrStageRunner_UIListC_Seq = oldSeq + 1u;
            } while (v1 < 39);
        }
        const uint32_t oldSeq = g.g_PrStageRunner_UIListC_Seq;
        PsxCall8001B000_801CBFDC(runtimeState,
                                 oldSeq,
                                 kGlobalUiListCCount7D74,
                                 kGlobalUiListCSource7D4C,
                                 kDescNinTtm7D08);
        PsxCall8001B000_801CBFDC(runtimeState,
                                 oldSeq,
                                 kGlobalUiListCItemCount5AEC,
                                 kGlobalUiListCVisibleSource5AE4,
                                 kDescFloor8038);
        g.g_PrStageRunner_UIListC_Seq = oldSeq + 1u;
        PsxCall8001B084_801CB190(
            runtimeState, kDescNinTtm7D08, 1, Work::W3244, 10);
        return 1;
    }

    g.g_PrStageRunner_UIListC_Active = false;
    return result;
}

static int PrStageRunner_UIList_UpdateD_801CA974(
    Stage1SceneSubmitRuntime::Impl& runtimeState) {
    auto& g = runtimeState.psx801CBFDC;
    int result = g.g_PrStageRunner_UIList_Block ? 1 : 0;
    if (!g.g_PrStageRunner_UIList_Block) {
        result = g.g_PrStageRunner_UIListD_ItemCount;
        if (g.g_PrStageRunner_UIListD_ItemCount > 0) {
            const uint32_t oldSeq = g.g_PrStageRunner_UIListD_Seq++;
            PsxCall8001B000_801CBFDC(runtimeState,
                                     oldSeq,
                                     kGlobalUiListDItemCount7D70,
                                     kGlobalUiListDSource7D48,
                                     kDescNinTen7CF8);
            PsxCall8001B084_801CB190(
                runtimeState, kDescNinTen7CF8, 1, Work::W3244, 10);
            return 1;
        }
        g.g_PrStageRunner_UIListD_Active = false;
    }
    return result;
}

static int PrStageRunner_UIList_UpdateA_801CAA24(
    Stage1SceneSubmitRuntime::Impl& runtimeState) {
    auto& g = runtimeState.psx801CBFDC;
    int result = g.g_PrStageRunner_UIList_Block ? 1 : 0;
    if (!g.g_PrStageRunner_UIList_Block) {
        result = g.g_PrStageRunner_UIListA_ItemCount;
        if (g.g_PrStageRunner_UIListA_ItemCount > 0) {
            const uint32_t oldSeq = g.g_PrStageRunner_UIListA_Seq;
            PsxCall8001B000_801CBFDC(runtimeState,
                                     oldSeq,
                                     kGlobalUiListAItemCount7D7C,
                                     kGlobalUiListASourceKamonL7D54,
                                     kDescKamonL7FB8);
            PsxCall8001B000_801CBFDC(runtimeState,
                                     oldSeq,
                                     kGlobalUiListAVisibleCount7D80,
                                     kGlobalUiListASourceKamonR7D58,
                                     kDescKamonR7FC8);
            g.g_PrStageRunner_UIListA_Seq = oldSeq + 1u;
            PsxCall8001B084_801CB190(
                runtimeState, kDescWakuL7EF0, 4, Work::W3244, 10);
            PsxCall8001B084_801CB190(
                runtimeState, kDescNinL7D28, 2, Work::W3244, 10);
            return 1;
        }
        g.g_PrStageRunner_UIListA_Active = false;
    }
    return result;
}

static int PrStageRunner_UIList_UpdateB_801CAB34(
    Stage1SceneSubmitRuntime::Impl& runtimeState) {
    auto& g = runtimeState.psx801CBFDC;
    int result = g.g_PrStageRunner_UIList_Block ? 1 : 0;
    if (!g.g_PrStageRunner_UIList_Block) {
        result = g.g_PrStageRunner_UIListB_ItemCount;
        if (g.g_PrStageRunner_UIListB_ItemCount > 0) {
            const uint32_t oldSeq = g.g_PrStageRunner_UIListB_Seq;
            int32_t v1 = g.g_PrStageRunner_UIListB_ItemCount;
            PsxCall8001B000_801CBFDC(runtimeState,
                                     oldSeq,
                                     &v1,
                                     0u,
                                     kGlobalUiListBSourceMadoL7E38,
                                     kDescMadoL7FD8);
            PsxCall8001B000_801CBFDC(runtimeState,
                                     oldSeq,
                                     kGlobalUiListBItemCount7E48,
                                     kGlobalUiListBSourceMadoR7E3C,
                                     kDescMadoR7FE8);
            PsxCall8001B000_801CBFDC(runtimeState,
                                     oldSeq,
                                     kGlobalUiListBVisibleCount7D78,
                                     kGlobalUiListBVisibleSource7D50,
                                     kDescNinShj7D18);
            g.g_PrStageRunner_UIListB_Seq = oldSeq + 1u;
            PsxCall8001B084_801CB190(
                runtimeState, kDescNinShj7D18, 1, Work::W3244, 10);
            return 1;
        }
        g.g_PrStageRunner_UIListB_Active = false;
    }
    return result;
}

static int PrStageRunner_RenderUI_801CB190(
    Stage1SceneSubmitRuntime::Impl& runtimeState) {
    auto& g = runtimeState.psx801CBFDC;

    if (!g.g_PrStageRunner_UIRender_Block &&
        !g.g_PrStageRunner_UIList_Block) {
        PrStageRunner_UIList_Init_801CA754(
            runtimeState, static_cast<int>(g.dword_801D3020));
    }
    if (!g.g_PrStageRunner_UIRender_Block2) {
        const uint32_t v0 = g.dword_801D3018++;
        PsxCall80013EA8_801CB190(runtimeState,
                                 kGlobalOnMimeTimelineB0C4,
                                 v0,
                                 1u);
        const uint32_t onSeq = g.dword_801D3020++;
        const int onResult =
            PsxCall8001B000_801CBFDC(runtimeState,
                                     onSeq,
                                     kGlobalOnCountB2D4,
                                     kGlobalOnCursor301C,
                                     kDescOn98F0);
        if (onResult <= 0 &&
            g.g_PrStageRunner_UIRender_RefreshRequest) {
            g.g_PrStageRunner_UIRender_RefreshRequest = false;
            g.g_PrStageRunner_UIRender_Block2 = true;
        }
        PsxCall8001B084_801CB190(
            runtimeState,
            kDescOn98F0,
            1,
            g.g_PrStageRunner_UIListC_Active ? Work::W3244 : Work::W5AF4,
            10);
    }

    PsxCall800141D8_801CB190(runtimeState);
    const uint32_t v3 = g.dword_801D3028++;
    PsxCall8001B000_801CBFDC(runtimeState,
                             v3,
                             kGlobalPaCountB338,
                             kGlobalPaCursor3024,
                             kDescPaB2D8);
    PsxCall8001B084_801CB190(
        runtimeState,
        kDescPaB2D8,
        1,
        g.g_PrStageRunner_UIListC_Active ? Work::W3244 : Work::W5AF4,
        10);

    if (g.g_PrStageRunner_UIListD_Active) {
        PrStageRunner_UIList_UpdateD_801CA974(runtimeState);
    }
    if (g.g_PrStageRunner_UIListC_Active) {
        PrStageRunner_UIList_UpdateC_801CA838(runtimeState);
    }
    if (g.g_PrStageRunner_UIListB_Active) {
        PrStageRunner_UIList_UpdateB_801CAB34(runtimeState);
    }
    if (g.g_PrStageRunner_UIListA_Active) {
        PrStageRunner_UIList_UpdateA_801CAA24(runtimeState);
    }

    PsxCall8001B084_801CB190(
        runtimeState, kDescFloor8038, 1, Work::W5AF4, 10);
    PsxCall8001B084_801CB190(
        runtimeState,
        kDescTama98D0 + 16u * g.dword_801D3014,
        1,
        g.g_PrStageRunner_UIListC_Active ? Work::W3244 : Work::W5AF4,
        10);

    if (g.g_PrStageRunner_UIList_Block) {
        PsxCall8001B084_801CB190(
            runtimeState, kDescTourou7F68, 1, Work::W3244, 10);
        PsxCall8001B084_801CB190(
            runtimeState, kDescAux7FF8, 3, Work::W3244, 10);
    } else {
        PsxCall8001B084_801CB190(
            runtimeState, kDescTourou7F68, 1, Work::W80A8, 8);
        PsxCall8001B084_801CB190(
            runtimeState, kDescTennjou7F78, 1, Work::W8058, 8);
        PsxCall8001B084_801CB190(
            runtimeState, kDescKabeR7F88, 2, Work::W8080, 8);
        PsxCall8001B084_801CB190(
            runtimeState, kDescKabeB7FA8, 1, Work::W80A8, 8);
        PsxCall8001B084_801CB190(
            runtimeState, kDescKamonL7FB8, 7, Work::W3244, 10);
        const int result = g.g_PrStageRunner_UIRender_Block ? 1 : 0;
        if (g.g_PrStageRunner_UIRender_Block) {
            return result;
        }
    }
    PsxCall8001B084_801CB190(
        runtimeState, kDescKage8028, 1, Work::W526C, 8);
    return 1;
}

static void PsxCall8001B590_801CAC34(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    int16_t x,
    int16_t y,
    PsxBackdropSpriteCommand801CAC34::Texture texture) {
    PsxBackdropSpriteCommand801CAC34 command{};
    command.texture = texture;
    command.x = static_cast<float>(x);
    command.y = static_cast<float>(y);
    command.alpha = 1.0f;
    runtimeState.backdropSprites801CAC34.push_back(command);
}

static void PrStageRunner_EmitBackdropPrimitives801CAC34(
    Stage1SceneSubmitRuntime::Impl& runtimeState) {
    runtimeState.backdropGradient801CAC34 = {};
    runtimeState.backdropSprites801CAC34.clear();

    const auto& cam = runtimeState.backdropCamera;
    const int32_t vx = static_cast<int32_t>(
        std::lround(cam.target[0] - cam.pos[0]));
    const int32_t vy = static_cast<int32_t>(
        std::lround(cam.target[1] - cam.pos[1]));
    const int32_t vz = static_cast<int32_t>(
        std::lround(cam.target[2] - cam.pos[2]));
    const int32_t length =
        PsxCall80041548_SquareRoot0(static_cast<int64_t>(vx) * vx +
                                    static_cast<int64_t>(vy) * vy +
                                    static_cast<int64_t>(vz) * vz);
    int32_t v16x = 0;
    int32_t v16y = 0;
    int32_t v17 = 0;
    if (length != 0) {
        v16x = PsxDivTrunc801CBFDC(static_cast<int64_t>(vx) * 4096,
                                   length);
        v16y = PsxDivTrunc801CBFDC(static_cast<int64_t>(vy) * 4096,
                                   length);
        v17 = PsxDivTrunc801CBFDC(static_cast<int64_t>(vz) * 4096,
                                  length);
    }

    const int v5 = PsxSar801CBFDC(static_cast<int64_t>(-440) * v16y, 12);
    int v6 = 0;
    if (v16x != 0 || v17 != 0) {
        v6 = PsxSar801CBFDC(Stage1PsxRatan2_8003B70C_801CAC34(v17, v16x),
                            1) -
             512;
    }

    runtimeState.backdropGradient801CAC34.valid = true;
    runtimeState.backdropGradient801CAC34.x = 0.0f;
    runtimeState.backdropGradient801CAC34.y = 0.0f;
    runtimeState.backdropGradient801CAC34.w = 320.0f;
    runtimeState.backdropGradient801CAC34.h = static_cast<float>(v5 + 145);
    runtimeState.backdropGradient801CAC34.topRgb = 9443328u;
    runtimeState.backdropGradient801CAC34.bottomRgb = 15237183u;

    int v7 = -640;
    int v8 = 0;
    int16_t v9 = static_cast<int16_t>(v6 - 640);
    do {
        ++v8;
        v7 += 320;
        PsxCall8001B590_801CAC34(
            runtimeState,
            static_cast<int16_t>(v9 + 240),
            static_cast<int16_t>(v5 + 35),
            PsxBackdropSpriteCommand801CAC34::Texture::YuKumo);
        PsxCall8001B590_801CAC34(
            runtimeState,
            static_cast<int16_t>(v9 - 30),
            static_cast<int16_t>(v5),
            PsxBackdropSpriteCommand801CAC34::Texture::YuKumo);
        v9 = static_cast<int16_t>(v6 + v7);
    } while (v8 < 5);

    int v10 = -708;
    int v11 = 0;
    int16_t v12 = static_cast<int16_t>(v6 - 708);
    do {
        ++v11;
        v10 += 256;
        PsxCall8001B590_801CAC34(
            runtimeState,
            v12,
            static_cast<int16_t>(v5 + 103),
            PsxBackdropSpriteCommand801CAC34::Texture::YuYama);
        v12 = static_cast<int16_t>(v6 + v10);
    } while (v11 < 7);

    PsxCall8001B590_801CAC34(
        runtimeState,
        static_cast<int16_t>(v6 + 110),
        static_cast<int16_t>(v5 + 10),
        PsxBackdropSpriteCommand801CAC34::Texture::YuHi);

    const int16_t v14 = static_cast<int16_t>(v6 + 40);
    if (runtimeState.psx801CBFDC.g_PrStageRunner_UIRender_Block2) {
        PsxCall8001B590_801CAC34(
            runtimeState,
            v14,
            static_cast<int16_t>(v5 - 2),
            PsxBackdropSpriteCommand801CAC34::Texture::OnKao);
        PsxCall8001B590_801CAC34(
            runtimeState,
            v14,
            static_cast<int16_t>(v5 + 93),
            PsxBackdropSpriteCommand801CAC34::Texture::YuJhan);
    }
}

static int PrStageRunner_DrawBackground_801CAC34(
    Stage1SceneSubmitRuntime::Impl& runtimeState) {
    auto& g = runtimeState.psx801CBFDC;

    if (!g.g_PrStageRunner_UIRender_Block2) {
        PsxCall8001B084_801CAC34(
            runtimeState, kDescOn98F0, 1, Work::W5AF4, 10);
    }
    PsxCall8001B084_801CAC34(
        runtimeState, kDescPaB2D8, 1, Work::W5AF4, 10);
    PsxCall8001B084_801CAC34(
        runtimeState, kDescFloor8038, 1, Work::W5AF4, 10);
    PsxCall8001B084_801CAC34(
        runtimeState,
        kDescTama98D0 + 16u * g.dword_801D3014,
        1,
        Work::W5AF4,
        10);
    if (g.g_PrStageRunner_UIList_Block) {
        PsxCall8001B084_801CAC34(
            runtimeState, kDescTourou7F68, 1, Work::W3244, 10);
        PsxCall8001B084_801CAC34(
            runtimeState, kDescAux7FF8, 3, Work::W3244, 10);
        PsxCall8001B084_801CAC34(
            runtimeState, kDescKage8028, 1, Work::W526C, 8);
    } else {
        if (!g.g_PrStageRunner_UIRender_Block) {
            PsxCall8001B084_801CAC34(
                runtimeState, kDescKage8028, 1, Work::W526C, 8);
        }
        PsxCall8001B084_801CAC34(
            runtimeState, kDescTourou7F68, 12, Work::W3244, 10);
    }
    PrStageRunner_EmitBackdropPrimitives801CAC34(runtimeState);
    return g.g_PrStageRunner_UIRender_Block2 ? 1 : 0;
}

static PsxPrStageRunnerCtx801CBFDC BuildStage1PsxRenderCtx801CBFDC(
    const PrStage1RuntimeSlotsSnapshot& runtimeSlots,
    int modeArg801CBFDC,
    const char* scriptBoxTextPtr10C,
    uint32_t suppressOneShotPulseMask) {
    PsxPrStageRunnerCtx801CBFDC ctx{};
    ctx.valid = runtimeSlots.valid;
    ctx.modeArg = modeArg801CBFDC;
    ctx.queryFrame = runtimeSlots.queryFrame;
    ctx.uiListHandles134StartFrame = runtimeSlots.scene.streamStartFrame;

    const uint32_t flag2000DynamicRowReload =
        runtimeSlots.sceneSubmit.flag2000DynamicRowReload801CBFDC
            ? 0x00002000u
            : 0u;
    const uint32_t oneShotPulseFlags =
        (runtimeSlots.sceneSubmit.ctxFlagsPulse801CBFDC |
         flag2000DynamicRowReload) &
        kOneShotCtxFlags801CBFDC &
        ~suppressOneShotPulseMask;
    ctx.flags_00 =
        (runtimeSlots.sceneSubmit.ctxFlagsFull &
         ~kOneShotCtxFlags801CBFDC) |
        (oneShotPulseFlags & kOneShotCtxFlags801CBFDC) |
        runtimeSlots.sceneSubmit.ctxFlags40_2000_4000;
    if (runtimeSlots.cameraPlaybackEnabled3034) {
        ctx.flags_00 |= 0x00000020u;
    }

    ctx.uiRenderBlock_4E =
        runtimeSlots.sceneSubmit.uiRenderBlock4CC4Valid
            ? runtimeSlots.sceneSubmit.uiRenderBlock4CC4
            : 0;
    ctx.tailGate54 =
        runtimeSlots.sceneSubmit.directScriptBoxGate54Known
            ? runtimeSlots.sceneSubmit.directScriptBoxGate54
            : 0u;
    ctx.tailArg68 =
        runtimeSlots.sceneSubmit.directScriptBoxArg68Known
            ? runtimeSlots.sceneSubmit.directScriptBoxArg68
            : 0;
    ctx.tailGate64 =
        runtimeSlots.sceneSubmit.directNumericStatusGate64Known
            ? runtimeSlots.sceneSubmit.directNumericStatusGate64
            : 0u;
    ctx.tailTextPtr10C = scriptBoxTextPtr10C;
    ctx.ctx72_72 =
        static_cast<int16_t>(runtimeSlots.sceneSubmit.ctx72);
    ctx.todStart_11C =
        static_cast<int16_t>(runtimeSlots.sceneSubmit.ctx11C);
    ctx.todEnd_120 =
        static_cast<int16_t>(runtimeSlots.sceneSubmit.ctx120);
    ctx.uiListSeedKind_7C =
        runtimeSlots.descBank.valid
            ? static_cast<int16_t>(runtimeSlots.descBank.selector)
            : 0;
    ctx.tamaSelector_FC =
        runtimeSlots.tamaSelector.valid ? runtimeSlots.tamaSelector.selector : 0u;
    if (runtimeSlots.paMotion.valid) {
        ctx.paDatHandle_DC = runtimeSlots.paMotion.datHandle;
        ctx.paVdfHandle_E8 = runtimeSlots.paMotion.vdfHandle;
    }
    if (runtimeSlots.onMotion.valid) {
        ctx.onDatHandle_E0 = runtimeSlots.onMotion.datHandle;
        ctx.onVdfHandle_EC = runtimeSlots.onMotion.vdfHandle;
    }
    ctx.listSlotF4 =
        runtimeSlots.performanceTod.valid ? runtimeSlots.performanceTod.handle : 0u;
    ctx.listSlotF4StartFrame = runtimeSlots.performanceTod.startFrame;
    ctx.listSlotF8 =
        runtimeSlots.row0Tod.valid ? runtimeSlots.row0Tod.handle : 0u;
    ctx.listSlotF8StartFrame = runtimeSlots.row0Tod.startFrame;
    if (runtimeSlots.camera.valid) {
        ctx.cameraBezHandle104 =
            static_cast<uint32_t>(
                static_cast<uint16_t>(runtimeSlots.camera.bezId));
        ctx.cameraSourceEventPsxAddr =
            runtimeSlots.camera.sourceEventPsxAddr;
    }

    ctx.uiListHandles134Valid =
        runtimeSlots.scene.valid &&
        runtimeSlots.scene.selectedRow != 0u;
    for (std::size_t i = 0; i < ctx.uiListHandles_134.size(); ++i) {
        ctx.uiListHandles_134[i] = runtimeSlots.scene.rowHandles[i];
    }

    ctx.uiListSeedSourcesValid = runtimeSlots.descBank.valid;
    ctx.uiListSeedActiveGroups = runtimeSlots.descBank.activeGroups;
    ctx.uiListSeedStartFrames = runtimeSlots.descBank.groupStartFrames;
    ctx.compactInputCaptured80024744 =
        runtimeSlots.sceneSubmit.compactRail80024744.valid;
    ctx.compactResetWobbleBanks80024744 =
        runtimeSlots.sceneSubmit.compactWobbleResetPulse80024744;
    ctx.compactTeacherResetPulse80024308 =
        runtimeSlots.sceneSubmit.compactTeacherResetPulse80024308;
    ctx.compactStudentResetPulse80024390 =
        runtimeSlots.sceneSubmit.compactStudentResetPulse80024390;
    ctx.compactPainterGate7A =
        runtimeSlots.sceneSubmit.compactRail80024744.painterGate7A;
    ctx.compactRowCount8A =
        runtimeSlots.sceneSubmit.compactRail80024744.rowCount8A;
    for (std::size_t i = 0;
         i < ctx.compactRows80024744.size() &&
         i < runtimeSlots.sceneSubmit.compactRail80024744.rows.size();
         ++i) {
        const auto& src =
            runtimeSlots.sceneSubmit.compactRail80024744.rows[i];
        PsxCompactRailRow80024744& dst = ctx.compactRows80024744[i];
        dst.teacherRaw8C = src.teacherRaw8C;
        dst.studentRaw9E = src.studentRaw9E;
        dst.bodyStreamPtr94 = src.bodyStreamPtr94;
        dst.bodyStreamBytes94 = src.bodyStreamBytes94;
    }

    for (std::size_t channel = 0u;
         channel < runtimeSlots.hudTimUploads.size() &&
         channel < ctx.shortLists_AC.size();
         ++channel) {
        const auto& upload = runtimeSlots.hudTimUploads[channel];
        if (!upload.valid) {
            continue;
        }
        bool hasTimId = false;
        for (uint16_t timId : upload.timIds) {
            if (timId != 0u) {
                hasTimId = true;
                break;
            }
        }
        if (!hasTimId) {
            continue;
        }
        ctx.flags_00 |= 0x00008000u;
        ctx.shortLists_AC[channel] = upload.timIds;
    }

    return ctx;
}

static PsxDrawDescAttrState428B0 DecodeDrawDescAttr800428B0(uint32_t attr) {
    PsxDrawDescAttrState428B0 state{};
    state.code = static_cast<uint8_t>(attr & 7u);
    state.abr = static_cast<uint8_t>((attr >> 3) & 3u);
    state.semiTransparent = ((attr >> 5) & 1u) != 0u;
    state.textureRaw = ((attr >> 6) & 1u) != 0u;
    state.tge = static_cast<uint8_t>((attr >> 9) & 7u);
    state.highPriority = ((attr >> 30) & 1u) != 0u;
    return state;
}

static uint8_t PsxPrimitiveWordStride8004274C(uint8_t mode, uint8_t flag) {
    switch (mode & 0xFDu) {
        case 0x20u:
            return (flag & 4u) != 0u ? 6u : 4u;
        case 0x21u:
        case 0x29u:
            return 4u;
        case 0x24u:
        case 0x31u:
        case 0x38u:
            return 6u;
        case 0x25u:
        case 0x34u:
        case 0x39u:
            return 7u;
        case 0x28u:
            return 5u;
        case 0x2Cu:
        case 0x2Du:
            return 8u;
        case 0x30u:
            return (flag & 4u) != 0u ? 7u : 5u;
        case 0x35u:
        case 0x3Cu:
            return 9u;
        case 0x3Du:
            return 11u;
        default:
            return 0u;
    }
}

static bool IsRawPrimitiveStreamContinuation800428B0(
    const PsxTmdRawPrimitivePacket428B0& previous,
    const PsxTmdRawPrimitivePacket428B0& current) {
    if (previous.rawPacketByteSize == 0u || current.rawPacketByteSize == 0u) {
        return false;
    }
    return current.rawPrimitiveIndex == previous.rawPrimitiveIndex + 1u &&
           current.rawPacketOffset ==
               previous.rawPacketOffset + previous.rawPacketByteSize;
}

static uint8_t PsxPrimitiveGroupWordStride8004274C(
    const PsxTmdPrimitive428B0& first) {
    if (!first.rawPacketKnown || first.rawPacketByteSize == 0u) {
        return 0u;
    }
    return PsxPrimitiveWordStride8004274C(first.psxMode, first.psxFlag);
}

static void BuildPrimitiveGroups8004274C(PsxTmdObjectView428B0& object) {
    object.primitiveGroups8004274C.clear();
    for (PsxTmdPrimitive428B0& primitive : object.primitives) {
        primitive.psxGroupCount8004274C = 0u;
    }

    const auto finishGroup =
        [&object](bool haveGroup,
                  uint32_t groupStart,
                  uint32_t groupEnd,
                  uint8_t groupMode,
                  uint8_t groupFlag) {
            if (!haveGroup || groupStart >= object.primitives.size() ||
                groupEnd < groupStart || groupEnd >= object.primitives.size()) {
                return;
            }
            const PsxTmdPrimitive428B0& first =
                object.primitives[groupStart];
            const uint16_t groupCount =
                static_cast<uint16_t>(groupEnd - groupStart + 1u);
            object.primitives[groupStart].psxGroupCount8004274C = groupCount;
            object.primitiveGroups8004274C.push_back({
                groupStart,
                groupCount,
                groupMode,
                groupFlag,
                PsxPrimitiveGroupWordStride8004274C(first),
            });
        };

    uint32_t groupStart = 0u;
    uint32_t groupEnd = 0u;
    uint8_t groupMode = 0u;
    uint8_t groupFlag = 0u;
    PsxTmdRawPrimitivePacket428B0 previousRaw{};
    bool haveGroup = false;

    for (const PsxTmdRawPrimitivePacket428B0& raw :
         object.rawPrimitivePackets) {
        if (!raw.parsed ||
            raw.parsedPrimitiveIndex >= object.primitives.size()) {
            finishGroup(haveGroup, groupStart, groupEnd, groupMode, groupFlag);
            haveGroup = false;
            previousRaw = raw;
            continue;
        }

        const uint32_t primitiveIndex = raw.parsedPrimitiveIndex;
        const PsxTmdPrimitive428B0& primitive =
            object.primitives[primitiveIndex];
        const bool sameGroup =
            haveGroup &&
            primitive.psxMode == groupMode &&
            primitive.psxFlag == groupFlag &&
            primitiveIndex == groupEnd + 1u &&
            IsRawPrimitiveStreamContinuation800428B0(previousRaw, raw);
        if (!sameGroup && haveGroup) {
            finishGroup(haveGroup, groupStart, groupEnd, groupMode, groupFlag);
        }
        if (!sameGroup) {
            groupStart = primitiveIndex;
            groupEnd = primitiveIndex;
            groupMode = primitive.psxMode;
            groupFlag = primitive.psxFlag;
            haveGroup = true;
        } else {
            groupEnd = primitiveIndex;
        }
        previousRaw = raw;
    }
    finishGroup(haveGroup, groupStart, groupEnd, groupMode, groupFlag);
}

static bool PsxGsTmdFastTableSlot800428B0(uint8_t mode,
                                          uint8_t flag,
                                          uint8_t& tableSlot) {
    const uint8_t psxPrimitiveCode = static_cast<uint8_t>(mode & 0xFDu);
    if ((psxPrimitiveCode == 0x20u || psxPrimitiveCode == 0x30u) &&
        (flag & 4u) != 0u) {
        return false;
    }

    switch (psxPrimitiveCode) {
    case 0x20u:
        tableSlot = 2u;
        break;
    case 0x21u:
        tableSlot = 6u;
        break;
    case 0x24u:
        tableSlot = 18u;
        break;
    case 0x25u:
        tableSlot = 22u;
        break;
    case 0x28u:
        tableSlot = 34u;
        break;
    case 0x29u:
        tableSlot = 38u;
        break;
    case 0x2Cu:
        tableSlot = 50u;
        break;
    case 0x2Du:
        tableSlot = 54u;
        break;
    case 0x30u:
        tableSlot = 10u;
        break;
    case 0x31u:
        tableSlot = 14u;
        break;
    case 0x34u:
        tableSlot = 26u;
        break;
    case 0x35u:
        tableSlot = 30u;
        break;
    case 0x38u:
        tableSlot = 42u;
        break;
    case 0x39u:
        tableSlot = 46u;
        break;
    case 0x3Cu:
        tableSlot = 58u;
        break;
    case 0x3Du:
        tableSlot = 62u;
        break;
    default:
        tableSlot = 0u;
        return false;
    }
    return true;
}

static bool PsxGsTmdFastSpecialTableBase800428B0(
    uint8_t mode,
    uint8_t flag,
    uint32_t& tableBase) {
    const uint8_t psxPrimitiveCode = static_cast<uint8_t>(mode & 0xFDu);
    if ((flag & 4u) == 0u) {
        return false;
    }
    if (psxPrimitiveCode == 0x20u) {
        tableBase = kPsxGsTmdFastFlag4TableBase8008EED8;
        return true;
    }
    if (psxPrimitiveCode == 0x30u) {
        tableBase = kPsxGsTmdFastFlag4TableBase8008EEE4;
        return true;
    }
    return false;
}

static PsxGsTmdFastHandler428B0 PsxResolveGsTmdFastHandler800428B0(
    uint8_t mode,
    uint8_t flag) {
    uint8_t tableSlot = 0u;
    if (!PsxGsTmdFastTableSlot800428B0(mode, flag, tableSlot)) {
        return PsxGsTmdFastHandler428B0::Unsupported;
    }
    return kPsxGsTmdFastTable8001C1E8[tableSlot];
}

static uint8_t PsxGsTmdFastVertexCount428B0(
    PsxGsTmdFastHandler428B0 handler) {
    return (handler == PsxGsTmdFastHandler428B0::F4NL ||
            handler == PsxGsTmdFastHandler428B0::NF4 ||
            handler == PsxGsTmdFastHandler428B0::TF4NL ||
            handler == PsxGsTmdFastHandler428B0::TNF4 ||
            handler == PsxGsTmdFastHandler428B0::G4NL ||
            handler == PsxGsTmdFastHandler428B0::NG4 ||
            handler == PsxGsTmdFastHandler428B0::TG4NL ||
            handler == PsxGsTmdFastHandler428B0::TNG4)
               ? 4u
               : 3u;
}

static bool PsxGsTmdFastIsQuad428B0(PsxGsTmdFastHandler428B0 handler) {
    return PsxGsTmdFastVertexCount428B0(handler) == 4u;
}

static bool PsxGsTmdFastIsTextured428B0(
    PsxGsTmdFastHandler428B0 handler) {
    return !(handler == PsxGsTmdFastHandler428B0::F3NL ||
             handler == PsxGsTmdFastHandler428B0::NF3 ||
             handler == PsxGsTmdFastHandler428B0::F4NL ||
             handler == PsxGsTmdFastHandler428B0::NF4 ||
             handler == PsxGsTmdFastHandler428B0::G3NL ||
             handler == PsxGsTmdFastHandler428B0::NG3 ||
             handler == PsxGsTmdFastHandler428B0::G4NL ||
             handler == PsxGsTmdFastHandler428B0::NG4);
}

static uint8_t PsxGsTmdFastPacketWordCount428B0(
    PsxGsTmdFastHandler428B0 handler) {
    switch (handler) {
        case PsxGsTmdFastHandler428B0::F3NL:
        case PsxGsTmdFastHandler428B0::NF3:
            return 5u;
        case PsxGsTmdFastHandler428B0::F4NL:
        case PsxGsTmdFastHandler428B0::NF4:
            return 6u;
        case PsxGsTmdFastHandler428B0::G3NL:
        case PsxGsTmdFastHandler428B0::NG3:
            return 7u;
        case PsxGsTmdFastHandler428B0::TF3NL:
        case PsxGsTmdFastHandler428B0::TNF3:
        case PsxGsTmdFastHandler428B0::TG3NL:
            return 8u;
        case PsxGsTmdFastHandler428B0::G4NL:
        case PsxGsTmdFastHandler428B0::NG4:
            return 9u;
        case PsxGsTmdFastHandler428B0::TF4NL:
        case PsxGsTmdFastHandler428B0::TNF4:
        case PsxGsTmdFastHandler428B0::TG4NL:
        case PsxGsTmdFastHandler428B0::TNG3:
            return 10u;
        case PsxGsTmdFastHandler428B0::TNG4:
            return 13u;
        case PsxGsTmdFastHandler428B0::Unsupported:
        default:
            return 0u;
    }
}

static uint32_t PsxGpuCodeWordForGsTmdFast428B0(
    const PsxTmdPrimitive428B0& primitive,
    PsxGsTmdFastHandler428B0 handler,
    const PsxDrawDescAttrState428B0& attr) {
    const uint32_t priorityBit = attr.highPriority ? 1u : 0u;
    if (handler == PsxGsTmdFastHandler428B0::F3NL ||
        handler == PsxGsTmdFastHandler428B0::NF3 ||
        handler == PsxGsTmdFastHandler428B0::F4NL ||
        handler == PsxGsTmdFastHandler428B0::NF4) {
        return primitive.psxRgbCodeWord | (priorityBit << 25);
    }
    if (handler == PsxGsTmdFastHandler428B0::TF3NL ||
        handler == PsxGsTmdFastHandler428B0::TF4NL) {
        return ((static_cast<uint32_t>(primitive.psxMode) |
                 (priorityBit << 1))
                << 24) |
               0x00808080u;
    }
    if (handler == PsxGsTmdFastHandler428B0::TG3NL) {
        return ((0x25u | (static_cast<uint32_t>(primitive.psxMode) & 2u) |
                 (priorityBit << 1))
                << 24) |
               0x00808080u;
    }
    if (handler == PsxGsTmdFastHandler428B0::TG4NL) {
        return ((0x2Du | (static_cast<uint32_t>(primitive.psxMode) & 2u) |
                 (priorityBit << 1))
                << 24) |
               0x00808080u;
    }
    const uint32_t gpuCode =
        ((static_cast<uint32_t>(primitive.psxMode) | (priorityBit << 1)) >> 1)
        << 25;
    return (primitive.psxRgbCodeWord & 0x00FFFFFFu) | gpuCode;
}

static uint32_t PsxColorWordForGsTmdFast428B0(
    const PsxTmdPrimitive428B0& primitive,
    uint8_t vertexSlot,
    const PsxDrawDescAttrState428B0& attr) {
    const uint32_t priorityBit = attr.highPriority ? 1u : 0u;
    const uint8_t slot =
        (std::min<uint8_t>)(vertexSlot, static_cast<uint8_t>(3u));
    return primitive.colorWords[slot] | (priorityBit << 25);
}

static uint32_t PsxPackSxy428B0(const PsxProjectedVertex428B0& vertex) {
    return static_cast<uint16_t>(vertex.x) |
           (static_cast<uint32_t>(static_cast<uint16_t>(vertex.y)) << 16);
}

static uint32_t PsxPackUv428B0(uint8_t u, uint8_t v, uint16_t high) {
    return static_cast<uint32_t>(u) |
           (static_cast<uint32_t>(v) << 8) |
           (static_cast<uint32_t>(high) << 16);
}

static int16_t PsxUnpackSxyX428B0(uint32_t word) {
    return static_cast<int16_t>(word & 0xFFFFu);
}

static int16_t PsxUnpackSxyY428B0(uint32_t word) {
    return static_cast<int16_t>((word >> 16) & 0xFFFFu);
}

static uint8_t PsxUnpackUvU428B0(uint32_t word) {
    return static_cast<uint8_t>(word & 0xFFu);
}

static uint8_t PsxUnpackUvV428B0(uint32_t word) {
    return static_cast<uint8_t>((word >> 8) & 0xFFu);
}

static float PsxPacketColorByteToFloat428B0(uint32_t word,
                                            uint32_t shift,
                                            bool textured) {
    const float value = static_cast<float>((word >> shift) & 0xFFu);
    return value / (textured ? 128.0f : 255.0f);
}

static uint8_t PsxPacketSxyWordIndex428B0(
    PsxGsTmdFastHandler428B0 handler,
    uint8_t vertexSlot) {
    if (handler == PsxGsTmdFastHandler428B0::F3NL ||
        handler == PsxGsTmdFastHandler428B0::NF3 ||
        handler == PsxGsTmdFastHandler428B0::F4NL ||
        handler == PsxGsTmdFastHandler428B0::NF4) {
        return static_cast<uint8_t>(2u + vertexSlot);
    }
    if (handler == PsxGsTmdFastHandler428B0::G3NL ||
        handler == PsxGsTmdFastHandler428B0::NG3 ||
        handler == PsxGsTmdFastHandler428B0::G4NL ||
        handler == PsxGsTmdFastHandler428B0::NG4) {
        return static_cast<uint8_t>(2u + vertexSlot * 2u);
    }
    if (handler == PsxGsTmdFastHandler428B0::TNG3 ||
        handler == PsxGsTmdFastHandler428B0::TNG4) {
        return static_cast<uint8_t>(2u + vertexSlot * 3u);
    }
    return static_cast<uint8_t>(2u + vertexSlot * 2u);
}

static uint8_t PsxPacketUvWordIndex428B0(
    PsxGsTmdFastHandler428B0 handler,
    uint8_t vertexSlot) {
    if (handler == PsxGsTmdFastHandler428B0::F3NL ||
        handler == PsxGsTmdFastHandler428B0::NF3 ||
        handler == PsxGsTmdFastHandler428B0::F4NL ||
        handler == PsxGsTmdFastHandler428B0::NF4 ||
        handler == PsxGsTmdFastHandler428B0::G3NL ||
        handler == PsxGsTmdFastHandler428B0::NG3 ||
        handler == PsxGsTmdFastHandler428B0::G4NL ||
        handler == PsxGsTmdFastHandler428B0::NG4) {
        return 0u;
    }
    if (handler == PsxGsTmdFastHandler428B0::TNG3 ||
        handler == PsxGsTmdFastHandler428B0::TNG4) {
        return static_cast<uint8_t>(3u + vertexSlot * 3u);
    }
    return static_cast<uint8_t>(3u + vertexSlot * 2u);
}

static uint8_t PsxPacketColorWordIndex428B0(
    PsxGsTmdFastHandler428B0 handler,
    uint8_t vertexSlot) {
    if (handler == PsxGsTmdFastHandler428B0::G3NL ||
        handler == PsxGsTmdFastHandler428B0::NG3 ||
        handler == PsxGsTmdFastHandler428B0::G4NL ||
        handler == PsxGsTmdFastHandler428B0::NG4) {
        return static_cast<uint8_t>(1u + vertexSlot * 2u);
    }
    if (handler == PsxGsTmdFastHandler428B0::TNG3 ||
        handler == PsxGsTmdFastHandler428B0::TNG4) {
        return static_cast<uint8_t>(1u + vertexSlot * 3u);
    }
    return 1u;
}

static PsxGpuPacket428B0 PsxBuildGsTmdFastPacket428B0(
    const PsxTmdPrimitive428B0& primitive,
    PsxGsTmdFastHandler428B0 handler,
    const PsxDrawDescAttrState428B0& attr,
    const std::array<PsxProjectedVertex428B0,
                     kPsxPrimitiveMaxVertexCount428B0>& projected,
    uint16_t otz) {
    PsxGpuPacket428B0 packet{};
    packet.handler = handler;
    packet.primitiveCode = static_cast<uint8_t>(primitive.psxMode & 0xFDu);
    packet.primitiveFlag = primitive.psxFlag;
    packet.primitiveFlag4Path = (primitive.psxFlag & 4u) != 0u;
    uint8_t handlerTableSlot = 0u;
    if (PsxGsTmdFastTableSlot800428B0(primitive.psxMode,
                                      primitive.psxFlag,
                                      handlerTableSlot)) {
        packet.handlerTableSlot8001C1E8 = handlerTableSlot;
        packet.handlerTableEntryAddress8001C1E8 =
            kPsxGsTmdFastTableBase8001C1E8 +
            static_cast<uint32_t>(handlerTableSlot) * 4u;
    }
    packet.primitiveCursorAdvanceHalfwords800428B0 =
        static_cast<uint16_t>(
            PsxPrimitiveWordStride8004274C(primitive.psxMode,
                                           primitive.psxFlag) *
            2u);
    packet.wordCount = PsxGsTmdFastPacketWordCount428B0(handler);
    packet.otz = otz;
    if (packet.wordCount == 0u) {
        return packet;
    }

    packet.words[0] = (static_cast<uint32_t>(packet.wordCount - 1u) << 24);
    packet.words[1] =
        PsxGpuCodeWordForGsTmdFast428B0(primitive, handler, attr);
    packet.words[2] = PsxPackSxy428B0(projected[0]);

    switch (handler) {
        case PsxGsTmdFastHandler428B0::F3NL:
        case PsxGsTmdFastHandler428B0::NF3:
            packet.words[3] = PsxPackSxy428B0(projected[1]);
            packet.words[4] = PsxPackSxy428B0(projected[2]);
            break;
        case PsxGsTmdFastHandler428B0::G3NL:
        case PsxGsTmdFastHandler428B0::NG3:
            packet.words[1] =
                PsxColorWordForGsTmdFast428B0(primitive, 0, attr);
            packet.words[3] =
                PsxColorWordForGsTmdFast428B0(primitive, 1, attr);
            packet.words[4] = PsxPackSxy428B0(projected[1]);
            packet.words[5] =
                PsxColorWordForGsTmdFast428B0(primitive, 2, attr);
            packet.words[6] = PsxPackSxy428B0(projected[2]);
            break;
        case PsxGsTmdFastHandler428B0::F4NL:
        case PsxGsTmdFastHandler428B0::NF4:
            packet.words[3] = PsxPackSxy428B0(projected[1]);
            packet.words[4] = PsxPackSxy428B0(projected[2]);
            packet.words[5] = PsxPackSxy428B0(projected[3]);
            break;
        case PsxGsTmdFastHandler428B0::G4NL:
        case PsxGsTmdFastHandler428B0::NG4:
            packet.words[1] =
                PsxColorWordForGsTmdFast428B0(primitive, 0, attr);
            packet.words[3] =
                PsxColorWordForGsTmdFast428B0(primitive, 1, attr);
            packet.words[4] = PsxPackSxy428B0(projected[1]);
            packet.words[5] =
                PsxColorWordForGsTmdFast428B0(primitive, 2, attr);
            packet.words[6] = PsxPackSxy428B0(projected[2]);
            packet.words[7] =
                PsxColorWordForGsTmdFast428B0(primitive, 3, attr);
            packet.words[8] = PsxPackSxy428B0(projected[3]);
            break;
        case PsxGsTmdFastHandler428B0::TF3NL:
        case PsxGsTmdFastHandler428B0::TNF3:
        case PsxGsTmdFastHandler428B0::TG3NL:
            packet.words[3] =
                PsxPackUv428B0(primitive.u[0], primitive.v[0],
                                primitive.clut);
            packet.words[4] = PsxPackSxy428B0(projected[1]);
            packet.words[5] =
                PsxPackUv428B0(primitive.u[1], primitive.v[1],
                                primitive.tpage);
            packet.words[6] = PsxPackSxy428B0(projected[2]);
            packet.words[7] =
                PsxPackUv428B0(primitive.u[2], primitive.v[2], 0);
            break;
        case PsxGsTmdFastHandler428B0::TNG3:
            packet.words[1] =
                PsxColorWordForGsTmdFast428B0(primitive, 0, attr);
            packet.words[3] =
                PsxPackUv428B0(primitive.u[0], primitive.v[0],
                                primitive.clut);
            packet.words[4] =
                PsxColorWordForGsTmdFast428B0(primitive, 1, attr);
            packet.words[5] = PsxPackSxy428B0(projected[1]);
            packet.words[6] =
                PsxPackUv428B0(primitive.u[1], primitive.v[1],
                                primitive.tpage);
            packet.words[7] =
                PsxColorWordForGsTmdFast428B0(primitive, 2, attr);
            packet.words[8] = PsxPackSxy428B0(projected[2]);
            packet.words[9] =
                PsxPackUv428B0(primitive.u[2], primitive.v[2], 0);
            break;
        case PsxGsTmdFastHandler428B0::TF4NL:
        case PsxGsTmdFastHandler428B0::TNF4:
        case PsxGsTmdFastHandler428B0::TG4NL:
            packet.words[3] =
                PsxPackUv428B0(primitive.u[0], primitive.v[0],
                                primitive.clut);
            packet.words[4] = PsxPackSxy428B0(projected[1]);
            packet.words[5] =
                PsxPackUv428B0(primitive.u[1], primitive.v[1],
                                primitive.tpage);
            packet.words[6] = PsxPackSxy428B0(projected[2]);
            packet.words[7] =
                PsxPackUv428B0(primitive.u[2], primitive.v[2], 0);
            packet.words[8] = PsxPackSxy428B0(projected[3]);
            packet.words[9] =
                PsxPackUv428B0(primitive.u[3], primitive.v[3], 0);
            break;
        case PsxGsTmdFastHandler428B0::TNG4:
            packet.words[1] =
                PsxColorWordForGsTmdFast428B0(primitive, 0, attr);
            packet.words[3] =
                PsxPackUv428B0(primitive.u[0], primitive.v[0],
                                primitive.clut);
            packet.words[4] =
                PsxColorWordForGsTmdFast428B0(primitive, 1, attr);
            packet.words[5] = PsxPackSxy428B0(projected[1]);
            packet.words[6] =
                PsxPackUv428B0(primitive.u[1], primitive.v[1],
                                primitive.tpage);
            packet.words[7] =
                PsxColorWordForGsTmdFast428B0(primitive, 2, attr);
            packet.words[8] = PsxPackSxy428B0(projected[2]);
            packet.words[9] =
                PsxPackUv428B0(primitive.u[2], primitive.v[2], 0);
            packet.words[10] =
                PsxColorWordForGsTmdFast428B0(primitive, 3, attr);
            packet.words[11] = PsxPackSxy428B0(projected[3]);
            packet.words[12] =
                PsxPackUv428B0(primitive.u[3], primitive.v[3], 0);
            break;
        case PsxGsTmdFastHandler428B0::Unsupported:
        default:
            packet.wordCount = 0;
            packet.words = {};
            break;
    }
    return packet;
}

static int16_t PsxSxyFromRtpt428B0(int64_t value) {
    return static_cast<int16_t>(
        (std::clamp<int64_t>)(value, INT16_MIN, INT16_MAX));
}

static bool PsxProjectVertexRtpt428B0(
    const PsxMatrix3x4Fixed428B0& matrix,
    const PsxProjectionParams428B0& projection,
    const PsxTmdVertex428B0& vertex,
    PsxProjectedVertex428B0& out) {
    const int32_t vx = vertex.x;
    const int32_t vy = vertex.y;
    const int32_t vz = vertex.z;
    const int32_t cx = PsxSar801CBFDC(
        static_cast<int64_t>(matrix.m[0][0]) * vx +
            static_cast<int64_t>(matrix.m[0][1]) * vy +
            static_cast<int64_t>(matrix.m[0][2]) * vz,
        12) + matrix.t[0];
    const int32_t cy = PsxSar801CBFDC(
        static_cast<int64_t>(matrix.m[1][0]) * vx +
            static_cast<int64_t>(matrix.m[1][1]) * vy +
            static_cast<int64_t>(matrix.m[1][2]) * vz,
        12) + matrix.t[1];
    const int32_t cz = PsxSar801CBFDC(
        static_cast<int64_t>(matrix.m[2][0]) * vx +
            static_cast<int64_t>(matrix.m[2][1]) * vy +
            static_cast<int64_t>(matrix.m[2][2]) * vz,
        12) + matrix.t[2];

    if (cz <= 0) {
        return false;
    }

    const int64_t projectedXNumerator =
        static_cast<int64_t>(cx) * projection.projectionDistance;
    const int64_t projectedYNumerator =
        static_cast<int64_t>(cy) * projection.projectionDistance;
    out.x = PsxSxyFromRtpt428B0(
        PsxDivTrunc801CBFDC(projectedXNumerator, cz) +
        projection.screenCenterX);
    out.y = PsxSxyFromRtpt428B0(
        PsxDivTrunc801CBFDC(projectedYNumerator, cz) +
        projection.screenCenterY);
    out.screenX = (std::clamp)(
        static_cast<float>(
            static_cast<double>(projectedXNumerator) /
                static_cast<double>(cz) +
            static_cast<double>(projection.screenCenterX)),
        static_cast<float>(INT16_MIN),
        static_cast<float>(INT16_MAX));
    out.screenY = (std::clamp)(
        static_cast<float>(
            static_cast<double>(projectedYNumerator) /
                static_cast<double>(cz) +
            static_cast<double>(projection.screenCenterY)),
        static_cast<float>(INT16_MIN),
        static_cast<float>(INT16_MAX));
    out.screenZ = static_cast<float>(cz);
    out.z = cz;
    return true;
}

static bool PsxProjectVertexPresentation428B0(
    const PsxMatrix3x4Float428B0& matrix,
    const PsxProjectionParams428B0& projection,
    const PsxTmdVertex428B0& vertex,
    PsxProjectedVertex428B0& out) {
    const float vx = static_cast<float>(vertex.x);
    const float vy = static_cast<float>(vertex.y);
    const float vz = static_cast<float>(vertex.z);
    const float cx = matrix.m[0][0] * vx + matrix.m[0][1] * vy +
                     matrix.m[0][2] * vz + matrix.t[0];
    const float cy = matrix.m[1][0] * vx + matrix.m[1][1] * vy +
                     matrix.m[1][2] * vz + matrix.t[1];
    const float cz = matrix.m[2][0] * vx + matrix.m[2][1] * vy +
                     matrix.m[2][2] * vz + matrix.t[2];

    if (cz <= 0.0f) {
        return false;
    }

    out.screenX = (std::clamp)(
        cx * static_cast<float>(projection.projectionDistance) / cz +
            static_cast<float>(projection.screenCenterX),
        static_cast<float>(INT16_MIN),
        static_cast<float>(INT16_MAX));
    out.screenY = (std::clamp)(
        cy * static_cast<float>(projection.projectionDistance) / cz +
            static_cast<float>(projection.screenCenterY),
        static_cast<float>(INT16_MIN),
        static_cast<float>(INT16_MAX));
    out.screenZ = cz;
    return true;
}

static int64_t PsxNclipMac0FromSxy428B0(
    const std::array<PsxProjectedVertex428B0,
                     kPsxPrimitiveMaxVertexCount428B0>& projected) {
    const int64_t x0 = projected[0].x;
    const int64_t y0 = projected[0].y;
    const int64_t x1 = projected[1].x;
    const int64_t y1 = projected[1].y;
    const int64_t x2 = projected[2].x;
    const int64_t y2 = projected[2].y;
    return x0 * y1 + x1 * y2 + x2 * y0 -
           x0 * y2 - x1 * y0 - x2 * y1;
}

static uint16_t PsxAvszOtZ428B0(
    const std::array<PsxProjectedVertex428B0,
                     kPsxPrimitiveMaxVertexCount428B0>& projected,
    uint8_t vertexCount) {
    if (vertexCount == 4u) {
        const int64_t sum =
            static_cast<int64_t>(projected[0].z) + projected[1].z +
            projected[2].z + projected[3].z;
        return static_cast<uint16_t>(
            (std::clamp<int64_t>)(
                (sum * kPsxGteZsf4_80040D6C) >> 12,
                0,
                0xFFFF));
    }
    if (vertexCount != 3u) {
        return 0;
    }
    const int64_t sum =
        static_cast<int64_t>(projected[0].z) + projected[1].z +
        projected[2].z;
    return static_cast<uint16_t>(
        (std::clamp<int64_t>)(
            (sum * kPsxGteZsf3_80040D6C) >> 12,
            0,
            0xFFFF));
}

template <typename Consumer>
bool PsxCall8001B084_428B0(const PsxSubmitRange& range,
                           Consumer psxCall800428B0) {
    const uint8_t otShift =
        (range.depth < 14u) ? static_cast<uint8_t>(14u - range.depth) : 0u;
    uint8_t workListFlushOrder = 0u;
    if (!PsxWorkListFlushOrder8001B084(range, workListFlushOrder)) {
        return false;
    }
    bool result = false;
    for (uint8_t ordinal = 0u; ordinal < range.descCount; ++ordinal) {
        psxCall800428B0(
            range.descAddr + static_cast<uint32_t>(ordinal) * 0x10u,
            workListFlushOrder,
            otShift,
            range.headInsert);
        result = (ordinal + 1u) < range.descCount;
    }
    return result;
}

static const TodData* ResolveStage1TodSource801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData,
    uint8_t handle) {
    if (handle == 0u) {
        return nullptr;
    }

    const uint8_t* bytes = nullptr;
    std::size_t byteSize = 0;
    if (!resolveResource(handle, bytes, byteSize, resourceUserData) ||
        bytes == nullptr ||
        byteSize == 0u) {
        return nullptr;
    }

    Stage1SceneSubmitTodCacheEntry801CBFDC& cache =
        runtimeState.todCacheByHandle[handle];
    if (cache.bytes != bytes ||
        cache.byteSize != byteSize) {
        cache = Stage1SceneSubmitTodCacheEntry801CBFDC{};
        cache.bytes = bytes;
        cache.byteSize = byteSize;
        if (!TodParser::Parse(bytes, byteSize, cache.tod)) {
            cache = Stage1SceneSubmitTodCacheEntry801CBFDC{};
            return nullptr;
        }
        cache.psxSourceBaseToken8001AFD8 =
            AllocatePsxTodSourceBaseToken8001AFD8(runtimeState,
                                                  byteSize);
    }

    return &cache.tod;
}

static int32_t PsxTodBlockCount801CBFDC(const TodData* tod) {
    if (tod == nullptr) {
        return 0;
    }
    return static_cast<int32_t>(tod->rawBlockCount);
}

static uint32_t PsxCall8001AFD8_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint8_t sourceHandle,
    uint32_t cursorOutAddr,
    uint32_t countOutAddr,
    uint32_t startFrame,
    const TodData* tod) {
    int32_t* countOut = ResolvePsxCountAddr801CBFDC(runtimeState,
                                                    countOutAddr);
    if (countOut == nullptr) {
        return 0u;
    }

    const int32_t count = PsxTodBlockCount801CBFDC(tod);
    *countOut = count;
    if (sourceHandle == 0u || tod == nullptr || tod->blocks.empty()) {
        StorePsxCursorCell801CBFDC(runtimeState, cursorOutAddr, 0u);
        ClearPsxTodCursorSource8001B000(runtimeState, cursorOutAddr, false);
        return 0u;
    }

    RegisterPsxTodCursorSource8001B000(runtimeState,
                                       cursorOutAddr,
                                       sourceHandle,
                                       startFrame,
                                       tod);
    PsxTodCursorSource8001B000& source =
        EnsurePsxTodCursorSource8001B000(runtimeState, cursorOutAddr);
    source.psxSourceBaseToken8001AFD8 =
        ResolvePsxTodSourceBaseToken8001AFD8(runtimeState,
                                             sourceHandle,
                                             tod);
    source.cursorValue8001B000 =
        MakePsxTodCursorValue8001B000(source);
    StorePsxCursorCell801CBFDC(runtimeState,
                               cursorOutAddr,
                               source.cursorValue8001B000);
    return source.cursorValue8001B000;
}

static void ApplyPrStageRunnerListSlotF4_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxPrStageRunnerCtx801CBFDC& ctx,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData) {
    const uint8_t handle = static_cast<uint8_t>(ctx.listSlotF4 & 0xFFu);
    if ((ctx.flags_00 & 0x00040000u) != 0u) {
        const TodData* tod =
            ResolveStage1TodSource801CBFDC(runtimeState,
                                           resolveResource,
                                           resourceUserData,
                                           handle);
        PsxCall8001AFD8_801CBFDC(runtimeState,
                                 handle,
                                 kGlobalPaCursor3024,
                                 kGlobalPaCountB338,
                                 ctx.listSlotF4StartFrame,
                                 tod);
        runtimeState.psx801CBFDC.dword_801D3028 = 0u;
        return;
    }

    if (ctx.listSlotF4 != 0u) {
        const TodData* tod =
            ResolveStage1TodSource801CBFDC(runtimeState,
                                           resolveResource,
                                           resourceUserData,
                                           handle);
        RegisterPsxTodCursorSource8001B000(runtimeState,
                                           kGlobalPaCursor3024,
                                           handle,
                                           ctx.listSlotF4StartFrame,
                                           tod);
    }
}

static void ApplyPrStageRunnerListSlotF8_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxPrStageRunnerCtx801CBFDC& ctx,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData) {
    const uint8_t handle = static_cast<uint8_t>(ctx.listSlotF8 & 0xFFu);
    if ((ctx.flags_00 & 0x00080000u) != 0u) {
        const TodData* tod =
            ResolveStage1TodSource801CBFDC(runtimeState,
                                           resolveResource,
                                           resourceUserData,
                                           handle);
        PsxCall8001AFD8_801CBFDC(runtimeState,
                                 handle,
                                 kGlobalOnCursor301C,
                                 kGlobalOnCountB2D4,
                                 ctx.listSlotF8StartFrame,
                                 tod);
        runtimeState.psx801CBFDC.dword_801D3020 = 0u;
        return;
    }

    if (ctx.listSlotF8 != 0u) {
        const TodData* tod =
            ResolveStage1TodSource801CBFDC(runtimeState,
                                           resolveResource,
                                           resourceUserData,
                                           handle);
        RegisterPsxTodCursorSource8001B000(runtimeState,
                                           kGlobalOnCursor301C,
                                           handle,
                                           ctx.listSlotF8StartFrame,
                                           tod);
    }
}

static void ApplyPrStageRunnerDynamicRowBlockE_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxPrStageRunnerCtx801CBFDC& ctx,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData) {
    const bool reloadDynamicRows = (ctx.flags_00 & 0x00002000u) != 0u;

    if (reloadDynamicRows) {
        std::array<uint8_t, PrStageSceneSubmitConstants::kDynamicRowSlotCount>
            handles{};
        for (std::size_t i = 0; i < handles.size(); ++i) {
            handles[i] =
                static_cast<uint8_t>(ctx.uiListHandles_134[i] & 0xFFu);
        }
        for (size_t i = 0;
             i < PrStageSceneSubmitConstants::kDynamicRowSlotCount;
             ++i) {
            const uint8_t handle = handles[i];
            const TodData* tod =
                ResolveStage1TodSource801CBFDC(runtimeState,
                                               resolveResource,
                                               resourceUserData,
                                               handle);
            PsxCall8001AFD8_801CBFDC(runtimeState,
                                     handle,
                                     kDynamicRowCursorAddrs801CBFDC[i],
                                     kGlobalUiListInitCounter7F64,
                                     ctx.uiListHandles134StartFrame,
                                     tod);
        }
        if (ctx.ctx72_72 >= 2) {
            auto& g = runtimeState.psx801CBFDC;
            g.g_PrStageRunner_UIRender_RefreshRequest = true;
            g.g_PrStageRunner_UIRender_Block2 = false;
        }
    }

    auto& g = runtimeState.psx801CBFDC;
    if ((ctx.flags_00 & 0x4040u) != 0u) {
        g.g_PrStageRunner_UIRender_Block2 = false;
    }
}

static void StoreStage1BackdropCameraFromPresentation801CB6BC(
    Stage1SceneSubmitRuntime::Impl& runtimeState) {
    const PsxCameraState801CBFDC& camera =
        runtimeState.cameraPresentationState801CBFDC;
    if (!camera.valid) {
        return;
    }
    for (size_t i = 0; i < 3u; ++i) {
        runtimeState.backdropCamera.pos[i] = camera.pos[i];
        runtimeState.backdropCamera.target[i] = camera.target[i];
    }
    runtimeState.projection.projectionDistance =
        camera.projectionDistance;
    runtimeState.projection.screenCenterX = camera.screenCenterX;
    runtimeState.projection.screenCenterY = camera.screenCenterY;
}

static void ApplyStage1SceneInitGateReset801CB6BC(
    Stage1SceneSubmitRuntime::Impl& runtimeState) {
    auto& g = runtimeState.psx801CBFDC;
    g.dword_801D3034 = 0u;
    g.g_PrStageRunner_UIRender_RefreshRequest = false;
    g.g_PrStageRunner_UIRender_Block2 = false;
    g.g_PrStageRunner_UIList_Block = false;
    PrPsxEventFrameDirect::SeedStage1Event4Gp38C8006EDCC(1u);
    g.g_PrStageRunner_UIListB_Active = false;
    g.g_PrStageRunner_UIListC_Active = false;
    g.g_PrStageRunner_UIListD_Active = false;
    g.g_PrStageRunner_UIListA_Active = false;
}

static void PsxCall8001AFD8SceneInit801CB6BC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData,
    uint8_t handle,
    uint32_t cursorOutAddr,
    uint32_t countOutAddr) {
    const TodData* tod = nullptr;
    if (resolveResource != nullptr) {
        tod = ResolveStage1TodSource801CBFDC(runtimeState,
                                             resolveResource,
                                             resourceUserData,
                                             handle);
    }
    PsxCall8001AFD8_801CBFDC(runtimeState,
                             handle,
                             cursorOutAddr,
                             countOutAddr,
                             0u,
                             tod);
}

static void InitializeStage1DescBankCursors801CB6BC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData) {
    auto& g = runtimeState.psx801CBFDC;

    PsxCall8001AFD8SceneInit801CB6BC(
        runtimeState,
        resolveResource,
        resourceUserData,
        kStage1DescBankLifecycleHandles801CBFDC[1u][0u],
        kStage1DescBankLifecycleCursorAddrs801CBFDC[1u][0u],
        kGlobalUiListDItemCount7D70);
    PsxCall8001AFD8SceneInit801CB6BC(
        runtimeState,
        resolveResource,
        resourceUserData,
        kStage1DescBankLifecycleHandles801CBFDC[0u][0u],
        kStage1DescBankLifecycleCursorAddrs801CBFDC[0u][0u],
        kGlobalUiListCCount7D74);
    PsxCall8001AFD8SceneInit801CB6BC(
        runtimeState,
        resolveResource,
        resourceUserData,
        kStage1DescBankVisibleHandles801CBFDC[2u][0u],
        kStage1DescBankVisibleCursorAddrs801CBFDC[2u][0u],
        kGlobalUiListBVisibleCount7D78);
    PsxCall8001AFD8SceneInit801CB6BC(
        runtimeState,
        resolveResource,
        resourceUserData,
        kStage1DescBankLifecycleHandles801CBFDC[3u][0u],
        kStage1DescBankLifecycleCursorAddrs801CBFDC[3u][0u],
        kGlobalUiListAItemCount7D7C);
    PsxCall8001AFD8SceneInit801CB6BC(
        runtimeState,
        resolveResource,
        resourceUserData,
        kStage1DescBankLifecycleHandles801CBFDC[3u][1u],
        kStage1DescBankLifecycleCursorAddrs801CBFDC[3u][1u],
        kGlobalUiListAVisibleCount7D80);

    g.dword_801D7D5C =
        LoadPsxCursorCell801CBFDC(runtimeState,
                                  kGlobalUiListDSource7D48);
    g.dword_801D7D84 = g.g_PrStageRunner_UIListD_ItemCount;
    g.dword_801D7D60 =
        LoadPsxCursorCell801CBFDC(runtimeState,
                                  kGlobalUiListCSource7D4C);
    g.dword_801D7D88 = g.dword_801D7D74;
    g.dword_801D7D64 =
        LoadPsxCursorCell801CBFDC(runtimeState,
                                  kGlobalUiListBVisibleSource7D50);
    g.dword_801D7D8C = g.dword_801D7D78;
    g.dword_801D7D68 =
        LoadPsxCursorCell801CBFDC(runtimeState,
                                  kGlobalUiListASourceKamonL7D54);
    g.dword_801D7D90 = g.g_PrStageRunner_UIListA_ItemCount;
    g.dword_801D7D6C =
        LoadPsxCursorCell801CBFDC(runtimeState,
                                  kGlobalUiListASourceKamonR7D58);
    g.dword_801D7D94 = g.dword_801D7D80;

    PsxCall8001AFD8SceneInit801CB6BC(
        runtimeState,
        resolveResource,
        resourceUserData,
        kStage1DescBankVisibleHandles801CBFDC[0u][0u],
        kStage1DescBankVisibleCursorAddrs801CBFDC[0u][0u],
        kGlobalUiListCItemCount5AEC);
    g.dword_801D5AE8 =
        LoadPsxCursorCell801CBFDC(runtimeState,
                                  kGlobalUiListCVisibleSource5AE4);
    g.dword_801D5AF0 = g.g_PrStageRunner_UIListC_ItemCount;

    PsxCall8001AFD8SceneInit801CB6BC(
        runtimeState,
        resolveResource,
        resourceUserData,
        kStage1DescBankLifecycleHandles801CBFDC[2u][0u],
        kStage1DescBankLifecycleCursorAddrs801CBFDC[2u][0u],
        kGlobalUiListBItemCount7E48);
    PsxCall8001AFD8SceneInit801CB6BC(
        runtimeState,
        resolveResource,
        resourceUserData,
        kStage1DescBankLifecycleHandles801CBFDC[2u][1u],
        kStage1DescBankLifecycleCursorAddrs801CBFDC[2u][1u],
        kGlobalUiListBItemCount7E48);
    g.dword_801D7E40 =
        LoadPsxCursorCell801CBFDC(runtimeState,
                                  kGlobalUiListBSourceMadoL7E38);
    g.dword_801D7E44 =
        LoadPsxCursorCell801CBFDC(runtimeState,
                                  kGlobalUiListBSourceMadoR7E3C);
    g.dword_801D7E4C = g.g_PrStageRunner_UIListB_ItemCount;
}

static void ApplyStage1InitialFaceTimUploads801CB6BC(
    Stage1SceneSubmitRuntime::Impl& runtimeState);

static void ApplyStage1SceneInitSideEffects801CB6BC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData,
    bool publishResetCamera) {
    auto& g = runtimeState.psx801CBFDC;

    const TodData* onTod = nullptr;
    const TodData* paTod = nullptr;
    if (resolveResource != nullptr) {
        onTod = ResolveStage1TodSource801CBFDC(
            runtimeState,
            resolveResource,
            resourceUserData,
            kStage1InitialOnTodHandle801CB6BC);
        paTod = ResolveStage1TodSource801CBFDC(
            runtimeState,
            resolveResource,
            resourceUserData,
            kStage1InitialPaTodHandle801CB6BC);
    }

    ApplyStage1InitialFaceTimUploads801CB6BC(runtimeState);

    g.dword_801D3020 = 0u;
    PsxCall8001AFD8_801CBFDC(runtimeState,
                             kStage1InitialOnTodHandle801CB6BC,
                             kGlobalOnCursor301C,
                             kGlobalOnCountB2D4,
                             0u,
                             onTod);
    PsxCall8001B000_801CBFDC(runtimeState,
                             0u,
                             kGlobalOnCountB2D4,
                             kGlobalOnCursor301C,
                             kDescOn98F0);

    g.dword_801D3028 = 0u;
    PsxCall8001AFD8_801CBFDC(runtimeState,
                             kStage1InitialPaTodHandle801CB6BC,
                             kGlobalPaCursor3024,
                             kGlobalPaCountB338,
                             0u,
                             paTod);
    PsxCall8001B000_801CBFDC(runtimeState,
                             0u,
                             kGlobalPaCountB338,
                             kGlobalPaCursor3024,
                             kDescPaB2D8);

    runtimeState.paMimeBinding801CBFDC = {
        true,
        kStage1InitialPaMimeDatHandle801CB6BC,
        kStage1InitialPaMimeVdfHandle801CB6BC,
    };
    runtimeState.primaryMimeCursorValid800141D8 = true;
    runtimeState.primaryMimeCursor20 = 0;
    runtimeState.primaryMimeEnd24 = 999;
    runtimeState.mimeSampleState80090240[0] = {};
    PsxCall800141D8_801CB190(runtimeState);

    InitializeStage1DescBankCursors801CB6BC(runtimeState,
                                            resolveResource,
                                            resourceUserData);

    runtimeState.onMimeBinding801CBFDC = {
        true,
        kStage1InitialOnMimeDatHandle801CB6BC,
        kStage1InitialOnMimeVdfHandle801CB6BC,
    };
    runtimeState.mimeSampleState80090240[1] = {};
    g.dword_801D3018 = 0u;
    PsxCall80013EA8_801CB190(runtimeState,
                             kGlobalOnMimeTimelineB0C4,
                             0u,
                             1u);

    if (resolveResource != nullptr) {
        const bool cameraLoaded =
            PsxCall800127F0_801CBFDC(runtimeState,
                                     kStage1InitialCameraBezId801CB6BC,
                                     0u,
                                     true,
                                     resolveResource,
                                     resourceUserData);
        if (publishResetCamera) {
            StoreStage1BackdropCameraFromPresentation801CB6BC(runtimeState);
        } else if (cameraLoaded) {
            runtimeState.cameraPresentationState801CBFDC =
                MakeCameraFromBezStartSample801CBFDC(
                    runtimeState.cameraBezPlayer801CBFDC);
            StoreStage1BackdropCameraFromPresentation801CB6BC(runtimeState);
        }
    }
}

static void InitializeStage1DescAndCoordBank801CB6BC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData,
    bool publishResetCamera) {
    const bool initializeCoordBank =
        !runtimeState.coordBankInitialized801CB6BC;
    if (initializeCoordBank) {
        ApplyStage1SceneInitGateReset801CB6BC(runtimeState);
        ClearStage1SceneInitCursorSources801CB6BC(runtimeState);
        for (const Stage1DrawDescInit801CB6BC& init :
             kStage1DrawDescInits801CB6BC) {
            PsxCall8004049C_801CB6BC(runtimeState, 0u, init.coordNodeAddr);
        }
        if (!runtimeState.descStaticBindingsInitialized801CB6BC) {
            for (const Stage1DrawDescInit801CB6BC& init :
                 kStage1DrawDescInits801CB6BC) {
                PsxCall8001AF1C_801CB6BC(runtimeState,
                                         init.resourceSlotAddr,
                                         init.descAddr,
                                         init.coordNodeAddr,
                                         init.objectCount8001AF1C);
            }
            runtimeState.descStaticBindingsInitialized801CB6BC = true;
        }
    }

    if (initializeCoordBank) {
        PsxCall8004049C_801CB6BC(
            runtimeState, kCoordKabeB31F4, kCoordMadoL7D98);
        PsxCall8004049C_801CB6BC(
            runtimeState, kCoordKabeB31F4, kCoordMadoR7DE8);
        PsxCall8004049C_801CB6BC(
            runtimeState, kCoordKabeB31F4, kCoordKamonLeft7E50);
        PsxCall8004049C_801CB6BC(
            runtimeState, kCoordKabeB31F4, kCoordKamonRight7EA0);
        PsxCall8004049C_801CB6BC(
            runtimeState, kCoordKamonLeft7E50, kCoordNinL7C44);
        PsxCall8004049C_801CB6BC(
            runtimeState, kCoordKamonRight7EA0, kCoordNinR7C94);
        PsxCall80040C74_801CB6BC(
            runtimeState,
            static_cast<uint32_t>(kStage1ProjectionH801CB6BC));
        ApplyStage1SceneInitSideEffects801CB6BC(runtimeState,
                                                resolveResource,
                                                resourceUserData,
                                                publishResetCamera);
        runtimeState.coordBankInitialized801CB6BC = true;
    }

    for (const PsxDrawDescState& state : runtimeState.localDescStates) {
        if (!state.coordValid || state.coordNodeAddr == 0u) {
            continue;
        }
        PsxCoordNode& node =
            EnsureCoordNode801CB6BC(runtimeState, state.coordNodeAddr);
        node.local = state.coord;
    }
}

static void ApplyPrStageRunnerUIListSeedBlockC_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxPrStageRunnerCtx801CBFDC& ctx) {
    auto& g = runtimeState.psx801CBFDC;
    const bool seedDescBank = (ctx.flags_00 & 0x00800000u) != 0u;
    if (seedDescBank && ctx.uiListSeedKind_7C == 4) {
        g.g_PrStageRunner_UIListA_Seq = 0u;
        g.g_PrStageRunner_UIListA_Active = true;
        g.dword_801D7D54 = g.dword_801D7D68;
        g.dword_801D7D58 = g.dword_801D7D6C;
        g.g_PrStageRunner_UIListA_ItemCount = g.dword_801D7D90;
        g.dword_801D7D80 = g.dword_801D7D94;
    }
    if (seedDescBank && ctx.uiListSeedKind_7C == 3) {
        g.g_PrStageRunner_UIListB_Seq = 0u;
        g.g_PrStageRunner_UIListB_Active = true;
        g.dword_801D7E38 = g.dword_801D7E40;
        g.dword_801D7E3C = g.dword_801D7E44;
        g.g_PrStageRunner_UIListB_ItemCount = g.dword_801D7E4C;
        g.dword_801D7D50 = g.dword_801D7D64;
        g.dword_801D7D78 = g.dword_801D7D8C;
    }
    if (seedDescBank && ctx.uiListSeedKind_7C == 1) {
        g.g_PrStageRunner_UIListC_Seq = 0u;
        g.g_PrStageRunner_UIListC_Active = true;
        g.dword_801D5AE4 = g.dword_801D5AE8;
        g.g_PrStageRunner_UIListC_ItemCount = g.dword_801D5AF0;
        g.dword_801D7D4C = g.dword_801D7D60;
        g.dword_801D7D74 = g.dword_801D7D88;
    }
    if (seedDescBank && ctx.uiListSeedKind_7C == 2) {
        g.g_PrStageRunner_UIListD_Seq = 0u;
        g.g_PrStageRunner_UIListD_Active = true;
        g.dword_801D7D48 = g.dword_801D7D5C;
        g.g_PrStageRunner_UIListD_ItemCount = g.dword_801D7D84;
    }
}

static void ApplyPrStageRunnerRenderGlobalsBlockG_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxPrStageRunnerCtx801CBFDC& ctx,
    uint8_t renderSubFrame8,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData) {
    auto& g = runtimeState.psx801CBFDC;
    g.g_PrStageRunner_UIRender_Block = ctx.uiRenderBlock_4E != 0;
    g.dword_801D3014 = ctx.tamaSelector_FC;
    if ((ctx.flags_00 & 0x00000400u) != 0u) {
        const int16_t cameraBezId =
            static_cast<int16_t>(
                static_cast<uint16_t>(ctx.cameraBezHandle104 & 0xFFFFu));
        g.dword_801D302C = 0u;
        PsxCall800127F0_801CBFDC(runtimeState,
                                 cameraBezId,
                                 ctx.cameraSourceEventPsxAddr,
                                 false,
                                 resolveResource,
                                 resourceUserData);
    }
    if (g.dword_801D3034 != 0u) {
        const bool logicFrame = renderSubFrame8 == 0u;
        const uint32_t tickBeforeAdvance =
            (logicFrame || g.dword_801D302C == 0u) ? g.dword_801D302C
                                                    : (g.dword_801D302C - 1u);
        if (logicFrame) {
            ++g.dword_801D302C;
            runtimeState.cameraAdvanceFrameValid801CBFDC = true;
            runtimeState.lastCameraQueryFrame801CBFDC = ctx.queryFrame;
        }
        PsxCall80012960_801CBFDC(
            runtimeState,
            static_cast<int32_t>(tickBeforeAdvance),
            renderSubFrame8);
        StoreStage1BackdropCameraFromPresentation801CB6BC(runtimeState);
    }
    runtimeState.backdropExtra4CBC =
        g.g_PrStageRunner_UIRender_Block2;
}

static void SyncRuntimeFromPrStageRunnerGlobals801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState) {
    const auto& g = runtimeState.psx801CBFDC;
    runtimeState.backdropExtra4CBC =
        g.g_PrStageRunner_UIRender_Block2;
}

static void PsxCall800140E0_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t datHandle,
    uint32_t vdfHandle,
    uint32_t,
    int16_t rangeBegin,
    int16_t rangeLen) {
    runtimeState.primaryMimeCursorValid800141D8 = true;
    runtimeState.primaryMimeCursor20 =
        static_cast<int32_t>(rangeBegin);
    runtimeState.primaryMimeEnd24 =
        static_cast<int32_t>(rangeBegin) + static_cast<int32_t>(rangeLen) - 1;
    runtimeState.paMimeBinding801CBFDC = {
        datHandle != 0u && vdfHandle != 0u,
        static_cast<uint8_t>(datHandle & 0xFFu),
        static_cast<uint8_t>(vdfHandle & 0xFFu),
    };
    runtimeState.mimeSampleState80090240[0] = {};
}

static void PsxCall80014050_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    int group,
    uint32_t datHandle,
    uint32_t vdfHandle,
    uint32_t,
    uint32_t,
    uint32_t* frameCounter) {
    if (frameCounter != nullptr) {
        *frameCounter = 0u;
    }
    runtimeState.onMimeBinding801CBFDC = {
        datHandle != 0u && vdfHandle != 0u,
        static_cast<uint8_t>(datHandle & 0xFFu),
        static_cast<uint8_t>(vdfHandle & 0xFFu),
    };
    if (group >= 0 &&
        static_cast<size_t>(group) <
            runtimeState.mimeSampleState80090240.size()) {
        runtimeState.mimeSampleState80090240[static_cast<size_t>(group)] = {};
    }
}

static void ApplyPrStageRunnerResourceBlockD_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxPrStageRunnerCtx801CBFDC& psxCtx,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData) {
    ApplyPrStageRunnerListSlotF4_801CBFDC(runtimeState,
                                          psxCtx,
                                          resolveResource,
                                          resourceUserData);
    if ((psxCtx.flags_00 & 0x00000010u) != 0u) {
        PsxCall800140E0_801CBFDC(
            runtimeState,
            psxCtx.paDatHandle_DC,
            psxCtx.paVdfHandle_E8,
            0u,
            psxCtx.todStart_11C,
            psxCtx.todEnd_120);
    } else if ((psxCtx.flags_00 & 0x00010000u) != 0u) {
        PsxCall800140E0_801CBFDC(runtimeState,
                                 psxCtx.paDatHandle_DC,
                                 psxCtx.paVdfHandle_E8,
                                 0u,
                                 0,
                                 999);
    }
    ApplyPrStageRunnerListSlotF8_801CBFDC(runtimeState,
                                          psxCtx,
                                          resolveResource,
                                          resourceUserData);
    if ((psxCtx.flags_00 & 0x00020000u) != 0u &&
        psxCtx.onDatHandle_E0 != 0u) {
        PsxCall80014050_801CBFDC(runtimeState,
                                 1,
                                 psxCtx.onDatHandle_E0,
                                 psxCtx.onVdfHandle_EC,
                                 kGlobalOnMimeTimelineB0C4,
                                 0x801DB0D4u,
                                 &runtimeState.psx801CBFDC.dword_801D3018);
    }
}

static void PsxCall8001ADEC_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint16_t timId,
    bool uploadClut,
    uint8_t channel) {
    if (timId == 0u) {
        return;
    }

    PsxTimUploadCommand801CBFDC command{};
    command.channel = channel;
    command.timId = timId;
    command.uploadClut = uploadClut;
    runtimeState.timUploadCommands801CBFDC.push_back(command);
}

static void ApplyStage1InitialFaceTimUploads801CB6BC(
    Stage1SceneSubmitRuntime::Impl& runtimeState) {
    for (uint16_t timId : kStage1InitialFaceTimHandles801D2FC0) {
        PsxCall8001ADEC_801CBFDC(runtimeState, timId, true, 0u);
    }
}

static bool TryBuildCompactRailFastSpriteRuntimeOwner8003FA20(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& out);

static void ApplyScriptBoxGlyphLoop8001C6A0_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    PsxScriptBoxCommand80023E10& command);

static void ApplyScriptBoxRectBoxFills8001C4EC_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxScriptBoxCommand80023E10& command);

static void ApplyScriptBoxCornerFastSprites8001C550_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxScriptBoxCommand80023E10& command);

static void PsxCall80023E10_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const char* textPtr10C) {
    PsxScriptBoxCommand80023E10 command{};
    command.textPtr10C = textPtr10C;
    command.textPtr10CKnown = true;
    command.maxGlyphs = 480u;
    command.workListSlotKnown = true;
    command.workListSlot =
        static_cast<uint8_t>(runtimeState.psxWorkSlotIndex8006EDA8 & 1u);
    command.workListBaseAddr = 0x80087288u;
    command.workListStrideBytes = 20u;
    command.workListAddrKnown = true;
    command.workListAddr =
        command.workListBaseAddr +
        static_cast<uint32_t>(command.workListSlot) *
            command.workListStrideBytes;
    ApplyScriptBoxRectBoxFills8001C4EC_801CBFDC(runtimeState, command);
    ApplyScriptBoxCornerFastSprites8001C550_801CBFDC(runtimeState, command);
    ApplyScriptBoxGlyphLoop8001C6A0_801CBFDC(runtimeState, command);
    runtimeState.scriptBoxCommands80023E10.push_back(command);
}

static void PsxCall8001E2E4_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxPrStageRunnerCtx801CBFDC& psxCtx,
    int16_t layoutArg68) {
    PsxNumericStatusCommand8001E2E4 command{};
    command.layoutArg68 = layoutArg68;
    command.uiRenderBlock4E = psxCtx.uiRenderBlock_4E;
    command.callGate64 = psxCtx.tailGate64;
    runtimeState.numericStatusCommands8001E2E4.push_back(command);
}

static bool TryBuildCompactRailFastSpriteRuntimeOwner8003FA20(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& out) {
    out = PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20{};
    if (!runtimeState.graphOwner801CBFDC.mainPageWorkLists80087288Initialized) {
        return false;
    }

    const uint8_t pageIndex = runtimeState.psxWorkSlotIndex8006EDA8 & 1u;
    const PrPsxGraphOwnerDirect::PsxGraphPageWorkList8001E374& pageWork =
        runtimeState.graphOwner801CBFDC.mainPageWorkLists80087288[pageIndex];
    return PrPsxGraphOwnerDirect::BuildRuntimeState8003FA20FromPageWork(
        runtimeState.graphOwner801CBFDC,
        pageWork,
        out);
}

static uint32_t PackScriptBox16Pair8003EE84(int32_t lo, int32_t hi) {
    return static_cast<uint32_t>(static_cast<uint16_t>(lo)) |
           (static_cast<uint32_t>(static_cast<uint16_t>(hi)) << 16);
}

static uint32_t MakeScriptBoxPacketLinkTag8003EF5C(
    uint32_t oldOtSlotValue,
    uint32_t payloadWordCount) {
    return (oldOtSlotValue & kPsxPacketAddrMask8003EF5C) |
           ((payloadWordCount & 0xFFu) << 24);
}

static uint32_t MakeScriptBoxBoxFillDrawModeWord8003EE84(uint32_t attr) {
    return ((attr >> 17) & 0x180u) | ((attr >> 23) & 0x60u) |
           0xE1000200u;
}

static uint32_t MakeScriptBoxBoxFillColorCodeWord8003EE84(uint32_t attr) {
    const uint8_t code =
        static_cast<uint8_t>(((attr >> 29) & 0x02u) | 0x60u);
    return static_cast<uint32_t>((attr >> 16) & 0xFFu) |
           (static_cast<uint32_t>((attr >> 8) & 0xFFu) << 8) |
           (static_cast<uint32_t>(attr & 0xFFu) << 16) |
           (static_cast<uint32_t>(code) << 24);
}

static bool CanSetScriptBoxRuntimeOtSlot8003FA20(
    const PrPsxFastSpriteSubmitDirect::RuntimeOrderingTableState8003FA20& ot,
    uint32_t slotAddr) {
    for (const PrPsxFastSpriteSubmitDirect::RuntimeOtSlotValue8003FA20& slot :
         ot.slotValues) {
        if (slot.valid && slot.addr == slotAddr) {
            return true;
        }
    }
    for (const PrPsxFastSpriteSubmitDirect::RuntimeOtSlotValue8003FA20& slot :
         ot.slotValues) {
        if (!slot.valid) {
            return true;
        }
    }
    return false;
}

static PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20*
FindScriptBoxRuntimePacketSlot8003FA20(
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime) {
    for (PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& slot :
         runtime.packetWrites) {
        if (!slot.valid) {
            return &slot;
        }
    }
    return nullptr;
}

static bool AppendScriptBoxBoxFillPacket8003EE84(
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime,
    const PrStage1ScriptBoxDirect::ScriptBoxRectSub80023E10& rect,
    uint32_t psxCallOrder) {
    const uint32_t attrHigh = rect.attr & 0xFF000000u;
    if (static_cast<int32_t>(attrHigh) < 0 || rect.w == 0u ||
        rect.h == 0u) {
        return false;
    }

    const uint32_t otSlotAddr =
        PrPsxFastSpriteSubmitDirect::ResolveRuntimeOtSlotAddr8003FA20(
            runtime.ot,
            rect.priority);
    if (!CanSetScriptBoxRuntimeOtSlot8003FA20(runtime.ot, otSlotAddr)) {
        return false;
    }

    PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20* packetSlot =
        FindScriptBoxRuntimePacketSlot8003FA20(runtime);
    if (packetSlot == nullptr) {
        return false;
    }

    const uint32_t oldOtSlotValue =
        PrPsxFastSpriteSubmitDirect::GetRuntimeOtSlotOldValue8003FA20(
            runtime.ot,
            otSlotAddr);
    const uint32_t packetAddr = runtime.dword_800901C8;
    const int32_t localX = static_cast<int32_t>(rect.x) - 160;
    const int32_t localY = static_cast<int32_t>(rect.y) - 120;

    PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20 write{};
    write.valid = true;
    write.addr = packetAddr;
    write.wordCount = kBoxFillPacketTotalWords8003EE84;
    write.words = {{
        MakeScriptBoxPacketLinkTag8003EF5C(
            oldOtSlotValue,
            kBoxFillPacketPayloadWords8003EE84),
        MakeScriptBoxBoxFillDrawModeWord8003EE84(attrHigh),
        MakeScriptBoxBoxFillColorCodeWord8003EE84(rect.attr),
        PackScriptBox16Pair8003EE84(
            localX + runtime.word_800917AA,
            localY + runtime.word_800917AC),
        PackScriptBox16Pair8003EE84(rect.w, rect.h),
        0u,
    }};
    write.wordKnown = {{true, true, true, true, true, false}};
    write.provenance.active = true;
    write.provenance.sourceKind =
        PrPsxFastSpriteSubmitDirect::FastSpriteSubmitSourceKind8003FA20::
            Stage1ScriptBox;
    write.provenance.sourceFunction = kFn8003EE84_GsSortBoxFill;
    write.provenance.callsite = kFn8001B718_BoxFillSubmitCallsite;
    write.provenance.helper = kFn8003EF5C_LinkPacket;
    write.provenance.psxCallOrder = psxCallOrder;
    write.provenance.priority = rect.priority;

    if (!PrPsxFastSpriteSubmitDirect::SetRuntimeOtSlotValue8003FA20(
            runtime.ot,
            otSlotAddr,
            packetAddr & kPsxPacketAddrMask8003EF5C)) {
        return false;
    }

    *packetSlot = write;
    runtime.dword_800901C8 =
        packetAddr + kBoxFillPacketAdvanceBytes8003EE84;
    return true;
}

static void ApplyScriptBoxRectBoxFills8001C4EC_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxScriptBoxCommand80023E10& command) {
    if (!command.workListSlotKnown || !command.workListAddrKnown) {
        return;
    }

    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 submitRuntime{};
    if (!TryBuildCompactRailFastSpriteRuntimeOwner8003FA20(runtimeState,
                                                           submitRuntime)) {
        return;
    }

    const PrStage1ScriptBoxDirect::ScriptBoxSub80023E10 box =
        PrStage1ScriptBoxDirect::BuildScriptBoxFrameSub80023E10(
            0u,
            command.textPtr10C,
            command.maxGlyphs,
            command.workListSlotKnown,
            command.workListSlot,
            command.workListBaseAddr,
            command.workListStrideBytes);
    if (!box.valid) {
        return;
    }

    bool appliedAny = false;
    uint32_t psxCallOrder = 1u;
    for (const PrStage1ScriptBoxDirect::ScriptBoxRectSub80023E10& rect :
         box.rects) {
        if (!rect.valid) {
            continue;
        }
        appliedAny =
            AppendScriptBoxBoxFillPacket8003EE84(submitRuntime,
                                                 rect,
                                                 psxCallOrder++) ||
            appliedAny;
    }
    if (!appliedAny) {
        return;
    }

    const PrPsxGraphOwnerDirect::PsxFastSpriteRuntimeCommitResult8003FA20
        commit =
            PrPsxGraphOwnerDirect::CommitRuntimeState8003FA20ToMainPageWork(
                runtimeState.graphOwner801CBFDC,
                runtimeState.psxWorkSlotIndex8006EDA8,
                submitRuntime);
    if (commit.committed && commit.allocatorUpdated) {
        runtimeState.currentPacketAllocator800901C8 = commit.allocatorAfter;
    }
}

static PrPsxFastSpriteSubmitDirect::FastSpriteRawLocalBytes8003FA20
BuildScriptBoxCornerRawLocalBytes8001B590(
    const PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20& local) {
    PrPsxFastSpriteSubmitDirect::FastSpriteRawLocalBytes8003FA20 stackSeed{};
    stackSeed.active = true;
    stackSeed.provenance =
        PrPsxFastSpriteSubmitDirect::FastSpriteRawLocalProvenance8003FA20::
            Stage1ScriptBoxCornerStackScratch8001B590;
    stackSeed.sourceFunction = 0x8001B590u;
    stackSeed.callsite = 0x8001B5D4u;
    stackSeed.localObjectAddr = 0x801FFE60u;
    stackSeed.lastWriterPc = 0x801CA644u;
    stackSeed.note =
        "v3 recorder frame 1780: 80023E10 corner path consumes "
        "local+0x14..0x16 bytes 1C 7B 1D before 8003FA20 word2";
    stackSeed.bytes[PrPsxFastSpriteSubmitDirect::kFastSpriteLocalRgbR8003FA20] =
        0x1Cu;
    stackSeed.bytes[PrPsxFastSpriteSubmitDirect::kFastSpriteLocalRgbG8003FA20] =
        0x7Bu;
    stackSeed.bytes[PrPsxFastSpriteSubmitDirect::kFastSpriteLocalRgbB8003FA20] =
        0x1Du;
    stackSeed.known[PrPsxFastSpriteSubmitDirect::kFastSpriteLocalRgbR8003FA20] =
        true;
    stackSeed.known[PrPsxFastSpriteSubmitDirect::kFastSpriteLocalRgbG8003FA20] =
        true;
    stackSeed.known[PrPsxFastSpriteSubmitDirect::kFastSpriteLocalRgbB8003FA20] =
        true;
    return PrPsxFastSpriteSubmitDirect::BuildRawLocalBytesFromKnownFields8003FA20(
        local,
        stackSeed,
        0x8001B590u,
        0x8001B5D4u,
        stackSeed.note);
}

static void ApplyScriptBoxCornerFastSprites8001C550_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxScriptBoxCommand80023E10& command) {
    if (!command.workListSlotKnown || !command.workListAddrKnown) {
        return;
    }

    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 submitRuntime{};
    if (!TryBuildCompactRailFastSpriteRuntimeOwner8003FA20(runtimeState,
                                                           submitRuntime)) {
        return;
    }

    const PrStage1ScriptBoxDirect::ScriptBoxSub80023E10 box =
        PrStage1ScriptBoxDirect::BuildScriptBoxFrameSub80023E10(
            0u,
            command.textPtr10C,
            command.maxGlyphs,
            command.workListSlotKnown,
            command.workListSlot,
            command.workListBaseAddr,
            command.workListStrideBytes);
    if (!box.valid) {
        return;
    }

    bool appliedAny = false;
    for (const PrStage1ScriptBoxDirect::ScriptBoxCornerSub80023E10& corner :
         box.corners) {
        if (!corner.valid || corner.templateAddr == 0u) {
            continue;
        }

        PrStage1Scene1Movie1Direct::Movie1PsxRawDrawAction action =
            PrStage1Scene1Movie1Direct::BuildRawDrawSubmitActionSub8001B590(
                0x80023E10u,
                corner.x,
                corner.y,
                corner.templateAddr,
                0u,
                false,
                corner.priority,
                command.workListAddr,
                command.workListSlot,
                command.workListBaseAddr);
        if (!action.b590.valid) {
            continue;
        }

        const PrPsxFastSpriteSubmitDirect::FastSpriteLocalFields8003FA20
            local =
                PrStage1Scene1Movie1Direct::BuildRawDrawFastSpriteLocalSub8001B590(
                    action.b590);
        const PrPsxFastSpriteSubmitDirect::FastSpriteRawLocalBytes8003FA20
            raw = BuildScriptBoxCornerRawLocalBytes8001B590(local);
        if (!PrPsxFastSpriteSubmitDirect::HasKnownRgbTail8003FA20(raw)) {
            continue;
        }

        PrPsxFastSpriteSubmitDirect::GsSortFastSpriteInput8003FA20 input =
            PrPsxFastSpriteSubmitDirect::BuildInputFromRuntime8003FA20(
                submitRuntime,
                local,
                raw,
                corner.priority);
        input.provenance.active = true;
        input.provenance.sourceKind =
            PrPsxFastSpriteSubmitDirect::
                FastSpriteSubmitSourceKind8003FA20::Stage1ScriptBox;
        input.provenance.rawLocalProvenance = raw.provenance;
        input.provenance.sourceFunction = 0x80023E10u;
        input.provenance.callsite = 0x8001B5D4u;
        input.provenance.helper = 0x8001C550u;
        input.provenance.psxCallOrder = corner.callOrder + 1u;
        input.provenance.localObjectAddr = raw.localObjectAddr;
        input.provenance.lastWriterPc = raw.lastWriterPc;

        const PrPsxFastSpriteSubmitDirect::GsSortFastSpriteResult8003FA20
            callResult =
                PrPsxFastSpriteSubmitDirect::PsxCall8003FA20_GsSortFastSprite(
                    input);
        const PrPsxFastSpriteSubmitDirect::RuntimeUpdate8003FA20 update =
            PrPsxFastSpriteSubmitDirect::ApplyRuntimeUpdate8003FA20(
                submitRuntime,
                callResult);
        appliedAny = appliedAny || update.packetWriteMirrored;
    }

    if (!appliedAny) {
        return;
    }

    const PrPsxGraphOwnerDirect::PsxFastSpriteRuntimeCommitResult8003FA20
        commit =
            PrPsxGraphOwnerDirect::CommitRuntimeState8003FA20ToMainPageWork(
                runtimeState.graphOwner801CBFDC,
                runtimeState.psxWorkSlotIndex8006EDA8,
                submitRuntime);
    if (commit.committed && commit.allocatorUpdated) {
        runtimeState.currentPacketAllocator800901C8 = commit.allocatorAfter;
    }
}

static PrStage1MovieTextDirect::Movie1TextCurrentGp872WorkCarrier
BuildScriptBoxCurrentWorkCarrier8001C6A0(
    const PsxScriptBoxCommand80023E10& command) {
    PrStage1MovieTextDirect::Movie1TextCurrentGp872WorkCarrier out{};
    out.usesCurrentGp872DrawBuffer = true;
    out.workBasePsxAddr = command.workListBaseAddr;
    out.workStrideBytes = command.workListStrideBytes;
    if (command.workListSlotKnown) {
        out.gp872SlotKnown = true;
        out.gp872Slot = static_cast<uint8_t>(command.workListSlot & 1u);
        out.workAddrKnown = command.workListAddrKnown;
        out.workAddr = command.workListAddr;
        out.drawOtagAddrKnown = command.workListAddrKnown;
        out.drawOtagAddr = command.workListAddr + out.workLastAddrOffset;
    }
    return out;
}

static PrStage1MovieTextDirect::Movie1TextDrawCommand
BuildScriptBoxGlyphLoopCommand8001C6A0(
    const PsxScriptBoxCommand80023E10& command) {
    PrStage1MovieTextDirect::Movie1TextDrawCommand out{};
    out.kind =
        PrStage1MovieTextDirect::Movie1TextDrawCommandKind::
            SubmitTextFastSpriteSequenceSub8001B954;
    out.psxFunctionAddr =
        PrStage1MovieTextDirect::kMovie1TextGlyphLoopFunctionSub8001B954;
    out.fastSpriteSourceKind =
        PrPsxFastSpriteSubmitDirect::
            FastSpriteSubmitSourceKind8003FA20::Stage1ScriptBox;
    out.usesCurrentGp872DrawBuffer = true;
    out.otBufferBasePsxAddr = command.workListBaseAddr;
    out.otBufferStrideBytes = command.workListStrideBytes;
    out.x = 28u;
    out.y = 194u;
    out.z = 0u;
    out.scale = command.maxGlyphs;
    out.textPtr = command.textPtr10C;
    out.textPsxAddr = 0u;
    out.emitsGsSortFastSprite = true;
    return out;
}

static void ApplyScriptBoxGlyphLoop8001C6A0_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    PsxScriptBoxCommand80023E10& command) {
    if (command.textPtr10C == nullptr || !command.workListSlotKnown) {
        return;
    }

    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 submitRuntime{};
    command.fastSpriteRuntimeOwnerKnown8003FA20 =
        TryBuildCompactRailFastSpriteRuntimeOwner8003FA20(runtimeState,
                                                           submitRuntime);
    if (!command.fastSpriteRuntimeOwnerKnown8003FA20) {
        return;
    }

    const PrStage1MovieTextDirect::Movie1TextFastSpriteSequenceSub8001B954
        sequence =
            PrStage1MovieTextDirect::BuildTextFastSpriteSequenceSub8001B954(
                BuildScriptBoxGlyphLoopCommand8001C6A0(command),
                PrStage1MovieTextDirect::
                    Movie1TextGlyphMetricTablesSub8001B954{},
                BuildScriptBoxCurrentWorkCarrier8001C6A0(command));
    const PrStage1MovieTextDirect::
        Movie1TextFastSpriteSequenceApplyResultSub8001B954 apply =
            PrStage1MovieTextDirect::ApplyTextFastSpriteSequenceSub8001B954(
                sequence,
                submitRuntime);
    if (!apply.valid || apply.appliedSubmitCount == 0u) {
        return;
    }

    const PrPsxGraphOwnerDirect::PsxFastSpriteRuntimeCommitResult8003FA20
        commit =
            PrPsxGraphOwnerDirect::CommitRuntimeState8003FA20ToMainPageWork(
                runtimeState.graphOwner801CBFDC,
                runtimeState.psxWorkSlotIndex8006EDA8,
                submitRuntime);
    command.glyphPacketRuntimeCommitted8003FA20 = commit.committed;
    if (commit.committed && commit.allocatorUpdated) {
        runtimeState.currentPacketAllocator800901C8 = commit.allocatorAfter;
    }
}

static bool TryBuildCompactRailGsSpriteRuntimeOwner8003F1B4(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4& out) {
    out = PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4{};
    if (!runtimeState.graphOwner801CBFDC.mainPageWorkLists80087288Initialized) {
        return false;
    }

    const uint8_t pageIndex = runtimeState.psxWorkSlotIndex8006EDA8 & 1u;
    const PrPsxGraphOwnerDirect::PsxGraphPageWorkList8001E374& pageWork =
        runtimeState.graphOwner801CBFDC.mainPageWorkLists80087288[pageIndex];
    return PrPsxGraphOwnerDirect::BuildRuntimeState8003F1B4FromPageWork(
        runtimeState.graphOwner801CBFDC,
        pageWork,
        out);
}

static PrStage1CompactRail80024744Direct::PsxCompactRailInput80024744
BuildCompactRailInput80024744_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxPrStageRunnerCtx801CBFDC& psxCtx,
    bool captureSubmitRuntime) {
    PrStage1CompactRail80024744Direct::PsxCompactRailInput80024744 input{};
    input.inputCaptured = psxCtx.compactInputCaptured80024744;
    input.command.painterGate7A = psxCtx.compactPainterGate7A;
    input.command.rowCount8A = psxCtx.compactRowCount8A;
    input.command.rows = psxCtx.compactRows80024744;
    input.command.resetWobbleBanks = psxCtx.compactResetWobbleBanks80024744;
    input.command.resetTeacherWobbleBank80024308 =
        psxCtx.compactTeacherResetPulse80024308;
    input.command.resetStudentWobbleBank80024390 =
        psxCtx.compactStudentResetPulse80024390;
    input.submitRuntimeWorkIndex8004019C =
        runtimeState.psxWorkSlotIndex8006EDA8;
    if (captureSubmitRuntime) {
        input.submitRuntime8003FA20Valid =
            TryBuildCompactRailFastSpriteRuntimeOwner8003FA20(
                runtimeState,
                input.submitRuntime8003FA20);
        input.submitRuntime8003F1B4Valid =
            TryBuildCompactRailGsSpriteRuntimeOwner8003F1B4(
                runtimeState,
                input.submitRuntime8003F1B4);
    }
    return input;
}

static bool IsSameCompactRailNoteScaleCommand80024744(
    const PsxCompactRailSpriteCommand80024744& lhs,
    const PsxCompactRailSpriteCommand80024744& rhs) {
    if (lhs.kind != PsxCompactRailSpriteCommand80024744::Kind::Note ||
        rhs.kind != PsxCompactRailSpriteCommand80024744::Kind::Note) {
        return false;
    }
    return lhs.row == rhs.row &&
           lhs.slot == rhs.slot &&
           lhs.raw == rhs.raw &&
           lhs.tpl.texX_hw == rhs.tpl.texX_hw &&
           lhs.tpl.texY_px == rhs.tpl.texY_px &&
           lhs.tpl.w == rhs.tpl.w &&
           lhs.tpl.h == rhs.tpl.h &&
           lhs.tpl.clutX_px == rhs.tpl.clutX_px &&
           lhs.tpl.clutY_px == rhs.tpl.clutY_px;
}

static void BlendCompactRailRenderOnlyNoteScales80024744(
    std::vector<PsxCompactRailSpriteCommand80024744>& commands,
    const std::vector<PsxCompactRailSpriteCommand80024744>& predictedCommands,
    float renderT) {
    renderT = std::clamp(renderT, 0.0f, 1.0f);
    if (renderT <= 0.0f) {
        return;
    }

    for (PsxCompactRailSpriteCommand80024744& command : commands) {
        if (command.kind != PsxCompactRailSpriteCommand80024744::Kind::Note) {
            continue;
        }
        const auto predicted = std::find_if(
            predictedCommands.begin(),
            predictedCommands.end(),
            [&](const PsxCompactRailSpriteCommand80024744& candidate) {
                return IsSameCompactRailNoteScaleCommand80024744(command,
                                                                 candidate);
            });
        if (predicted == predictedCommands.end()) {
            continue;
        }
        command.scaleX += (predicted->scaleX - command.scaleX) * renderT;
        command.scaleY += (predicted->scaleY - command.scaleY) * renderT;
    }
}

static void PsxCall80024744_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxPrStageRunnerCtx801CBFDC& psxCtx,
    uint8_t renderSubFrame8,
    bool advanceLogicFrame) {
    if (!advanceLogicFrame) {
        if (!psxCtx.compactInputCaptured80024744 ||
            psxCtx.compactRowCount8A <= 0 ||
            runtimeState.compactRailSpriteCommands80024744.empty()) {
            return;
        }
        auto input = BuildCompactRailInput80024744_801CBFDC(
            runtimeState,
            psxCtx,
            false);
        input.command.resetWobbleBanks = false;
        input.command.resetTeacherWobbleBank80024308 = false;
        input.command.resetStudentWobbleBank80024390 = false;
        auto predictedRuntime = runtimeState.compactRailRuntime80024744;
        std::vector<PsxCompactRailCommand80024744> predictedCommandsLog;
        std::vector<PsxCompactRailSpriteCommand80024744>
            predictedSpriteCommands;
        PrStage1CompactRail80024744Direct::PsxCall80024744(
            input,
            predictedRuntime,
            predictedCommandsLog,
            predictedSpriteCommands,
            nullptr,
            nullptr);
        BlendCompactRailRenderOnlyNoteScales80024744(
            runtimeState.compactRailSpriteCommands80024744,
            predictedSpriteCommands,
            static_cast<float>(renderSubFrame8) / 255.0f);
        return;
    }

    PrStage1CompactRail80024744Direct::PsxCompactRailInput80024744 input =
        BuildCompactRailInput80024744_801CBFDC(runtimeState,
                                               psxCtx,
                                               true);
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20
        outputRuntime8003FA20{};
    PrPsxGsSpriteSubmitDirect::RuntimeState8003F1B4
        outputRuntime8003F1B4{};
    PrStage1CompactRail80024744Direct::PsxCall80024744(
        input,
        runtimeState.compactRailRuntime80024744,
        runtimeState.compactRailCommands80024744,
        runtimeState.compactRailSpriteCommands80024744,
        input.submitRuntime8003FA20Valid ? &outputRuntime8003FA20 : nullptr,
        input.submitRuntime8003F1B4Valid ? &outputRuntime8003F1B4 : nullptr);
    if (input.submitRuntime8003FA20Valid) {
        const PrPsxGraphOwnerDirect::PsxFastSpriteRuntimeCommitResult8003FA20
            commit =
                PrPsxGraphOwnerDirect::
                    CommitRuntimeState8003FA20ToMainPageWork(
                        runtimeState.graphOwner801CBFDC,
                        runtimeState.psxWorkSlotIndex8006EDA8,
                        outputRuntime8003FA20);
        if (commit.committed && commit.allocatorUpdated) {
            runtimeState.currentPacketAllocator800901C8 =
                commit.allocatorAfter;
        }
    }
    if (input.submitRuntime8003F1B4Valid) {
        const PrPsxGraphOwnerDirect::PsxGsSpriteRuntimeCommitResult8003F1B4
            commit =
                PrPsxGraphOwnerDirect::
                    CommitRuntimeState8003F1B4ToMainPageWork(
                        runtimeState.graphOwner801CBFDC,
                        runtimeState.psxWorkSlotIndex8006EDA8,
                        outputRuntime8003F1B4);
        if (commit.committed && commit.allocatorUpdated) {
            runtimeState.currentPacketAllocator800901C8 =
                commit.allocatorAfter;
        }
    }
}

static void ApplyPrStageRunnerShortListBlockF_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxPrStageRunnerCtx801CBFDC& psxCtx) {
    if ((psxCtx.flags_00 & 0x00008000u) == 0u) {
        return;
    }
    for (std::size_t slot = 0; slot < psxCtx.shortLists_AC.size(); ++slot) {
        const auto& timIds = psxCtx.shortLists_AC[slot];
        for (uint16_t timId : timIds) {
            if (timId == 0u) {
                break;
            }
            PsxCall8001ADEC_801CBFDC(runtimeState,
                                     timId,
                                     slot < 1u,
                                     static_cast<uint8_t>(slot));
        }
    }
}

static void ApplyPrStageRunnerTailCallsBeforeUI_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxPrStageRunnerCtx801CBFDC& psxCtx,
    uint8_t renderSubFrame8,
    bool advanceLogicFrame) {
    if (psxCtx.tailGate54 != 0u && psxCtx.tailArg68 != 0) {
        PsxCall80023E10_801CBFDC(runtimeState,
                                 psxCtx.tailTextPtr10C);
    }
    if (psxCtx.tailGate64 != 0u) {
        PsxCall8001E2E4_801CBFDC(runtimeState, psxCtx, psxCtx.tailArg68);
    }
    PsxCall80024744_801CBFDC(runtimeState,
                             psxCtx,
                             renderSubFrame8,
                             advanceLogicFrame);
}

static void EnsurePreventTransitionTileMask801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    PsxPreventTransitionSetup801CBFDC setup) {
    if (runtimeState.preventTransitionTileMaskInitialized801CBFDC &&
        runtimeState.preventTransitionTileMaskSetup801CBFDC == setup) {
        return;
    }

    const uint32_t ffd4Arg =
        setup == PsxPreventTransitionSetup801CBFDC::FillActiveTiles8001FCBC
            ? 1u
            : 2u;
    PrStage1Scene1Movie1Direct::InitTransitionTileMaskRuntimeSub8001FFD4(
        runtimeState.preventTransitionTileMask801CBFDC,
        ffd4Arg);
    runtimeState.preventTransitionGp196801CBFDC = 0u;
    runtimeState.preventTransitionTileMaskInitialized801CBFDC = true;
    runtimeState.preventTransitionTileMaskSetup801CBFDC = setup;
}

static void PsxCall8001FCBC_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    int arg0,
    int arg1,
    bool advanceLogicFrame) {
    runtimeState.preventTransition801CBFDC = {};
    runtimeState.preventTransition801CBFDC.setup =
        PsxPreventTransitionSetup801CBFDC::FillActiveTiles8001FCBC;
    runtimeState.preventTransition801CBFDC.setupArg0 = arg0;
    runtimeState.preventTransition801CBFDC.setupArg1 = arg1;
    EnsurePreventTransitionTileMask801CBFDC(
        runtimeState,
        runtimeState.preventTransition801CBFDC.setup);
    if (advanceLogicFrame) {
        (void)PrStage1Scene1Movie1Direct::AdvanceTransitionTileMaskSub8001FCBC(
            runtimeState.preventTransitionTileMask801CBFDC,
            runtimeState.preventTransitionGp196801CBFDC,
            static_cast<uint32_t>(arg0),
            static_cast<uint32_t>(arg1));
    }
}

static void PsxCall8001FC40_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    int arg0,
    int arg1,
    bool advanceLogicFrame) {
    runtimeState.preventTransition801CBFDC = {};
    runtimeState.preventTransition801CBFDC.setup =
        PsxPreventTransitionSetup801CBFDC::ClearActiveTiles8001FC40;
    runtimeState.preventTransition801CBFDC.setupArg0 = arg0;
    runtimeState.preventTransition801CBFDC.setupArg1 = arg1;
    EnsurePreventTransitionTileMask801CBFDC(
        runtimeState,
        runtimeState.preventTransition801CBFDC.setup);
    if (advanceLogicFrame) {
        (void)PrStage1Scene1Movie1Direct::AdvanceTransitionTileMaskSub8001FC40(
            runtimeState.preventTransitionTileMask801CBFDC,
            runtimeState.preventTransitionGp196801CBFDC,
            static_cast<uint32_t>(arg0),
            static_cast<uint32_t>(arg1));
    }
}

static void PsxCall8001FDC0_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    int arg0) {
    runtimeState.preventTransition801CBFDC.drawCall8001FDC0 = true;
    runtimeState.preventTransition801CBFDC.drawArg0 = arg0;
}

static void PsxCall80040F90_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint32_t packetAllocatorBase8006ED50) {
    runtimeState.currentPacketAllocator800901C8 = packetAllocatorBase8006ED50;
    PrPsxGraphOwnerDirect::PsxCall80040F90_SetPacketAllocator(
        runtimeState.graphOwner801CBFDC,
        packetAllocatorBase8006ED50);
}

static void PsxCall80040CC8_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    PsxWorkListId work) {
    PsxWorkListState801CBFDC& state =
        PsxWorkListState80040CC8(runtimeState, work);
    state.work = work;
    state.slotIndex8006EDA8 = runtimeState.psxWorkSlotIndex8006EDA8;
    state.psxWorkBaseAddr = PsxWorkBaseAddr8001B084(work);
    state.psxWorkAddr =
        state.psxWorkBaseAddr +
        static_cast<uint32_t>(state.slotIndex8006EDA8) * 20u;
    state.otOrder = 14u;
    state.otBaseAddr =
        PsxWorkListOtBaseAddr80040CC8(work, state.slotIndex8006EDA8);
    PsxClearWorkList80040CC8(state, 0u, 0u);
}

static void PsxCall8001E374_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint8_t pageIndex) {
    (void)PrPsxGraphOwnerDirect::PsxCall8001E374_ClearMainPageWork(
        runtimeState.graphOwner801CBFDC,
        pageIndex);
}

static void ClearPrStageRunnerWorkListsBlockA_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    bool preserveCompactRailCommands) {
    PsxCall80040F90_801CBFDC(runtimeState,
                             PsxPacketAllocatorBase8006ED50(
                                 runtimeState.psxWorkSlotIndex8006EDA8));
    PsxCall80040CC8_801CBFDC(runtimeState, PsxWorkListId::W7B1C);
    PsxCall80040CC8_801CBFDC(runtimeState, PsxWorkListId::W3244);
    PsxCall80040CC8_801CBFDC(runtimeState, PsxWorkListId::W8058);
    PsxCall80040CC8_801CBFDC(runtimeState, PsxWorkListId::W8080);
    PsxCall80040CC8_801CBFDC(runtimeState, PsxWorkListId::W80A8);
    PsxCall80040CC8_801CBFDC(runtimeState, PsxWorkListId::W526C);
    PsxCall80040CC8_801CBFDC(runtimeState, PsxWorkListId::W5AF4);
    PsxCall8001E374_801CBFDC(runtimeState,
                             runtimeState.psxWorkSlotIndex8006EDA8);
    runtimeState.submitRanges801CAC34.clear();
    runtimeState.submitRanges801CB190.clear();
    runtimeState.timUploadCommands801CBFDC.clear();
    runtimeState.scriptBoxCommands80023E10.clear();
    runtimeState.numericStatusCommands8001E2E4.clear();
    if (!preserveCompactRailCommands) {
        runtimeState.compactRailCommands80024744.clear();
        runtimeState.compactRailSpriteCommands80024744.clear();
    }
    runtimeState.todSideEffectStats80028054 = {};
    runtimeState.preventTransition801CBFDC = {};
}

static void PsxCall80040370_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState) {
    runtimeState.lastGraphFlip80040370 =
        PrPsxGraphOwnerDirect::PsxCall80040370_FlipGraph(
            runtimeState.graphOwner801CBFDC);
}

static void PsxCall80040060_801C9178(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    uint8_t r,
    uint8_t g,
    uint8_t b) {
    runtimeState.drawEnvClearColor80040060.valid = true;
    runtimeState.drawEnvClearColor80040060.r = r;
    runtimeState.drawEnvClearColor80040060.g = g;
    runtimeState.drawEnvClearColor80040060.b = b;
}

static void PsxCall801C9178_FrameTail(
    Stage1SceneSubmitRuntime::Impl& runtimeState) {
    PsxCall80040370_801CBFDC(runtimeState);
    if (runtimeState.psx801CBFDC.g_PrStageRunner_UIList_Block) {
        PsxCall80040060_801C9178(runtimeState, 0x86u, 0x67u, 0x53u);
    } else {
        PsxCall80040060_801C9178(runtimeState, 0x65u, 0x3Au, 0x5Au);
    }
}

static bool PrStageRunner_RenderFrame_801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxPrStageRunnerCtx801CBFDC& psxCtx,
    uint8_t renderSubFrame8,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData) {
    runtimeState.psxWorkSlotIndex8006EDA8 =
        PsxCall8004019C_801CBFDC(runtimeState);
    const bool advanceLogicFrame = renderSubFrame8 == 0u;
    const bool preserveRenderOnlyCompactRail =
        !advanceLogicFrame &&
        psxCtx.compactInputCaptured80024744 &&
        psxCtx.compactRowCount8A > 0;
    ClearPrStageRunnerWorkListsBlockA_801CBFDC(
        runtimeState,
        preserveRenderOnlyCompactRail);
    const bool publishSceneInitCamera =
        psxCtx.modeArg != 2 &&
        ((psxCtx.flags_00 & 0x00000020u) != 0u ||
         runtimeState.psx801CBFDC.dword_801D3034 != 0u);
    InitializeStage1DescAndCoordBank801CB6BC(runtimeState,
                                             resolveResource,
                                             resourceUserData,
                                             publishSceneInitCamera);
    if ((psxCtx.flags_00 & 0x00000020u) != 0u) {
        runtimeState.psx801CBFDC.dword_801D3034 = 1u;
    }

    if (psxCtx.modeArg == 2) {
        PsxCall8001FCBC_801CBFDC(runtimeState, 4, 4, advanceLogicFrame);
        PsxCall8001FDC0_801CBFDC(runtimeState, 2);
        PrStageRunner_DrawBackground_801CAC34(runtimeState);
        return runtimeState.backdropExtra4CBC;
    }
    if (runtimeState.psx801CBFDC.dword_801D3034 == 0u) {
        PsxCall8001FC40_801CBFDC(runtimeState, 4, 3, advanceLogicFrame);
        PsxCall8001FDC0_801CBFDC(runtimeState, 2);
        PrStageRunner_DrawBackground_801CAC34(runtimeState);
        return runtimeState.backdropExtra4CBC;
    }

    ApplyPrStageRunnerUIListSeedBlockC_801CBFDC(runtimeState, psxCtx);

    ApplyPrStageRunnerResourceBlockD_801CBFDC(runtimeState,
                                              psxCtx,
                                              resolveResource,
                                              resourceUserData);

    ApplyPrStageRunnerDynamicRowBlockE_801CBFDC(runtimeState,
                                                psxCtx,
                                                resolveResource,
                                                resourceUserData);

    ApplyPrStageRunnerShortListBlockF_801CBFDC(runtimeState, psxCtx);

    ApplyPrStageRunnerRenderGlobalsBlockG_801CBFDC(runtimeState,
                                                   psxCtx,
                                                   renderSubFrame8,
                                                   resolveResource,
                                                   resourceUserData);

    PrStageRunner_EmitBackdropPrimitives801CAC34(runtimeState);
    ApplyPrStageRunnerTailCallsBeforeUI_801CBFDC(
        runtimeState,
        psxCtx,
        renderSubFrame8,
        advanceLogicFrame);
    PrStageRunner_RenderUI_801CB190(runtimeState);
    SyncRuntimeFromPrStageRunnerGlobals801CBFDC(runtimeState);
    return runtimeState.backdropExtra4CBC;
}

static bool PsxCall80040FA0_GsSetRefView2(
    const PsxCameraState801CBFDC& camera,
    PsxFixedMatrix3x480040FA0& gsWsMatrix80092880) {
    std::array<int32_t, 8> rview2 = {{
        PsxRoundFloatToI32_80040FA0(camera.pos[0]),
        PsxRoundFloatToI32_80040FA0(camera.pos[1]),
        PsxRoundFloatToI32_80040FA0(camera.pos[2]),
        PsxRoundFloatToI32_80040FA0(camera.target[0]),
        PsxRoundFloatToI32_80040FA0(camera.target[1]),
        PsxRoundFloatToI32_80040FA0(camera.target[2]),
        camera.twist,
        0,
    }};
    PsxFixedMatrixIdentity800416E0(gsWsMatrix80092880);

    std::array<int32_t, 6> scaled{};
    PsxCall80041374_ScaleGsRView2(rview2, scaled);

    const int32_t dx = scaled[3] - scaled[0];
    const int32_t dy = scaled[4] - scaled[1];
    const int32_t dz = scaled[5] - scaled[2];
    const int32_t dist = PsxCall80041548_SquareRoot0(
        static_cast<int64_t>(dx) * dx +
        static_cast<int64_t>(dy) * dy +
        static_cast<int64_t>(dz) * dz);
    if (dist == 0) {
        return false;
    }

    if (camera.twist != 0) {
        constexpr double kTau = 6.28318530717958647692;
        const int angle = -camera.twist / 360;
        const double rad = static_cast<double>(angle & 0xFFF) *
                           (kTau / 4096.0);
        PsxFixedMatrix3x480040FA0 rotZ{};
        PsxCall800416E0_RotMatrixByAxis(
            rotZ,
            static_cast<int16_t>(std::lround(std::sin(rad) * 4096.0)),
            static_cast<int16_t>(std::lround(std::cos(rad) * 4096.0)),
            'z');
        PsxFixedMatrixMul3x380040FA0(
            gsWsMatrix80092880,
            rotZ,
            gsWsMatrix80092880);
    }

    const int32_t v5 = PsxDivTrunc801CBFDC(
        static_cast<int64_t>(scaled[4] - scaled[1]) * 4096,
        dist);
    const int32_t horizontal = PsxCall80041548_SquareRoot0(
        static_cast<int64_t>(dx) * dx + static_cast<int64_t>(dz) * dz);

    PsxFixedMatrix3x480040FA0 rotX{};
    PsxCall800416E0_RotMatrixByAxis(
        rotX,
        static_cast<int16_t>(v5),
        static_cast<int16_t>(PsxDivTrunc801CBFDC(
            static_cast<int64_t>(horizontal) * 4096,
            dist)),
        'x');
    PsxFixedMatrixMul3x380040FA0(gsWsMatrix80092880,
                                 rotX,
                                 gsWsMatrix80092880);

    if (horizontal != 0) {
        PsxFixedMatrix3x480040FA0 rotY{};
        PsxCall800416E0_RotMatrixByAxis(
            rotY,
            static_cast<int16_t>(-PsxDivTrunc801CBFDC(
                static_cast<int64_t>(scaled[3] - scaled[0]) * 4096,
                horizontal)),
            static_cast<int16_t>(PsxDivTrunc801CBFDC(
                static_cast<int64_t>(scaled[5] - scaled[2]) * 4096,
                horizontal)),
            'y');
        PsxFixedMatrixMul3x380040FA0(gsWsMatrix80092880,
                                     rotY,
                                     gsWsMatrix80092880);
    }

    std::array<int32_t, 3> negPos = {{
        -rview2[0],
        -rview2[1],
        -rview2[2],
    }};
    std::array<int32_t, 3> trans{};
    PsxCall8003ABA4_ApplyMatrixLv(gsWsMatrix80092880, negPos, trans);
    for (int r = 0; r < 3; ++r) {
        gsWsMatrix80092880.t[r] = trans[r];
    }
    return true;
}

static PsxViewMatrixRows428B0 BuildViewRowsFromCamera801CBFDC(
    const PsxCameraState801CBFDC& camera) {
    PsxViewMatrixRows428B0 viewRows{};
    PsxFixedMatrix3x480040FA0 gsWsMatrix80092880{};
    if (!PsxCall80040FA0_GsSetRefView2(camera, gsWsMatrix80092880)) {
        return viewRows;
    }

    for (size_t r = 0; r < kPsxViewMatrixRowCount428B0; ++r) {
        const size_t rowBase = r * kPsxViewMatrixColumnCount428B0;
        for (size_t c = 0; c < 3u; ++c) {
            viewRows[rowBase + c] = gsWsMatrix80092880.m[r][c];
        }
        viewRows[rowBase + 3u] = gsWsMatrix80092880.t[r];
    }
    return viewRows;
}

static bool AdvanceStage1SceneSubmitWithViewRows801CBFDC190(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxPrStageRunnerCtx801CBFDC& psxCtx,
    const PsxBackdropCamera801CAC34& backdropCamera,
    const PsxViewMatrixRows428B0& viewRows,
    int32_t projectionDistance,
    int32_t screenCenterX,
    int32_t screenCenterY,
    uint8_t renderSubFrame8,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData) {
    PsxProjectionParams428B0 projection{};
    projection.projectionDistance = projectionDistance;
    projection.screenCenterX = screenCenterX;
    projection.screenCenterY = screenCenterY;

    const bool backdropExtra4CBC =
        PrStageRunner_RenderFrame_801CBFDC(runtimeState,
                                           psxCtx,
                                           renderSubFrame8,
                                           resolveResource,
                                           resourceUserData);
    PsxCall801C9178_FrameTail(runtimeState);

    runtimeState.advanceFrameValid = true;
    runtimeState.lastAdvanceQueryFrame = psxCtx.queryFrame;
    if (runtimeState.cameraPresentationState801CBFDC.valid) {
        const PsxCameraState801CBFDC& finalCamera =
            runtimeState.cameraPresentationState801CBFDC;
        runtimeState.viewRows = BuildViewRowsFromCamera801CBFDC(finalCamera);
        runtimeState.projection.projectionDistance =
            finalCamera.projectionDistance;
        runtimeState.projection.screenCenterX = finalCamera.screenCenterX;
        runtimeState.projection.screenCenterY = finalCamera.screenCenterY;
        for (size_t i = 0; i < 3u; ++i) {
            runtimeState.backdropCamera.pos[i] = finalCamera.pos[i];
            runtimeState.backdropCamera.target[i] = finalCamera.target[i];
        }
    } else {
        runtimeState.viewRows = viewRows;
        runtimeState.projection = projection;
        runtimeState.backdropCamera = backdropCamera;
    }
    return backdropExtra4CBC;
}

static bool AdvanceStage1SceneSubmitFromPsxCtx801CBFDC190(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxPrStageRunnerCtx801CBFDC& psxCtx,
    const PsxCameraState801CBFDC& camera,
    uint8_t renderSubFrame8,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData) {
    if (!psxCtx.valid) {
        return false;
    }

    const PsxCameraState801CBFDC effectiveCamera =
        camera.valid ? camera : MakeStage1CameraDefaults801CB6BC();
    PsxBackdropCamera801CAC34 backdropCamera{};
    for (size_t i = 0; i < 3u; ++i) {
        backdropCamera.pos[i] = effectiveCamera.pos[i];
        backdropCamera.target[i] = effectiveCamera.target[i];
    }
    return AdvanceStage1SceneSubmitWithViewRows801CBFDC190(
        runtimeState,
        psxCtx,
        backdropCamera,
        BuildViewRowsFromCamera801CBFDC(effectiveCamera),
        effectiveCamera.projectionDistance,
        effectiveCamera.screenCenterX,
        effectiveCamera.screenCenterY,
        renderSubFrame8,
        resolveResource,
        resourceUserData);
}

static bool AdvanceStage1CameraConsumer801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxPrStageRunnerCtx801CBFDC& psxCtx,
    uint8_t renderSubFrame8,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData,
    PsxCameraState801CBFDC& outCamera);

bool AdvanceStage1SceneSubmitRuntime801CBFDC190(
    Stage1SceneSubmitRuntime& runtime,
    const PrStage1RuntimeSlotsSnapshot& runtimeSlots,
    const char* scriptBoxTextPtr10C,
    uint8_t renderSubFrame8,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData) {
    if (!runtimeSlots.valid) {
        runtime = Stage1SceneSubmitRuntime{};
        return false;
    }

    PsxCameraState801CBFDC camera{};
    auto& runtimeState =
        Stage1SceneSubmitRuntimePrivateAccess::MutableImpl(runtime);
    uint32_t suppressOneShotPulseMask =
        renderSubFrame8 != 0u ? kOneShotCtxFlags801CBFDC : 0u;
    if (renderSubFrame8 == 0u &&
        runtimeState.oneShotPulseConsumed801CBFDCValid &&
        runtimeState.oneShotPulseConsumed801CBFDCQueryFrame ==
            runtimeSlots.queryFrame) {
        suppressOneShotPulseMask |=
            runtimeState.oneShotPulseConsumed801CBFDCFlags;
    }
    const PsxPrStageRunnerCtx801CBFDC psxCtx =
        BuildStage1PsxRenderCtx801CBFDC(runtimeSlots,
                                        0,
                                        scriptBoxTextPtr10C,
                                        suppressOneShotPulseMask);
    if (renderSubFrame8 == 0u &&
        runtimeSlots.sceneSubmit.sceneInitPulse801CB6BC) {
        runtimeState.coordBankInitialized801CB6BC = false;
    }
    (void)AdvanceStage1CameraConsumer801CBFDC(runtimeState,
                                             psxCtx,
                                             renderSubFrame8,
                                             resolveResource,
                                             resourceUserData,
                                             camera);

    const bool advanced = AdvanceStage1SceneSubmitFromPsxCtx801CBFDC190(
        runtimeState,
        psxCtx,
        camera,
        renderSubFrame8,
        resolveResource,
        resourceUserData);
    if (renderSubFrame8 == 0u) {
        if (!runtimeState.oneShotPulseConsumed801CBFDCValid ||
            runtimeState.oneShotPulseConsumed801CBFDCQueryFrame !=
                runtimeSlots.queryFrame) {
            runtimeState.oneShotPulseConsumed801CBFDCValid = true;
            runtimeState.oneShotPulseConsumed801CBFDCQueryFrame =
                runtimeSlots.queryFrame;
            runtimeState.oneShotPulseConsumed801CBFDCFlags = 0u;
        }
        runtimeState.oneShotPulseConsumed801CBFDCFlags |=
            psxCtx.flags_00 & kOneShotCtxFlags801CBFDC;
    }
    return advanced;
}

void ResetOwnedStage1SceneSubmitRuntime801CBFDC190() {
    OwnedStage1SceneSubmitRuntime801CBFDC190() = Stage1SceneSubmitRuntime{};
}

const Stage1SceneSubmitRuntime&
GetOwnedStage1SceneSubmitRuntime801CBFDC190() {
    return OwnedStage1SceneSubmitRuntime801CBFDC190();
}

bool AdvanceOwnedStage1SceneSubmitRuntime801CBFDC190(
    const PrStage1RuntimeSlotsSnapshot& runtimeSlots,
    const char* scriptBoxTextPtr10C,
    uint8_t renderSubFrame8,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData) {
    return AdvanceStage1SceneSubmitRuntime801CBFDC190(
        OwnedStage1SceneSubmitRuntime801CBFDC190(),
        runtimeSlots,
        scriptBoxTextPtr10C,
        renderSubFrame8,
        resolveResource,
        resourceUserData);
}

bool ApplyStage1SceneSubmitRuntimeHudBee4RawActions801CBFDC190(
    Stage1SceneSubmitRuntime& runtime,
    const PrStage1HudPresentationDirectBee4ActionCarrier& carrier,
    PrStage1HudPresentationDirectBee4ConsumeResult* outResult) {
    if (outResult != nullptr) {
        *outResult = PrStage1HudPresentationDirectBee4ConsumeResult{};
    }

    auto& runtimeState =
        Stage1SceneSubmitRuntimePrivateAccess::MutableImpl(runtime);
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 submitRuntime{};
    if (!TryBuildCompactRailFastSpriteRuntimeOwner8003FA20(runtimeState,
                                                           submitRuntime)) {
        return false;
    }

    PrStage1HudPresentationDirectBee4ConsumeResult result =
        PrStage1HudPresentationDirectConsumeSub8001DF24Bee4RawActions(
            carrier,
            submitRuntime,
            true);
    if (outResult != nullptr) {
        *outResult = result;
    }
    if (!result.valid) {
        return false;
    }

    const PrPsxGraphOwnerDirect::PsxFastSpriteRuntimeCommitResult8003FA20
        commit =
            PrPsxGraphOwnerDirect::CommitRuntimeState8003FA20ToMainPageWork(
                runtimeState.graphOwner801CBFDC,
                runtimeState.psxWorkSlotIndex8006EDA8,
                submitRuntime);
    if (commit.committed && commit.allocatorUpdated) {
        runtimeState.currentPacketAllocator800901C8 = commit.allocatorAfter;
    }
    return result.consumedCount > 0u;
}

bool ApplyOwnedStage1SceneSubmitRuntimeHudBee4RawActions801CBFDC190(
    const PrStage1HudPresentationDirectBee4ActionCarrier& carrier,
    PrStage1HudPresentationDirectBee4ConsumeResult* outResult) {
    return ApplyStage1SceneSubmitRuntimeHudBee4RawActions801CBFDC190(
        OwnedStage1SceneSubmitRuntime801CBFDC190(),
        carrier,
        outResult);
}

static bool AdvanceStage1CameraConsumer801CBFDC(
    Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxPrStageRunnerCtx801CBFDC& psxCtx,
    uint8_t renderSubFrame8,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData,
    PsxCameraState801CBFDC& outCamera) {
    outCamera = PsxCameraState801CBFDC{};
    if (!psxCtx.valid) {
        return false;
    }

    const bool cameraPlaybackRequested =
        (psxCtx.flags_00 & 0x00000020u) != 0u;
    if (cameraPlaybackRequested &&
        (runtimeState.cameraUsingNeutral801CBFDC ||
         !runtimeState.cameraBezPlayer801CBFDC.loaded)) {
        PsxCall800127F0_801CBFDC(runtimeState,
                                 kStage1InitialCameraBezId801CB6BC,
                                 0u,
                                 true,
                                 resolveResource,
                                 resourceUserData);
    }

    if (renderSubFrame8 != 0u &&
        runtimeState.psx801CBFDC.dword_801D3034 != 0u) {
        const uint32_t tickBeforeAdvance =
            runtimeState.psx801CBFDC.dword_801D302C == 0u
                ? 0u
                : (runtimeState.psx801CBFDC.dword_801D302C - 1u);
        PsxCall80012960_801CBFDC(
            runtimeState,
            static_cast<int32_t>(tickBeforeAdvance),
            renderSubFrame8);
    }

    outCamera = runtimeState.cameraPresentationState801CBFDC;
    return outCamera.valid;
}

static bool ResolveStage1MimeActorForModelRef801CB190(
    const PsxDrawModelRef& modelRef,
    PsxMimeActor801CB190& outActor) {
    if (modelRef.set != ModelSet::Stage1Main) {
        return false;
    }
    if (modelRef.index == kStage1OnionModelIndex801CB190) {
        outActor = PsxMimeActor801CB190::Onion;
        return true;
    }
    if (modelRef.index == kStage1ParappaModelIndex801CB190) {
        outActor = PsxMimeActor801CB190::Parappa;
        return true;
    }
    return false;
}

static size_t PsxMimeSampleSlot80090240(PsxMimeActor801CB190 actor) {
    return actor == PsxMimeActor801CB190::Onion ? 1u : 0u;
}

static bool ApplyStage1MimeDeformationFromState801CB190(
    const Stage1SceneSubmitRuntime::Impl& runtimeState,
    PsxMimeActor801CB190 actor,
    const VdfData* vdf,
    const DatData* dat,
    uint32_t objectIndex,
    const std::vector<TmdVertex>& baseVerts,
    std::vector<TmdVertex>& outVerts) {
    if (vdf == nullptr || dat == nullptr) {
        outVerts.clear();
        return false;
    }

    const size_t sampleSlot = PsxMimeSampleSlot80090240(actor);
    const PsxMimeSampleState80090240& sampleState =
        runtimeState.mimeSampleState80090240[sampleSlot];
    if (!sampleState.valid) {
        outVerts.clear();
        return false;
    }

    PsxDatTimelineState80013D10 datState{};
    if (PsxCall80013D10_801CB190(datState, dat, 0u) < 0) {
        outVerts.clear();
        return false;
    }
    PsxCall80013E04_801CB190(
        datState, static_cast<uint16_t>(sampleState.frame));
    outVerts = baseVerts;
    return PsxCall80013AA8_801CB190(datState,
                                    vdf,
                                    objectIndex,
                                    outVerts);
}

static void ApplyStage1MimeDeformationToModelView801CB190(
    const Stage1SceneSubmitRuntime::Impl& runtimeState,
    const PsxDrawModelRef& modelRef,
    PsxMimeResourceProvider801CB190 mimeProvider,
    void* mimeProviderUserData,
    PsxTmdModelView428B0& model) {
    PsxMimeActor801CB190 actor{};
    if (!ResolveStage1MimeActorForModelRef801CB190(modelRef, actor)) {
        return;
    }

    const DatData* dat = nullptr;
    const VdfData* vdf = nullptr;
    const PsxMimeResourceBinding801CBFDC& binding =
        (actor == PsxMimeActor801CB190::Onion)
            ? runtimeState.onMimeBinding801CBFDC
            : runtimeState.paMimeBinding801CBFDC;
    if (!binding.valid) {
        return;
    }
    if (mimeProvider == nullptr ||
        !mimeProvider(binding.datHandle,
                      binding.vdfHandle,
                      dat,
                      vdf,
                      mimeProviderUserData) ||
        dat == nullptr ||
        vdf == nullptr) {
        return;
    }

    for (size_t objectIndex = 0;
         objectIndex < model.objects.size();
         ++objectIndex) {
        PsxTmdObjectView428B0& object = model.objects[objectIndex];
        std::vector<TmdVertex> baseVerts;
        baseVerts.reserve(object.vertices.size());
        for (const PsxTmdVertex428B0& vertex : object.vertices) {
            baseVerts.push_back({
                vertex.x,
                vertex.y,
                vertex.z,
                0,
            });
        }

        std::vector<TmdVertex> deformedVerts;
        if (!ApplyStage1MimeDeformationFromState801CB190(
                runtimeState,
                actor,
                vdf,
                dat,
                static_cast<uint32_t>(objectIndex),
                baseVerts,
                deformedVerts)) {
            continue;
        }

        object.vertices.clear();
        object.vertices.reserve(deformedVerts.size());
        for (const TmdVertex& vertex : deformedVerts) {
            object.vertices.push_back({
                vertex.x,
                vertex.y,
                vertex.z,
            });
        }
    }
}

static PsxTmdModelView428B0 BuildPsxTmdModelView428B0(
    const TmdModel& model) {
    PsxTmdModelView428B0 view{};
    view.objects.reserve(model.objects.size());
    for (const TmdObject& object : model.objects) {
        PsxTmdObjectView428B0 viewObject{};
        viewObject.vertices.reserve(object.vertices.size());
        for (const TmdVertex& vertex : object.vertices) {
            viewObject.vertices.push_back({
                vertex.x,
                vertex.y,
                vertex.z,
            });
        }
        viewObject.rawPrimitivePackets.reserve(object.rawPrimitivePackets.size());
        for (const TmdRawPrimitivePacket& raw : object.rawPrimitivePackets) {
            viewObject.rawPrimitivePackets.push_back({
                raw.rawPrimitiveIndex,
                raw.rawPacketOffset,
                raw.rawPacketByteSize,
                raw.olen,
                raw.ilen,
                raw.flag,
                raw.mode,
                raw.parsed,
                raw.parsedPrimitiveIndex,
            });
        }
        viewObject.primitives.reserve(object.primitives.size());
        for (const TmdPrimitive& primitive : object.primitives) {
            PsxTmdPrimitive428B0 viewPrimitive{};
            viewPrimitive.rawPacketKnown = primitive.rawPacketKnown;
            viewPrimitive.rawPrimitiveIndex = primitive.rawPrimitiveIndex;
            viewPrimitive.rawPacketOffset = primitive.rawPacketOffset;
            viewPrimitive.rawPacketByteSize = primitive.rawPacketByteSize;
            viewPrimitive.quad = primitive.quad;
            viewPrimitive.vertexIndices = {
                primitive.v0_idx,
                primitive.v1_idx,
                primitive.v2_idx,
                primitive.v3_idx,
            };
            viewPrimitive.u = {
                primitive.u0,
                primitive.u1,
                primitive.u2,
                primitive.u3,
            };
            viewPrimitive.v = {
                primitive.v0,
                primitive.v1,
                primitive.v2,
                primitive.v3,
            };
            viewPrimitive.textured = primitive.textured;
            viewPrimitive.semiTransparent = primitive.semiTransparent;
            viewPrimitive.tpage = primitive.tpage;
            viewPrimitive.clut = primitive.clut;
            viewPrimitive.abr = primitive.abr;
            viewPrimitive.r = primitive.r;
            viewPrimitive.g = primitive.g;
            viewPrimitive.b = primitive.b;
            for (size_t ci = 0; ci < viewPrimitive.colorWords.size(); ++ci) {
                viewPrimitive.colorWords[ci] = primitive.colorWords[ci];
            }
            viewPrimitive.psxRgbCodeWord =
                static_cast<uint32_t>(primitive.r) |
                (static_cast<uint32_t>(primitive.g) << 8) |
                (static_cast<uint32_t>(primitive.b) << 16) |
                (static_cast<uint32_t>(primitive.pad1) << 24);
            viewPrimitive.psxMode = primitive.mode;
            viewPrimitive.psxFlag = primitive.flag;
            viewPrimitive.psxWordStride8004274C = primitive.ilen + 1u;
            viewObject.primitives.push_back(viewPrimitive);
        }
        BuildPrimitiveGroups8004274C(viewObject);
        view.objects.push_back(std::move(viewObject));
    }
    return view;
}

static void PsxCall800428B0_DrawDescEntry(
    uint32_t descAddr,
    uint8_t workListFlushOrder,
    uint8_t otShift,
    bool headInsert,
    const std::vector<PsxDrawDescState>& localDescStates,
    const std::vector<PsxCoordNode>& coordNodes,
    const Stage1SceneSubmitRuntime::Impl& runtimeState,
    PsxTmdModelProvider428B0 modelProvider,
    void* modelProviderUserData,
    PsxMimeResourceProvider801CB190 mimeProvider,
    void* mimeProviderUserData,
    std::map<uint32_t, PsxTmdModelView428B0>& modelViews,
    const PsxMatrix3x4Fixed428B0& view,
    const PsxMatrix3x4Float428B0* presentationView,
    const PsxProjectionParams428B0& projection,
    std::vector<PsxGpuPacketGap428B0>& packetGaps,
    PsxOtWorkLists428B0& workLists) {
    const auto descStateIt =
        std::find_if(localDescStates.begin(),
                     localDescStates.end(),
                     [descAddr](const PsxDrawDescState& candidate) {
                         return candidate.descAddr == descAddr;
                     });
    if (descStateIt == localDescStates.end()) {
        return;
    }
    const PsxDrawDescState& descState = *descStateIt;
    if (static_cast<int32_t>(descState.attr) < 0) {
        return;
    }

    const PsxDrawDescAttrState428B0 attrState =
        DecodeDrawDescAttr800428B0(descState.attr);

    const uint32_t modelViewKey =
        (static_cast<uint32_t>(descState.modelRef.set) << 16) |
        static_cast<uint32_t>(descState.modelRef.index);
    auto modelViewInsert =
        modelViews.try_emplace(modelViewKey, PsxTmdModelView428B0{});
    PsxTmdModelView428B0& model = modelViewInsert.first->second;
    if (modelViewInsert.second) {
        if (modelProvider == nullptr) {
            return;
        }
        const TmdModel* sourceModel =
            modelProvider(descState.descAddr,
                          descState.modelRef,
                          modelProviderUserData);
        if (sourceModel == nullptr) {
            return;
        }
        model = BuildPsxTmdModelView428B0(*sourceModel);
        ApplyStage1MimeDeformationToModelView801CB190(
            runtimeState,
            descState.modelRef,
            mimeProvider,
            mimeProviderUserData,
            model);
    }

    const uint32_t nodeAddr = descState.coordNodeAddr;
    TodCoordMatrix coordWorld{};
    if (!PsxCall80041A68_8001B084(coordNodes, nodeAddr, coordWorld)) {
        return;
    }
    const PsxMatrix3x4Fixed428B0 combined =
        PsxCall80040544_8001B084(view, coordWorld);
    PsxMatrix3x4Float428B0 presentationCombined{};
    const bool presentationCombinedKnown = presentationView != nullptr;
    if (presentationCombinedKnown) {
        presentationCombined =
            PsxCall80040544Presentation428B0(*presentationView, coordWorld);
    }

    if (descState.objectDescToken8004274C == 0u) {
        return;
    }
    const size_t objectIndex = descState.objectIndex8004274C;
    if (objectIndex >= model.objects.size()) {
        return;
    }
    const PsxTmdObjectView428B0& object = model.objects[objectIndex];
    if (object.primitiveGroups8004274C.empty()) {
        return;
    }
    for (const PsxTmdPrimitiveGroup8004274C& group :
         object.primitiveGroups8004274C) {
        if (group.primitiveStartIndex >= object.primitives.size()) {
            continue;
        }
        const PsxTmdPrimitive428B0& firstPrimitive =
            object.primitives[group.primitiveStartIndex];
        const uint16_t psxGroupCount =
            firstPrimitive.psxGroupCount8004274C;
        if (psxGroupCount == 0u ||
            (group.primitiveCount != 0u &&
             psxGroupCount != group.primitiveCount)) {
            continue;
        }
        const uint32_t groupEnd =
            group.primitiveStartIndex + psxGroupCount;
        if (groupEnd > object.primitives.size() || group.psxWordStride == 0u) {
            continue;
        }
        for (uint32_t primitiveIndex = group.primitiveStartIndex;
             primitiveIndex < groupEnd;
             ++primitiveIndex) {
            const PsxTmdPrimitive428B0& primitive =
                object.primitives[primitiveIndex];
            if (primitive.psxMode != group.psxMode ||
                primitive.psxFlag != group.psxFlag ||
                (primitive.psxWordStride8004274C != 0u &&
                 primitive.psxWordStride8004274C != group.psxWordStride)) {
                continue;
            }
            const PsxGsTmdFastHandler428B0 handler =
                PsxResolveGsTmdFastHandler800428B0(primitive.psxMode,
                                                   primitive.psxFlag);
            if (handler == PsxGsTmdFastHandler428B0::Unsupported) {
                uint32_t specialTableBase = 0;
                if (PsxGsTmdFastSpecialTableBase800428B0(
                        primitive.psxMode,
                        primitive.psxFlag,
                        specialTableBase)) {
                    PsxGpuPacketGap428B0 gap{};
                    gap.primitive = &primitive;
                    gap.gap =
                        PsxGpuPacketDispatchGap428B0::
                            Flag4SpecialTableUnsupported;
                    gap.specialTableBase800428B0 = specialTableBase;
                    packetGaps.push_back(gap);
                }
                continue;
            }
            const uint8_t vertexCount =
                PsxGsTmdFastVertexCount428B0(handler);
            if (((handler == PsxGsTmdFastHandler428B0::F4NL ||
                  handler == PsxGsTmdFastHandler428B0::NF4 ||
                  handler == PsxGsTmdFastHandler428B0::TF4NL ||
                  handler == PsxGsTmdFastHandler428B0::TNF4 ||
                  handler == PsxGsTmdFastHandler428B0::G4NL ||
                  handler == PsxGsTmdFastHandler428B0::NG4 ||
                  handler == PsxGsTmdFastHandler428B0::TG4NL ||
                  handler == PsxGsTmdFastHandler428B0::TNG4)) !=
                primitive.quad) {
                continue;
            }
            std::array<
                PsxProjectedVertex428B0,
                kPsxPrimitiveMaxVertexCount428B0>
                projected{};
            uint8_t vi = 0u;
            for (; vi < vertexCount; ++vi) {
                const uint16_t vertexIndex = primitive.vertexIndices[vi];
                if (vertexIndex >= object.vertices.size()) {
                    break;
                }
                const PsxTmdVertex428B0& vertex =
                    object.vertices[vertexIndex];
                if (!PsxProjectVertexRtpt428B0(
                        combined,
                        projection,
                        vertex,
                        projected[vi])) {
                    break;
                }
                if (presentationCombinedKnown) {
                    (void)PsxProjectVertexPresentation428B0(
                        presentationCombined,
                        projection,
                        vertex,
                        projected[vi]);
                }
            }
            if (vi != vertexCount) {
                continue;
            }
            if (PsxNclipMac0FromSxy428B0(projected) <= 0) {
                continue;
            }

            PsxProjectedPrimitiveWorkEntry428B0 entry{};
            entry.primitive = &primitive;
            entry.attr = attrState;
            entry.sourceModelRefKnown = true;
            entry.sourceModelRef = descState.modelRef;
            entry.projectedDepthKnown = true;
            entry.projectedScreenKnown = true;
            entry.projectedVertexCount = vertexCount;
            for (uint8_t depthIndex = 0u; depthIndex < vertexCount;
                 ++depthIndex) {
                entry.projectedDepth[depthIndex] = projected[depthIndex].z;
                entry.projectedScreenX[depthIndex] =
                    projected[depthIndex].screenX;
                entry.projectedScreenY[depthIndex] =
                    projected[depthIndex].screenY;
                entry.projectedScreenZ[depthIndex] =
                    projected[depthIndex].screenZ;
            }
            const uint16_t otz =
                PsxAvszOtZ428B0(projected, vertexCount);
            entry.packet =
                PsxBuildGsTmdFastPacket428B0(primitive,
                                             handler,
                                             attrState,
                                             projected,
                                             otz);
            if (entry.packet.wordCount == 0u) {
                continue;
            }
            const int otBucket =
                static_cast<int>(otz >> otShift);
            std::vector<PsxProjectedPrimitiveWorkEntry428B0>& bucket =
                workLists[workListFlushOrder][otBucket];
            if (headInsert) {
                bucket.insert(bucket.begin(), entry);
            } else {
                bucket.push_back(entry);
            }
        }
    }
}

void BuildStage1GpuPacketCommands428B0(
    const Stage1SceneSubmitRuntime& runtime,
    PsxTmdModelProvider428B0 modelProvider,
    void* modelProviderUserData,
    PsxMimeResourceProvider801CB190 mimeProvider,
    void* mimeProviderUserData,
    PsxGpuPacketCommandConsumer428B0 consumeCommand,
    void* commandUserData) {
    const auto& runtimeState =
        Stage1SceneSubmitRuntimePrivateAccess::ConstImpl(runtime);
    if (!runtimeState.advanceFrameValid) {
        return;
    }

    PsxMatrix3x4Fixed428B0 view{};
    for (size_t r = 0; r < kPsxViewMatrixRowCount428B0; ++r) {
        const size_t rowBase = r * kPsxViewMatrixColumnCount428B0;
        for (size_t c = 0; c < 3u; ++c) {
            view.m[r][c] = runtimeState.viewRows[rowBase + c];
        }
        view.t[r] = runtimeState.viewRows[rowBase + 3u];
    }
    PsxOtWorkLists428B0 workLists{};
    std::vector<PsxGpuPacketGap428B0> packetGaps;
    std::map<uint32_t, PsxTmdModelView428B0> modelViews;
    PsxMatrix3x4Float428B0 presentationView{};
    const PsxMatrix3x4Float428B0* presentationViewPtr = nullptr;
    if (runtimeState.cameraPresentationState801CBFDC.valid &&
        BuildPresentationViewMatrix428B0(
            runtimeState.cameraPresentationState801CBFDC,
            presentationView)) {
        presentationViewPtr = &presentationView;
    }
    for (const PsxSubmitRange& range : runtimeState.submitRanges801CAC34) {
        PsxCall8001B084_428B0(
            range,
            [&](uint32_t descAddr,
                uint8_t workListFlushOrder,
                uint8_t otShift,
                bool headInsert) {
                PsxCall800428B0_DrawDescEntry(
                    descAddr,
                    workListFlushOrder,
                    otShift,
                    headInsert,
                    runtimeState.localDescStates,
                    runtimeState.coordNodes,
                    runtimeState,
                    modelProvider,
                    modelProviderUserData,
                    mimeProvider,
                    mimeProviderUserData,
                    modelViews,
                    view,
                    presentationViewPtr,
                    runtimeState.projection,
                    packetGaps,
                    workLists);
            });
    }
    for (const PsxSubmitRange& range : runtimeState.submitRanges801CB190) {
        PsxCall8001B084_428B0(
            range,
            [&](uint32_t descAddr,
                uint8_t workListFlushOrder,
                uint8_t otShift,
                bool headInsert) {
                PsxCall800428B0_DrawDescEntry(
                    descAddr,
                    workListFlushOrder,
                    otShift,
                    headInsert,
                    runtimeState.localDescStates,
                    runtimeState.coordNodes,
                    runtimeState,
                    modelProvider,
                    modelProviderUserData,
                    mimeProvider,
                    mimeProviderUserData,
                    modelViews,
                    view,
                    presentationViewPtr,
                    runtimeState.projection,
                    packetGaps,
                    workLists);
            });
    }
    if (consumeCommand == nullptr) {
        return;
    }

    for (const PsxGpuPacketGap428B0& gap : packetGaps) {
        if (gap.primitive == nullptr) {
            continue;
        }
        PsxGpuPacketCommand428B0 command{};
        command.valid = false;
        command.handler = PsxGpuPacketHandler428B0::Unsupported;
        command.dispatchKnown800428B0 = true;
        command.dispatchGap800428B0 = gap.gap;
        FillPacketCommandPrimitiveMetadata428B0(command, *gap.primitive);
        command.specialTablePath800428B0 =
            gap.specialTableBase800428B0 != 0u;
        command.specialTableBase800428B0 =
            gap.specialTableBase800428B0;
        command.specialTableWriterKnown800428B0 = false;
        command.handlerTargetKnown800428B0 = false;
        command.handlerTarget800428B0 = 0u;
        consumeCommand(command, commandUserData);
    }

    uint32_t psxCallOrder = 0;
    for (size_t workListIndex = 0; workListIndex < workLists.size();
         ++workListIndex) {
        const auto& workList = workLists[workListIndex];
        for (const auto& bucket : workList) {
            for (const PsxProjectedPrimitiveWorkEntry428B0& entry : bucket.second) {
                const PsxTmdPrimitive428B0& primitive = *entry.primitive;
                PsxGpuPacketCommand428B0 command{};
                command.valid = entry.packet.wordCount != 0u;
                command.handler =
                    ToPublicGpuPacketHandler428B0(entry.packet.handler);
                command.dispatchKnown800428B0 = true;
                command.dispatchGap800428B0 =
                    PsxGpuPacketDispatchGap428B0::None;
                FillPacketCommandPrimitiveMetadata428B0(command, primitive);
                command.handlerTableSlot8001C1E8 =
                    entry.packet.handlerTableSlot8001C1E8;
                command.handlerTableEntryAddress8001C1E8 =
                    entry.packet.handlerTableEntryAddress8001C1E8;
                command.wordCount = entry.packet.wordCount;
                command.words = entry.packet.words;
                command.otz = entry.packet.otz;
                command.projectedDepthKnown800428B0 =
                    entry.projectedDepthKnown;
                command.projectedScreenKnown800428B0 =
                    entry.projectedScreenKnown;
                command.projectedVertexCount800428B0 =
                    entry.projectedVertexCount;
                for (uint8_t depthIndex = 0u;
                     depthIndex < entry.projectedVertexCount &&
                         depthIndex < command.projectedDepth800428B0.size();
                     ++depthIndex) {
                    command.projectedDepth800428B0[depthIndex] =
                        entry.projectedDepth[depthIndex];
                    command.projectedScreenX800428B0[depthIndex] =
                        entry.projectedScreenX[depthIndex];
                    command.projectedScreenY800428B0[depthIndex] =
                        entry.projectedScreenY[depthIndex];
                    command.projectedScreenZ800428B0[depthIndex] =
                        entry.projectedScreenZ[depthIndex];
                }
                command.otBucket = bucket.first;
                command.workListFlushOrder =
                    static_cast<uint8_t>(workListIndex);
                command.psxCallOrder = psxCallOrder++;
                command.semiTransparent =
                    primitive.semiTransparent || entry.attr.semiTransparent;
                command.abr =
                    entry.attr.semiTransparent ? entry.attr.abr
                                               : primitive.abr;
                command.sourceModelRefKnown800428B0 =
                    entry.sourceModelRefKnown;
                command.sourceModelSet800428B0 =
                    static_cast<uint8_t>(entry.sourceModelRef.set);
                command.sourceModelIndex800428B0 =
                    entry.sourceModelRef.index;
                consumeCommand(command, commandUserData);
            }
        }
    }
}

void BuildStage1DrawTriangleCommands801CBFDC(
    const Stage1SceneSubmitRuntime& runtime,
    PsxTmdModelProvider428B0 modelProvider,
    void* modelProviderUserData,
    PsxMimeResourceProvider801CB190 mimeProvider,
    void* mimeProviderUserData,
    PsxDrawTriangleCommandConsumer428B0 consumeCommand,
    void* commandUserData) {
    if (consumeCommand == nullptr) {
        return;
    }

    std::pair<PsxDrawTriangleCommandConsumer428B0, void*> consumer{
        consumeCommand,
        commandUserData};

    BuildStage1GpuPacketCommands428B0(
        runtime,
        modelProvider,
        modelProviderUserData,
        mimeProvider,
        mimeProviderUserData,
        +[](const PsxGpuPacketCommand428B0& packetCommand,
            void* userData) {
            auto* consumer =
                static_cast<std::pair<PsxDrawTriangleCommandConsumer428B0,
                                      void*>*>(userData);
            if (consumer == nullptr || consumer->first == nullptr ||
                !packetCommand.valid) {
                return;
            }

            const PsxGsTmdFastHandler428B0 handler =
                ToInternalGpuPacketHandler428B0(packetCommand.handler);
            if (handler == PsxGsTmdFastHandler428B0::Unsupported) {
                return;
            }

            const uint8_t triangleCount =
                PsxGsTmdFastIsQuad428B0(handler) ? 2u : 1u;
            for (uint8_t ti = 0; ti < triangleCount; ++ti) {
                const auto& slots = kPsxTriangleSlots428B0[ti];
                PsxDrawTriangleCommand428B0 command{};
                for (size_t i = 0; i < kPsxTriangleVertexCount428B0; ++i) {
                    const uint8_t slot = slots[i];
                    const uint8_t sxyWordIndex =
                        PsxPacketSxyWordIndex428B0(handler, slot);
                    const uint32_t sxyWord =
                        packetCommand.words[sxyWordIndex];
                    command.x[i] =
                        static_cast<float>(PsxUnpackSxyX428B0(sxyWord));
                    command.y[i] =
                        static_cast<float>(PsxUnpackSxyY428B0(sxyWord));
                    if (packetCommand.projectedScreenKnown800428B0 &&
                        slot < packetCommand.projectedVertexCount800428B0) {
                        command.x[i] =
                            packetCommand.projectedScreenX800428B0[slot];
                        command.y[i] =
                            packetCommand.projectedScreenY800428B0[slot];
                        if (packetCommand.projectedScreenZ800428B0[slot] >
                            0.0f) {
                            command.perspectiveW[i] =
                                packetCommand.projectedScreenZ800428B0[slot];
                        }
                    }
                    if (command.perspectiveW[i] <= 1.0f &&
                        packetCommand.projectedDepthKnown800428B0 &&
                        slot < packetCommand.projectedVertexCount800428B0 &&
                        packetCommand.projectedDepth800428B0[slot] > 0) {
                        command.perspectiveW[i] = static_cast<float>(
                            packetCommand.projectedDepth800428B0[slot]);
                    }
                    const uint8_t uvWordIndex =
                        PsxPacketUvWordIndex428B0(handler, slot);
                    const uint32_t uvWord =
                        uvWordIndex != 0u ? packetCommand.words[uvWordIndex]
                                          : 0u;
                    command.u[i] = PsxUnpackUvU428B0(uvWord);
                    command.v[i] = PsxUnpackUvV428B0(uvWord);
                }

                command.textured = PsxGsTmdFastIsTextured428B0(handler);
                const uint8_t uv0WordIndex =
                    PsxPacketUvWordIndex428B0(handler, 0);
                const uint8_t uv1WordIndex =
                    PsxPacketUvWordIndex428B0(handler, 1);
                command.tpage =
                    (command.textured && uv1WordIndex != 0u)
                        ? static_cast<uint16_t>(
                              packetCommand.words[uv1WordIndex] >> 16)
                        : 0u;
                command.clut =
                    (command.textured && uv0WordIndex != 0u)
                        ? static_cast<uint16_t>(
                              packetCommand.words[uv0WordIndex] >> 16)
                        : 0u;
                for (size_t i = 0; i < kPsxTriangleVertexCount428B0; ++i) {
                    const uint8_t slot = slots[i];
                    const uint32_t colorWord =
                        packetCommand.words
                            [PsxPacketColorWordIndex428B0(handler, slot)];
                    command.r[i] = PsxPacketColorByteToFloat428B0(
                        colorWord,
                        0,
                        command.textured);
                    command.g[i] = PsxPacketColorByteToFloat428B0(
                        colorWord,
                        8,
                        command.textured);
                    command.b[i] = PsxPacketColorByteToFloat428B0(
                        colorWord,
                        16,
                        command.textured);
                }

                float alphaScale = 1.0f;
                if (packetCommand.semiTransparent) {
                    switch (packetCommand.abr & 0x03u) {
                        case 1u:
                        case 2u:
                            alphaScale = 1.0f;
                            break;
                        case 3u:
                            alphaScale = 0.25f;
                            break;
                        case 0u:
                        default:
                            alphaScale = 0.5f;
                            break;
                    }
                }
                command.a = {alphaScale, alphaScale, alphaScale};
                command.blend = kPsxDrawBlendAlpha428B0;
                if (packetCommand.semiTransparent) {
                    switch (packetCommand.abr & 0x03u) {
                        case 1u:
                        case 3u:
                            command.blend = kPsxDrawBlendAdditive428B0;
                            break;
                        case 2u:
                            command.blend = kPsxDrawBlendSubtractive428B0;
                            break;
                        case 0u:
                        default:
                            command.blend = kPsxDrawBlendAlpha428B0;
                            break;
                    }
                }
                consumer->first(command, consumer->second);
            }
        },
        &consumer);
}

void BuildStage1BackdropCommands801CAC34(
    const Stage1SceneSubmitRuntime& runtime,
    PsxBackdropCommandConsumer801CAC34 consumeCommand,
    void* commandUserData) {
    const auto& runtimeState =
        Stage1SceneSubmitRuntimePrivateAccess::ConstImpl(runtime);
    if (!runtimeState.advanceFrameValid || consumeCommand == nullptr) {
        return;
    }

    const PsxBackdropSpriteCommand801CAC34* sprites =
        runtimeState.backdropSprites801CAC34.empty()
            ? nullptr
            : runtimeState.backdropSprites801CAC34.data();
    consumeCommand(runtimeState.backdropGradient801CAC34,
                   sprites,
                   runtimeState.backdropSprites801CAC34.size(),
                  commandUserData);
}

bool BuildStage1DrawEnvClearColorCommand80040060(
    const Stage1SceneSubmitRuntime& runtime,
    PsxDrawEnvClearColorCommand80040060& outCommand) {
    const auto& runtimeState =
        Stage1SceneSubmitRuntimePrivateAccess::ConstImpl(runtime);
    outCommand = runtimeState.drawEnvClearColor80040060;
    return runtimeState.advanceFrameValid && outCommand.valid;
}

bool BuildStage1PreventTransitionDrawPlan801CBFDC(
    const Stage1SceneSubmitRuntime& runtime,
    float vx,
    float vy,
    float vs,
    PrStage1Scene1Movie1Direct::Movie1DrawPlan& outPlan) {
    const auto& runtimeState =
        Stage1SceneSubmitRuntimePrivateAccess::ConstImpl(runtime);
    outPlan = PrStage1Scene1Movie1Direct::Movie1DrawPlan{};
    if (!runtimeState.advanceFrameValid ||
        !runtimeState.preventTransition801CBFDC.drawCall8001FDC0) {
        return false;
    }
    return PrStage1Scene1Movie1Direct::
        BuildTransitionTileMaskDrawPlanSub8001FDC0(
            runtimeState.preventTransitionTileMask801CBFDC,
            static_cast<uint16_t>(
                runtimeState.preventTransition801CBFDC.drawArg0),
            vx,
            vy,
            vs,
            outPlan);
}

void BuildStage1TimUploadCommands801CBFDC(
    const Stage1SceneSubmitRuntime& runtime,
    PsxTimUploadCommandConsumer801CBFDC consumeCommand,
    void* commandUserData) {
    if (consumeCommand == nullptr) {
        return;
    }
    const auto& runtimeState =
        Stage1SceneSubmitRuntimePrivateAccess::ConstImpl(runtime);
    if (!runtimeState.advanceFrameValid) {
        return;
    }
    for (const PsxTimUploadCommand801CBFDC& command :
         runtimeState.timUploadCommands801CBFDC) {
        consumeCommand(command, commandUserData);
    }
}

void BuildStage1ScriptBoxCommands80023E10(
    const Stage1SceneSubmitRuntime& runtime,
    PsxScriptBoxCommandConsumer80023E10 consumeCommand,
    void* commandUserData) {
    if (consumeCommand == nullptr) {
        return;
    }
    const auto& runtimeState =
        Stage1SceneSubmitRuntimePrivateAccess::ConstImpl(runtime);
    if (!runtimeState.advanceFrameValid) {
        return;
    }
    for (const PsxScriptBoxCommand80023E10& command :
         runtimeState.scriptBoxCommands80023E10) {
        consumeCommand(command, commandUserData);
    }
}

static PsxFastSpritePacketCommand8003FA20
BuildFastSpritePacketCommand8003FA20(
    const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& write,
    uint32_t packetMirrorOrder,
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20* runtime) {
    PsxFastSpritePacketCommand8003FA20 out{};
    out.valid = write.valid;
    out.packetAddr = write.addr;
    out.wordCount = write.wordCount;
    out.words = write.words;
    out.wordKnown = write.wordKnown;
    out.provenanceKnown = write.provenance.active;
    out.sourceKind = static_cast<uint8_t>(write.provenance.sourceKind);
    out.rawLocalProvenance =
        static_cast<uint8_t>(write.provenance.rawLocalProvenance);
    out.sourceFunction = write.provenance.sourceFunction;
    out.callsite = write.provenance.callsite;
    out.helper = write.provenance.helper;
    out.psxCallOrder =
        packetMirrorOrder != 0u ? packetMirrorOrder
                                : write.provenance.psxCallOrder;
    out.localObjectAddr = write.provenance.localObjectAddr;
    out.lastWriterPc = write.provenance.lastWriterPc;
    out.priority = write.provenance.priority;
    out.word2CommandKnown =
        write.word2CommandKnown ||
        (write.wordKnown.size() > 2u && write.wordKnown[2]);
    out.word2CommandCode = write.word2CommandKnown
        ? write.word2CommandCode
        : static_cast<uint8_t>((write.words[2] >> 24) & 0xFFu);
    if (runtime != nullptr && runtime->drawEnvOffsetKnown800401AC) {
        out.drawEnvOffsetKnown800401AC = true;
        out.drawEnvOffsetX80091738 = runtime->drawEnvOffsetX80091738;
        out.drawEnvOffsetY8009173A = runtime->drawEnvOffsetY8009173A;
    }
    const std::size_t knownCount =
        out.wordCount < out.wordKnown.size()
            ? static_cast<std::size_t>(out.wordCount)
            : out.wordKnown.size();
    out.allWordsKnown = out.valid && out.wordCount != 0u &&
                        knownCount == out.wordCount;
    for (std::size_t i = 0; i < knownCount; ++i) {
        out.allWordsKnown = out.allWordsKnown && out.wordKnown[i];
    }
    const auto rawLocalProvenance =
        static_cast<PrPsxFastSpriteSubmitDirect::
                        FastSpriteRawLocalProvenance8003FA20>(
            out.rawLocalProvenance);
    const bool colorProvenanceAuthoritative =
        rawLocalProvenance ==
            PrPsxFastSpriteSubmitDirect::
                FastSpriteRawLocalProvenance8003FA20::StaticLocalFields ||
        rawLocalProvenance ==
            PrPsxFastSpriteSubmitDirect::
                FastSpriteRawLocalProvenance8003FA20::
                    Stage1GlyphStackScratch8001B954 ||
        rawLocalProvenance ==
            PrPsxFastSpriteSubmitDirect::
                FastSpriteRawLocalProvenance8003FA20::
                    Stage1ScriptBoxCornerStackScratch8001B590;
    out.colorAuthoritative =
        out.wordKnown[2] && colorProvenanceAuthoritative;
    out.rawTextureKnown = out.word2CommandKnown;
    out.rawTexture = (out.word2CommandCode & 0x01u) != 0u;
    out.renderPayloadKnown =
        out.wordCount ==
            PrPsxFastSpriteSubmitDirect::
                kGsSortFastSpritePacketWordCount8003FA20 &&
        out.wordKnown[1] &&
        ((out.rawTextureKnown && out.rawTexture) || out.colorAuthoritative) &&
        out.wordKnown[3] &&
        out.wordKnown[4] && out.wordKnown[5];
    out.auditOnlyPartial = out.valid && !out.renderPayloadKnown;

    out.textureKnown = out.wordKnown[1] && out.wordKnown[4];
    if (out.textureKnown) {
        out.tpage = static_cast<uint16_t>(out.words[1] & 0x01FFu);
        out.u = static_cast<uint8_t>(out.words[4] & 0xFFu);
        out.v = static_cast<uint8_t>((out.words[4] >> 8) & 0xFFu);
        out.clut = static_cast<uint16_t>((out.words[4] >> 16) & 0xFFFFu);
    }

    out.colorKnown = out.wordKnown[2] && out.colorAuthoritative;
    if (out.colorKnown) {
        out.r = static_cast<uint8_t>(out.words[2] & 0xFFu);
        out.g = static_cast<uint8_t>((out.words[2] >> 8) & 0xFFu);
        out.b = static_cast<uint8_t>((out.words[2] >> 16) & 0xFFu);
    }

    out.geometryKnown = out.wordKnown[3] && out.wordKnown[5];
    if (out.geometryKnown) {
        out.x = static_cast<int16_t>(out.words[3] & 0xFFFFu);
        out.y = static_cast<int16_t>((out.words[3] >> 16) & 0xFFFFu);
        out.w = static_cast<uint16_t>(out.words[5] & 0xFFFFu);
        out.h = static_cast<uint16_t>((out.words[5] >> 16) & 0xFFFFu);
    }
    return out;
}

void BuildStage1FastSpritePacketCommands8003FA20(
    const Stage1SceneSubmitRuntime& runtime,
    PsxFastSpritePacketCommandConsumer8003FA20 consumeCommand,
    void* commandUserData) {
    if (consumeCommand == nullptr) {
        return;
    }
    const auto& runtimeState =
        Stage1SceneSubmitRuntimePrivateAccess::ConstImpl(runtime);
    if (!runtimeState.advanceFrameValid ||
        !runtimeState.graphOwner801CBFDC.mainPageWorkLists80087288Initialized) {
        return;
    }

    const uint8_t pageIndex = runtimeState.psxWorkSlotIndex8006EDA8 & 1u;
    const PrPsxGraphOwnerDirect::PsxGraphPageWorkList8001E374& pageWork =
        runtimeState.graphOwner801CBFDC.mainPageWorkLists80087288[pageIndex];
    if (!pageWork.work.packetWriteMirrorKnown) {
        return;
    }
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20 submitRuntime{};
    submitRuntime.packetWrites = pageWork.work.packetWriteMirror;
    submitRuntime.drawEnvOffsetKnown800401AC =
        runtimeState.graphOwner801CBFDC.drawOffset.setDrawEnvCalled;
    submitRuntime.drawEnvOffsetX80091738 =
        runtimeState.graphOwner801CBFDC.drawOffset.word_80091738;
    submitRuntime.drawEnvOffsetY8009173A =
        runtimeState.graphOwner801CBFDC.drawOffset.word_8009173A;
    BuildFastSpritePacketCommands8003FA20(submitRuntime,
                                          consumeCommand,
                                          commandUserData);
}

static bool ResolveGsSpritePriorityFromOtMirror8003F1B4(
    const PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8& work,
    uint32_t packetAddr,
    uint16_t& outPriority) {
    if (!work.gsSpriteOtSlotMirrorKnown || work.headAddr_04 == 0u) {
        return false;
    }
    const uint32_t packetLink =
        packetAddr &
        PrPsxGsSpriteSubmitDirect::kGsSortSpritePacketAddrMask8003F1B4;
    for (const PrPsxGsSpriteSubmitDirect::RuntimeOtSlotValue8003F1B4&
             slot : work.gsSpriteOtSlotMirror) {
        if (!slot.valid || slot.value != packetLink) {
            continue;
        }
        const uint32_t lengthBytes = work.x_08 * 4u;
        if (work.headAddr_04 < lengthBytes) {
            continue;
        }
        const uint32_t base = work.headAddr_04 - lengthBytes;
        if (slot.addr < base) {
            continue;
        }
        const uint32_t delta = slot.addr - base;
        if ((delta & 3u) != 0u) {
            continue;
        }
        const uint32_t priority = delta / 4u;
        if (priority > 0xFFFFu) {
            continue;
        }
        outPriority = static_cast<uint16_t>(priority);
        return true;
    }
    return false;
}

static PsxGsSpritePacketCommand8003F1B4
BuildGsSpritePacketCommand8003F1B4(
    const PrPsxGsSpriteSubmitDirect::RuntimePacketWrite8003F1B4& write,
    const PrPsxGraphOwnerDirect::PsxGraphWorkList80040CC8& work,
    uint32_t packetMirrorOrder) {
    PsxGsSpritePacketCommand8003F1B4 out{};
    out.valid = write.valid;
    out.packetAddr = write.addr;
    out.wordCount = write.wordCount;
    out.words = write.words;
    out.wordKnown = write.wordKnown;
    out.psxCallOrder =
        write.submitOrderKnown ? write.submitOrder : packetMirrorOrder;
    out.priorityKnown = write.priorityKnown;
    out.priority = write.priority;
    uint16_t otMirrorPriority = 0u;
    const bool otMirrorPriorityKnown =
        ResolveGsSpritePriorityFromOtMirror8003F1B4(work,
                                                    write.addr,
                                                    otMirrorPriority);
    if (out.priorityKnown && otMirrorPriorityKnown &&
        out.priority != otMirrorPriority) {
        out.valid = false;
        return out;
    }
    if (!out.priorityKnown && otMirrorPriorityKnown) {
        out.priorityKnown = true;
        out.priority = otMirrorPriority;
    }
    if (!out.valid ||
        (out.wordCount !=
             PrPsxGsSpriteSubmitDirect::
                 kGsSortSpritePacketFastTotalWords8003F1B4 &&
         out.wordCount !=
             PrPsxGsSpriteSubmitDirect::
                 kGsSortSpritePacketTransformTotalWords8003F1B4)) {
        out.valid = false;
        return out;
    }

    const std::size_t knownCount =
        out.wordCount < out.wordKnown.size()
            ? static_cast<std::size_t>(out.wordCount)
            : out.wordKnown.size();
    out.allWordsKnown = knownCount == out.wordCount && out.wordCount != 0u;
    for (std::size_t i = 0; i < knownCount; ++i) {
        out.allWordsKnown = out.allWordsKnown && out.wordKnown[i];
    }

    out.transformPath =
        write.packetPath ==
        PrPsxGsSpriteSubmitDirect::GsSortSpritePacketPath8003F1B4::Transform;
    if (write.packetPath ==
        PrPsxGsSpriteSubmitDirect::GsSortSpritePacketPath8003F1B4::None) {
        out.transformPath =
            out.wordCount ==
            PrPsxGsSpriteSubmitDirect::
                kGsSortSpritePacketTransformTotalWords8003F1B4;
    }
    if (!out.transformPath) {
        out.colorKnown = out.wordKnown[2];
        out.geometryKnown = out.wordKnown[3] && out.wordKnown[5];
        out.textureKnown = out.wordKnown[1] && out.wordKnown[4];
        if (out.colorKnown) {
            const uint32_t color = out.words[2];
            out.r = static_cast<uint8_t>(color & 0xFFu);
            out.g = static_cast<uint8_t>((color >> 8) & 0xFFu);
            out.b = static_cast<uint8_t>((color >> 16) & 0xFFu);
        }
        if (out.geometryKnown) {
            out.x[0] = static_cast<int16_t>(out.words[3] & 0xFFFFu);
            out.y[0] = static_cast<int16_t>((out.words[3] >> 16) & 0xFFFFu);
            out.w = static_cast<uint16_t>(out.words[5] & 0xFFFFu);
            out.h = static_cast<uint16_t>((out.words[5] >> 16) & 0xFFFFu);
        }
        if (out.textureKnown) {
            out.tpage = static_cast<uint16_t>(out.words[1] & 0x01FFu);
            out.u[0] = static_cast<uint8_t>(out.words[4] & 0xFFu);
            out.v[0] = static_cast<uint8_t>((out.words[4] >> 8) & 0xFFu);
            out.clut = static_cast<uint16_t>((out.words[4] >> 16) & 0xFFFFu);
        }
    } else {
        out.colorKnown = out.wordKnown[1];
        out.geometryKnown =
            out.wordKnown[2] && out.wordKnown[4] &&
            out.wordKnown[6] && out.wordKnown[8];
        out.textureKnown =
            out.wordKnown[3] && out.wordKnown[5] &&
            out.wordKnown[7] && out.wordKnown[9];
        if (out.colorKnown) {
            const uint32_t color = out.words[1];
            out.r = static_cast<uint8_t>(color & 0xFFu);
            out.g = static_cast<uint8_t>((color >> 8) & 0xFFu);
            out.b = static_cast<uint8_t>((color >> 16) & 0xFFu);
        }
        if (out.geometryKnown) {
            constexpr std::array<uint8_t, 4> kSxyWords{{2u, 4u, 6u, 8u}};
            for (std::size_t i = 0; i < kSxyWords.size(); ++i) {
                const uint32_t sxy = out.words[kSxyWords[i]];
                out.x[i] = static_cast<int16_t>(sxy & 0xFFFFu);
                out.y[i] = static_cast<int16_t>((sxy >> 16) & 0xFFFFu);
            }
        }
        if (out.textureKnown) {
            constexpr std::array<uint8_t, 4> kUvWords{{3u, 5u, 7u, 9u}};
            for (std::size_t i = 0; i < kUvWords.size(); ++i) {
                const uint32_t uv = out.words[kUvWords[i]];
                out.u[i] = static_cast<uint8_t>(uv & 0xFFu);
                out.v[i] = static_cast<uint8_t>((uv >> 8) & 0xFFu);
            }
            out.clut = static_cast<uint16_t>((out.words[3] >> 16) & 0xFFFFu);
            out.tpage = static_cast<uint16_t>((out.words[5] >> 16) & 0x01FFu);
        }
    }

    out.renderPayloadKnown =
        out.valid && out.priorityKnown && out.allWordsKnown &&
        out.geometryKnown && out.colorKnown && out.textureKnown;
    return out;
}

void BuildStage1GsSpritePacketCommands8003F1B4(
    const Stage1SceneSubmitRuntime& runtime,
    PsxGsSpritePacketCommandConsumer8003F1B4 consumeCommand,
    void* commandUserData) {
    if (consumeCommand == nullptr) {
        return;
    }
    const auto& runtimeState =
        Stage1SceneSubmitRuntimePrivateAccess::ConstImpl(runtime);
    if (!runtimeState.advanceFrameValid ||
        !runtimeState.graphOwner801CBFDC.mainPageWorkLists80087288Initialized) {
        return;
    }

    const uint8_t pageIndex = runtimeState.psxWorkSlotIndex8006EDA8 & 1u;
    const PrPsxGraphOwnerDirect::PsxGraphPageWorkList8001E374& pageWork =
        runtimeState.graphOwner801CBFDC.mainPageWorkLists80087288[pageIndex];
    if (!pageWork.work.gsSpritePacketWriteMirrorKnown ||
        !pageWork.work.gsSpriteOtSlotMirrorKnown) {
        return;
    }

    std::vector<const PrPsxGsSpriteSubmitDirect::RuntimePacketWrite8003F1B4*>
        validWrites;
    for (const PrPsxGsSpriteSubmitDirect::RuntimePacketWrite8003F1B4& write :
         pageWork.work.gsSpritePacketWriteMirror) {
        if (!write.valid) {
            continue;
        }
        validWrites.push_back(&write);
    }

    uint32_t packetMirrorOrder = 1u;
    for (auto it = validWrites.rbegin(); it != validWrites.rend(); ++it) {
        const PsxGsSpritePacketCommand8003F1B4 command =
            BuildGsSpritePacketCommand8003F1B4(**it,
                                              pageWork.work,
                                              packetMirrorOrder++);
        if (command.valid) {
            consumeCommand(command, commandUserData);
        }
    }
}

void BuildFastSpritePacketCommands8003FA20(
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime,
    PsxFastSpritePacketCommandConsumer8003FA20 consumeCommand,
    void* commandUserData) {
    if (consumeCommand == nullptr) {
        return;
    }
    std::vector<const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20*>
        validWrites;
    for (const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& write :
         runtime.packetWrites) {
        if (!write.valid) {
            continue;
        }
        validWrites.push_back(&write);
    }

    uint32_t packetMirrorOrder = 1u;
    for (auto it = validWrites.rbegin(); it != validWrites.rend(); ++it) {
        consumeCommand(BuildFastSpritePacketCommand8003FA20(**it,
                                                            packetMirrorOrder++,
                                                            &runtime),
                       commandUserData);
    }
}

static PsxBoxFillPacketCommand8003EE84 BuildBoxFillPacketCommand8003EE84(
    const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& write,
    uint32_t packetMirrorOrder) {
    PsxBoxFillPacketCommand8003EE84 out{};
    out.valid = write.valid && write.wordCount == 5u &&
                write.provenance.sourceFunction ==
                    kFn8003EE84_GsSortBoxFill;
    if (!out.valid) {
        return out;
    }

    out.packetAddr = write.addr;
    out.psxCallOrder =
        packetMirrorOrder != 0u ? packetMirrorOrder
                                : write.provenance.psxCallOrder;
    out.provenanceKnown = write.provenance.active;
    out.sourceKind = static_cast<uint8_t>(write.provenance.sourceKind);
    out.sourceFunction = write.provenance.sourceFunction;
    out.priority = write.provenance.priority;
    out.wordsKnown = true;
    for (uint32_t i = 0; i < 5u; ++i) {
        out.wordsKnown = out.wordsKnown && write.wordKnown[i];
    }
    out.word0LinkTag = write.words[0];
    out.word1DrawMode = write.words[1];
    out.word2ColorCode = write.words[2];
    out.word3Xy = write.words[3];
    out.word4Wh = write.words[4];
    if (out.wordsKnown) {
        out.r = static_cast<uint8_t>(out.word2ColorCode & 0xFFu);
        out.g = static_cast<uint8_t>((out.word2ColorCode >> 8) & 0xFFu);
        out.b = static_cast<uint8_t>((out.word2ColorCode >> 16) & 0xFFu);
        out.commandCode =
            static_cast<uint8_t>((out.word2ColorCode >> 24) & 0xFFu);
        out.x = static_cast<int16_t>(out.word3Xy & 0xFFFFu);
        out.y = static_cast<int16_t>((out.word3Xy >> 16) & 0xFFFFu);
        out.w = static_cast<uint16_t>(out.word4Wh & 0xFFFFu);
        out.h = static_cast<uint16_t>((out.word4Wh >> 16) & 0xFFFFu);
    }
    return out;
}

void BuildStage1BoxFillPacketCommands8003EE84(
    const Stage1SceneSubmitRuntime& runtime,
    PsxBoxFillPacketCommandConsumer8003EE84 consumeCommand,
    void* commandUserData) {
    if (consumeCommand == nullptr) {
        return;
    }
    const auto& runtimeState =
        Stage1SceneSubmitRuntimePrivateAccess::ConstImpl(runtime);
    if (!runtimeState.advanceFrameValid ||
        !runtimeState.graphOwner801CBFDC.mainPageWorkLists80087288Initialized) {
        return;
    }

    const uint8_t pageIndex = runtimeState.psxWorkSlotIndex8006EDA8 & 1u;
    const PrPsxGraphOwnerDirect::PsxGraphPageWorkList8001E374& pageWork =
        runtimeState.graphOwner801CBFDC.mainPageWorkLists80087288[pageIndex];
    if (!pageWork.work.packetWriteMirrorKnown) {
        return;
    }

    std::vector<const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20*>
        validWrites;
    for (const PrPsxFastSpriteSubmitDirect::RuntimePacketWrite8003FA20& write :
         pageWork.work.packetWriteMirror) {
        if (!write.valid) {
            continue;
        }
        validWrites.push_back(&write);
    }

    uint32_t packetMirrorOrder = 1u;
    for (auto it = validWrites.rbegin(); it != validWrites.rend(); ++it) {
        const PsxBoxFillPacketCommand8003EE84 command =
            BuildBoxFillPacketCommand8003EE84(**it, packetMirrorOrder++);
        if (command.valid) {
            consumeCommand(command, commandUserData);
        }
    }
}

void BuildStage1CompactRailSpriteCommands80024744(
    const Stage1SceneSubmitRuntime& runtime,
    PsxCompactRailSpriteCommandConsumer80024744 consumeCommand,
    void* commandUserData) {
    if (consumeCommand == nullptr) {
        return;
    }
    const auto& runtimeState =
        Stage1SceneSubmitRuntimePrivateAccess::ConstImpl(runtime);
    if (!runtimeState.advanceFrameValid) {
        return;
    }
    for (const PsxCompactRailSpriteCommand80024744& command :
         runtimeState.compactRailSpriteCommands80024744) {
        consumeCommand(command, commandUserData);
    }
}

} // namespace PrStageSceneSubmitDirect
