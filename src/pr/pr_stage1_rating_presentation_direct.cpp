#include "pr_stage1_rating_presentation_direct.h"

#include <array>

namespace {

constexpr std::array<uint8_t, 5> kStage1PostTailFlag0200A654ArgByRow7A60 = {
    5u,
    1u,
    2u,
    3u,
    6u,
};

} // namespace

PrStage1RatingPresentationDirectFrameUpdateFlag0200Branch9094Result
PrStage1RatingPresentationDirectResolveFrameUpdateFlag0200Branch9094(
    const PrStage1RatingPresentationDirectFrameUpdateFlag0200Branch9094Input& input) {
    PrStage1RatingPresentationDirectFrameUpdateFlag0200Branch9094Result out{};
    if (!input.ctxFlag0200Set) {
        return out;
    }

    out.branchTaken = true;
    switch (input.transitionAnim18E) {
    case 1:
        out.ctxECResourceIndex = 74u;
        out.ctxE0ResourceIndex = 126u;
        break;
    case 2:
        out.ctxECResourceIndex = 75u;
        out.ctxE0ResourceIndex = 127u;
        break;
    case 4:
        out.ctxECResourceIndex = 76u;
        out.ctxE0ResourceIndex = 128u;
        break;
    case 3:
        out.ctxECResourceIndex = 77u;
        out.ctxE0ResourceIndex = 129u;
        break;
    default:
        break;
    }

    if (out.ctxECResourceIndex != 0u && out.ctxE0ResourceIndex != 0u) {
        out.resourcePairKnown = true;
        out.setCtxFlag20000 = true;
    }
    return out;
}

PrStage1RatingPresentationDirectPostTailFlag0200Dispatch7A60Result
PrStage1RatingPresentationDirectResolvePostTailFlag0200Dispatch7A60(
    const PrStage1RatingPresentationDirectPostTailFlag0200Dispatch7A60Input& input) {
    PrStage1RatingPresentationDirectPostTailFlag0200Dispatch7A60Result out{};
    if (!input.ctxFlag0200Set) {
        return out;
    }

    if (input.rightRankActiveRow4E <
        kStage1PostTailFlag0200A654ArgByRow7A60.size()) {
        out.call1A654 = true;
        out.call1A654Arg =
            kStage1PostTailFlag0200A654ArgByRow7A60[input.rightRankActiveRow4E];
    }
    return out;
}
