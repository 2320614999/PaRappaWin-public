#include "pr_ss0_state16_runtime_one_shot_direct.h"

#include <algorithm>

namespace PrSS0State16RuntimeOneShotDirect {

void ClearRuntimeState16CardReadTypedFactsOneShot(
    State16RuntimeTypedFactsOneShot& queue) {
    queue = {};
}

bool QueueRuntimeState16CardReadTypedFactsOneShot(
    State16RuntimeTypedFactsOneShot& queue,
    const PrStage1SaveCardHalDirect::State16CardReadRuntimeTypedFacts800179B4&
        facts) {
    ClearRuntimeState16CardReadTypedFactsOneShot(queue);
    if (!facts.fullPayloadBytesKnown ||
        facts.fullPayloadBytes == nullptr ||
        facts.fullPayloadByteCount <
            PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4) {
        return false;
    }

    queue.pending = true;
    queue.facts = facts;
    std::copy(facts.fullPayloadBytes,
              facts.fullPayloadBytes +
                  PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4,
              queue.fullPayloadBytes.begin());
    queue.facts.fullPayloadBytes = queue.fullPayloadBytes.data();
    queue.facts.fullPayloadByteCount =
        PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4;
    return true;
}

bool ImportPendingState16TypedFactsBeforeConfirm(
    State16RuntimeTypedFactsOneShot& queue,
    int32_t selectedBlockIndex) {
    if (!queue.pending) {
        return false;
    }
    PrStage1SaveCardHalDirect::State16CardReadRuntimeTypedFacts800179B4
        facts = queue.facts;
    facts.fullPayloadBytes = queue.fullPayloadBytes.data();
    facts.fullPayloadByteCount =
        PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4;
    const bool published =
        PrStage1SaveCardHalDirect::
            PublishRuntimeState16CardReadTypedCarrier800179B4FromTypedFacts(
                facts,
                selectedBlockIndex);
    ClearRuntimeState16CardReadTypedFactsOneShot(queue);
    return published;
}

}  // namespace PrSS0State16RuntimeOneShotDirect
