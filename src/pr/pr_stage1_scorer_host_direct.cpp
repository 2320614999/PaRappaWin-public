#include "pr_stage1_scorer_host_direct.h"

#include "pr_pad.h"
#include "pr_stage1_scorer_direct.h"

namespace {

constexpr uint16_t kStage1GameplayAcceptedInputMask =
    (uint16_t)PrPadButton::Left |
    (uint16_t)PrPadButton::Right |
    (uint16_t)PrPadButton::Triangle |
    (uint16_t)PrPadButton::Circle |
    (uint16_t)PrPadButton::Cross |
    (uint16_t)PrPadButton::Square;
constexpr uint16_t kStage1AcceptedProducerControlPassthroughMask18 = 0x0900u;

static uint16_t NormalizeStage1AcceptedInputMask(uint16_t rawMask) {
    const uint16_t filtered =
        rawMask & (kStage1GameplayAcceptedInputMask |
                   (uint16_t)PrPadButton::Up |
                   (uint16_t)PrPadButton::Down);
    if (filtered == 0u || (filtered & (filtered - 1u)) != 0u) {
        return 0u;
    }
    if (filtered == (uint16_t)PrPadButton::Up) {
        return (uint16_t)PrPadButton::Left;
    }
    if (filtered == (uint16_t)PrPadButton::Down) {
        return (uint16_t)PrPadButton::Right;
    }
    return filtered;
}

static uint8_t ResolveStage1ClassToken20_24B54(uint32_t controlMask18);

static uint8_t ResolveStage1AcceptedProducerClassToken20(uint16_t controlMask18) {
    return ResolveStage1ClassToken20_24B54(controlMask18);
}

static uint16_t ResolveStage1WriterRawControlSample18(uint16_t rawMask) {
    uint16_t writerMask18 = rawMask;
    if ((writerMask18 & (uint16_t)PrPadButton::Up) != 0u) {
        writerMask18 =
            (uint16_t)((writerMask18 & ~(uint16_t)PrPadButton::Up) |
                       (uint16_t)PrPadButton::Left);
    } else if ((writerMask18 & (uint16_t)PrPadButton::Down) != 0u) {
        writerMask18 =
            (uint16_t)((writerMask18 & ~(uint16_t)PrPadButton::Down) |
                       (uint16_t)PrPadButton::Right);
    }
    return writerMask18;
}

static uint8_t ResolveStage1ClassToken20_24B54(uint32_t controlMask18) {
    const uint8_t a1 = static_cast<uint8_t>(controlMask18 & 0xFFu);
    int result = 1;
    if ((a1 & 0x10u) == 0u) {
        result = 2;
        if ((a1 & 0x20u) == 0u) {
            result = 3;
            if ((a1 & 0x40u) == 0u) {
                result = 4;
                if ((a1 & 0x80u) == 0u) {
                    result = 5;
                    if ((a1 & 0x04u) == 0u) {
                        result = 5;
                        if ((a1 & 0x01u) == 0u) {
                            result = 7;
                            if ((a1 & 0x08u) == 0u) {
                                return static_cast<uint8_t>(
                                    (a1 & 0x02u) != 0u ? 7u : 0u);
                            }
                        }
                    }
                }
            }
        }
    }
    return static_cast<uint8_t>(result);
}

struct Stage1AcceptedProducerControlWriterResolvedSample {
    PrStage1ScorerHostDirectAcceptedControlWriterSourceKind sourceKind =
        PrStage1ScorerHostDirectAcceptedControlWriterSourceKind::None;
    uint16_t rawControlSample18 = 0;
    uint16_t normalizedInputMask18 = 0;
    uint16_t controlMask18 = 0;
    uint16_t passthroughBits18 = 0;
    bool updatesClassToken20 = false;
    uint8_t classToken20 = 0;
};

static bool TryResolveStage1AcceptedProducerScriptedWriterSample(
    PrStage1ScorerHostDirectAcceptedReplayBufferState& sharedBuffer,
    bool replayMode52,
    int32_t tick96,
    PrStage1ScorerHostDirectAcceptedScriptedWriterState& runtime,
    Stage1AcceptedProducerControlWriterResolvedSample& out) {
    out = Stage1AcceptedProducerControlWriterResolvedSample{};
    if (!replayMode52 || tick96 < 0) {
        runtime = PrStage1ScorerHostDirectAcceptedScriptedWriterState{};
        return false;
    }

    PrStage1ScorerHostDirectAcceptedScriptedWriterState nextRuntime = runtime;
    nextRuntime.active = false;
    nextRuntime.dueTick96EEF8 = 0u;
    nextRuntime.classMaskEEFC = 0u;
    if (sharedBuffer.writeCount901C0 > sharedBuffer.publishedCount901BC) {
        sharedBuffer.writeCount901C0 = sharedBuffer.publishedCount901BC;
    }
    if (sharedBuffer.writeCount901C0 >= sharedBuffer.publishedCount901BC ||
        sharedBuffer.writeCount901C0 >= kPrStage1ScorerHostDirectReplayBufferCapacity) {
        runtime = nextRuntime;
        return false;
    }

    const uint32_t slot901C0 = sharedBuffer.writeCount901C0;
    nextRuntime.dueTick96EEF8 = sharedBuffer.tick96EEF8[(size_t)slot901C0];
    nextRuntime.classMaskEEFC = sharedBuffer.classMaskEEFC[(size_t)slot901C0];
    if ((uint32_t)tick96 < nextRuntime.dueTick96EEF8 ||
        nextRuntime.classMaskEEFC == 0u) {
        runtime = nextRuntime;
        return false;
    }

    nextRuntime.active = true;
    sharedBuffer.writeCount901C0 = slot901C0 + 1u;
    runtime = nextRuntime;
    out.sourceKind =
        PrStage1ScorerHostDirectAcceptedControlWriterSourceKind::ScriptedSharedBufferSample;
    out.rawControlSample18 = (uint16_t)runtime.classMaskEEFC;
    out.normalizedInputMask18 = NormalizeStage1AcceptedInputMask(out.rawControlSample18);
    out.passthroughBits18 =
        out.rawControlSample18 & kStage1AcceptedProducerControlPassthroughMask18;
    out.controlMask18 = out.rawControlSample18;
    out.classToken20 = ResolveStage1AcceptedProducerClassToken20(out.controlMask18);
    out.updatesClassToken20 = true;
    return true;
}

static Stage1AcceptedProducerControlWriterResolvedSample
ResolveStage1AcceptedProducerControlWriterResolvedSample(
    bool replayMode52,
    int32_t tick96,
    uint16_t rawMask,
    PrStage1ScorerHostDirectAcceptedReplayBufferState& replay,
    PrStage1ScorerHostDirectAcceptedScriptedWriterState& scriptedWriter) {
    Stage1AcceptedProducerControlWriterResolvedSample out{};
    if (TryResolveStage1AcceptedProducerScriptedWriterSample(
            replay,
            replayMode52,
            tick96,
            scriptedWriter,
            out)) {
        return out;
    }

    if (replayMode52) {
        out.rawControlSample18 = (rawMask == 0u) ? 0u : 0x0800u;
        out.normalizedInputMask18 = NormalizeStage1AcceptedInputMask(out.rawControlSample18);
        out.passthroughBits18 =
            out.rawControlSample18 & kStage1AcceptedProducerControlPassthroughMask18;
        out.controlMask18 = out.rawControlSample18;
        out.classToken20 = ResolveStage1AcceptedProducerClassToken20(out.controlMask18);
        out.updatesClassToken20 = (rawMask != 0u);
        out.sourceKind =
            (rawMask == 0u)
                ? PrStage1ScorerHostDirectAcceptedControlWriterSourceKind::
                      LocalReplayPreserveClear
                : PrStage1ScorerHostDirectAcceptedControlWriterSourceKind::
                      LocalControlSample800;
        return out;
    }

    out.rawControlSample18 =
        ResolveStage1WriterRawControlSample18(rawMask);
    out.normalizedInputMask18 = NormalizeStage1AcceptedInputMask(out.rawControlSample18);
    out.passthroughBits18 =
        out.rawControlSample18 & kStage1AcceptedProducerControlPassthroughMask18;
    out.controlMask18 = out.rawControlSample18;
    out.classToken20 = ResolveStage1AcceptedProducerClassToken20(out.controlMask18);
    out.updatesClassToken20 = true;

    if (out.rawControlSample18 == 0u) {
        out.sourceKind =
            PrStage1ScorerHostDirectAcceptedControlWriterSourceKind::LocalZeroClear;
    } else if (out.rawControlSample18 == 0x0800u &&
               out.normalizedInputMask18 == 0u &&
               out.passthroughBits18 == 0x0800u) {
        out.sourceKind =
            PrStage1ScorerHostDirectAcceptedControlWriterSourceKind::LocalControlSample800;
    } else if (out.passthroughBits18 != 0u) {
        out.sourceKind =
            PrStage1ScorerHostDirectAcceptedControlWriterSourceKind::LocalPassthroughControl;
    } else {
        out.sourceKind =
            PrStage1ScorerHostDirectAcceptedControlWriterSourceKind::LocalLiveNormalized;
    }
    return out;
}

}  // namespace

