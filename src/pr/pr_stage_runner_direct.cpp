#include "pr_stage_runner_direct.h"

namespace {

constexpr uint32_t kFn80026FA4 = 0x80026FA4u;
constexpr uint32_t kFn8001A478 = 0x8001A478u;
constexpr uint32_t kFn8001A654 = 0x8001A654u;
constexpr uint32_t kFn80014344 = 0x80014344u;
constexpr uint32_t kFn80024E98 = 0x80024E98u;
constexpr uint32_t kFn80024FC0 = 0x80024FC0u;
constexpr uint32_t kFn80024E54 = 0x80024E54u;
constexpr uint32_t kFn80094440 = 0x80094440u;
constexpr uint32_t kFn800143F0 = 0x800143F0u;
constexpr uint32_t kFn8001681C = 0x8001681Cu;
constexpr uint32_t kFn80016758 = 0x80016758u;
constexpr uint32_t kFn8001670C = 0x8001670Cu;
constexpr uint32_t kFn800259C0 = 0x800259C0u;
constexpr uint32_t kFn80015350 = 0x80015350u;
constexpr uint32_t kFn80035560 = 0x80035560u;
constexpr uint32_t kFn8001A7F8 = 0x8001A7F8u;
constexpr uint32_t kFn8001A3B8 = 0x8001A3B8u;
constexpr uint32_t kFn80035510 = 0x80035510u;
constexpr uint32_t kFn80024B54 = 0x80024B54u;
constexpr uint32_t kFn80014614 = 0x80014614u;
constexpr uint32_t kFn80094434 = 0x80094434u;
constexpr uint32_t kFn8009443C = 0x8009443Cu;
constexpr uint32_t kFn80014C5C = 0x80014C5Cu;
constexpr uint32_t kFn80026EF8 = 0x80026EF8u;
constexpr uint32_t kFn8001A4A4 = 0x8001A4A4u;
constexpr uint32_t kFn8001A694 = 0x8001A694u;
constexpr uint32_t kFn8001B120 = 0x8001B120u;
constexpr uint32_t kFn800357D4 = 0x800357D4u;
constexpr uint32_t kFn80035838 = 0x80035838u;
constexpr uint32_t kFn800166AC = 0x800166ACu;
constexpr uint32_t kFn80024308 = 0x80024308u;
constexpr uint32_t kFn80024390 = 0x80024390u;
constexpr uint32_t kFn800169E0 = 0x800169E0u;
constexpr uint32_t kFn801C7520 = 0x801C7520u;
constexpr uint32_t kFn801C7338 = 0x801C7338u;
constexpr uint32_t kFn801C75D0 = 0x801C75D0u;
constexpr uint32_t kFn801C76CC = 0x801C76CCu;
constexpr uint32_t kFn801C8D78 = 0x801C8D78u;
constexpr uint32_t kFn801C89B0 = 0x801C89B0u;
constexpr uint32_t kFn801C88C8 = 0x801C88C8u;
constexpr uint32_t kFn801C8828 = 0x801C8828u;
constexpr uint32_t kFn801C895C = 0x801C895Cu;
constexpr uint32_t kFn801C8604 = 0x801C8604u;

constexpr uint32_t kRunnerFlagAcceptedInput = 0x00000001u;
constexpr uint32_t kRunnerFlagTickAdvance = 0x00000008u;
constexpr uint32_t kRunnerFlagFlag40 = 0x00000040u;
constexpr uint32_t kRunnerFlagFlag80 = 0x00000080u;
constexpr uint32_t kRunnerFlagFlag100 = 0x00000100u;
constexpr uint32_t kRunnerFlagFlag200 = 0x00000200u;
constexpr uint32_t kRunnerFlagFlag800 = 0x00000800u;
constexpr uint32_t kRunnerFlagFlag1000 = 0x00001000u;
constexpr uint32_t kRunnerFlagFlag2000 = 0x00002000u;
constexpr uint32_t kRunnerFlagFlag4000 = 0x00004000u;
constexpr uint32_t kRunnerFlagFlag20000 = 0x00020000u;
constexpr uint32_t kRunnerFlagFlag40000 = 0x00040000u;
constexpr uint32_t kRunnerFlagFlag80000 = 0x00080000u;
constexpr uint32_t kRunnerFlagFlag100000 = 0x00100000u;

void PrStageRunnerDirectEmitAction(
    PrStageRunnerDirectActionTrace& trace,
    PrStageRunnerDirectActionKind kind,
    uint32_t psxAddress = 0u,
    bool arg0Known = false,
    int32_t arg0 = 0,
    bool arg1Known = false,
    int32_t arg1 = 0) {
    if (trace.count >= trace.actions.size()) {
        trace.overflow = true;
        return;
    }

    PrStageRunnerDirectAction& action = trace.actions[trace.count++];
    action.kind = kind;
    action.psxAddress = psxAddress;
    action.arg0Known = arg0Known;
    action.arg0 = arg0;
    action.arg1Known = arg1Known;
    action.arg1 = arg1;
}

uint32_t PrStageRunnerDirectNormalizeInputMask7A60(uint32_t rawMask) {
    if ((rawMask & 1u) != 0u) {
        return (rawMask & 0xFFFFFFFAu) | 4u;
    }
    if ((rawMask & 2u) != 0u) {
        return (rawMask & 0xFFFFFFF5u) | 8u;
    }
    return rawMask;
}

uint8_t PrStageRunnerDirectResolveClassToken24B54(uint32_t controlMask18) {
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

void PrStageRunnerDirectRefreshTickFields7A60(
    int32_t tick96,
    uint16_t* bar,
    uint8_t* beat,
    uint8_t* tick) {
    if (bar != nullptr) {
        *bar = (uint16_t)((int16_t)tick96 / 384 + 1);
    }
    if (beat != nullptr) {
        *beat = (uint8_t)((char)((tick96 % 384) / 96) + 1);
    }
    if (tick != nullptr) {
        *tick = (uint8_t)((tick96 % 384) % 96 + 1);
    }
}

bool PrStageRunnerDirectFrameUpdateInputsComplete801C9094(
    const PrStageRunnerDirectFrameUpdate9094Input& input,
    const PrStageRunnerDirectFrameUpdate9094Result& result) {
    if (!input.sameFrameCtxKnown || !input.sameFrameGlobalsKnown) {
        return false;
    }

    if (result.globals.eventStreamFlag != 0u &&
        !input.eventStreamFlagDescriptorValid) {
        return false;
    }
    if (result.globals.eventStreamId != 0u &&
        !input.eventStreamIdDescriptorValid) {
        return false;
    }

    const bool compactRailRequired =
        result.consumedFlagStreamEvent &&
        (result.consumedEventFlags04 &
         (kRunnerFlagFlag800 | kRunnerFlagFlag100000)) != 0u;
    if (compactRailRequired && !input.compactRailRecKnown) {
        return false;
    }

    return true;
}

void PrStageRunnerDirectResetCompactLaneState801C9094(
    PrStageRunnerDirectFrameUpdate9094Result& out) {
    out.ctx.word8A = 0;
    out.ctx.word8E = -1;
    out.ctx.word8C = -1;
    out.ctx.word90 = 0;
    out.ctx.word9E = -1;
    out.ctx.wordA0 = -1;
    out.ctx.wordA2 = 0;
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call80024390,
        kFn80024390);
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call80024308,
        kFn80024308);
}

