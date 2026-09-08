#pragma once

#include "pr_stage1_save_card_hal_direct.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace PrSS0State16RuntimeEnvelopeImportDirect {

struct State16RuntimeTypedFactsEnvelope {
    PrStage1SaveCardHalDirect::State16CardReadRuntimeTypedFacts800179B4
        facts{};
    std::array<uint8_t,
               PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4>
        fullPayloadBytes{};
    std::string source;
    std::string authority;
    std::string fullPayloadBytesSha256;
};

struct State16RuntimeEnvelopeStaging {
    bool active = false;
    std::string headerArgs;
    std::string payloadHex;
    std::array<bool,
               PrStage1SaveCardHalDirect::kCardReadBlockBytes800179B4>
        payloadByteKnown{};
    std::size_t payloadBytesKnown = 0u;
};

bool ParseValidatedLiveGdbState16RuntimeEnvelopeArgs(
    const std::string& args,
    State16RuntimeTypedFactsEnvelope& out,
    std::string& error);

void ClearStagedValidatedLiveGdbState16RuntimeEnvelope(
    State16RuntimeEnvelopeStaging& staging);
bool BeginStagedValidatedLiveGdbState16RuntimeEnvelopeArgs(
    State16RuntimeEnvelopeStaging& staging,
    const std::string& args,
    std::string& error);
bool AppendStagedValidatedLiveGdbState16RuntimeEnvelopeChunk(
    State16RuntimeEnvelopeStaging& staging,
    const std::string& args,
    std::string& error);
bool CommitStagedValidatedLiveGdbState16RuntimeEnvelope(
    State16RuntimeEnvelopeStaging& staging,
    State16RuntimeTypedFactsEnvelope& out,
    std::string& error);

}  // namespace PrSS0State16RuntimeEnvelopeImportDirect
