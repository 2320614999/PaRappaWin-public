#include "pr/pr_psx_gte_direct.h"

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

RtptRtps4Input8003F710 MakeInput() {
    RtptRtps4Input8003F710 input{};
    input.matrixKnown = true;
    input.matrix.words = {{
        0x00001000u,
        0x00000000u,
        0x00001000u,
        0x00000000u,
        0x00001000u,
        0u,
        0u,
        40000u,
    }};
    input.controlKnown = true;
    input.verticesKnown = true;
    input.vertices = {{
        {0, 0, -2000, 0},
        {0, 0, 0, 0},
        {0, 0, 100, 0},
        {0, 0, 32767, 0},
    }};
    return input;
}

void CheckDepthCandidatesUnknown(const RtptRtps4Output8003F710& output) {
    CHECK(!output.szAfterRtpt.candidateKnown);
    for (bool elementCandidateKnown :
         output.szAfterRtpt.elementCandidateKnown) {
        CHECK(!elementCandidateKnown);
    }
    CHECK(!output.szAfterRtps.candidateKnown);
    for (bool elementCandidateKnown :
         output.szAfterRtps.elementCandidateKnown) {
        CHECK(!elementCandidateKnown);
    }
    CHECK(!output.sz3AfterRtpsCandidateKnown);
    CHECK(!output.returnValueCandidateKnown);

    CHECK(!output.szAfterRtpt.known);
    for (bool elementKnown : output.szAfterRtpt.elementKnown) {
        CHECK(!elementKnown);
    }
    CHECK(!output.szAfterRtps.known);
    for (bool elementKnown : output.szAfterRtps.elementKnown) {
        CHECK(!elementKnown);
    }
    CHECK(!output.sz3AfterRtpsKnown);
    CHECK(!output.returnValueKnown);
}

void TestKnownGeometryProducesCandidatesOnly() {
    const RtptRtps4Output8003F710 output =
        PsxCall8003F710_RotTransPers4RtptRtpsGap(MakeInput());
    const std::array<uint32_t, 3> expectedRtpt = {{38000u, 40000u, 40100u}};
    const std::array<uint32_t, 4> expectedRtps = {{
        38000u,
        40000u,
        40100u,
        65535u,
    }};

    CHECK(!output.szAfterRtpt.known);
    CHECK(!output.szAfterRtpt.elementKnown[0]);
    CHECK(!output.szAfterRtpt.elementCandidateKnown[0]);
    CHECK(!output.szAfterRtpt.candidateKnown);
    for (std::size_t i = 0; i < expectedRtpt.size(); ++i) {
        CHECK(!output.szAfterRtpt.elementKnown[i + 1u]);
        CHECK(output.szAfterRtpt.elementCandidateKnown[i + 1u]);
        CHECK(output.szAfterRtpt.candidateSz[i + 1u] == expectedRtpt[i]);
    }

    CHECK(!output.szAfterRtps.known);
    CHECK(output.szAfterRtps.candidateKnown);
    for (std::size_t i = 0; i < expectedRtps.size(); ++i) {
        CHECK(!output.szAfterRtps.elementKnown[i]);
        CHECK(output.szAfterRtps.elementCandidateKnown[i]);
        CHECK(output.szAfterRtps.candidateSz[i] == expectedRtps[i]);
    }
    CHECK(!output.sz3AfterRtpsKnown);
    CHECK(output.sz3AfterRtpsCandidateKnown);
    CHECK(output.sz3AfterRtpsCandidate == 65535u);
    CHECK(!output.returnValueKnown);
    CHECK(output.returnValueCandidateKnown);
    CHECK(output.returnValueCandidate == 16383u);

    CHECK(!output.gap.controlInputComplete);
    CHECK(!output.inputsCompleteForGeometry);
    CHECK(!output.sxyWordsKnown);
    for (const GteSxy& sxy : output.sxy) {
        CHECK(!sxy.known);
        CHECK(!sxy.candidateKnown);
    }
    CHECK(!output.flagAfterRtptKnown);
    CHECK(!output.flagAfterRtpsKnown);
    CHECK(!output.flagOrKnown);
    CHECK(!output.ir0Known);
    CHECK(!output.returnValueKnown);
    CHECK(!output.known);
    CHECK(output.gap.szFifoPriorStateGap);
}

void TestMissingMatrixKeepsDepthCandidatesUnknown() {
    RtptRtps4Input8003F710 input = MakeInput();
    input.matrixKnown = false;
    CheckDepthCandidatesUnknown(
        PsxCall8003F710_RotTransPers4RtptRtpsGap(input));
}

void TestMissingVerticesKeepsDepthCandidatesUnknown() {
    RtptRtps4Input8003F710 input = MakeInput();
    input.verticesKnown = false;
    CheckDepthCandidatesUnknown(
        PsxCall8003F710_RotTransPers4RtptRtpsGap(input));
}

}  // namespace

int main() {
    TestKnownGeometryProducesCandidatesOnly();
    TestMissingMatrixKeepsDepthCandidatesUnknown();
    TestMissingVerticesKeepsDepthCandidatesUnknown();

    if (g_failedChecks != 0) {
        return 1;
    }
    std::printf("test_ss0_psx_gte_rtps_fifo: ok\n");
    return 0;
}
