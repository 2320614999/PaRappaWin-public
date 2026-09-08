#include "pr/pr_ss0_title_transform_direct.h"
#include "pr/pr_ss0_title_tmd_backend.h"

#include <array>
#include <cstdio>
#include <limits>
#include <vector>

namespace {

using namespace PrSS0TitleTransformDirect;

int g_failedChecks = 0;

#define CHECK(expr)                                                       \
    do {                                                                  \
        if (!(expr)) {                                                    \
            std::printf("CHECK failed %s:%d: %s\n", __FILE__, __LINE__, \
                        #expr);                                           \
            ++g_failedChecks;                                             \
        }                                                                 \
    } while (0)

bool SameTriangle(
    const std::array<PrPsxGteDirect::VertexS16, 3>& left,
    const std::array<PrPsxGteDirect::VertexS16, 3>& right)
{
    for (std::size_t index = 0u; index < left.size(); ++index) {
        if (left[index].x != right[index].x ||
            left[index].y != right[index].y ||
            left[index].z != right[index].z ||
            left[index].pad != right[index].pad) {
            return false;
        }
    }
    return true;
}

PrPsxGteDirect::GteControlState MakeTitleGteControl()
{
    PrPsxGteDirect::GteControlState control{};
    control.geomScreenKnown = true;
    control.geomScreen = 440u;
    control.geomOffsetKnown = true;
    control.geomOffsetX = 0;
    control.geomOffsetY = 0;
    control.depthCueKnown = true;
    control.depthCueA = -4194;
    control.depthCueB = 0x01400000;
    control.zScaleFactorKnown = true;
    control.zScaleFactor3 = 341;
    control.zScaleFactor4 = 256;
    return control;
}

TodData MakePaLoc2Tod()
{
    TodData tod{};
    tod.rawBlockCount = 1;
    tod.blockCount = 1;
    tod.rawBytes.resize(144u);
    tod.blocks.resize(1u);

    TodBlock& block = tod.blocks[0];
    block.rawOffset = 8u;
    block.rawSize = 132u;
    block.unk0 = 0x21u;
    block.cmdCount = 9u;
    block.triggerTime = 0u;
    block.commands.resize(9u);
    for (TodCommand& command : block.commands) {
        command.header = 0x01000000u;
    }

    TodCommand& coord = block.commands[8];
    coord.header = 0x09040001u;
    coord.data = {
        0x00001000u,
        0x00000000u,
        0x00001000u,
        0x00000000u,
        0x00001000u,
        0x00000000u,
        0x00000000u,
        0x00000320u,
    };
    return tod;
}

TodData MakeTitleRuntimeTod()
{
    TodData tod{};
    tod.rawBlockCount = 2u;
    tod.blockCount = 2u;
    tod.blocks.resize(2u);

    TodBlock& first = tod.blocks[0];
    first.cmdCount = 1u;
    first.triggerTime = 0u;
    first.commands.resize(1u);
    first.commands[0].header = 0x09040001u;
    first.commands[0].data = {
        0x00001000u, 0u, 0x00001000u, 0u, 0x00001000u,
        10u, 20u, 30u,
    };

    TodBlock& second = tod.blocks[1];
    second.cmdCount = 1u;
    second.triggerTime = 2u;
    second.commands.resize(1u);
    second.commands[0].header = 0x09040001u;
    second.commands[0].data = {
        0xF0000000u, 0x10000000u, 0u, 0u, 0x00001000u,
        static_cast<uint32_t>(-40), 50u, 60u,
    };
    return tod;
}

TmdObject MakeFirstPaKagePrimitive()
{
    TmdObject object{};
    object.vertices = {
        {-755, -1268, -185, 0},
        {-753, -977, -78, 0},
        {-503, -978, -78, 0},
    };

    TmdPrimitive primitive{};
    primitive.rawPacketKnown = true;
    primitive.rawPacketByteSize = 28u;
    primitive.mode = 0x25u;
    primitive.flag = 1u;
    primitive.ilen = 6u;
    primitive.olen = 7u;
    primitive.textured = true;
    primitive.quad = false;
    primitive.v0_idx = 0u;
    primitive.v1_idx = 2u;
    primitive.v2_idx = 1u;
    object.primitives.push_back(primitive);
    return object;
}

VdfData MakeVdf()
{
    VdfData vdf{};
    vdf.keys = 2u;
    vdf.keyList.resize(2u);

    VdfKey& first = vdf.keyList[0];
    first.obj = 0u;
    first.vertTop = 0u;
    first.nVert = 3u;
    first.deltas = {
        {4096, -4096, 2048, 0},
        {0, 4096, -4096, 0},
        {-4096, 0, 4096, 0},
    };

    VdfKey& second = vdf.keyList[1];
    second.obj = 0u;
    second.vertTop = 0u;
    second.nVert = 3u;
    second.deltas = {
        {2048, 2048, 2048, 0},
        {2048, 2048, 2048, 0},
        {2048, 2048, 2048, 0},
    };
    return vdf;
}

DatData MakeDat()
{
    DatData dat{};
    dat.keys = 2u;
    dat.maxFrames = 2u;
    dat.keyList.resize(2u);
    dat.keyList[0].frames = 2u;
    dat.keyList[0].influence = {4096, 2048};
    dat.keyList[1].frames = 2u;
    dat.keyList[1].influence = {0, -4096};
    return dat;
}

MimeRuntimeInput80013EA8 MakeRuntimeMime(const VdfData& vdf,
                                         const DatData& dat)
{
    MimeRuntimeInput80013EA8 runtime{};
    runtime.bindingKnown = true;
    runtime.vdf = &vdf;
    runtime.dat = &dat;
    runtime.objectIndex = 0u;
    runtime.sampleFrameKnown = true;
    runtime.sampleFrame = 1u;
    runtime.loopFlagKnown = true;
    runtime.loopFlag = 0u;
    return runtime;
}

PrPsxGteDirect::Matrix3x4 MakeMatrix(
    const std::array<std::array<int16_t, 3>, 3>& rotation,
    const std::array<int32_t, 3>& translation)
{
    const auto pair = [](int16_t low, int16_t high) {
        return static_cast<uint32_t>(static_cast<uint16_t>(low)) |
               (static_cast<uint32_t>(static_cast<uint16_t>(high)) << 16u);
    };
    PrPsxGteDirect::Matrix3x4 matrix{};
    matrix.words[0] = pair(rotation[0][0], rotation[0][1]);
    matrix.words[1] = pair(rotation[0][2], rotation[1][0]);
    matrix.words[2] = pair(rotation[1][1], rotation[1][2]);
    matrix.words[3] = pair(rotation[2][0], rotation[2][1]);
    matrix.words[4] = pair(rotation[2][2], 0);
    matrix.words[5] = static_cast<uint32_t>(translation[0]);
    matrix.words[6] = static_cast<uint32_t>(translation[1]);
    matrix.words[7] = static_cast<uint32_t>(translation[2]);
    return matrix;
}

std::array<std::array<int16_t, 3>, 3> IdentityRotation()
{
    return {{{{4096, 0, 0}}, {{0, 4096, 0}}, {{0, 0, 4096}}}};
}

void WriteSigned32LittleEndian(std::vector<uint8_t>& bytes,
                              std::size_t offset,
                              int32_t value)
{
    const uint32_t bits = static_cast<uint32_t>(value);
    bytes[offset + 0u] = static_cast<uint8_t>(bits & 0xFFu);
    bytes[offset + 1u] = static_cast<uint8_t>((bits >> 8u) & 0xFFu);
    bytes[offset + 2u] = static_cast<uint8_t>((bits >> 16u) & 0xFFu);
    bytes[offset + 3u] = static_cast<uint8_t>((bits >> 24u) & 0xFFu);
}

void WriteTitleCameraRow(
    std::vector<uint8_t>& bytes,
    std::size_t index,
    const std::array<int32_t, 3>& position,
    const std::array<int32_t, 3>& target,
    int32_t twist = 0,
    uint32_t superAddress = 0u)
{
    const std::size_t offset =
        kTitleCameraTableOffset801C6FB4 +
        index * kTitleCameraRecordStride801C6FB4;
    for (std::size_t lane = 0u; lane < 3u; ++lane) {
        WriteSigned32LittleEndian(bytes, offset + lane * 4u,
                                  position[lane]);
        WriteSigned32LittleEndian(bytes, offset + (lane + 3u) * 4u,
                                  target[lane]);
    }
    WriteSigned32LittleEndian(bytes, offset + 0x18u, twist);
    WriteSigned32LittleEndian(
        bytes, offset + 0x1Cu, static_cast<int32_t>(superAddress));
}

std::vector<uint8_t> MakeExactTitleCameraComod0()
{
    std::vector<uint8_t> bytes(kScene0Comod0ExpectedBytes, 0u);
    for (std::size_t index = 0u;
         index < kTitleCameraRecordCount801C6FB4;
         ++index) {
        WriteTitleCameraRow(
            bytes, index, {{78, -1437, -9869}}, {{0, -1400, 0}});
    }
    WriteTitleCameraRow(
        bytes, 1u, {{156, -1474, -9768}}, {{0, -1400, 0}});
    WriteTitleCameraRow(
        bytes, 200u, {{1784, -3545, -6791}}, {{-8, -1613, 69}});
    WriteTitleCameraRow(
        bytes, 299u, {{0, -1600, -8790}}, {{0, -1600, 0}});
    return bytes;
}

void TestExactTitleCameraTable801C6FB4()
{
    std::vector<uint8_t> comod0 = MakeExactTitleCameraComod0();
    TitleCameraTable801C6FB4 table{};
    CHECK(ParseExactTitleCameraTable801C6FB4(
        comod0.data(), comod0.size(), table));
    CHECK(table.known);
    CHECK(table.views[0].position ==
          (std::array<int32_t, 3>{{78, -1437, -9869}}));
    CHECK(table.views[1].position ==
          (std::array<int32_t, 3>{{156, -1474, -9768}}));
    CHECK(table.views[200].target ==
          (std::array<int32_t, 3>{{-8, -1613, 69}}));
    CHECK(table.views[299].position ==
          (std::array<int32_t, 3>{{0, -1600, -8790}}));

    std::vector<uint8_t> truncated = comod0;
    truncated.pop_back();
    CHECK(!ParseExactTitleCameraTable801C6FB4(
        truncated.data(), truncated.size(), table));
    CHECK(!table.known);

    std::vector<uint8_t> unsupported = comod0;
    WriteTitleCameraRow(
        unsupported, 2u, {{78, -1437, -9869}}, {{0, -1400, 0}}, 1);
    CHECK(!ParseExactTitleCameraTable801C6FB4(
        unsupported.data(), unsupported.size(), table));

    std::vector<uint8_t> wrongAnchor = comod0;
    WriteTitleCameraRow(
        wrongAnchor, 200u, {{1785, -3545, -6791}}, {{-8, -1613, 69}});
    CHECK(!ParseExactTitleCameraTable801C6FB4(
        wrongAnchor.data(), wrongAnchor.size(), table));
}

void TestTitleCameraMatrix80041D3C()
{
    TitleCameraView801C6FB4 view{};
    TitleCameraMatrixResult80041D3C matrix =
        BuildTitleCameraMatrix80041D3C(view);
    CHECK(!matrix.cameraMatrixKnown80092880);
    CHECK(matrix.failure == Failure::TitleCameraSourceUnknown);

    view.known = true;
    view.position = {{78, -1437, -9869}};
    view.target = {{0, -1400, 0}};
    matrix = BuildTitleCameraMatrix80041D3C(view);
    CHECK(matrix.cameraMatrixKnown80092880);
    CHECK(matrix.failure == Failure::None);
    CHECK(matrix.cameraMatrix80092880.words ==
          (std::array<uint32_t, 8>{{
              0x00000FFFu, 0x00000020u, 0xFFF10FFFu, 0x000FFFE0u,
              0x00000FFEu, 0xFFFFFFFFu, 0x00000578u, 0x0000268Eu,
          }}));

    view.position = {{1784, -3545, -6791}};
    view.target = {{-8, -1613, 69}};
    matrix = BuildTitleCameraMatrix80041D3C(view);
    CHECK(matrix.cameraMatrixKnown80092880);
    CHECK(matrix.cameraMatrix80092880.words ==
          (std::array<uint32_t, 8>{{
              0x00000F7Bu, 0x010F040Au, 0xFBEE0F6Fu, 0x0434FC1Au,
              0x00000EEEu, 0xFFFFFFF4u, 0x00000625u, 0x00001E16u,
          }}));

    view.target = view.position;
    matrix = BuildTitleCameraMatrix80041D3C(view);
    CHECK(!matrix.cameraMatrixKnown80092880);
    CHECK(matrix.failure == Failure::TitleCameraDegenerateView);

    view.target = {{-8, -1613, 69}};
    view.twist = 1;
    matrix = BuildTitleCameraMatrix80041D3C(view);
    CHECK(matrix.failure == Failure::TitleCameraTwistUnsupported);
    view.twist = 0;
    view.superAddress = 0x80100000u;
    matrix = BuildTitleCameraMatrix80041D3C(view);
    CHECK(matrix.failure == Failure::TitleCameraSuperUnsupported);
}

void TestTitleCameraCursorReset801CB59C()
{
    std::vector<uint8_t> comod0 = MakeExactTitleCameraComod0();
    TitleCameraTable801C6FB4 table{};
    CHECK(ParseExactTitleCameraTable801C6FB4(
        comod0.data(), comod0.size(), table));

    TitleCameraCursorState801CB59C state{};
    TitleCameraAdvanceResult801C6410 advance =
        AdvanceTitleCamera801C6410(state, &table);
    CHECK(!advance.accepted);
    CHECK(advance.failure == Failure::TitleCameraCursorUnknown);

    state.cursorKnown = true;
    advance = AdvanceTitleCamera801C6410(state, &table);
    CHECK(advance.accepted);
    CHECK(advance.sampledIndex == 0u);
    CHECK(advance.nextState.cursor == 1u);
    CHECK(advance.cameraMatrixKnown80092880);
    CHECK(advance.cameraMatrix80092880.words[0] == 0x00000FFFu);

    advance = AdvanceTitleCamera801C6410(advance.nextState, &table);
    CHECK(advance.accepted);
    CHECK(advance.sampledIndex == 1u);
    CHECK(advance.nextState.cursor == 2u);

    state.cursor = 299u;
    advance = AdvanceTitleCamera801C6410(state, &table);
    CHECK(advance.accepted);
    CHECK(advance.sampledIndex == 299u);
    CHECK(advance.nextState.cursor == 300u);
    advance = AdvanceTitleCamera801C6410(advance.nextState, &table);
    CHECK(!advance.accepted);
    CHECK(advance.sourceExhausted);
    CHECK(!advance.cameraMatrixKnown80092880);
    CHECK(advance.nextState.cursor == 300u);

    state.cursor = 301u;
    advance = AdvanceTitleCamera801C6410(state, &table);
    CHECK(!advance.accepted);
    CHECK(advance.failure == Failure::TitleCameraCursorOutOfRange);

    state.cursor = 2u;
    table.views[2].twist = 1;
    advance = AdvanceTitleCamera801C6410(state, &table);
    CHECK(!advance.accepted);
    CHECK(advance.failure == Failure::TitleCameraTwistUnsupported);
    CHECK(advance.nextState.cursor == 2u);
    CHECK(!advance.cameraMatrixKnown80092880);
}

void TestStrictPaLoc2CoordExtraction()
{
    TodCoordMatrix coord{};
    TodData tod = MakePaLoc2Tod();
    CHECK(ExtractPaLoc2Coord80028054(tod, coord));
    CHECK(coord.m[0][0] == 4096);
    CHECK(coord.m[1][1] == 4096);
    CHECK(coord.m[2][2] == 4096);
    CHECK(coord.t[0] == 0);
    CHECK(coord.t[1] == 0);
    CHECK(coord.t[2] == 800);

    tod.blocks[0].commands[7] = tod.blocks[0].commands[8];
    CHECK(!ExtractPaLoc2Coord80028054(tod, coord));

    tod = MakePaLoc2Tod();
    tod.blocks[0].commands[8].data[7] = 799u;
    CHECK(!ExtractPaLoc2Coord80028054(tod, coord));

    tod = MakePaLoc2Tod();
    tod.blocks[0].rawSize = 136u;
    CHECK(!ExtractPaLoc2Coord80028054(tod, coord));

    tod = MakePaLoc2Tod();
    tod.rawBytes[140] = 1u;
    CHECK(!ExtractPaLoc2Coord80028054(tod, coord));
}

void TestBaseCarrierKeepsRuntimeSourcesUnknown()
{
    const TmdObject object = MakeFirstPaKagePrimitive();
    const PrPsxGteDirect::GteControlState control = MakeTitleGteControl();
    const BaseCarrierResult base =
        BuildMode25BaseTriangleCarrier801C5E60(
            &control, &object, 0u, 0u, true, ModelPath::PaKage);
    CHECK(base.baseCarrierReady);
    CHECK(base.failure == Failure::None);
    CHECK(base.carrier.known);
    CHECK(base.carrier.control.geomScreenKnown);
    CHECK(base.carrier.control.geomScreen == 440u);
    CHECK(base.carrier.control.geomOffsetX == 0);
    CHECK(base.carrier.control.geomOffsetY == 0);
    CHECK(base.carrier.control.zScaleFactor3 == 341);
    CHECK(base.carrier.vertexIndices ==
          (std::array<uint16_t, 3>{{0u, 2u, 1u}}));
    CHECK(base.carrier.objectIndex == 0u);
    CHECK(base.carrier.objectVertexCount == 3u);
    CHECK(base.carrier.path == ModelPath::PaKage);
    CHECK(base.carrier.baseVertices[0].x == -755);
    CHECK(base.carrier.baseVertices[0].y == -1268);
    CHECK(base.carrier.baseVertices[0].z == -185);
    CHECK(base.carrier.baseVertices[1].x == -503);
    CHECK(base.carrier.baseVertices[1].y == -978);
    CHECK(base.carrier.baseVertices[1].z == -78);
    CHECK(base.carrier.baseVertices[2].x == -753);
    CHECK(base.carrier.baseVertices[2].y == -977);
    CHECK(base.carrier.baseVertices[2].z == -78);
    CHECK(!base.carrier.runtimeMimeBindingKnown);
    CHECK(!base.carrier.runtimeMimeCursorKnown);
    CHECK(!base.carrier.runtimeVertexDeformationKnown);
    CHECK(!base.carrier.runtimeMatrixKnown);
    CHECK(!base.carrier.rtpt3InputKnown);
}

void TestExactMimeDeformationAndRtpt3Join()
{
    const TmdObject object = MakeFirstPaKagePrimitive();
    const PrPsxGteDirect::GteControlState control = MakeTitleGteControl();
    const BaseCarrierResult base =
        BuildMode25BaseTriangleCarrier801C5E60(
            &control, &object, 0u, 0u, true, ModelPath::PaKage);
    const VdfData vdf = MakeVdf();
    const DatData dat = MakeDat();
    MimeRuntimeInput80013EA8 runtime = MakeRuntimeMime(vdf, dat);

    const DeformationResult deformed =
        ApplyMode25Mime80013EA8(base.carrier, runtime);
    CHECK(deformed.deformedTriangleReady);
    CHECK(deformed.failure == Failure::None);
    CHECK(deformed.carrier.known);
    CHECK(deformed.carrier.path == ModelPath::PaKage);
    CHECK(deformed.carrier.sampleFrame == 1u);
    CHECK(deformed.carrier.vertices[0].x == -755);
    CHECK(deformed.carrier.vertices[0].y == -5364);
    CHECK(deformed.carrier.vertices[0].z == -1209);
    CHECK(deformed.carrier.vertices[1].x == -4599);
    CHECK(deformed.carrier.vertices[1].y == -3026);
    CHECK(deformed.carrier.vertices[1].z == -78);
    CHECK(deformed.carrier.vertices[2].x == -2801);
    CHECK(deformed.carrier.vertices[2].y == -977);
    CHECK(deformed.carrier.vertices[2].z == -4174);

    runtime.sampleFrame = 3u;
    runtime.loopFlag = 1u;
    const DeformationResult looped =
        ApplyMode25Mime80013EA8(base.carrier, runtime);
    CHECK(looped.deformedTriangleReady);
    CHECK(looped.carrier.sampleFrame == 1u);
    CHECK(looped.carrier.vertices[1].x == -4599);

    DatData zeroDat = dat;
    zeroDat.keyList[0].influence = {0, 0};
    zeroDat.keyList[1].influence = {0, 0};
    runtime = MakeRuntimeMime(vdf, zeroDat);
    const DeformationResult zero =
        ApplyMode25Mime80013EA8(base.carrier, runtime);
    CHECK(zero.deformedTriangleReady);
    CHECK(SameTriangle(zero.carrier.vertices,
                       base.carrier.baseVertices));

    CHECK(!BuildMode25Rtpt3Input801C5E60(
               Mode25DeformedTriangleCarrier{}, nullptr, false)
               .ready);
    const Rtpt3InputResult missingMatrix =
        BuildMode25Rtpt3Input801C5E60(
            deformed.carrier, nullptr, false);
    CHECK(!missingMatrix.ready);
    CHECK(missingMatrix.failure == Failure::RuntimeMatrixUnknown);

    PrPsxGteDirect::Matrix3x4 matrix{};
    matrix.words = {{1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u}};
    const Rtpt3InputResult input =
        BuildMode25Rtpt3Input801C5E60(
            deformed.carrier, &matrix, true);
    CHECK(input.ready);
    CHECK(input.failure == Failure::None);
    CHECK(input.input.known);
    CHECK(input.input.matrix.words == matrix.words);
    CHECK(SameTriangle(input.input.vertices,
                       deformed.carrier.vertices));
    CHECK(input.input.path == ModelPath::PaKage);
    CHECK(input.input.sampleFrame == 1u);

    CHECK(!ExecuteMode25Rtpt3At800428B0(Mode25Rtpt3Input{}).ready);
    PrPsxGteDirect::Matrix3x4 exactMatrix{};
    exactMatrix.words = {{
        0x00001000u,
        0x00000000u,
        0x00001000u,
        0x00000000u,
        0x00001000u,
        0u,
        0u,
        2000u,
    }};
    const Rtpt3InputResult executableInput =
        BuildMode25Rtpt3Input801C5E60(
            zero.carrier, &exactMatrix, true);
    CHECK(executableInput.ready);
    const Rtpt3ExecutionResult800428B0 executed =
        ExecuteMode25Rtpt3At800428B0(executableInput.input);
    CHECK(executed.ready);
    CHECK(executed.failure == Failure::None);
    CHECK(executed.input.known);
    CHECK(executed.input.path == ModelPath::PaKage);
    CHECK(executed.input.sampleFrame == 1u);
    CHECK(executed.output.known);
    CHECK(executed.output.flagAfterRtptKnown);
    CHECK(executed.output.ir0Known);
    CHECK(!executed.output.szAfterRtpt.elementKnown[0]);
    CHECK(executed.output.szAfterRtpt.sz[1] == 1815u);
    CHECK(executed.output.szAfterRtpt.sz[2] == 1922u);
    CHECK(executed.output.szAfterRtpt.sz[3] == 1922u);
    CHECK(executed.geometryReady);
    CHECK(executed.geometry.projectedSxyKnown);
    CHECK(executed.geometry.nclip.known);
    CHECK(executed.geometry.visibilityKnown);
    CHECK(executed.geometry.avsz3.known);
    CHECK(executed.geometry.otzKnown);
    CHECK(executed.geometry.otz == 471u);
}

void TestAllModelPathsPreserveTransformProvenance()
{
    const TmdObject object = MakeFirstPaKagePrimitive();
    const PrPsxGteDirect::GteControlState control = MakeTitleGteControl();
    const VdfData vdf = MakeVdf();
    DatData dat = MakeDat();
    dat.keyList[0].influence = {0, 0};
    dat.keyList[1].influence = {0, 0};
    const MimeRuntimeInput80013EA8 runtime = MakeRuntimeMime(vdf, dat);
    const PrPsxGteDirect::Matrix3x4 matrix =
        MakeMatrix(IdentityRotation(), {{0, 0, 2000}});
    const std::array<ModelPath, 4> paths = {{
        ModelPath::Pa,
        ModelPath::PaKage,
        ModelPath::Lo,
        ModelPath::Hp,
    }};

    for (ModelPath path : paths) {
        const BaseCarrierResult base =
            BuildMode25BaseTriangleCarrier801C5E60(
                &control, &object, 0u, 0u, true, path);
        CHECK(base.baseCarrierReady);
        CHECK(base.failure == Failure::None);
        CHECK(base.carrier.path == path);

        const DeformationResult deformed =
            ApplyMode25Mime80013EA8(base.carrier, runtime);
        CHECK(deformed.deformedTriangleReady);
        CHECK(deformed.failure == Failure::None);
        CHECK(deformed.carrier.path == path);

        const Rtpt3InputResult joined =
            BuildMode25Rtpt3Input801C5E60(
                deformed.carrier, &matrix, true);
        CHECK(joined.ready);
        CHECK(joined.failure == Failure::None);
        CHECK(joined.input.path == path);

        const Rtpt3ExecutionResult800428B0 executed =
            ExecuteMode25Rtpt3At800428B0(joined.input);
        CHECK(executed.ready);
        CHECK(executed.failure == Failure::None);
        CHECK(executed.input.path == path);
    }

    const BaseCarrierResult base =
        BuildMode25BaseTriangleCarrier801C5E60(
            &control, &object, 0u, 0u, true, ModelPath::Lo);
    Mode25BaseTriangleCarrier invalidBase = base.carrier;
    invalidBase.path = static_cast<ModelPath>(0xFFu);
    const DeformationResult invalidDeformation =
        ApplyMode25Mime80013EA8(invalidBase, runtime);
    CHECK(!invalidDeformation.deformedTriangleReady);
    CHECK(invalidDeformation.failure == Failure::UnsupportedModelPath);

    const DeformationResult deformed =
        ApplyMode25Mime80013EA8(base.carrier, runtime);
    Mode25DeformedTriangleCarrier invalidDeformed = deformed.carrier;
    invalidDeformed.path = static_cast<ModelPath>(0xFFu);
    const Rtpt3InputResult invalidJoin =
        BuildMode25Rtpt3Input801C5E60(invalidDeformed, &matrix, true);
    CHECK(!invalidJoin.ready);
    CHECK(invalidJoin.failure == Failure::UnsupportedModelPath);

    const Rtpt3InputResult joined =
        BuildMode25Rtpt3Input801C5E60(deformed.carrier, &matrix, true);
    Mode25Rtpt3Input invalidInput = joined.input;
    invalidInput.path = static_cast<ModelPath>(0xFFu);
    const Rtpt3ExecutionResult800428B0 invalidExecution =
        ExecuteMode25Rtpt3At800428B0(invalidInput);
    CHECK(!invalidExecution.ready);
    CHECK(invalidExecution.failure == Failure::UnsupportedModelPath);
}

void TestExplicitTitleRuntimeMatrixComposition()
{
    TitleRuntimeMatrixInput801C5E60 input{};
    TitleRuntimeMatrixResult801C5E60 result =
        BuildTitleRuntimeMatrix801C5E60(input);
    CHECK(!result.runtimeMatrixKnown);
    CHECK(result.failure == Failure::DrawCoordWorldUnknown);

    input.drawCoordWorldKnown800417A4 = true;
    input.drawCoordWorld800417A4 =
        MakeMatrix(IdentityRotation(), {{10, 20, 30}});
    result = BuildTitleRuntimeMatrix801C5E60(input);
    CHECK(!result.runtimeMatrixKnown);
    CHECK(result.failure == Failure::PanelWithCameraUnknown);

    input.panelWithCameraKnown80041A68 = true;
    input.panelWithCamera80041A68 =
        MakeMatrix(IdentityRotation(), {{100, -200, 300}});
    result = BuildTitleRuntimeMatrix801C5E60(input);
    CHECK(result.runtimeMatrixKnown);
    CHECK(result.failure == Failure::None);
    CHECK(result.runtimeMatrix800406D8.words[0] == 0x00001000u);
    CHECK(result.runtimeMatrix800406D8.words[1] == 0u);
    CHECK(result.runtimeMatrix800406D8.words[2] == 0x00001000u);
    CHECK(result.runtimeMatrix800406D8.words[3] == 0u);
    CHECK(result.runtimeMatrix800406D8.words[4] == 0x00001000u);
    CHECK(static_cast<int32_t>(result.runtimeMatrix800406D8.words[5]) == 110);
    CHECK(static_cast<int32_t>(result.runtimeMatrix800406D8.words[6]) == -180);
    CHECK(static_cast<int32_t>(result.runtimeMatrix800406D8.words[7]) == 330);

    const std::array<std::array<int16_t, 3>, 3> rotateZ90 =
        {{{{0, -4096, 0}}, {{4096, 0, 0}}, {{0, 0, 4096}}}};
    input.panelWithCamera80041A68 =
        MakeMatrix(rotateZ90, {{100, -200, 300}});
    result = BuildTitleRuntimeMatrix801C5E60(input);
    CHECK(result.runtimeMatrixKnown);
    CHECK(result.runtimeMatrix800406D8.words[0] == 0xF0000000u);
    CHECK(result.runtimeMatrix800406D8.words[1] == 0x10000000u);
    CHECK(result.runtimeMatrix800406D8.words[2] == 0u);
    CHECK(result.runtimeMatrix800406D8.words[3] == 0u);
    CHECK(result.runtimeMatrix800406D8.words[4] == 0x00001000u);
    CHECK(static_cast<int32_t>(result.runtimeMatrix800406D8.words[5]) == 80);
    CHECK(static_cast<int32_t>(result.runtimeMatrix800406D8.words[6]) == -190);
    CHECK(static_cast<int32_t>(result.runtimeMatrix800406D8.words[7]) == 330);

    input.panelWithCamera80041A68 = MakeMatrix(
        IdentityRotation(),
        {{(std::numeric_limits<int32_t>::max)(), 0, 0}});
    result = BuildTitleRuntimeMatrix801C5E60(input);
    CHECK(!result.runtimeMatrixKnown);
    CHECK(result.failure == Failure::RuntimeMatrixOverflow);
}

void TestTitleTodDrawCoordProducer8001B000()
{
    const TodData tod = MakeTitleRuntimeTod();
    TitleTodCursorState8001B000 state{};

    TitleTodAdvanceResult8001B000 result =
        AdvanceTitleTodDrawCoord8001B000(state, &tod);
    CHECK(!result.accepted);
    CHECK(result.failure == Failure::TitleTodBindingUnknown);

    state.bindingKnown = true;
    state.resourceHandle = 3u;
    state.sampleSeqKnown = true;
    result = AdvanceTitleTodDrawCoord8001B000(state, &tod);
    CHECK(result.accepted);
    CHECK(result.blockAdvanced);
    CHECK(!result.sourceExhausted);
    CHECK(result.nextState.sampleSeq == 1u);
    CHECK(result.nextState.blockIndex == 1u);
    CHECK(result.drawCoordWorldKnown800417A4);
    CHECK(result.drawCoordWorld800417A4.words[0] == 0x00001000u);
    CHECK(static_cast<int32_t>(result.drawCoordWorld800417A4.words[5]) == 10);
    CHECK(static_cast<int32_t>(result.drawCoordWorld800417A4.words[6]) == 20);
    CHECK(static_cast<int32_t>(result.drawCoordWorld800417A4.words[7]) == 30);

    result = AdvanceTitleTodDrawCoord8001B000(result.nextState, &tod);
    CHECK(result.accepted);
    CHECK(!result.blockAdvanced);
    CHECK(result.nextState.sampleSeq == 2u);
    CHECK(result.nextState.blockIndex == 1u);
    CHECK(static_cast<int32_t>(result.drawCoordWorld800417A4.words[5]) == 10);

    result = AdvanceTitleTodDrawCoord8001B000(result.nextState, &tod);
    CHECK(result.accepted);
    CHECK(result.blockAdvanced);
    CHECK(result.sourceExhausted);
    CHECK(result.nextState.blockIndex == 2u);
    // 80028054 writes the type-4 source halfwords by columns.  This
    // deliberately asymmetric rotation locks the original transpose order.
    CHECK(result.drawCoordWorld800417A4.words[0] == 0x10000000u);
    CHECK(result.drawCoordWorld800417A4.words[1] == 0xF0000000u);
    CHECK(result.drawCoordWorld800417A4.words[2] == 0x00000000u);
    CHECK(result.drawCoordWorld800417A4.words[3] == 0x00000000u);
    CHECK(result.drawCoordWorld800417A4.words[4] == 0x00001000u);
    CHECK(static_cast<int32_t>(result.drawCoordWorld800417A4.words[5]) == -40);

    TodData malformed = tod;
    malformed.blocks[0].commands[0].data.pop_back();
    const TitleTodCursorState8001B000 beforeFailure = state;
    result = AdvanceTitleTodDrawCoord8001B000(state, &malformed);
    CHECK(!result.accepted);
    CHECK(result.failure == Failure::TitleTodSourceIncomplete);
    CHECK(result.nextState.sampleSeq == beforeFailure.sampleSeq);
    CHECK(result.nextState.blockIndex == beforeFailure.blockIndex);

    malformed = tod;
    malformed.blocks[0].commands[0].header = 0x01050001u;
    malformed.blocks[0].commands[0].data.clear();
    result = AdvanceTitleTodDrawCoord8001B000(state, &malformed);
    CHECK(!result.accepted);
    CHECK(result.failure == Failure::TitleTodCommandUnsupported);

    malformed = tod;
    malformed.rawBlockCount = 3u;
    result = AdvanceTitleTodDrawCoord8001B000(state, &malformed);
    CHECK(!result.accepted);
    CHECK(result.failure == Failure::TitleTodSourceIncomplete);

    state.sampleSeq = (std::numeric_limits<uint32_t>::max)();
    result = AdvanceTitleTodDrawCoord8001B000(state, &tod);
    CHECK(!result.accepted);
    CHECK(result.failure == Failure::TitleTodCursorOverflow);
}

void TestTitleCoordWithCamera80041A68()
{
    const std::array<std::array<int16_t, 3>, 3> rotateZ90 =
        {{{{0, -4096, 0}}, {{4096, 0, 0}}, {{0, 0, 4096}}}};
    const std::array<std::array<int16_t, 3>, 3> rotateX90 =
        {{{{4096, 0, 0}}, {{0, 0, -4096}}, {{0, 4096, 0}}}};

    TitleCoordWithCameraInput80041A68 input{};
    input.cameraMatrixKnown80092880 = true;
    input.cameraMatrix80092880 =
        MakeMatrix(rotateZ90, {{100, 200, 300}});
    TitleCoordWithCameraResult80041A68 result =
        BuildTitleCoordWithCamera80041A68(input);
    CHECK(!result.coordWithCameraKnown80041A68);
    CHECK(result.failure == Failure::DrawCoordWorldUnknown);

    input.drawCoordWorldKnown800417A4 = true;
    input.drawCoordWorld800417A4 =
        MakeMatrix(rotateX90, {{10, 20, 30}});
    input.cameraMatrixKnown80092880 = false;
    result = BuildTitleCoordWithCamera80041A68(input);
    CHECK(!result.coordWithCameraKnown80041A68);
    CHECK(result.failure == Failure::TitleCameraMatrixUnknown);

    input.cameraMatrixKnown80092880 = true;
    result = BuildTitleCoordWithCamera80041A68(input);
    CHECK(result.coordWithCameraKnown80041A68);
    CHECK(result.failure == Failure::None);
    CHECK(result.coordWithCamera80041A68.words[0] == 0u);
    CHECK(result.coordWithCamera80041A68.words[1] == 0x10001000u);
    CHECK(result.coordWithCamera80041A68.words[2] == 0u);
    CHECK(result.coordWithCamera80041A68.words[3] == 0x10000000u);
    CHECK(result.coordWithCamera80041A68.words[4] == 0u);
    CHECK(static_cast<int32_t>(result.coordWithCamera80041A68.words[5]) ==
          80);
    CHECK(static_cast<int32_t>(result.coordWithCamera80041A68.words[6]) ==
          210);
    CHECK(static_cast<int32_t>(result.coordWithCamera80041A68.words[7]) ==
          330);
}

void TestTitlePanelCoordAndCameraProducer801C5D8C80041A68()
{
    TitlePanelCoordInput801C5D8C panelInput{};
    TitlePanelCoordResult801C5D8C panel =
        BuildTitlePanelCoord801C5D8C(panelInput);
    CHECK(!panel.panelCoordKnown801CD7BC);
    CHECK(panel.failure == Failure::TitlePanelCoordInputUnknown);

    panelInput.known = true;
    panel = BuildTitlePanelCoord801C5D8C(panelInput);
    CHECK(!panel.panelCoordKnown801CD7BC);
    CHECK(panel.failure == Failure::TitlePanelCoordFrameCountInvalid);

    panelInput.deltaX = -163;
    panelInput.totalFrames = 204;
    panelInput.deltaY = 192;
    panel = BuildTitlePanelCoord801C5D8C(panelInput);
    CHECK(panel.panelCoordKnown801CD7BC);
    CHECK(panel.panelCoord801CD7BC.words[0] == 0x0CC81000u);
    CHECK(panel.panelCoord801CD7BC.words[1] == 0u);
    CHECK(panel.panelCoord801CD7BC.words[2] == 0u);
    CHECK(panel.panelCoord801CD7BC.words[3] == 0xF0F10000u);
    CHECK(panel.panelCoord801CD7BC.words[4] == 0x00001000u);
    CHECK(panel.panelCoord801CD7BC.words[5] == 0u);
    CHECK(panel.panelCoord801CD7BC.words[6] == 0u);
    CHECK(panel.panelCoord801CD7BC.words[7] == 0u);

    TitlePanelCoordInput801C5D8C overflowInput{};
    overflowInput.known = true;
    overflowInput.deltaX = (std::numeric_limits<int32_t>::max)();
    overflowInput.totalFrames = 1;
    overflowInput.deltaY = 0;
    panel = BuildTitlePanelCoord801C5D8C(overflowInput);
    CHECK(!panel.panelCoordKnown801CD7BC);
    CHECK(panel.failure == Failure::TitlePanelCoordOverflow);

    TitlePanelWithCameraInput80041A68 composeInput{};
    TitlePanelWithCameraResult80041A68 composed =
        BuildTitlePanelWithCamera80041A68(composeInput);
    CHECK(!composed.panelWithCameraKnown80041A68);
    CHECK(composed.failure == Failure::TitlePanelCoordUnknown);

    composeInput.panelCoordKnown801CD7BC = true;
    composeInput.panelCoord801CD7BC =
        BuildTitlePanelCoord801C5D8C(panelInput).panelCoord801CD7BC;
    composed = BuildTitlePanelWithCamera80041A68(composeInput);
    CHECK(!composed.panelWithCameraKnown80041A68);
    CHECK(composed.failure == Failure::TitleCameraMatrixUnknown);

    composeInput.cameraMatrixKnown80092880 = true;
    composeInput.cameraMatrix80092880 =
        MakeMatrix(IdentityRotation(), {{10, 20, 30}});
    composed = BuildTitlePanelWithCamera80041A68(composeInput);
    CHECK(composed.panelWithCameraKnown80041A68);
    CHECK(composed.panelWithCamera80041A68.words[0] == 0x0CC81000u);
    CHECK(composed.panelWithCamera80041A68.words[3] == 0xF0F10000u);
    CHECK(static_cast<int32_t>(
              composed.panelWithCamera80041A68.words[5]) == 10);
    CHECK(static_cast<int32_t>(
              composed.panelWithCamera80041A68.words[6]) == 20);
    CHECK(static_cast<int32_t>(
              composed.panelWithCamera80041A68.words[7]) == 30);
}

void TestBaseCarrierFailClosedInputs()
{
    TmdObject object = MakeFirstPaKagePrimitive();
    PrPsxGteDirect::GteControlState control = MakeTitleGteControl();

    BaseCarrierResult result = BuildMode25BaseTriangleCarrier801C5E60(
        nullptr, &object, 0u, 0u, true, ModelPath::PaKage);
    CHECK(!result.baseCarrierReady);
    CHECK(result.failure == Failure::GteControlUnknown);

    control.geomScreen = 441u;
    result = BuildMode25BaseTriangleCarrier801C5E60(
        &control, &object, 0u, 0u, true, ModelPath::PaKage);
    CHECK(!result.baseCarrierReady);
    CHECK(result.failure == Failure::GteControlMismatch);

    control = MakeTitleGteControl();
    result = BuildMode25BaseTriangleCarrier801C5E60(
        &control, nullptr, 0u, 0u, true, ModelPath::PaKage);
    CHECK(!result.baseCarrierReady);
    CHECK(result.failure == Failure::NullObject);

    result = BuildMode25BaseTriangleCarrier801C5E60(
        &control, &object, 0u, 0u, false, ModelPath::PaKage);
    CHECK(!result.baseCarrierReady);
    CHECK(result.failure == Failure::SourceParseIncomplete);

    object.primitives[0].mode = 0x24u;
    result = BuildMode25BaseTriangleCarrier801C5E60(
        &control, &object, 0u, 0u, true, ModelPath::PaKage);
    CHECK(!result.baseCarrierReady);
    CHECK(result.failure == Failure::UnsupportedPrimitiveShape);

    object = MakeFirstPaKagePrimitive();
    object.primitives[0].v1_idx = 3u;
    result = BuildMode25BaseTriangleCarrier801C5E60(
        &control, &object, 0u, 0u, true, ModelPath::PaKage);
    CHECK(!result.baseCarrierReady);
    CHECK(result.failure == Failure::VertexIndexOutOfRange);

    object = MakeFirstPaKagePrimitive();
    result = BuildMode25BaseTriangleCarrier801C5E60(
        &control,
        &object,
        0u,
        0u,
        true,
        static_cast<ModelPath>(0xFFu));
    CHECK(!result.baseCarrierReady);
    CHECK(result.failure == Failure::UnsupportedModelPath);
}

void TestMimeFailClosedInputs()
{
    const TmdObject object = MakeFirstPaKagePrimitive();
    const PrPsxGteDirect::GteControlState control = MakeTitleGteControl();
    const BaseCarrierResult base =
        BuildMode25BaseTriangleCarrier801C5E60(
            &control, &object, 0u, 0u, true, ModelPath::PaKage);
    VdfData vdf = MakeVdf();
    DatData dat = MakeDat();
    MimeRuntimeInput80013EA8 runtime = MakeRuntimeMime(vdf, dat);

    DeformationResult result = ApplyMode25Mime80013EA8(
        Mode25BaseTriangleCarrier{}, runtime);
    CHECK(result.failure == Failure::BaseTriangleUnknown);

    runtime.bindingKnown = false;
    result = ApplyMode25Mime80013EA8(base.carrier, runtime);
    CHECK(result.failure == Failure::RuntimeMimeBindingUnknown);
    runtime = MakeRuntimeMime(vdf, dat);

    runtime.vdf = nullptr;
    result = ApplyMode25Mime80013EA8(base.carrier, runtime);
    CHECK(result.failure == Failure::VdfSourceUnknown);
    runtime = MakeRuntimeMime(vdf, dat);
    runtime.dat = nullptr;
    result = ApplyMode25Mime80013EA8(base.carrier, runtime);
    CHECK(result.failure == Failure::DatSourceUnknown);
    runtime = MakeRuntimeMime(vdf, dat);
    runtime.sampleFrameKnown = false;
    result = ApplyMode25Mime80013EA8(base.carrier, runtime);
    CHECK(result.failure == Failure::RuntimeMimeCursorUnknown);
    runtime = MakeRuntimeMime(vdf, dat);
    runtime.loopFlagKnown = false;
    result = ApplyMode25Mime80013EA8(base.carrier, runtime);
    CHECK(result.failure == Failure::RuntimeMimeLoopFlagUnknown);
    runtime = MakeRuntimeMime(vdf, dat);
    runtime.loopFlag = 2u;
    result = ApplyMode25Mime80013EA8(base.carrier, runtime);
    CHECK(result.failure == Failure::UnsupportedMimeLoopFlag);

    runtime = MakeRuntimeMime(vdf, dat);
    vdf.keys = 1u;
    result = ApplyMode25Mime80013EA8(base.carrier, runtime);
    CHECK(result.failure == Failure::MimeKeyCountMismatch);
    vdf = MakeVdf();
    runtime = MakeRuntimeMime(vdf, dat);
    vdf.keyList.pop_back();
    result = ApplyMode25Mime80013EA8(base.carrier, runtime);
    CHECK(result.failure == Failure::VdfShapeMismatch);
    vdf = MakeVdf();
    runtime = MakeRuntimeMime(vdf, dat);
    dat.keyList[0].influence.pop_back();
    result = ApplyMode25Mime80013EA8(base.carrier, runtime);
    CHECK(result.failure == Failure::DatShapeMismatch);
    dat = MakeDat();
    runtime = MakeRuntimeMime(vdf, dat);
    runtime.objectIndex = 1u;
    result = ApplyMode25Mime80013EA8(base.carrier, runtime);
    CHECK(result.failure == Failure::MimeObjectSourceMissing);

    runtime = MakeRuntimeMime(vdf, dat);
    vdf.keyList[0].nVert = 4u;
    vdf.keyList[0].deltas.push_back({1, 1, 1, 0});
    result = ApplyMode25Mime80013EA8(base.carrier, runtime);
    CHECK(result.failure == Failure::VdfShapeMismatch);
}

void TestResourceIngressDoesNotClaimRuntimeVertexAuthority()
{
    PrSS0TitleTmdBackend::ResourceState state{};
    state.scene0ComodAccepted = true;
    state.titleHudTablesLoadedFromComod0801C6Dxx = true;
    state.titleCameraTableKnown801C6FB4 = true;
    state.loadedModelCount = static_cast<uint32_t>(
        PrSS0TitleTmdBackend::ModelKind::Count);
    state.registeredTpageCount = 1u;
    state.loadedTimCount = 1u;
    state.titlePacketWorkKnown801C609C = true;

    CHECK(!PrSS0TitleTmdBackend::IsResourceIngressReady(state));
    state.cmOpBezResource27Loaded = true;
    CHECK(!PrSS0TitleTmdBackend::IsResourceIngressReady(state));
    CHECK(!state.firstPaKageRuntimeMimeBindingKnown);
    CHECK(!state.firstPaKageRuntimeMimeCursorKnown);
    CHECK(!state.firstPaKageRuntimeVertexDeformationKnown);
    CHECK(!state.firstPaKageRuntimeMatrixKnown);
    CHECK(!state.firstPaKageMode25Rtpt3InputKnown);
    CHECK(!state.firstPaKageMode25Rtpt3OutputKnown);
    CHECK(!state.firstPaKageMode25GeometryKnown);

    state.loadedMimePairCount = static_cast<uint32_t>(
        PrSS0TitleTmdBackend::MimeKind::Count);
    CHECK(!PrSS0TitleTmdBackend::IsResourceIngressReady(state));
    state.loadedTitleTodCount = static_cast<uint32_t>(
        PrSS0TitleTmdBackend::TitleTodKind::Count);
    CHECK(!PrSS0TitleTmdBackend::IsResourceIngressReady(state));
    state.paLoc2TodLoaded = true;
    CHECK(!PrSS0TitleTmdBackend::IsResourceIngressReady(state));
    state.paLoc2CoordKnown = true;
    CHECK(PrSS0TitleTmdBackend::IsResourceIngressReady(state));

    state.firstPaKageMode25BaseTriangleKnown = true;
    CHECK(PrSS0TitleTmdBackend::IsResourceIngressReady(state));
    CHECK(!state.firstPaKageRuntimeMimeBindingKnown);
    CHECK(!state.firstPaKageRuntimeMimeCursorKnown);
    CHECK(!state.firstPaKageRuntimeVertexDeformationKnown);
    CHECK(!state.firstPaKageRuntimeMatrixKnown);
    CHECK(!state.firstPaKageMode25Rtpt3InputKnown);
    CHECK(!state.firstPaKageMode25Rtpt3OutputKnown);
    CHECK(!state.firstPaKageMode25GeometryKnown);

    --state.loadedMimePairCount;
    CHECK(!PrSS0TitleTmdBackend::IsResourceIngressReady(state));
    ++state.loadedMimePairCount;
    --state.loadedTitleTodCount;
    CHECK(!PrSS0TitleTmdBackend::IsResourceIngressReady(state));
    ++state.loadedTitleTodCount;

    state.loadedTimCount = 0u;
    CHECK(!PrSS0TitleTmdBackend::IsResourceIngressReady(state));
}

void TestTitleMimeInitState801C609C()
{
    using namespace PrSS0TitleTmdBackend;
    TitleMimeInitState801C609C state{};
    state.known = true;
    state.exactCallOrder = true;
    state.baseRestoreCalled800139F8 = true;
    state.baseRestoreCallCount800139F8 = 4u;
    state.baseChannelRestored800139F8.fill(true);
    state.channelInitCallCount8001385C = 3u;
    state.scratchClearCalled80013E40 = true;
    state.scratchZeroByteCount80013E40 =
        static_cast<uint32_t>(state.scratch80090240.size());
    state.transitionGridInitCalled8001EEE8 = true;
    state.transitionGrid8001EEAC.fill(1u);

    const auto initializeChannel = [&](uint8_t index,
                                       ModelKind model,
                                       MimeKind mime,
                                       uint32_t destination) {
        auto& channel = state.channels[index];
        channel.known = true;
        channel.channel = index;
        channel.modelKind = model;
        channel.mimeKind = mime;
        channel.channelStateAddress80074B08 =
            kTitleMimeChannelStateBase80074B08 +
            kTitleMimeChannelStateStride8001385C * index;
        channel.baseSnapshotAddress8006F6A8 =
            kTitleMimeBaseSnapshotBank8006F6A8 +
            kTitleMimeBaseSnapshotStride8001385C * index;
        channel.vdfStateAddress80074B80 =
            kTitleMimeVdfStateBase80074B80 +
            kTitleMimeChannelStateStride8001385C * index;
        channel.vdfWorkspaceAddress8006FB08 =
            kTitleMimeVdfWorkspaceBase8006FB08 +
            kTitleMimeVdfWorkspaceStride8001385C * index;
        channel.scratchAddress80090240 =
            kTitleMimeScratchBase80090240 +
            kTitleMimeScratchStride80013E40 * index;
        channel.destinationVertexAddressA4 = destination;
        channel.modelObjectCount = 1u;
        channel.modelVertexCount = 1u;
        channel.vdfKeyCount = 1u;
        channel.datKeyCount = 1u;
        channel.tmdBound80013650 = true;
        channel.baseSnapshotCommitted8001371C = true;
        channel.vdfBound800137BC = true;
    };
    initializeChannel(1u, ModelKind::Lo, MimeKind::Logo, 0x801CB66Cu);
    initializeChannel(2u,
                      ModelKind::PaKage,
                      MimeKind::PaOki,
                      0x801CD81Cu);
    initializeChannel(3u, ModelKind::Hp, MimeKind::Hiphop, 0x801CC71Cu);

    CHECK(IsExactTitleMimeInitState801C609C(state));
    state.transitionGrid8001EEAC[0] = 0u;
    CHECK(!IsExactTitleMimeInitState801C609C(state));
    state.transitionGrid8001EEAC[0] = 1u;
    state.channels[2].destinationVertexAddressA4 = 0u;
    CHECK(!IsExactTitleMimeInitState801C609C(state));
}

} // namespace

int main()
{
    TestStrictPaLoc2CoordExtraction();
    TestExactTitleCameraTable801C6FB4();
    TestTitleCameraMatrix80041D3C();
    TestTitleCameraCursorReset801CB59C();
    TestBaseCarrierKeepsRuntimeSourcesUnknown();
    TestExactMimeDeformationAndRtpt3Join();
    TestAllModelPathsPreserveTransformProvenance();
    TestExplicitTitleRuntimeMatrixComposition();
    TestTitleTodDrawCoordProducer8001B000();
    TestTitleCoordWithCamera80041A68();
    TestTitlePanelCoordAndCameraProducer801C5D8C80041A68();
    TestBaseCarrierFailClosedInputs();
    TestMimeFailClosedInputs();
    TestResourceIngressDoesNotClaimRuntimeVertexAuthority();
    TestTitleMimeInitState801C609C();

    if (g_failedChecks != 0) {
        std::printf("test_ss0_title_transform_direct: FAIL (%d checks)\n",
                    g_failedChecks);
        return 1;
    }
    std::printf("test_ss0_title_transform_direct: PASS\n");
    return 0;
}
