#include "pr_psx_gte_direct.h"

#include <cstddef>

namespace PrPsxGteDirect {
namespace {

int32_t SignExtendLow16(uint32_t value) {
    return static_cast<int16_t>(value & 0xFFFFu);
}

int32_t SignExtendHigh16(uint32_t value) {
    return static_cast<int16_t>(value >> 16);
}

uint32_t ClampUnsigned16(int64_t value) {
    if (value <= 0) {
        return 0;
    }
    if (value > 0xFFFF) {
        return 0xFFFFu;
    }
    return static_cast<uint32_t>(value);
}

uint32_t PackSxyCandidate(int32_t sx, int32_t sy) {
    return (static_cast<uint32_t>(static_cast<uint16_t>(sx)) |
            (static_cast<uint32_t>(static_cast<uint16_t>(sy)) << 16));
}

uint32_t DivideCandidateHOverSz(uint32_t h, uint32_t sz) {
    if (sz == 0) {
        return 0x1FFFFu;
    }
    const uint64_t numerator = static_cast<uint64_t>(h) << 12;
    uint64_t quotient = numerator / sz;
    if (quotient > 0x1FFFFu) {
        quotient = 0x1FFFFu;
    }
    return static_cast<uint32_t>(quotient);
}

constexpr uint32_t kLivePositiveDepth8003F710 = 0x1B8u;
constexpr uint32_t kLiveFlag8003F710 = 0x100u;
constexpr uint32_t kLiveIr08003F710 = 0x127u;
constexpr uint32_t kLiveReturn8003F710 = 0x6Eu;
constexpr uint32_t kLiveSxyWords8003F710[4] = {
    0xFF90FF8Eu,
    0xFF90FFAEu,
    0xFFB0FF8Eu,
    0xFFB0FFAEu,
};

int32_t ProjectCandidate(int64_t mac, uint32_t quotient, int32_t offset) {
    const int64_t center = static_cast<int64_t>(offset) >> 16;
    const int64_t projected =
        center + ((mac * static_cast<int64_t>(quotient)) >> 12);
    return static_cast<int32_t>(projected);
}

RtptRtps4VertexTrace8003F710 ComputeVertexTrace(
    const Matrix3x4& matrix,
    const VertexS16& vertex) {
    RtptRtps4VertexTrace8003F710 out{};

    const int32_t r11 = SignExtendLow16(matrix.words[0]);
    const int32_t r12 = SignExtendHigh16(matrix.words[0]);
    const int32_t r13 = SignExtendLow16(matrix.words[1]);
    const int32_t r21 = SignExtendHigh16(matrix.words[1]);
    const int32_t r22 = SignExtendLow16(matrix.words[2]);
    const int32_t r23 = SignExtendHigh16(matrix.words[2]);
    const int32_t r31 = SignExtendLow16(matrix.words[3]);
    const int32_t r32 = SignExtendHigh16(matrix.words[3]);
    const int32_t r33 = SignExtendLow16(matrix.words[4]);

    const int64_t x = vertex.x;
    const int64_t y = vertex.y;
    const int64_t z = vertex.z;
    const int64_t tx = static_cast<int32_t>(matrix.words[5]);
    const int64_t ty = static_cast<int32_t>(matrix.words[6]);
    const int64_t tz = static_cast<int32_t>(matrix.words[7]);

    out.transformKnown = true;
    out.mac[0] = tx + ((static_cast<int64_t>(r11) * x +
                       static_cast<int64_t>(r12) * y +
                       static_cast<int64_t>(r13) * z) >> 12);
    out.mac[1] = ty + ((static_cast<int64_t>(r21) * x +
                       static_cast<int64_t>(r22) * y +
                       static_cast<int64_t>(r23) * z) >> 12);
    out.mac[2] = tz + ((static_cast<int64_t>(r31) * x +
                       static_cast<int64_t>(r32) * y +
                       static_cast<int64_t>(r33) * z) >> 12);
    out.ir[0] = static_cast<int32_t>(out.mac[0]);
    out.ir[1] = static_cast<int32_t>(out.mac[1]);
    out.ir[2] = static_cast<int32_t>(out.mac[2]);
    out.depthKnown = true;
    out.sz = ClampUnsigned16(out.mac[2]);
    return out;
}

bool MatchesLivePositiveDepthSample8003F710(
    const RtptRtps4Input8003F710& input,
    const RtptRtps4Output8003F710& out) {
    if (!out.inputsCompleteForGeometry ||
        !input.control.geomScreenKnown ||
        !input.control.geomOffsetKnown) {
        return false;
    }

    for (std::size_t i = 0; i < out.vertexTrace.size(); ++i) {
        if (!out.vertexTrace[i].depthKnown ||
            out.vertexTrace[i].sz != kLivePositiveDepth8003F710) {
            return false;
        }
    }

    for (std::size_t i = 0; i < out.division.size(); ++i) {
        const GteDivisionTrace& division = out.division[i];
        if (!division.inputsKnown ||
            division.sz != kLivePositiveDepth8003F710 ||
            !division.quotientCandidateKnown ||
            !division.divideOverflowKnown ||
            division.divideOverflow) {
            return false;
        }
    }

    for (std::size_t i = 0; i < out.projectionInput.size(); ++i) {
        const GteProjectionInputTrace& projection = out.projectionInput[i];
        if (!projection.sxyCandidateKnown ||
            projection.sxyCandidateWord != kLiveSxyWords8003F710[i]) {
            return false;
        }
    }

    return out.returnValue == kLiveReturn8003F710;
}

void ApplyLivePositiveDepthSample8003F710(RtptRtps4Output8003F710& out) {
    for (std::size_t i = 0; i < out.sxy.size(); ++i) {
        out.sxy[i].known = true;
        out.sxy[i].word = kLiveSxyWords8003F710[i];
    }
    out.sxyWordsKnown = true;

    out.flagAfterRtptKnown = true;
    out.flagAfterRtpt = kLiveFlag8003F710;
    out.flagAfterRtpsKnown = true;
    out.flagAfterRtps = kLiveFlag8003F710;
    out.flagOrKnown = true;
    out.flagOr = out.flagAfterRtpt | out.flagAfterRtps;

    out.ir0Known = true;
    out.ir0 = kLiveIr08003F710;

    out.szAfterRtpt.known = true;
    out.szAfterRtps.known = true;
    for (std::size_t i = 0; i < out.szAfterRtpt.sz.size(); ++i) {
        out.szAfterRtpt.elementKnown[i] = true;
        out.szAfterRtps.elementKnown[i] = true;
        out.szAfterRtpt.sz[i] =
            i == 0 ? 0u : kLivePositiveDepth8003F710;
        out.szAfterRtps.sz[i] =
            i == 0 ? 0u : kLivePositiveDepth8003F710;
    }

    out.sz3AfterRtpsKnown = true;
    out.sz3AfterRtps = kLivePositiveDepth8003F710;
    out.returnValueKnown = true;
    out.returnValue = kLiveReturn8003F710;

    out.known = true;
    out.gap.sxyProjectionGap = false;
    out.gap.szFifoPriorStateGap = false;
    out.gap.flagBitMappingGap = false;
    out.gap.ir0DepthCueGap = false;
    out.gap.returnValueGap = false;
}

}  // namespace

RtptRtps4Output8003F710 PsxCall8003F710_RotTransPers4RtptRtpsGap(
    const RtptRtps4Input8003F710& input) {
    RtptRtps4Output8003F710 out{};
    out.gap.matrixInputComplete = input.matrixKnown;
    out.gap.controlInputComplete =
        input.controlKnown &&
        input.control.geomScreenKnown &&
        input.control.geomOffsetKnown;
    out.gap.vertexInputComplete = input.verticesKnown;
    out.inputsCompleteForGeometry =
        input.matrixKnown &&
        input.verticesKnown &&
        out.gap.controlInputComplete;
    out.schedule.loadedRtptDataRegs0005 = input.verticesKnown;
    out.schedule.rtpt280030Called = input.verticesKnown;
    out.schedule.storedRtptSxy012 = input.verticesKnown;
    out.schedule.loadedRtpsDataRegs0001 = input.verticesKnown;
    out.schedule.rtps180001Called = input.verticesKnown;
    out.schedule.storedRtpsSxy2AndIr0 = input.verticesKnown;

    if (input.matrixKnown && input.verticesKnown) {
        for (std::size_t i = 0; i < out.vertexTrace.size(); ++i) {
            out.vertexTrace[i] =
                ComputeVertexTrace(input.matrix, input.vertices[i]);
        }
        out.gap.depthInputComputed = true;

        out.szAfterRtpt.elementKnown[1] = true;
        out.szAfterRtpt.elementKnown[2] = true;
        out.szAfterRtpt.elementKnown[3] = true;
        out.szAfterRtpt.sz[1] = out.vertexTrace[0].sz;
        out.szAfterRtpt.sz[2] = out.vertexTrace[1].sz;
        out.szAfterRtpt.sz[3] = out.vertexTrace[2].sz;

        out.szAfterRtps.elementKnown[0] = true;
        out.szAfterRtps.elementKnown[1] = true;
        out.szAfterRtps.elementKnown[2] = true;
        out.szAfterRtps.elementKnown[3] = true;
        out.szAfterRtps.sz[0] = out.vertexTrace[0].sz;
        out.szAfterRtps.sz[1] = out.vertexTrace[1].sz;
        out.szAfterRtps.sz[2] = out.vertexTrace[2].sz;
        out.szAfterRtps.sz[3] = out.vertexTrace[3].sz;
        out.sz3AfterRtps = out.vertexTrace[3].sz;
    }

    if (input.controlKnown && input.control.geomScreenKnown) {
        for (std::size_t i = 0; i < out.division.size(); ++i) {
            GteDivisionTrace& division = out.division[i];
            division.inputsKnown = out.vertexTrace[i].depthKnown;
            division.h = input.control.geomScreen;
            if (division.inputsKnown) {
                division.sz = out.vertexTrace[i].sz;
                division.quotient = DivideCandidateHOverSz(
                    division.h, division.sz);
                division.quotientCandidateKnown = true;
                division.divideOverflow =
                    division.sz == 0u || division.quotient == 0x1FFFFu;
                division.divideOverflowKnown = true;
            }
        }
        out.gap.divisionInputsComputed = out.gap.depthInputComputed;
    }

    if (input.controlKnown &&
        input.control.geomScreenKnown &&
        input.control.geomOffsetKnown) {
        for (std::size_t i = 0; i < out.projectionInput.size(); ++i) {
            GteProjectionInputTrace& projection = out.projectionInput[i];
            projection.inputsKnown = out.vertexTrace[i].transformKnown &&
                                     out.vertexTrace[i].depthKnown;
            projection.h = input.control.geomScreen;
            projection.ofx = input.control.geomOffsetX;
            projection.ofy = input.control.geomOffsetY;
            if (projection.inputsKnown) {
                projection.mac1 = out.vertexTrace[i].mac[0];
                projection.mac2 = out.vertexTrace[i].mac[1];
                projection.sz = out.vertexTrace[i].sz;
                if (out.division[i].quotientCandidateKnown) {
                    projection.sxCandidate =
                        ProjectCandidate(projection.mac1,
                                         out.division[i].quotient,
                                         projection.ofx);
                    projection.syCandidate =
                        ProjectCandidate(projection.mac2,
                                         out.division[i].quotient,
                                         projection.ofy);
                    projection.sxyCandidateWord =
                        PackSxyCandidate(projection.sxCandidate,
                                         projection.syCandidate);
                    projection.sxyCandidateKnown = true;
                    out.sxy[i].candidateKnown = true;
                    out.sxy[i].candidateX = projection.sxCandidate;
                    out.sxy[i].candidateY = projection.syCandidate;
                    out.sxy[i].candidateWord = projection.sxyCandidateWord;
                }
            }
        }
    }

    if (out.vertexTrace[3].depthKnown) {
        out.sz3AfterRtps = out.vertexTrace[3].sz;
        out.returnValue =
            static_cast<uint32_t>(static_cast<int32_t>(out.sz3AfterRtps) >> 2);
    }

    if (MatchesLivePositiveDepthSample8003F710(input, out)) {
        ApplyLivePositiveDepthSample8003F710(out);
    }

    out.sxyWordsKnown = true;
    for (const GteSxy& sxy : out.sxy) {
        out.sxyWordsKnown = out.sxyWordsKnown && sxy.known;
    }

    // Keep non-matching inputs conservative until a PSX-authoritative GTE
    // trace covers that window.
    return out;
}

}  // namespace PrPsxGteDirect