int32_t PrStageRunnerDirectRoundXaTick801C7560(
    int32_t sceneEntryField348,
    int32_t xaSectorReadValueA7A4,
    int32_t* product348ByReadValue32) {
    const uint32_t product32 =
        (uint32_t)sceneEntryField348 * (uint32_t)xaSectorReadValueA7A4;
    const int32_t signedProduct32 = (int32_t)product32;
    if (product348ByReadValue32 != nullptr) {
        *product348ByReadValue32 = signedProduct32;
    }

    // 801C7560: (double)(field348 * xaRead) / 4500.0 + 0.5,
    // then PSX soft-float converts back to integer before adding field356.
    const double rounded =
        ((double)signedProduct32 / 4500.0) + 0.5;
    return (int32_t)rounded;
}

void PrStageRunnerDirectRefreshTimecodeFields801C7560(
    PrStageRunnerDirectTimecodeState801C7560& state,
    int32_t* callbackReturnValue) {
    const int32_t tick = state.tick801C364C;
    const int32_t tickInBar = tick % 384;
    const int32_t beatBase = tickInBar / 96;

    state.bar801C3644 = (int16_t)tick / 384 + 1;
    state.beat801C3646 = beatBase + 1;
    state.tickInBeat801C3647 = (tickInBar % 96) + 1;

    if (callbackReturnValue != nullptr) {
        *callbackReturnValue = 96 * beatBase;
    }
}

} // namespace

uint32_t PrStageRunnerDirectPackTimecodeWord801C3644(
    const PrStageRunnerDirectTimecodeState801C7560& state) {
    return static_cast<uint16_t>(state.bar801C3644) |
           (static_cast<uint32_t>(
                static_cast<uint8_t>(state.beat801C3646)) << 16) |
           (static_cast<uint32_t>(
                static_cast<uint8_t>(state.tickInBeat801C3647)) << 24);
}

PrStageRunnerDirectTimecodeSnapshot801C7560
PrStageRunnerDirectMakeTimecodeSnapshot801C7560(
    const PrStageRunnerDirectTimecodeState801C7560& state) {
    PrStageRunnerDirectTimecodeSnapshot801C7560 out{};
    out.known = true;
    out.state = state;
    out.word801C3644 =
        PrStageRunnerDirectPackTimecodeWord801C3644(state);
    return out;
}

