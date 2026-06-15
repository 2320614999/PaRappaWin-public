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
};

struct GteDivisionTrace {
    bool known = false;
    bool inputsKnown = false;
    uint32_t h = 0;
    uint32_t sz = 0;
    bool quotientCandidateKnown = false;
    bool divideOverflow = false;
    bool divideOverflowKnown = false;
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
    bool transformKnown = false;
    std::array<int64_t, 3> mac{};
    std::array<int32_t, 3> ir{};
    bool depthKnown = false;
    uint32_t sz = 0;
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
    bool returnValueKnown = false;
    uint32_t returnValue = 0;
    std::array<GteDivisionTrace, 4> division{};
    std::array<GteProjectionInputTrace, 4> projectionInput{};
};

RtptRtps4Output8003F710 PsxCall8003F710_RotTransPers4RtptRtpsGap(
    const RtptRtps4Input8003F710& input);

}  // namespace PrPsxGteDirect
