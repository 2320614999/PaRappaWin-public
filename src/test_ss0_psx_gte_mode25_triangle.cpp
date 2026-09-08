#include "pr/pr_psx_gte_direct.h"
#include "pr/pr_psx_tmd_submit_direct.h"

#include <array>
#include <cstdio>

namespace {

using namespace PrPsxGteDirect;

int g_failedChecks = 0;

#define CHECK(expr)                                                        \
    do {                                                                   \
        if (!(expr)) {                                                     \
            std::printf("CHECK failed %s:%d: %s\n", __FILE__, __LINE__,  \
                        #expr);                                            \
            ++g_failedChecks;                                              \
        }                                                                  \
    } while (0)

uint32_t PackSxy(int16_t x, int16_t y) {
    return static_cast<uint32_t>(static_cast<uint16_t>(x)) |
           (static_cast<uint32_t>(static_cast<uint16_t>(y)) << 16u);
}

GteSxy CandidateSxy(int16_t x, int16_t y) {
    GteSxy sxy{};
    sxy.candidateKnown = true;
    sxy.candidateX = x;
    sxy.candidateY = y;
    sxy.candidateWord = PackSxy(x, y);
    return sxy;
}

GteSxy AuthoritativeSxy(int16_t x, int16_t y) {
    GteSxy sxy = CandidateSxy(x, y);
    sxy.known = true;
    sxy.word = PackSxy(x, y);
    return sxy;
}

Mode25TriangleGeometryInput MakeCandidateInput() {
    Mode25TriangleGeometryInput input{};
    input.sxy = {{
        CandidateSxy(0, 0),
        CandidateSxy(2, 0),
        CandidateSxy(0, 3),
    }};
    input.zScaleFactor3Known = true;
    input.zScaleFactor3 = 4096;
    input.szAfterRtpt.elementCandidateKnown = {{true, true, true, true}};
    input.szAfterRtpt.candidateSz = {{60000u, 10u, 20u, 30u}};
    return input;
}

GteControlState MakeRtpt3Control(int32_t dqa = 0,
                                 int32_t dqb = 0) {
    GteControlState control{};
    control.geomScreenKnown = true;
    control.geomScreen = 440u;
    control.geomOffsetKnown = true;
    control.geomOffsetX = 0;
    control.geomOffsetY = 0;
    control.depthCueKnown = true;
    control.depthCueA = dqa;
    control.depthCueB = dqb;
    return control;
}

Matrix3x4 MakeIdentityRtpt3Matrix(int32_t tx,
                                  int32_t ty,
                                  int32_t tz) {
    Matrix3x4 matrix{};
    matrix.words = {{
        0x00001000u,
        0x00000000u,
        0x00001000u,
        0x00000000u,
        0x00001000u,
        static_cast<uint32_t>(tx),
        static_cast<uint32_t>(ty),
        static_cast<uint32_t>(tz),
    }};
    return matrix;
}

Rtpt3ExactInput280030 MakeTitleReplayRtpt3Input() {
    Rtpt3ExactInput280030 input{};
    input.matrixKnown = true;
    input.matrix.words = {{
        0xF4D70433u,
        0x07FB0F6Eu,
        0xFDD10865u,
        0xF211F2C2u,
        0x0000039Eu,
        0xFFFFFFFDu,
        0x000004B0u,
        0x00001831u,
    }};
    input.controlKnown = true;
    input.control = MakeRtpt3Control(-4194, 0x01400000);
    input.verticesKnown = true;
    // Recording index 7213 CPU input order for the first PA_KAGE mode-25
    // primitive. Recording index 7215 supplies only the output-side GP0 SXY
    // oracle; neither recording is a Windows runtime source.
    input.vertices = {{
        {-854, -1116, 406, 0},
        {-567, -871, 274, 0},
        {-783, -808, 381, 0},
    }};
    return input;
}

void TestExactRtpt3TitleReplayVector() {
    const Rtpt3ExactOutput280030 output =
        ExecuteRtpt3Exact280030(MakeTitleReplayRtpt3Input());
    CHECK(output.known);
    CHECK(output.flagAfterRtptKnown);
    CHECK(output.flagAfterRtpt == 0u);
    CHECK(output.ir0Known);
    CHECK(output.ir0 == 1250u);
    CHECK(!output.szAfterRtpt.known);
    CHECK(!output.szAfterRtpt.elementKnown[0]);
    CHECK(output.szAfterRtpt.elementKnown[1]);
    CHECK(output.szAfterRtpt.elementKnown[2]);
    CHECK(output.szAfterRtpt.elementKnown[3]);
    CHECK(output.szAfterRtpt.sz[1] == 7963u);
    CHECK(output.szAfterRtpt.sz[2] == 7482u);
    CHECK(output.szAfterRtpt.sz[3] == 7630u);
    CHECK(output.sxy[0].known);
    CHECK(output.sxy[1].known);
    CHECK(output.sxy[2].known);
    CHECK(output.sxy[0].word == 0x00070034u);
    CHECK(output.sxy[1].word == 0x0018002Au);
    CHECK(output.sxy[2].word == 0x00130029u);

    Mode25TriangleGeometryInput geometryInput{};
    geometryInput.sxy = output.sxy;
    geometryInput.szAfterRtpt = output.szAfterRtpt;
    geometryInput.zScaleFactor3Known = true;
    geometryInput.zScaleFactor3 = 341;
    const Mode25TriangleGeometryTrace geometry =
        TraceMode25TriangleGeometry(geometryInput);
    CHECK(geometry.projectedSxyKnown);
    CHECK(geometry.nclip.known);
    CHECK(geometry.nclip.mac0 == 67);
    CHECK(geometry.visibilityKnown);
    CHECK(geometry.visible);
    CHECK(geometry.avsz3.known);
    CHECK(geometry.otzKnown);
    CHECK(geometry.otz == 1921u);
}

void TestExactRtpt3FailClosedInputs() {
    Rtpt3ExactInput280030 input = MakeTitleReplayRtpt3Input();
    input.matrixKnown = false;
    Rtpt3ExactOutput280030 output = ExecuteRtpt3Exact280030(input);
    CHECK(!output.known);
    CHECK(!output.flagAfterRtptKnown);
    CHECK(!output.ir0Known);
    CHECK(!output.sxy[0].known);
    CHECK(!output.szAfterRtpt.elementKnown[1]);

    input = MakeTitleReplayRtpt3Input();
    input.control.depthCueKnown = false;
    output = ExecuteRtpt3Exact280030(input);
    CHECK(!output.known);

    input = MakeTitleReplayRtpt3Input();
    input.control.geomScreen = 0x10000u;
    output = ExecuteRtpt3Exact280030(input);
    CHECK(!output.known);

    input = MakeTitleReplayRtpt3Input();
    input.control.depthCueA = 0x8000;
    output = ExecuteRtpt3Exact280030(input);
    CHECK(!output.known);
}

void TestExactRtpt3DivideAndScreenFlags() {
    Rtpt3ExactInput280030 input{};
    input.matrixKnown = true;
    input.matrix = MakeIdentityRtpt3Matrix(0, 0, 0);
    input.controlKnown = true;
    input.control = MakeRtpt3Control();
    input.verticesKnown = true;
    input.vertices = {{
        {32767, 32767, 1, 0},
        {32767, 32767, 1, 0},
        {32767, 32767, 1, 0},
    }};

    const Rtpt3ExactOutput280030 output =
        ExecuteRtpt3Exact280030(input);
    CHECK(output.known);
    CHECK(output.flagAfterRtpt == 0x80036000u);
    CHECK(output.sxy[0].word == 0x03FF03FFu);
    CHECK(output.sxy[1].word == 0x03FF03FFu);
    CHECK(output.sxy[2].word == 0x03FF03FFu);
    CHECK(output.szAfterRtpt.sz[1] == 1u);
    CHECK(output.szAfterRtpt.sz[2] == 1u);
    CHECK(output.szAfterRtpt.sz[3] == 1u);
}

void TestExactRtpt3SzAndMacFlags() {
    Rtpt3ExactInput280030 input{};
    input.matrixKnown = true;
    input.matrix = MakeIdentityRtpt3Matrix(0, 0, 100000);
    input.controlKnown = true;
    input.control = MakeRtpt3Control();
    input.verticesKnown = true;
    input.vertices = {{
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    }};

    Rtpt3ExactOutput280030 output = ExecuteRtpt3Exact280030(input);
    CHECK(output.known);
    CHECK(output.flagAfterRtpt == 0x80440000u);
    CHECK(output.szAfterRtpt.sz[1] == 0xFFFFu);
    CHECK(output.szAfterRtpt.sz[2] == 0xFFFFu);
    CHECK(output.szAfterRtpt.sz[3] == 0xFFFFu);

    input.matrix = MakeIdentityRtpt3Matrix(0x7FFFFFFF, 0, 1000);
    input.vertices = {{
        {32767, 0, 1000, 0},
        {32767, 0, 1000, 0},
        {32767, 0, 1000, 0},
    }};
    output = ExecuteRtpt3Exact280030(input);
    CHECK(output.known);
    CHECK(output.flagAfterRtpt == 0xC1004000u);
    CHECK(output.sxy[0].word == 0x0000FC00u);
    CHECK(output.szAfterRtpt.sz[1] == 2000u);
}

void TestExactRtpt3FinalMac3Overflow() {
    Rtpt3ExactInput280030 input{};
    input.matrixKnown = true;
    input.matrix = MakeIdentityRtpt3Matrix(0, 0, 0x7FFFFFFF);
    input.controlKnown = true;
    input.control = MakeRtpt3Control();
    input.verticesKnown = true;
    input.vertices = {{
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
    }};

    const Rtpt3ExactOutput280030 output =
        ExecuteRtpt3Exact280030(input);
    CHECK(output.known);
    CHECK(output.flagAfterRtpt == 0x90460000u);
    CHECK(output.szAfterRtpt.sz[1] == 0u);
    CHECK(output.szAfterRtpt.sz[2] == 0u);
    CHECK(output.szAfterRtpt.sz[3] == 0u);
    CHECK(output.sxy[0].word == 0u);
    CHECK(output.sxy[1].word == 0u);
    CHECK(output.sxy[2].word == 0u);
}

void TestExactRtpt3DivideBoundary() {
    Rtpt3ExactInput280030 input{};
    input.matrixKnown = true;
    input.matrix = MakeIdentityRtpt3Matrix(0, 0, 0);
    input.controlKnown = true;
    input.control = MakeRtpt3Control();
    input.verticesKnown = true;
    input.vertices = {{
        {256, 0, 220, 0},
        {256, 0, 220, 0},
        {256, 0, 220, 0},
    }};

    Rtpt3ExactOutput280030 output = ExecuteRtpt3Exact280030(input);
    CHECK(output.known);
    CHECK(output.flagAfterRtpt == 0x80020000u);
    CHECK(output.sxy[0].word == 0x000001FFu);

    input.vertices = {{
        {256, 0, 221, 0},
        {256, 0, 221, 0},
        {256, 0, 221, 0},
    }};
    output = ExecuteRtpt3Exact280030(input);
    CHECK(output.known);
    CHECK(output.flagAfterRtpt == 0u);
    CHECK(output.sxy[0].word == 0x000001FDu);
}

void TestExactRtpt3UnrSeedIndexEdges() {
    Rtpt3ExactInput280030 input{};
    input.matrixKnown = true;
    input.matrix = MakeIdentityRtpt3Matrix(0, 0, 32768);
    input.controlKnown = true;
    input.control = MakeRtpt3Control();
    input.control.geomScreen = 0xFFFFu;
    input.verticesKnown = true;
    input.vertices = {{
        {256, 0, 0, 0},
        {256, 0, 32767, 0},
        {256, 0, 0, 0},
    }};

    const Rtpt3ExactOutput280030 output =
        ExecuteRtpt3Exact280030(input);
    CHECK(output.known);
    CHECK(output.flagAfterRtpt == 0x00400000u);
    CHECK(output.szAfterRtpt.sz[1] == 32768u);
    CHECK(output.szAfterRtpt.sz[2] == 65535u);
    CHECK(output.szAfterRtpt.sz[3] == 32768u);
    CHECK(output.sxy[0].word == 0x000001FFu);
    CHECK(output.sxy[1].word == 0x000000FFu);
    CHECK(output.sxy[2].word == 0x000001FFu);
}

void CheckCandidateBoundary(const Mode25TriangleGeometryTrace& trace) {
    CHECK(!trace.projectedSxyKnown);
    CHECK(trace.nclip.inputsCandidateKnown);
    CHECK(!trace.nclip.inputsAuthoritative);
    CHECK(trace.nclip.candidateKnown);
    CHECK(!trace.nclip.known);
    CHECK(!trace.nclip.overflowKnown);
    CHECK(trace.avsz3.inputsCandidateKnown);
    CHECK(!trace.avsz3.inputsAuthoritative);
    CHECK(trace.avsz3.candidateKnown);
    CHECK(!trace.avsz3.known);
    CHECK(!trace.avsz3.saturationKnown);
    CHECK(!trace.visibilityKnown);
    CHECK(!trace.otzKnown);
}

void TestCandidateNclipWinding() {
    Mode25TriangleGeometryInput input = MakeCandidateInput();
    Mode25TriangleGeometryTrace trace = TraceMode25TriangleGeometry(input);
    CheckCandidateBoundary(trace);
    CHECK(trace.nclip.mac0Candidate == 6);
    CHECK(trace.sxy[0].candidateWord == PackSxy(0, 0));
    CHECK(!trace.sxy[0].known);

    input.sxy = {{
        CandidateSxy(0, 0),
        CandidateSxy(2, 0),
        CandidateSxy(4, 0),
    }};
    trace = TraceMode25TriangleGeometry(input);
    CHECK(trace.nclip.candidateKnown);
    CHECK(trace.nclip.mac0Candidate == 0);

    input.sxy = {{
        CandidateSxy(0, 0),
        CandidateSxy(0, 3),
        CandidateSxy(2, 0),
    }};
    trace = TraceMode25TriangleGeometry(input);
    CHECK(trace.nclip.candidateKnown);
    CHECK(trace.nclip.mac0Candidate == -6);
}

void TestAuthoritativeSxyWinding() {
    Mode25TriangleGeometryInput input = MakeCandidateInput();
    input.sxy = {{
        AuthoritativeSxy(0, 0),
        AuthoritativeSxy(2, 0),
        AuthoritativeSxy(0, 3),
    }};
    Mode25TriangleGeometryTrace trace = TraceMode25TriangleGeometry(input);
    CHECK(trace.projectedSxyKnown);
    CHECK(trace.nclip.inputsCandidateKnown);
    CHECK(trace.nclip.inputsAuthoritative);
    CHECK(trace.nclip.candidateKnown);
    CHECK(trace.nclip.mac0Candidate == 6);
    CHECK(trace.nclip.known);
    CHECK(trace.nclip.mac0 == 6);
    CHECK(trace.nclip.overflowKnown);
    CHECK(!trace.nclip.overflow);
    CHECK(trace.visibilityKnown);
    CHECK(trace.visible);
    for (const GteSxy& sxy : trace.sxy) {
        CHECK(sxy.known);
        CHECK(sxy.candidateKnown);
    }

    input.sxy = {{
        AuthoritativeSxy(0, 0),
        AuthoritativeSxy(2, 0),
        AuthoritativeSxy(4, 0),
    }};
    trace = TraceMode25TriangleGeometry(input);
    CHECK(trace.nclip.known);
    CHECK(trace.nclip.mac0 == 0);
    CHECK(trace.nclip.overflowKnown);
    CHECK(!trace.nclip.overflow);
    CHECK(trace.visibilityKnown);
    CHECK(!trace.visible);

    input.sxy = {{
        AuthoritativeSxy(0, 0),
        AuthoritativeSxy(0, 3),
        AuthoritativeSxy(2, 0),
    }};
    trace = TraceMode25TriangleGeometry(input);
    CHECK(trace.nclip.known);
    CHECK(trace.nclip.mac0 == -6);
    CHECK(trace.nclip.overflowKnown);
    CHECK(!trace.nclip.overflow);
    CHECK(trace.visibilityKnown);
    CHECK(!trace.visible);
}

void TestAuthoritativeNclipOverflow() {
    Mode25TriangleGeometryInput input = MakeCandidateInput();
    input.sxy = {{
        AuthoritativeSxy(-32768, -32768),
        AuthoritativeSxy(32767, -32768),
        AuthoritativeSxy(-32768, 32767),
    }};

    const Mode25TriangleGeometryTrace trace =
        TraceMode25TriangleGeometry(input);
    CHECK(trace.nclip.candidateKnown);
    CHECK(trace.nclip.mac0Candidate == 4294836225ll);
    CHECK(trace.nclip.known);
    CHECK(trace.nclip.mac0 == -131071);
    CHECK(trace.nclip.overflowKnown);
    CHECK(trace.nclip.overflow);
}

void TestAvsz3UsesRtptSlotsOneThroughThree() {
    Mode25TriangleGeometryInput input = MakeCandidateInput();
    Mode25TriangleGeometryTrace trace = TraceMode25TriangleGeometry(input);
    CheckCandidateBoundary(trace);
    CHECK(trace.avsz3.zScaleFactor3 == 4096);
    CHECK(trace.avsz3.mac0Candidate == 245760);
    CHECK(trace.avsz3.otzCandidate == 60u);
    CHECK(!trace.avsz3.saturationCandidate);

    input.szAfterRtpt.candidateSz[0] = 1u;
    trace = TraceMode25TriangleGeometry(input);
    CHECK(trace.avsz3.mac0Candidate == 245760);
    CHECK(trace.avsz3.otzCandidate == 60u);
}

void TestAuthoritativeAvsz3Publishes() {
    Mode25TriangleGeometryInput input = MakeCandidateInput();
    input.szAfterRtpt.elementCandidateKnown = {};
    input.szAfterRtpt.candidateSz = {};
    input.szAfterRtpt.elementKnown = {{false, true, true, true}};
    input.szAfterRtpt.sz = {{65535u, 10u, 20u, 30u}};

    const Mode25TriangleGeometryTrace trace =
        TraceMode25TriangleGeometry(input);
    CHECK(trace.avsz3.inputsAuthoritative);
    CHECK(trace.avsz3.inputsCandidateKnown);
    CHECK(trace.avsz3.candidateKnown);
    CHECK(trace.avsz3.mac0Candidate == 245760);
    CHECK(trace.avsz3.otzCandidate == 60u);
    CHECK(trace.avsz3.known);
    CHECK(trace.avsz3.mac0 == 245760);
    CHECK(trace.avsz3.otz == 60u);
    CHECK(trace.avsz3.saturationKnown);
    CHECK(!trace.avsz3.saturated);
    CHECK(trace.otzKnown);
    CHECK(trace.otz == 60u);
}

void TestOutOfRangeZsf3DoesNotPublish() {
    Mode25TriangleGeometryInput input = MakeCandidateInput();
    input.zScaleFactor3 = 32768;
    input.szAfterRtpt.elementCandidateKnown = {};
    input.szAfterRtpt.candidateSz = {};
    input.szAfterRtpt.elementKnown = {{false, true, true, true}};
    input.szAfterRtpt.sz = {{0u, 10u, 20u, 30u}};

    const Mode25TriangleGeometryTrace trace =
        TraceMode25TriangleGeometry(input);
    CHECK(trace.avsz3.inputsAuthoritative);
    CHECK(trace.avsz3.candidateKnown);
    CHECK(trace.avsz3.mac0Candidate == 1966080);
    CHECK(trace.avsz3.otzCandidate == 480u);
    CHECK(!trace.avsz3.known);
    CHECK(!trace.avsz3.saturationKnown);
    CHECK(!trace.otzKnown);
}

void TestMissingInputsSuppressCandidates() {
    {
        Mode25TriangleGeometryInput input = MakeCandidateInput();
        input.sxy[1] = {};
        const Mode25TriangleGeometryTrace trace =
            TraceMode25TriangleGeometry(input);
        CHECK(!trace.nclip.inputsCandidateKnown);
        CHECK(!trace.nclip.candidateKnown);
    }
    {
        Mode25TriangleGeometryInput input = MakeCandidateInput();
        input.zScaleFactor3Known = false;
        const Mode25TriangleGeometryTrace trace =
            TraceMode25TriangleGeometry(input);
        CHECK(!trace.avsz3.inputsCandidateKnown);
        CHECK(!trace.avsz3.candidateKnown);
    }
    {
        Mode25TriangleGeometryInput input = MakeCandidateInput();
        input.szAfterRtpt.elementCandidateKnown[2] = false;
        const Mode25TriangleGeometryTrace trace =
            TraceMode25TriangleGeometry(input);
        CHECK(!trace.avsz3.inputsCandidateKnown);
        CHECK(!trace.avsz3.inputsAuthoritative);
        CHECK(!trace.avsz3.candidateKnown);
    }
}

void TestAuthoritativeAvsz3Saturation() {
    Mode25TriangleGeometryInput input = MakeCandidateInput();
    input.szAfterRtpt.elementCandidateKnown = {};
    input.szAfterRtpt.candidateSz = {};
    input.szAfterRtpt.elementKnown = {{false, true, true, true}};
    input.szAfterRtpt.sz = {{0u, 65535u, 65535u, 65535u}};
    const Mode25TriangleGeometryTrace trace =
        TraceMode25TriangleGeometry(input);
    CHECK(trace.avsz3.candidateKnown);
    CHECK(trace.avsz3.mac0Candidate == 805294080);
    CHECK(trace.avsz3.saturationCandidate);
    CHECK(trace.avsz3.otzCandidate == 65535u);
    CHECK(trace.avsz3.inputsAuthoritative);
    CHECK(trace.avsz3.known);
    CHECK(trace.avsz3.mac0 == 805294080);
    CHECK(trace.avsz3.otz == 65535u);
    CHECK(trace.avsz3.saturationKnown);
    CHECK(trace.avsz3.saturated);
    CHECK(trace.otzKnown);
    CHECK(trace.otz == 65535u);
}

struct SubmitFixture {
    TmdObject object{};
    PrPsxTmdSubmitDirect::Input input{};

    SubmitFixture() {
        object.vertices.resize(3u);
        object.primitives.resize(1u);
        TmdPrimitive& primitive = object.primitives[0];
        primitive.rawPacketKnown = true;
        primitive.rawPacketByteSize = 28u;
        primitive.mode = 0x25u;
        primitive.ilen = 6u;
        primitive.textured = true;
        primitive.quad = false;

        input.object = &object;
        input.sourceParseComplete = true;
        input.otShiftKnown = true;
        input.otShift = PrPsxTmdSubmitDirect::kRequiredOtShift;
        input.highPriorityKnown = true;
        input.highPriority = false;
        input.packetAllocatorKnown = true;
        input.packetAllocatorAddr = 0x1000u;
        input.orderingTable.headKnown = true;
        input.orderingTable.head = 0x2000u;
        input.orderingTable.lengthKnown = true;
        input.orderingTable.length = 16u;
        input.orderingTable.slotOldValueKnown = true;
        input.orderingTable.slotOldValue = 0x1234u;
    }
};

void MapGeometryToSubmit(
    const Mode25TriangleGeometryTrace& geometry,
    PrPsxTmdSubmitDirect::Input& submit) {
    for (std::size_t i = 0; i < submit.projectedSxy.size(); ++i) {
        submit.projectedSxy[i].known =
            geometry.projectedSxyKnown && geometry.sxy[i].known;
        submit.projectedSxy[i].value = geometry.sxy[i].word;
    }
    submit.visibilityKnown = geometry.visibilityKnown;
    submit.visible = geometry.visible;
    submit.otzKnown = geometry.otzKnown;
    submit.otz = geometry.otz;
}

void TestSubmitMappingStaysFailClosed() {
    {
        const Mode25TriangleGeometryTrace geometry =
            TraceMode25TriangleGeometry(MakeCandidateInput());
        SubmitFixture fixture;
        MapGeometryToSubmit(geometry, fixture.input);
        CHECK(!fixture.input.projectedSxy[0].known);
        CHECK(!fixture.input.visibilityKnown);
        CHECK(!fixture.input.otzKnown);
        const PrPsxTmdSubmitDirect::Result result =
            PrPsxTmdSubmitDirect::Build(fixture.input);
        CHECK(result.failure ==
              PrPsxTmdSubmitDirect::Failure::ProjectedSxyUnknown);
        CHECK(!result.readyToCommit);
    }
    {
        Mode25TriangleGeometryInput input = MakeCandidateInput();
        input.sxy = {{
            AuthoritativeSxy(0, 0),
            AuthoritativeSxy(2, 0),
            AuthoritativeSxy(0, 3),
        }};
        const Mode25TriangleGeometryTrace geometry =
            TraceMode25TriangleGeometry(input);
        SubmitFixture fixture;
        MapGeometryToSubmit(geometry, fixture.input);
        CHECK(fixture.input.projectedSxy[0].known);
        CHECK(fixture.input.visibilityKnown);
        CHECK(!fixture.input.otzKnown);
        const PrPsxTmdSubmitDirect::Result result =
            PrPsxTmdSubmitDirect::Build(fixture.input);
        CHECK(result.failure ==
              PrPsxTmdSubmitDirect::Failure::OtzUnknown);
        CHECK(!result.readyToCommit);
    }
    {
        SubmitFixture fixture;
        for (std::size_t i = 0; i < fixture.input.projectedSxy.size(); ++i) {
            fixture.input.projectedSxy[i].known = true;
            fixture.input.projectedSxy[i].value = PackSxy(
                static_cast<int16_t>(i), static_cast<int16_t>(i));
        }
        const PrPsxTmdSubmitDirect::Result result =
            PrPsxTmdSubmitDirect::Build(fixture.input);
        CHECK(result.failure ==
              PrPsxTmdSubmitDirect::Failure::VisibilityUnknown);
        CHECK(!result.readyToCommit);
    }
}

void TestAuthoritativeGeometryReadyToCommit() {
    Mode25TriangleGeometryInput input = MakeCandidateInput();
    input.sxy = {{
        AuthoritativeSxy(0, 0),
        AuthoritativeSxy(2, 0),
        AuthoritativeSxy(0, 3),
    }};
    input.szAfterRtpt.elementCandidateKnown = {};
    input.szAfterRtpt.candidateSz = {};
    input.szAfterRtpt.elementKnown = {{false, true, true, true}};
    input.szAfterRtpt.sz = {{0u, 10u, 20u, 30u}};

    const Mode25TriangleGeometryTrace geometry =
        TraceMode25TriangleGeometry(input);
    SubmitFixture fixture;
    MapGeometryToSubmit(geometry, fixture.input);
    CHECK(fixture.input.projectedSxy[0].known);
    CHECK(fixture.input.visibilityKnown);
    CHECK(fixture.input.visible);
    CHECK(fixture.input.otzKnown);
    CHECK(fixture.input.otz == 60u);

    const PrPsxTmdSubmitDirect::Result result =
        PrPsxTmdSubmitDirect::Build(fixture.input);
    CHECK(result.failure == PrPsxTmdSubmitDirect::Failure::None);
    CHECK(result.readyToCommit);
    CHECK(result.packetWrite.marked);
    CHECK(result.otDelta.marked);
    CHECK(result.allocatorDelta.marked);
}

}  // namespace

int main() {
    TestExactRtpt3TitleReplayVector();
    TestExactRtpt3FailClosedInputs();
    TestExactRtpt3DivideAndScreenFlags();
    TestExactRtpt3SzAndMacFlags();
    TestExactRtpt3FinalMac3Overflow();
    TestExactRtpt3DivideBoundary();
    TestExactRtpt3UnrSeedIndexEdges();
    TestCandidateNclipWinding();
    TestAuthoritativeSxyWinding();
    TestAuthoritativeNclipOverflow();
    TestAvsz3UsesRtptSlotsOneThroughThree();
    TestAuthoritativeAvsz3Publishes();
    TestOutOfRangeZsf3DoesNotPublish();
    TestMissingInputsSuppressCandidates();
    TestAuthoritativeAvsz3Saturation();
    TestSubmitMappingStaysFailClosed();
    TestAuthoritativeGeometryReadyToCommit();

    if (g_failedChecks != 0) {
        return 1;
    }
    std::printf("test_ss0_psx_gte_mode25_triangle: ok\n");
    return 0;
}