PrStageRunnerDirectInitResult801C79E4
PrStageRunnerDirectApplySub801C79E4(
    const PrStageRunnerDirectInitInput801C79E4& input) {
    PrStageRunnerDirectInitResult801C79E4 out{};

    out.writeDword801D303CZero = true;
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::StoreDword801D303CZero);

    out.audioResetBarrier26FA4 = true;
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call80026FA4,
        kFn80026FA4);

    out.initUi801C7520 = true;
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call801C7520InitUI,
        kFn801C7520);

    out.setBgm8001A478 = true;
    out.bgmArg = input.stageBgmArgSeg06;
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call8001A478,
        kFn8001A478,
        true,
        input.stageBgmArgSeg06);

    out.setFilter8001A654 = true;
    out.filterChannel = (uint8_t)input.stageXaChannelSeg04;
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call8001A654,
        kFn8001A654,
        true,
        input.stageXaChannelSeg04);

    out.resetScorer80014344 = true;
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call80014344,
        kFn80014344);

    out.resetEventRuntime80024E98 = true;
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call80024E98,
        kFn80024E98);

    out.bindRunnerContext80024FC0 = true;
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call80024FC0,
        kFn80024FC0);

    out.callsResetState801C7338 = true;
    out.returnValue = input.stageRunnerResetStateReturn7338;
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call801C7338ResetState,
        kFn801C7338);
    return out;
}

PrStageRunnerDirectStartupSetup7A60Result
PrStageRunnerDirectResolveStartupSetup7A60(
    const PrStageRunnerDirectStartupSetup7A60Input& input) {
    PrStageRunnerDirectStartupSetup7A60Result out{};
    uint16_t recordsMode916DAAfterSetup = input.recordsMode916DA;

    if (input.transitionState916D0 == 1u) {
        out.copyRecordsMode916DATo801D3040 = true;
        out.dword801D3040Value = input.recordsMode916DA;
        out.clearRecordsMode916DA = true;
        out.call24E54Zero = true;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call80024E54,
            kFn80024E54,
            true,
            0);
        out.writeWord41 = true;
        out.word41Value = 1u;
        out.call94440 = true;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call80094440,
            kFn80094440);
        out.firstCall143F0 = true;
        out.firstCall143F0ArgKnown = true;
        out.firstCall143F0Arg = 0;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call800143F0,
            kFn800143F0,
            true,
            0);
        out.call259C0 = true;
        out.call259C0ArgKnown = true;
        out.call259C0Arg = 0;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call800259C0,
            kFn800259C0,
            true,
            0);
        recordsMode916DAAfterSetup = 0u;
    } else if (input.transitionState916D0 == 2u) {
        out.copyRecordsMode916DATo801D3040 = true;
        out.dword801D3040Value = input.recordsMode916DA;
        out.clearRecordsMode916DA = true;
        out.call24E54Zero = true;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call80024E54,
            kFn80024E54,
            true,
            0);
        out.writeWord41 = true;
        out.word41Value = 1u;
        out.call1681C = true;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call8001681C,
            kFn8001681C);
        out.call16758 = true;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call80016758,
            kFn80016758);
        out.firstCall143F0 = true;
        out.firstCall143F0ArgKnown = input.specialSetupReturn16758Known;
        out.firstCall143F0Arg = input.specialSetupReturn16758;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call800143F0,
            kFn800143F0,
            input.specialSetupReturn16758Known,
            input.specialSetupReturn16758);
        out.call259C0 = true;
        out.call259C0ArgKnown = input.specialSetupReturn16758Known;
        out.call259C0Arg = input.specialSetupReturn16758;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call800259C0,
            kFn800259C0,
            input.specialSetupReturn16758Known,
            input.specialSetupReturn16758);
        recordsMode916DAAfterSetup = 0u;
    } else {
        out.call24E54Zero = true;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call80024E54,
            kFn80024E54,
            true,
            0);
        out.writeWord41 = true;
        out.word41Value = 0u;
        out.call1670C = true;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call8001670C,
            kFn8001670C);
        out.firstCall143F0 = true;
        out.firstCall143F0ArgKnown = input.defaultSetupReturn1670CKnown;
        out.firstCall143F0Arg = input.defaultSetupReturn1670C;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call800143F0,
            kFn800143F0,
            input.defaultSetupReturn1670CKnown,
            input.defaultSetupReturn1670C);
        out.call259C0 = true;
        out.call259C0ArgKnown = input.defaultSetupReturn1670CKnown;
        out.call259C0Arg = input.defaultSetupReturn1670C;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call800259C0,
            kFn800259C0,
            input.defaultSetupReturn1670CKnown,
            input.defaultSetupReturn1670C);
    }

    if (recordsMode916DAAfterSetup == 1u) {
        out.tailCall143F0Zero = true;
        out.tailCall259C0Zero = true;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call800143F0,
            kFn800143F0,
            true,
            0);
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call800259C0,
            kFn800259C0,
            true,
            0);
    }
    return out;
}