uint16_t PrStage1ScorerHostDirectResolveObservedRawSample18(
    bool carrierAvailable,
    uint16_t carrierRawControlSample18,
    uint16_t heldMask) {
    if (carrierAvailable) {
        return carrierRawControlSample18;
    }
    return ResolveStage1WriterRawControlSample18(heldMask);
}

PrStage1ScorerHostDirectHudOverlayStart801CA1ECResult
PrStage1ScorerHostDirectHudOverlayStart801CA1EC(
    const PrStage1ScorerHostDirectHudOverlayStart801CA1ECInput& input) {
    PrStage1ScorerHostDirectHudOverlayStart801CA1ECResult out{};

    if (!input.call801CA1EC || input.rightRankActiveRow4E == 0u) {
        return out;
    }

    const uint32_t lookaheadMask =
        input.lookaheadDescriptor44Available ? input.lookaheadDescriptor44Mask8 : 0u;
    const uint32_t currentMask =
        input.currentDescriptor40Available ? input.currentDescriptor40Mask8 : 0u;
    if ((input.writerControlSample18 & lookaheadMask) == 0u &&
        (input.writerControlSample18 & currentMask) == 0u) {
        out.selectorWrite = true;
        out.selector = 1u;
        out.rebuildRequested = true;
        out.ctxFlag800000Set = true;
    }
    return out;
}

