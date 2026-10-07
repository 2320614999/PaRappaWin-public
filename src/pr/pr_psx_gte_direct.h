#pragma once

#include <array>
#include <cstdint>

namespace PrPsxGteDirect {

struct GteControlState {
    bool geomScreenKnown = false;
    uint32_t geomScreen = 0;
    bool geomOffsetKnown = false;
    int32_t geomOffsetX = 0;
    int32_t geomOffsetY = 0;
    bool depthCueKnown = false;
    int32_t depthCueA = 0;
    int32_t depthCueB = 0;
    bool zScaleFactorKnown = false;
    int32_t zScaleFactor3 = 0;
    int32_t zScaleFactor4 = 0;
};

struct Matrix3x4 {
    std::array<uint32_t, 8> words{};
};

// Registers touched by the original matrix helpers. This is shared arithmetic,
// not a second scene owner or a substitute for the remaining GTE commands.
struct MatrixRegisters {
    Matrix3x4 matrix{}; // control 0..7; control 4 is sign-extended RT33.
    uint32_t vectorXY0 = 0;
    int32_t vectorZ0 = 0;
    std::array<int32_t, 3> ir{};
    std::array<int32_t, 3> mac{};
    uint32_t flags = 0;
    // Registers needed by the original unlit TMD handlers. Kept in the same
    // state as the matrix helpers; a rejected primitive still changes FIFOs.
    std::array<uint32_t, 2> vectorXY12{};
    std::array<int32_t, 2> vectorZ12{};
    std::array<uint32_t, 3> rgb{};
    std::array<uint32_t, 3> sxy{};
    std::array<uint16_t, 4> sz{};
    int32_t mac0 = 0;
    int32_t ir0 = 0;
    uint16_t otz = 0;
    int32_t ofx = 0, ofy = 0;
    uint16_t h = 0;
    int16_t dqa = 0;
    int32_t dqb = 0;
    int16_t zsf3 = 0, zsf4 = 0;
    // 原查表开方使用 LZCS/LZCR，写入不改变 MAC、IR 或 FLAG。
    uint32_t lzcs = 0, lzcr = 0;
    uint32_t rgbc = 0;
    std::array<int32_t,3> farColor{}; // control 21..23, initialized by gte_init.
};

// MVMVA with mx=rotation, cv=none, lm=0. The original helpers use
// 41E012 (IR, sf=0), 49E012 (IR, sf=1) and 486012 (V0, sf=1).
void ExecuteRotationMvmva(MatrixRegisters& state, bool fromIr, bool shift12);
// The original handlers use sf=1/lm=0 only. These update the actual register
// state; the existing partial/candidate tracing APIs below remain unchanged.
void ExecutePerspective(MatrixRegisters& state, bool triple);
void ExecuteNclip(MatrixRegisters& state);
void ExecuteAverageZ(MatrixRegisters& state, bool four);
// 原 MIMe 的 GPF(sf=1,lm=0)，同时推进 RGB FIFO 并保留颜色命令字节。
void ExecuteGeneralMultiply(MatrixRegisters& state);

struct VertexS16 {
    int16_t x = 0;
    int16_t y = 0;
    int16_t z = 0;
    int16_t pad = 0;
};

struct GteSxy {
    bool known = false;
    uint32_t word = 0;
    bool candidateKnown = false;
    int32_t candidateX = 0;
    int32_t candidateY = 0;
    uint32_t candidateWord = 0;
};

struct GteSzFifo {
    bool known = false;
    std::array<uint32_t, 4> sz{};
    std::array<bool, 4> elementKnown{};
    bool candidateKnown = false;
    std::array<uint32_t, 4> candidateSz{};
    std::array<bool, 4> elementCandidateKnown{};
};

struct GteDivisionTrace {
    bool known = false;
    bool inputsKnown = false;
    uint32_t h = 0;
    uint32_t sz = 0;
    bool quotientCandidateKnown = false;
    bool divideOverflow = false;
    bool divideOverflowKnown = false;
    bool divideOverflowCandidate = false;
    bool divideOverflowCandidateKnown = false;
    uint32_t quotient = 0;
    uint32_t flagBits = 0;
};

struct GteProjectionInputTrace {
    bool inputsKnown = false;
    int64_t mac1 = 0;
    int64_t mac2 = 0;
    uint32_t sz = 0;
    uint32_t h = 0;
    int32_t ofx = 0;
    int32_t ofy = 0;
    bool sxyCandidateKnown = false;
    int32_t sxCandidate = 0;
    int32_t syCandidate = 0;
    uint32_t sxyCandidateWord = 0;
};

struct RtptRtps4VertexTrace8003F710 {
    bool transformCandidateKnown = false;
    std::array<int64_t, 3> macCandidate{};
    std::array<int32_t, 3> irCandidate{};
    bool depthCandidateKnown = false;
    uint32_t szCandidate = 0;
};

struct GteNclipTrace {
    bool inputsAuthoritative = false;
    bool inputsCandidateKnown = false;
    bool candidateKnown = false;
    int64_t mac0Candidate = 0;
    bool known = false;
    int32_t mac0 = 0;
    bool overflowKnown = false;
    bool overflow = false;
};

struct GteAvsz3Trace {
    bool inputsAuthoritative = false;
    bool inputsCandidateKnown = false;
    bool candidateKnown = false;
    int32_t zScaleFactor3 = 0;
    int64_t mac0Candidate = 0;
    uint16_t otzCandidate = 0;
    bool saturationCandidate = false;
    bool known = false;
    int32_t mac0 = 0;
    uint16_t otz = 0;
    bool saturationKnown = false;
    bool saturated = false;
};

struct Mode25TriangleGeometryInput {
    std::array<GteSxy, 3> sxy{};
    GteSzFifo szAfterRtpt{};
    bool zScaleFactor3Known = false;
    int32_t zScaleFactor3 = 0;
};

struct Mode25TriangleGeometryTrace {
    std::array<GteSxy, 3> sxy{};
    GteSzFifo szAfterRtpt{};
    GteNclipTrace nclip{};
    GteAvsz3Trace avsz3{};
    bool projectedSxyKnown = false;
    bool visibilityKnown = false;
    bool visible = false;
    bool otzKnown = false;
    uint16_t otz = 0;
};

struct RtptRtps4GapState8003F710 {
    bool matrixInputComplete = false;
    bool controlInputComplete = false;
    bool vertexInputComplete = false;
    bool depthInputComputed = false;
    bool divisionInputsComputed = false;
    bool sxyProjectionGap = true;
    bool szFifoPriorStateGap = true;
    bool flagBitMappingGap = true;
    bool ir0DepthCueGap = true;
    bool returnValueGap = true;
};

struct RtptRtps4Schedule8003F710 {
    bool loadedRtptDataRegs0005 = false;
    bool rtpt280030Called = false;
    bool storedRtptSxy012 = false;
    bool loadedRtpsDataRegs0001 = false;
    bool rtps180001Called = false;
    bool storedRtpsSxy2AndIr0 = false;
};

struct RtptRtps4Input8003F710 {
    bool matrixKnown = false;
    Matrix3x4 matrix{};
    bool controlKnown = false;
    GteControlState control{};
    bool verticesKnown = false;
    std::array<VertexS16, 4> vertices{};
};

struct RtptRtps4Output8003F710 {
    bool known = false;
    bool inputsCompleteForGeometry = false;
    RtptRtps4GapState8003F710 gap{};
    RtptRtps4Schedule8003F710 schedule{};
    std::array<RtptRtps4VertexTrace8003F710, 4> vertexTrace{};
    std::array<GteSxy, 4> sxy{};
    bool sxyWordsKnown = false;
    bool ir0Known = false;
    uint32_t ir0 = 0;
    GteSzFifo szAfterRtpt{};
    GteSzFifo szAfterRtps{};
    bool flagAfterRtptKnown = false;
    uint32_t flagAfterRtpt = 0;
    bool flagAfterRtpsKnown = false;
    uint32_t flagAfterRtps = 0;
    bool flagOrKnown = false;
    uint32_t flagOr = 0;
    bool sz3AfterRtpsKnown = false;
    uint32_t sz3AfterRtps = 0;
    bool sz3AfterRtpsCandidateKnown = false;
    uint32_t sz3AfterRtpsCandidate = 0;
    bool returnValueKnown = false;
    uint32_t returnValue = 0;
    bool returnValueCandidateKnown = false;
    uint32_t returnValueCandidate = 0;
    std::array<GteDivisionTrace, 4> division{};
    std::array<GteProjectionInputTrace, 4> projectionInput{};
};

// Exact three-vertex RTPT command input. This is deliberately separate from
// the four-vertex 8003F710 gap trace: a complete input here is eligible for
// authoritative RTPT output, while candidate fields never are.
struct Rtpt3ExactInput280030 {
    bool matrixKnown = false;
    Matrix3x4 matrix{};
    bool controlKnown = false;
    GteControlState control{};
    bool verticesKnown = false;
    std::array<VertexS16, 3> vertices{};
};

struct Rtpt3ExactOutput280030 {
    bool known = false;
    std::array<GteSxy, 3> sxy{};
    // RTPT replaces SZ1..SZ3 but preserves the prior SZ0 dependency. The
    // element-known mask therefore publishes only slots 1..3.
    GteSzFifo szAfterRtpt{};
    bool flagAfterRtptKnown = false;
    uint32_t flagAfterRtpt = 0;
    bool ir0Known = false;
    uint16_t ir0 = 0;
};

RtptRtps4Output8003F710 PsxCall8003F710_RotTransPers4RtptRtpsGap(
    const RtptRtps4Input8003F710& input);

Rtpt3ExactOutput280030 ExecuteRtpt3Exact280030(
    const Rtpt3ExactInput280030& input);

Mode25TriangleGeometryTrace TraceMode25TriangleGeometry(
    const Mode25TriangleGeometryInput& input);

}  // namespace PrPsxGteDirect