PrStageRunnerDirectFinalReturn7A60Result
PrStageRunnerDirectResolveFinalReturn7A60(
    const PrStageRunnerDirectFinalReturn7A60Input& input) {
    PrStageRunnerDirectFinalReturn7A60Result out{};
    out.word59Known = input.word59Known;
    out.word59IsOne = input.word59IsOne;
    out.word60Known = input.word60Known;
    out.word60IsOne = input.word60IsOne;

    if (input.recordsModeDAEqualsOne || input.stageStatus166ACKnown) {
        out.gate78OpenResultKnown = true;
        out.gate78OpenResult =
            (input.recordsModeDAEqualsOne ||
             (input.stageStatus166ACKnown && input.stageStatus166AC < 4u))
                ? 1
                : 2;
    }

    if (!input.word59Known) {
        return out;
    }

    if (input.word59IsOne) {
        out.resultKnown = true;
        out.result = 2;
        return out;
    }

    if (!input.word60Known) {
        out.result = 3;
        return out;
    }

    if (!input.word60IsOne) {
        out.resultKnown = true;
        out.result = 3;
        return out;
    }

    out.calls166AC = true;
    if (input.recordsModeDAEqualsOne) {
        out.recordsModeReturnsOne = true;
        out.resultKnown = true;
        out.result = 1;
        return out;
    }

    out.result = 2;
    if (!input.stageStatus166ACKnown) {
        return out;
    }

    if (input.stageStatus166AC < 4u) {
        out.stageStatusReturnsOne = true;
        out.result = 1;
    }
    out.resultKnown = true;
    return out;
}

PrStageRunnerDirectPreamble7A60Result
PrStageRunnerDirectResolvePreamble7A60(
    const PrStageRunnerDirectPreamble7A60Input& input) {
    PrStageRunnerDirectPreamble7A60Result out{};

    out.writeDword34 =
        input.recordsMode916DA != 0u ? 24 : input.sceneEntryField360HalfSource / 2;
    out.writeWord55 = true;
    out.word55Value = input.word916F6;
    if (input.word916F6 == 1u) {
        out.writeWord54 = true;
        out.word54Value = 1u;
    }

    if (input.recordsMode916DA == 1u) {
        out.tailReset143F0_259C0 = true;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call800143F0,
            kFn800143F0,
            true,
            0);
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call800259C0,
            kFn800259C0,
            true,
            0);
    }

    out.writeDword801D303CZero = true;
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::StoreDword801D303CZero);
    return out;
}

PrStageRunnerDirectInputDispatch7A60Result
PrStageRunnerDirectResolveInputDispatch7A60(
    const PrStageRunnerDirectInputDispatch7A60Input& input) {
    PrStageRunnerDirectInputDispatch7A60Result out{};
    out.replayReadIndex901C0After = input.replayReadIndex901C0;

    if (!input.replayModeWord41) {
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call80035510,
            kFn80035510,
            true,
            1);
        if (!input.livePoll35510Known) {
            return out;
        }

        const uint32_t normalized =
            PrStageRunnerDirectNormalizeInputMask7A60(input.livePoll35510);
        out.writeCtx18 = true;
        out.ctx18Value = normalized;
        if (normalized == 0u) {
            out.writeCtx20 = true;
            out.ctx20Known = true;
            out.ctx20Value = 0;
            out.liveNoInput = true;
            return out;
        }

        out.writeCtx20 = true;
        out.ctx20Known = true;
        out.ctx20Value =
            PrStageRunnerDirectResolveClassToken24B54(normalized);
        out.setCtxFlagAcceptedInput = true;
        out.writeCtx10CurrentTick = true;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call80024B54,
            kFn80024B54,
            true,
            (int32_t)normalized);
        return out;
    }

    const bool replaySlotDue =
        input.replayReadIndex901C0 < input.replayPublishedCount901BC &&
        input.replaySlot.available &&
        input.ctxTick0C >= input.replaySlot.tick96;
    if (!replaySlotDue) {
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call80035510,
            kFn80035510,
            true,
            1);
        if (!input.livePoll35510Known) {
            return out;
        }
        if (input.livePoll35510 == 0u) {
            out.writeCtx18 = true;
            out.ctx18Value = 0u;
            out.waitingForReplaySlot = true;
            return out;
        }

        out.writeCtx18 = true;
        out.ctx18Value = 2048u;
        out.writeCtx20 = true;
        out.ctx20Known = true;
        out.ctx20Value = PrStageRunnerDirectResolveClassToken24B54(2048u);
        out.setCtxFlagAcceptedInput = true;
        out.usedReplayFallbackInput2048 = true;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call80024B54,
            kFn80024B54,
            true,
            2048);
        return out;
    }

    out.incrementReplayReadIndex901C0 = true;
    out.replayReadIndex901C0After = input.replayReadIndex901C0 + 1u;
    out.writeCtx18 = true;
    out.ctx18Value =
        PrStageRunnerDirectNormalizeInputMask7A60(input.replaySlot.rawMask);
    out.writeCtx20 = true;
    out.ctx20Known = true;
    out.ctx20Value = PrStageRunnerDirectResolveClassToken24B54(out.ctx18Value);
    out.setCtxFlagAcceptedInput = true;
    out.writeCtx10ReplayTick = true;
    out.ctx10ReplayTick = input.replaySlot.tick96;
    PrStageRunnerDirectRefreshTickFields7A60(
        input.replaySlot.tick96,
        &out.ctxWord04Bar,
        &out.ctxByte0ABeat,
        &out.ctxByte0BTick);
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call80024B54,
        kFn80024B54,
        true,
        (int32_t)out.ctx18Value);
    return out;
}