PrStage1ScorerHostDirectHudOverlayUpdate801CA264Result
PrStage1ScorerHostDirectHudOverlayUpdate801CA264(
    const PrStage1ScorerHostDirectHudOverlayUpdate801CA264Input& input) {
    static constexpr uint16_t kStage1FollowUpCameraShjBezHandle = 213u;
    static constexpr uint16_t kStage1FollowUpCameraTenBezHandle = 214u;
    PrStage1ScorerHostDirectHudOverlayUpdate801CA264Result out{};

    if (!input.call801CA264) {
        return out;
    }

    const bool mainGateActive =
        (input.writerControlSample18 & 0x1000u) != 0u &&
        input.lookaheadDescriptor44SelectorByte0 != 0u;
    if (!mainGateActive || input.acceptedTailGateFramesD3058 > 0 ||
        input.rightRankActiveRow4E == 0u) {
        return out;
    }

    out.selectorWrite = true;
    out.selector = 0u;
    out.rebuildRequested = false;
    if (input.currentBeat1Byte06 == 4u &&
        (input.currentBar1Word04 == 37u || input.currentBar1Word04 == 42u)) {
        out.selector = 2u;
        out.rebuildRequested = true;
        out.ctxFlag400Set = true;
        out.ctxFlag800000Set = true;
        out.cameraWrite = true;
        out.cameraBezHandle = kStage1FollowUpCameraTenBezHandle;
        out.followUpTimerFrames = 132;
    } else if (input.currentBeat1Byte06 == 2u &&
               (input.writerControlSample18 & 0x000Au) != 0u) {
        out.selector = 3u;
        out.rebuildRequested = true;
        out.ctxFlag400Set = true;
        out.ctxFlag800000Set = true;
        out.cameraWrite = true;
        out.cameraBezHandle = kStage1FollowUpCameraShjBezHandle;
        out.followUpTimerFrames = 66;
    } else if (input.currentBeat1Byte06 == 4u &&
               (input.writerControlSample18 & 0x0040u) != 0u) {
        out.selector = 4u;
        out.rebuildRequested = true;
        out.ctxFlag800000Set = true;
        out.followUpTimerFrames = 132;
    }
    return out;
}

