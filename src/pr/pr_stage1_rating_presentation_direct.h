#pragma once

#include <cstdint>

struct PrStage1RatingPresentationDirectFrameUpdateFlag0200Branch9094Input {
    bool ctxFlag0200Set = false;
    uint16_t transitionAnim18E = 0u;
};

struct PrStage1RatingPresentationDirectFrameUpdateFlag0200Branch9094Result {
    bool branchTaken = false;
    bool resourcePairKnown = false;
    uint16_t ctxE0ResourceIndex = 0u;
    uint16_t ctxECResourceIndex = 0u;
    bool setCtxFlag20000 = false;
};

struct PrStage1RatingPresentationDirectPostTailFlag0200Dispatch7A60Input {
    bool ctxFlag0200Set = false;
    uint16_t rightRankActiveRow4E = 0u;
};

struct PrStage1RatingPresentationDirectPostTailFlag0200Dispatch7A60Result {
    bool call1A654 = false;
    uint8_t call1A654Arg = 0u;
};

PrStage1RatingPresentationDirectFrameUpdateFlag0200Branch9094Result
PrStage1RatingPresentationDirectResolveFrameUpdateFlag0200Branch9094(
    const PrStage1RatingPresentationDirectFrameUpdateFlag0200Branch9094Input& input);

PrStage1RatingPresentationDirectPostTailFlag0200Dispatch7A60Result
PrStage1RatingPresentationDirectResolvePostTailFlag0200Dispatch7A60(
    const PrStage1RatingPresentationDirectPostTailFlag0200Dispatch7A60Input& input);