PrStageRunnerDirectCtx0FlagsSnapshot801C3640
PrStageRunnerDirectMakeCtx0BaseFlagsSnapshot801C7A60(
    bool acceptedInputHit) {
    PrStageRunnerDirectCtx0FlagsSnapshot801C3640 out{};
    out.baseKnown = true;
    out.baseFlags = 0u;
    if (acceptedInputHit) {
        out.baseFlags |= kRunnerFlagAcceptedInput;
    }
    out.finalFlags = out.baseFlags;
    return out;
}

PrStageRunnerDirectPostFrame7A60Result
PrStageRunnerDirectResolvePostFrame7A60(
    const PrStageRunnerDirectPostFrame7A60Input& input) {
    PrStageRunnerDirectPostFrame7A60Result out{};

    out.acceptedMask9FF = input.ctxInput18 & 0x09FFu;
    out.acceptedGateOpen =
        (input.ctxFlags00 & kRunnerFlagAcceptedInput) != 0u &&
        out.acceptedMask9FF != 0u;
    out.acceptedMaskChanged =
        (input.ctxFlags00 & kRunnerFlagAcceptedInput) != 0u &&
        out.acceptedMask9FF != input.previousInputMask801CCBB8;
    if (out.acceptedMaskChanged) {
        if (!input.replayModeWord41) {
            out.backupCtx08FromCtx04 = true;
            out.backupCtx10FromCtx0C = true;
        }
        out.callAcceptedProducer14614 = true;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call80014614,
            kFn80014614);
        if (input.scorerAccept14614Known &&
            !input.scorerAccept14614ReturnsNonZero) {
            out.callAcceptedProducer94434 = true;
            PrStageRunnerDirectEmitAction(
                out.trace,
                PrStageRunnerDirectActionKind::Call80094434,
                kFn80094434,
                false,
                0,
                true,
                1);
        }
        out.callAcceptedProducer9443C = true;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call8009443C,
            kFn8009443C);

        if (input.fixedBonusGate916FA &&
            (input.ctxInput18 & 0x0100u) != 0u) {
            out.writeCtx3C = true;
            out.ctx3CValue = 1;
            out.fixedBonus250Requested = true;
            PrStageRunnerDirectEmitAction(
                out.trace,
                PrStageRunnerDirectActionKind::Call80014C5C,
                kFn80014C5C,
                true,
                250);
        } else if (input.ctxInput18 == 2048u) {
            out.continueMainLoop = false;
        }
    }

    if ((input.ctxFlags00 & kRunnerFlagFlag200) != 0u) {
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call8001A654,
            kFn8001A654);
    } else if ((input.ctxFlags00 & kRunnerFlagFlag40) != 0u) {
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call80026EF8,
            kFn80026EF8);
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call8001A4A4,
            kFn8001A4A4,
            true,
            1);
    } else if ((input.ctxFlags00 & kRunnerFlagFlag100) != 0u) {
        out.continueMainLoop = false;
    }

    out.writeDword801CCBB8 = true;
    out.dword801CCBB8Value = out.acceptedMask9FF;
    return out;
}

PrStageRunnerDirectMainLoopTail7A60Result
PrStageRunnerDirectResolveMainLoopTail7A60(
    const PrStageRunnerDirectMainLoopTail7A60Input& input) {
    PrStageRunnerDirectMainLoopTail7A60Result out{};

    if (input.postFrameContinues) {
        out.callWait15350 = true;
        out.wait15350Arg =
            2 * input.sceneEntryField352FallbackTickAdvance;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call80015350,
            kFn80015350,
            false,
            0,
            true,
            out.wait15350Arg);

        out.callWait35560 = true;
        out.wait35560Arg = 2;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call80035560,
            kFn80035560,
            true,
            out.wait35560Arg);

        out.callPresentFrame = true;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call801C76CCPresentFrame,
            kFn801C76CC);

        out.callContinuationGate1A7F8 = true;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call8001A7F8,
            kFn8001A7F8);
        if (input.continuationGate1A7F8Known) {
            out.continuationGateKnown = true;
            out.continuationGateAllowsNextFrame =
                !input.continuationGate1A7F8ReturnsOne;
        }

        out.callFrameExit1A3B8 = true;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call8001A3B8,
            kFn8001A3B8);
        if (input.frameExit1A3B8Known) {
            out.frameExitKnown = true;
            out.frameExitTaken = input.frameExit1A3B8ReturnsOne;
            if (out.frameExitTaken) {
                out.callEarlyExit1A694 = true;
                PrStageRunnerDirectEmitAction(
                    out.trace,
                    PrStageRunnerDirectActionKind::Call8001A694,
                    kFn8001A694);
                out.callEarlyExit35838 = true;
                PrStageRunnerDirectEmitAction(
                    out.trace,
                    PrStageRunnerDirectActionKind::Call80035838,
                    kFn80035838);
                out.setWord59OnFrameExit = true;
            }
        }
    }

    if (!input.postFrameContinues) {
        out.cleanupRequiredKnown = true;
        out.cleanupRequired = true;
    } else if (input.frameExit1A3B8Known &&
               input.frameExit1A3B8ReturnsOne) {
        out.cleanupRequiredKnown = true;
        out.cleanupRequired = true;
    } else if (input.continuationGate1A7F8Known &&
               input.continuationGate1A7F8ReturnsOne) {
        out.cleanupRequiredKnown = true;
        out.cleanupRequired = true;
    } else if (input.continuationGate1A7F8Known &&
               input.frameExit1A3B8Known) {
        out.cleanupRequiredKnown = true;
        out.cleanupRequired = false;
    }

    if (!out.cleanupRequiredKnown || !out.cleanupRequired) {
        return out;
    }

    out.clearWord50 = true;
    out.clearWord42 = true;
    out.clearWord61 = true;
    out.cleanupDrainFrameCount = 4u;
    for (uint8_t i = 0u; i < out.cleanupDrainFrameCount; ++i) {
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call801C75D0RenderFrame,
            kFn801C75D0,
            false,
            0,
            true,
            7);
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call80035560,
            kFn80035560,
            true,
            2);
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call801C76CCPresentFrame,
            kFn801C76CC);
    }

    out.finalCall1B120 = true;
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call8001B120,
        kFn8001B120,
        true,
        1);
    out.finalCall26FA4 = true;
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call80026FA4,
        kFn80026FA4);
    out.finalCall357D4Zero = true;
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call800357D4,
        kFn800357D4,
        true,
        0);
    out.finalCall1A694 = true;
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call8001A694,
        kFn8001A694);
    return out;
}

