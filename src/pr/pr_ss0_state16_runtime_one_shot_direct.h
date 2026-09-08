#pragma once

#include "pr_stage1_save_card_hal_direct.h"

#include <array>
#include <cstdint>

namespace PrSS0State16RuntimeOneShotDirect {

struct State16RuntimeTypedFactsOneShot {
    bool pending = false;
    PrStage1SaveCardHalDirect::State16CardReadRuntimeTypedFacts800179B4 facts{};
    std::array<uint8_t, PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4>
        fullPayloadBytes{};
};

void ClearRuntimeState16CardReadTypedFactsOneShot(
    State16RuntimeTypedFactsOneShot& queue);

bool QueueRuntimeState16CardReadTypedFactsOneShot(
    State16RuntimeTypedFactsOneShot& queue,
    const PrStage1SaveCardHalDirect::State16CardReadRuntimeTypedFacts800179B4&
        facts);

bool ImportPendingState16TypedFactsBeforeConfirm(
    State16RuntimeTypedFactsOneShot& queue,
    int32_t selectedBlockIndex);

}  // namespace PrSS0State16RuntimeOneShotDirect