int32_t PrStage1ScorerHostDirectResolveWritePageOrdinal38(
    bool currentWritePageOrdinalKnown,
    uint16_t currentWritePageOrdinal1Based,
    int32_t tick96) {
    if (currentWritePageOrdinalKnown) {
        return (int32_t)currentWritePageOrdinal1Based;
    }
    return (int32_t)PrStage1ScorerDirectResolvePageOrdinal56_24FD0(tick96);
}

void PrStage1ScorerHostDirectUpdateAcceptedCarrierCore(
    const PrStage1ScorerHostDirectAcceptedCarrierCoreUpdateInput& input,
    PrStage1ScorerHostDirectAcceptedReplayBufferState& replay,
    PrStage1ScorerHostDirectAcceptedScriptedWriterState& scriptedWriter,
    PrStage1ScorerHostDirectAcceptedCarrierCore& carrier) {
    const uint8_t previousClassToken20 = carrier.classToken20;
    const bool previousAcceptedTick96Known = carrier.acceptedTick96Known;
    const int32_t previousAcceptedTick96 = carrier.acceptedTick96;
    const uint32_t previousAcceptedTick96LastUpdateQueryFrame =
        carrier.acceptedTick96LastUpdateQueryFrame;
    const uint8_t previousAcceptedTick96LastUpdateSourceKind =
        carrier.acceptedTick96LastUpdateSourceKind;
    const uint16_t previousAcceptedTick96LastUpdateControlMask18 =
        carrier.acceptedTick96LastUpdateControlMask18;
    const uint8_t previousAcceptedTick96LastUpdateClassToken20 =
        carrier.acceptedTick96LastUpdateClassToken20;
    const bool previousAcceptedTick96LastUpdateViaPostCtx10 =
        carrier.acceptedTick96LastUpdateViaPostCtx10;
    const uint32_t previousAcceptedTick96LastUpdateCtxInput18 =
        carrier.acceptedTick96LastUpdateCtxInput18;
    const uint32_t previousAcceptedTick96LastUpdatePreviousInputMask801CCBB8 =
        carrier.acceptedTick96LastUpdatePreviousInputMask801CCBB8;
    const bool previousAcceptedTick96LastUpdateAcceptedMaskChanged =
        carrier.acceptedTick96LastUpdateAcceptedMaskChanged;

    const bool replayMode52 = input.replayMode52;
    Stage1AcceptedProducerControlWriterResolvedSample writerSample{};
    if (input.inputDispatchConsumedReplaySlot) {
        scriptedWriter.active = true;
        scriptedWriter.dueTick96EEF8 =
            input.inputDispatchReplayTickKnown
                ? static_cast<uint32_t>(input.inputDispatchReplayTick96)
                : 0u;
        scriptedWriter.classMaskEEFC = input.rawControlMask18;
        writerSample.sourceKind =
            PrStage1ScorerHostDirectAcceptedControlWriterSourceKind::
                ScriptedSharedBufferSample;
        writerSample.rawControlSample18 = input.rawControlMask18;
        writerSample.normalizedInputMask18 =
            NormalizeStage1AcceptedInputMask(writerSample.rawControlSample18);
        writerSample.passthroughBits18 =
            writerSample.rawControlSample18 &
            kStage1AcceptedProducerControlPassthroughMask18;
        writerSample.controlMask18 = writerSample.rawControlSample18;
        writerSample.classToken20 =
            input.writerClassToken20Known
                ? input.writerClassToken20
                : ResolveStage1AcceptedProducerClassToken20(
                      writerSample.controlMask18);
        writerSample.updatesClassToken20 = true;
    } else {
        writerSample = ResolveStage1AcceptedProducerControlWriterResolvedSample(
            replayMode52,
            input.tick96,
            input.rawControlMask18,
            replay,
            scriptedWriter);
    }

    carrier = PrStage1ScorerHostDirectAcceptedCarrierCore{};
    carrier.available = true;
    carrier.controlWriterSourceKind = (uint8_t)writerSample.sourceKind;
    carrier.rawControlSample18 = writerSample.rawControlSample18;
    carrier.controlMask18 = writerSample.controlMask18;
    const uint8_t resolvedClassToken20 =
        input.writerClassToken20Known ? input.writerClassToken20
                                      : writerSample.classToken20;
    carrier.classToken20 =
        writerSample.updatesClassToken20 ? resolvedClassToken20
                                         : previousClassToken20;
    carrier.halfWindow34 = input.halfWindow34;
    carrier.substate50 = input.descriptorSubstate50;

    const bool updatesAcceptedTick96FromCurrentTick =
        input.inputDispatchWriteCurrentTick && input.tick96 >= 0;
    const bool updatesAcceptedTick96 =
        updatesAcceptedTick96FromCurrentTick ||
        writerSample.sourceKind ==
        PrStage1ScorerHostDirectAcceptedControlWriterSourceKind::
            ScriptedSharedBufferSample;
    if (updatesAcceptedTick96) {
        carrier.acceptedTick96Known = true;
        carrier.acceptedTick96 = updatesAcceptedTick96FromCurrentTick
                                     ? input.tick96
                                     : (input.inputDispatchConsumedReplaySlot &&
                                                input.inputDispatchReplayTickKnown
                                            ? input.inputDispatchReplayTick96
                                            : (int32_t)scriptedWriter
                                                  .dueTick96EEF8);
        carrier.acceptedTick96LastUpdateQueryFrame = input.queryFrame30;
        carrier.acceptedTick96LastUpdateSourceKind =
            (uint8_t)writerSample.sourceKind;
        carrier.acceptedTick96LastUpdateControlMask18 = writerSample.controlMask18;
        carrier.acceptedTick96LastUpdateClassToken20 = resolvedClassToken20;
        carrier.acceptedTick96LastUpdateViaPostCtx10 = false;
        carrier.acceptedTick96LastUpdateCtxInput18 = 0u;
        carrier.acceptedTick96LastUpdatePreviousInputMask801CCBB8 = 0u;
        carrier.acceptedTick96LastUpdateAcceptedMaskChanged = false;
    } else if (previousAcceptedTick96Known) {
        carrier.acceptedTick96Known = true;
        carrier.acceptedTick96 = previousAcceptedTick96;
        carrier.acceptedTick96LastUpdateQueryFrame =
            previousAcceptedTick96LastUpdateQueryFrame;
        carrier.acceptedTick96LastUpdateSourceKind =
            previousAcceptedTick96LastUpdateSourceKind;
        carrier.acceptedTick96LastUpdateControlMask18 =
            previousAcceptedTick96LastUpdateControlMask18;
        carrier.acceptedTick96LastUpdateClassToken20 =
            previousAcceptedTick96LastUpdateClassToken20;
        carrier.acceptedTick96LastUpdateViaPostCtx10 =
            previousAcceptedTick96LastUpdateViaPostCtx10;
        carrier.acceptedTick96LastUpdateCtxInput18 =
            previousAcceptedTick96LastUpdateCtxInput18;
        carrier.acceptedTick96LastUpdatePreviousInputMask801CCBB8 =
            previousAcceptedTick96LastUpdatePreviousInputMask801CCBB8;
        carrier.acceptedTick96LastUpdateAcceptedMaskChanged =
            previousAcceptedTick96LastUpdateAcceptedMaskChanged;
    }

    const int32_t cadenceTick96 =
        carrier.acceptedTick96Known ? carrier.acceptedTick96 : input.tick96;
    const uint16_t phase384 = (cadenceTick96 < 0)
                                  ? 0u
                                  : (uint16_t)(((uint32_t)cadenceTick96 +
                                                (uint32_t)carrier.halfWindow34) %
                                               384u);
    carrier.phase384 = phase384;
    carrier.writePageOrdinal38 = input.writePageOrdinal38;
    carrier.recordSlot24 = (uint8_t)(phase384 / 24u);
    carrier.recordRemainder24 = (uint8_t)(phase384 % 24u);
    carrier.timingTemplateSlot48 = (uint8_t)(phase384 / 48u);
    carrier.recordedSplit =
        carrier.recordRemainder24 <=
        (uint8_t)(2u * (uint32_t)carrier.halfWindow34);
}