PrStageRunnerDirectTimecodeResult801C7560
PrStageRunnerDirectUpdateTimecode801C7560(
    const PrStageRunnerDirectTimecodeInput801C7560& input) {
    PrStageRunnerDirectTimecodeResult801C7560 out{};
    out.state = input.state;
    out.previousXaReadValue801D303C =
        input.state.dword801D303CPreviousXaReadValue;
    out.xaSectorReadValueA7A4 = input.xaSectorReadValueA7A4;
    out.sceneEntryField348 = input.sceneEntryField348;
    out.sceneEntryField352FallbackTickAdvance =
        input.sceneEntryField352FallbackTickAdvance;
    out.sceneEntryField356TickOffset = input.sceneEntryField356TickOffset;

    if (input.state.dword801D303CPreviousXaReadValue >=
        input.xaSectorReadValueA7A4) {
        out.usedFallbackField352Increment = true;
        out.state.tick801C364C +=
            input.sceneEntryField352FallbackTickAdvance;
    } else {
        out.xaRoundedTickBeforeOffset =
            PrStageRunnerDirectRoundXaTick801C7560(
                input.sceneEntryField348,
                input.xaSectorReadValueA7A4,
                &out.xaProduct348ByReadValue32);
        out.state.tick801C364C =
            out.xaRoundedTickBeforeOffset +
            input.sceneEntryField356TickOffset;
    }

    out.state.dword801D303CPreviousXaReadValue =
        input.xaSectorReadValueA7A4;
    PrStageRunnerDirectRefreshTimecodeFields801C7560(
        out.state,
        &out.callbackReturnValue);
    return out;
}

PrStageRunnerDirectCtx0FinalFlagsSnapshot801C9094
PrStageRunnerDirectMakeCtx0FinalFlagsSnapshot801C9094(
    const PrStageRunnerDirectFrameUpdate9094Result& result) {
    PrStageRunnerDirectCtx0FinalFlagsSnapshot801C9094 out{};
    out.finalKnown = result.ctx0FinalInputsComplete801C9094;
    out.finalFlags = result.ctx.flags00;
    out.sourceFrameUpdate801C9094 = true;
    out.returnEarlyAfterFlag40Reset = result.returnEarlyAfterFlag40Reset;
    out.consumedFlagStreamEvent = result.consumedFlagStreamEvent;
    out.consumedIdStreamEvent = result.consumedIdStreamEvent;
    return out;
}

