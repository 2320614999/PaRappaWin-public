#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

constexpr size_t kPrStage1ScorerHostDirectReplayBufferCapacity = 600u;

enum class PrStage1ScorerHostDirectAcceptedControlWriterSourceKind : uint8_t {
    None = 0u,
    LocalZeroClear,
    LocalLiveNormalized,
    LocalPassthroughControl,
    LocalControlSample800,
    LocalReplayPreserveClear,
    ScriptedSharedBufferSample,
};

struct PrStage1ScorerHostDirectAcceptedScriptedWriterState {
    bool active = false;
    uint32_t dueTick96EEF8 = 0;
    uint32_t classMaskEEFC = 0;
};

struct PrStage1ScorerHostDirectAcceptedReplayBufferState {
    bool replayMirrorKnown8008EEF8 = false;
    bool replayMirrorProducerKnown8008EEF8 = false;
    uint32_t replayMirrorProducerFunction = 0;
    bool replayMirrorByteCountKnown8008EEF8 = false;
    uint32_t replayMirrorKnownByteCount8008EEF8 = 0;
    uint32_t writeCount901C0 = 0;
    uint32_t publishedCount901BC = 0;
    std::array<uint32_t, kPrStage1ScorerHostDirectReplayBufferCapacity> tick96EEF8{};
    std::array<uint32_t, kPrStage1ScorerHostDirectReplayBufferCapacity> classMaskEEFC{};
};

struct PrStage1ScorerHostDirectAcceptedCarrierCore {
    bool available = false;
    uint8_t controlWriterSourceKind = 0;
    uint16_t rawControlSample18 = 0;
    uint16_t controlMask18 = 0;
    uint8_t classToken20 = 0;
    uint8_t halfWindow34 = 0;
    uint8_t substate50 = 0;
    bool acceptedTick96Known = false;
    int32_t acceptedTick96 = 0;
    uint32_t acceptedTick96LastUpdateQueryFrame = 0u;
    uint8_t acceptedTick96LastUpdateSourceKind = 0u;
    uint16_t acceptedTick96LastUpdateControlMask18 = 0u;
    uint8_t acceptedTick96LastUpdateClassToken20 = 0u;
    bool acceptedTick96LastUpdateViaPostCtx10 = false;
    uint32_t acceptedTick96LastUpdateCtxInput18 = 0u;
    uint32_t acceptedTick96LastUpdatePreviousInputMask801CCBB8 = 0u;
    bool acceptedTick96LastUpdateAcceptedMaskChanged = false;
    uint16_t phase384 = 0;
    int32_t writePageOrdinal38 = 0;
    uint8_t recordSlot24 = 0;
    uint8_t recordRemainder24 = 0;
    uint8_t timingTemplateSlot48 = 0;
    bool recordedSplit = false;
};

struct PrStage1ScorerHostDirectAcceptedCarrierCoreUpdateInput {
    bool replayMode52 = false;
    int32_t tick96 = 0;
    uint32_t queryFrame30 = 0u;
    uint16_t rawControlMask18 = 0u;
    bool writerClassToken20Known = false;
    uint8_t writerClassToken20 = 0u;
    bool inputDispatchConsumedReplaySlot = false;
    bool inputDispatchWriteCurrentTick = false;
    bool inputDispatchReplayTickKnown = false;
    int32_t inputDispatchReplayTick96 = 0;
    uint8_t descriptorSubstate50 = 0u;
    uint8_t halfWindow34 = 0u;
    int32_t writePageOrdinal38 = 0;
};

struct PrStage1ScorerHostDirectSteadyInput7A60Result {
    bool writeCtx18 = false;
    bool ctx18Known = false;
    uint32_t ctx18Value = 0u;
    bool writeCtx20 = false;
    bool ctx20Known = false;
    int32_t ctx20Value = 0;
    bool incrementReplayReadIndex901C0 = false;
    uint32_t replayReadIndex901C0After = 0u;
    bool writeCtx10CurrentTick = false;
    bool writeCtx10ReplayTick = false;
    int32_t ctx10ReplayTick = 0;
};

struct PrStage1ScorerHostDirectHudOverlayStart801CA1ECInput {
    bool call801CA1EC = false;
    uint16_t rightRankActiveRow4E = 0u;
    uint16_t writerControlSample18 = 0u;
    bool currentDescriptor40Available = false;
    uint32_t currentDescriptor40Mask8 = 0u;
    bool lookaheadDescriptor44Available = false;
    uint32_t lookaheadDescriptor44Mask8 = 0u;
};

struct PrStage1ScorerHostDirectHudOverlayStart801CA1ECResult {
    bool selectorWrite = false;
    uint8_t selector = 0u;
    bool rebuildRequested = false;
    bool ctxFlag800000Set = false;
};

struct PrStage1ScorerHostDirectHudOverlayUpdate801CA264Input {
    bool call801CA264 = false;
    uint16_t rightRankActiveRow4E = 0u;
    uint16_t writerControlSample18 = 0u;
    int32_t acceptedTailGateFramesD3058 = 0;
    uint8_t lookaheadDescriptor44SelectorByte0 = 0u;
    uint16_t currentBar1Word04 = 0u;
    uint8_t currentBeat1Byte06 = 0u;
};

struct PrStage1ScorerHostDirectHudOverlayUpdate801CA264Result {
    bool selectorWrite = false;
    uint8_t selector = 0u;
    bool rebuildRequested = false;
    bool ctxFlag400Set = false;
    bool ctxFlag800000Set = false;
    bool cameraWrite = false;
    uint16_t cameraBezHandle = 0u;
    int32_t followUpTimerFrames = 0;
};

uint16_t PrStage1ScorerHostDirectResolveObservedRawSample18(
    bool carrierAvailable,
    uint16_t carrierRawControlSample18,
    uint16_t heldMask);
PrStage1ScorerHostDirectHudOverlayStart801CA1ECResult
PrStage1ScorerHostDirectHudOverlayStart801CA1EC(
    const PrStage1ScorerHostDirectHudOverlayStart801CA1ECInput& input);
PrStage1ScorerHostDirectHudOverlayUpdate801CA264Result
PrStage1ScorerHostDirectHudOverlayUpdate801CA264(
    const PrStage1ScorerHostDirectHudOverlayUpdate801CA264Input& input);
int32_t PrStage1ScorerHostDirectResolveWritePageOrdinal38(
    bool currentWritePageOrdinalKnown,
    uint16_t currentWritePageOrdinal1Based,
    int32_t tick96);

void PrStage1ScorerHostDirectUpdateAcceptedCarrierCore(
    const PrStage1ScorerHostDirectAcceptedCarrierCoreUpdateInput& input,
    PrStage1ScorerHostDirectAcceptedReplayBufferState& replay,
    PrStage1ScorerHostDirectAcceptedScriptedWriterState& scriptedWriter,
    PrStage1ScorerHostDirectAcceptedCarrierCore& carrier);