PrStageRunnerDirectFrameUpdate9094Result
PrStageRunnerDirectFrameUpdate801C9094(
    const PrStageRunnerDirectFrameUpdate9094Input& input) {
    PrStageRunnerDirectFrameUpdate9094Result out{};
    out.ctx = input.ctx;
    out.globals = input.globals;

    if (out.globals.word8008ECF8 > 0) {
        --out.globals.word8008ECF8;
        if (out.globals.word8008ECF8 == 0) {
            // 801C9094 clears unk_8008ECE0 and ctx[66]; the storage
            // owner is outside this helper, so only the action boundary is kept.
        }
    }
    if (out.globals.word8008ECFA > 0) {
        --out.globals.word8008ECFA;
        if (out.globals.word8008ECFA == 0) {
            // 801C9094 clears unk_8008ECE4 and ctx[67].
        }
    }
    if (out.globals.word8008ECFC > 0) {
        --out.globals.word8008ECFC;
        if (out.globals.word8008ECFC == 0 &&
            out.globals.eventStreamFlag != 0u) {
            PrStageRunnerDirectEmitAction(
                out.trace,
                PrStageRunnerDirectActionKind::Call801C88C8HudSetRes10000,
                kFn801C88C8);
        }
    }
    if (out.globals.unk8008ED00 == 5u &&
        out.globals.word8008ECFE > 0) {
        --out.globals.word8008ECFE;
        if (out.globals.word8008ECFE == 0) {
            PrStageRunnerDirectEmitAction(
                out.trace,
                PrStageRunnerDirectActionKind::Call801C8828HudSetRes20000,
                kFn801C8828);
            out.globals.word8008ECFE = 33;
        }
    }

    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call801C89B0HudTimedListTickSlot,
        kFn801C89B0,
        true,
        0);
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call801C89B0HudTimedListTickSlot,
        kFn801C89B0,
        true,
        1);
    PrStageRunnerDirectEmitAction(
        out.trace,
        PrStageRunnerDirectActionKind::Call801C89B0HudTimedListTickSlot,
        kFn801C89B0,
        true,
        2);

    if (out.globals.eventStreamDone) {
        out.globals.eventStreamDone = false;
        if (out.ctx.word4E != 0u) {
            out.setCtxFlag40000 = true;
            out.setCtxFlag80000 = true;
            out.ctx.flags00 |= kRunnerFlagFlag40000 | kRunnerFlagFlag80000;
        } else {
            out.setCtxFlag40000 = true;
            out.ctx.flags00 |= kRunnerFlagFlag40000;
        }
    }

    if (out.globals.dword801D3054 > 0) {
        --out.globals.dword801D3054;
    }
    if (out.globals.dword801D3058 > 0) {
        --out.globals.dword801D3058;
    }

    if ((out.ctx.flags00 & kRunnerFlagFlag40) != 0u) {
        out.globals.eventStreamFlag = 0u;
        out.globals.eventStreamId = out.ctx.word4E != 0u ? 4u : 5u;
        out.globals.unk8008ED20 = 1u;
        out.globals.word8008ECFC = 0;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call801C8D78EventStreamReset,
            kFn801C8D78,
            true,
            out.globals.eventStreamId);
        PrStageRunnerDirectResetCompactLaneState801C9094(out);
        out.returnEarlyAfterFlag40Reset = true;
        out.ctx0FinalInputsComplete801C9094 =
            PrStageRunnerDirectFrameUpdateInputsComplete801C9094(input, out);
        out.ctx0FinalFlagsSnapshot801C9094 =
            PrStageRunnerDirectMakeCtx0FinalFlagsSnapshot801C9094(out);
        return out;
    }

    if (out.globals.unk8008ED1C) {
        out.globals.unk8008ED1C = false;
        out.globals.eventStreamFlag = 0u;
        out.ctx.word78 = 1u;
        if (out.ctx.word4E != 0u) {
            if (out.ctx.word4E == 1u) {
                out.globals.eventStreamId = 2u;
            } else {
                out.globals.eventStreamId = 4u;
                out.ctx.word76 = 1u;
                out.ctx.flags00 |= kRunnerFlagFlag40;
            }
        } else {
            out.globals.eventStreamId = 3u;
        }
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call801C8D78EventStreamReset,
            kFn801C8D78,
            true,
            out.globals.eventStreamId);
        PrStageRunnerDirectResetCompactLaneState801C9094(out);
    } else if ((out.ctx.flags00 & kRunnerFlagFlag2000) != 0u) {
        out.globals.eventStreamId = out.ctx.word72SceneFamily < 2u ? 6u : 8u;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call801C8D78EventStreamReset,
            kFn801C8D78,
            true,
            out.globals.eventStreamId);
        PrStageRunnerDirectResetCompactLaneState801C9094(out);
    } else if ((out.ctx.flags00 & kRunnerFlagFlag4000) != 0u) {
        out.globals.eventStreamId = 7u;
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call801C8D78EventStreamReset,
            kFn801C8D78,
            true,
            7);
    }

    if ((out.ctx.flags00 & kRunnerFlagFlag200) != 0u) {
        uint8_t resourceA = 0u;
        uint8_t resourceB = 0u;
        switch (out.ctx.word72Rating) {
            case 1u:
                resourceA = 74u;
                resourceB = 126u;
                break;
            case 2u:
                resourceA = 75u;
                resourceB = 127u;
                break;
            case 3u:
                resourceA = 77u;
                resourceB = 129u;
                break;
            case 4u:
                resourceA = 76u;
                resourceB = 128u;
                break;
            default:
                break;
        }
        if (resourceA != 0u) {
            out.globals.dword801D3054 = 41;
            if (resourceB != 0u) {
                out.ctx.flags00 |= kRunnerFlagFlag20000;
                out.setCtxFlag20000 = true;
                out.globals.word8008ECFE = 33;
            }
        }
    }

    if (out.globals.eventStreamFlag != 0u &&
        input.eventStreamFlagDescriptorValid &&
        input.activeFlagStream.valid &&
        input.activeFlagStream.index < input.activeFlagStream.count &&
        input.activeFlagStream.dueEventKnown &&
        out.ctx.tick0C >= input.activeFlagStream.dueFrame) {
        out.consumedFlagStreamEvent = true;
        out.consumedEventFlags04 = input.activeFlagStream.flags04;
        const uint32_t flags04 = input.activeFlagStream.flags04;
        if ((flags04 & kRunnerFlagFlag800) != 0u) {
            out.ctx.flags00 |= kRunnerFlagFlag800;
            out.setCtxFlag0800 = true;
            if (input.compactRailRecKnown) {
                if (input.compactRailRecSignedMode <= 0) {
                    out.ctx.word8A =
                        (uint16_t)(-input.compactRailRecSignedMode);
                    out.ctx.word8C = -1;
                    out.ctx.word8E = -1;
                    out.globals.dword801D304C =
                        input.compactRailRecPrimaryStreamPtr;
                    out.globals.dword801D3050 =
                        input.compactRailRecSecondaryStreamPtr;
                } else {
                    out.ctx.word8A =
                        (uint16_t)input.compactRailRecSignedMode;
                    out.ctx.word8C = input.compactRailRecStartIndex;
                    out.ctx.word8E = -1;
                    out.ctx.word90 = 1u;
                    out.globals.dword801D304C =
                        input.compactRailRecPrimaryStreamPtr;
                    out.globals.dword801D3050 =
                        input.compactRailRecSecondaryStreamPtr;
                }
            }
        } else if ((flags04 & kRunnerFlagFlag1000) != 0u ||
                   (out.ctx.flags00 & kRunnerFlagFlag2000) != 0u) {
            out.resetCompactRailLanes = true;
            PrStageRunnerDirectResetCompactLaneState801C9094(out);
        }

        if ((flags04 & kRunnerFlagFlag100000) != 0u) {
            out.ctx.wordA2 = 1u;
            out.ctx.word9E = 0;
            out.ctx.wordA0 = -1;
            out.ctx.flags00 |= kRunnerFlagFlag100000;
            out.setCtxFlag100000 = true;
            if (input.compactRailRecKnown &&
                input.compactRailRecSignedMode < 0) {
                PrStageRunnerDirectEmitAction(
                    out.trace,
                    PrStageRunnerDirectActionKind::Call80024390,
                    kFn80024390);
            }
        }

        if ((flags04 & kRunnerFlagFlag80) != 0u) {
            out.globals.unk8008ED1C = true;
            out.ctx.word54 = 1u;
            PrStageRunnerDirectEmitAction(
                out.trace,
                PrStageRunnerDirectActionKind::Call800169E0,
                kFn800169E0);
        }
    }

    if (out.globals.eventStreamId != 0u) {
        if (input.eventStreamIdDescriptorValid &&
            input.activeIdStream.valid &&
            input.activeIdStream.index < input.activeIdStream.count &&
            input.activeIdStream.dueEventKnown &&
            out.ctx.tick0C >= out.globals.dword801D3048 +
                                input.activeIdStream.dueFrame) {
            out.consumedIdStreamEvent = true;
            PrStageRunnerDirectEmitAction(
                out.trace,
                PrStageRunnerDirectActionKind::Call801C895CApplyScriptEvent,
                kFn801C895C,
                true,
                out.globals.eventStreamId);
            PrStageRunnerDirectEmitAction(
                out.trace,
                PrStageRunnerDirectActionKind::
                    Call801C8604ApplyScriptEventSetText,
                kFn801C8604,
                true,
                out.globals.eventStreamId);
        } else if (input.eventStreamIdDescriptorValid &&
                   input.activeIdStream.valid &&
                   input.activeIdStream.index >= input.activeIdStream.count) {
            if (out.globals.eventStreamId == 6u ||
                out.globals.eventStreamId == 8u) {
                out.globals.eventStreamDone = true;
                out.eventStreamDoneRaised = true;
            } else if ((out.globals.eventStreamId >= 2u &&
                        out.globals.eventStreamId <= 5u)) {
                out.ctx.flags00 |= kRunnerFlagFlag100;
                out.setCtxFlag0100 = true;
            }
            out.resolvedEventStreamId = (uint8_t)out.globals.eventStreamId;
            out.globals.eventStreamId = 0u;
        }
    } else if (out.globals.eventStreamFlag != 0u &&
               out.consumedFlagStreamEvent) {
        PrStageRunnerDirectEmitAction(
            out.trace,
            PrStageRunnerDirectActionKind::Call801C895CApplyScriptEvent,
            kFn801C895C,
            true,
            out.globals.eventStreamFlag);
        if (out.ctx.word4E != 0u) {
            PrStageRunnerDirectEmitAction(
                out.trace,
                PrStageRunnerDirectActionKind::
                    Call801C8604ApplyScriptEventSetText,
                kFn801C8604,
                true,
                out.globals.eventStreamFlag);
        }
    }

    out.ctx0FinalInputsComplete801C9094 =
        PrStageRunnerDirectFrameUpdateInputsComplete801C9094(input, out);
    out.ctx0FinalFlagsSnapshot801C9094 =
        PrStageRunnerDirectMakeCtx0FinalFlagsSnapshot801C9094(out);
    return out;
}
