#include "pr_stage1_loader_spu_hal.h"

namespace PrStage1LoaderSpuHal {

namespace {

Action MakeWrapperAction(ActionKind kind,
                         uint32_t wrapperFunction,
                         const State& before) {
    Action action{};
    action.kind = kind;
    action.wrapperFunction = wrapperFunction;
    action.vabIdBefore = before.word_800943A8;
    action.vabIdAfter = before.word_800943A8;
    action.voiceBefore = before.word_800943AC;
    action.voiceAfter = before.word_800943AC;
    action.flushBusyFlag = before.dword_800943B4;
    return action;
}

void AppendIfPresent(ActionList* out, const Action& action) {
    if (out != nullptr) {
        Append(*out, action);
    }
}

}  // namespace

void Clear(ActionList& out) {
    out = {};
}

bool Append(ActionList& out, const Action& action) {
    if (out.count >= static_cast<uint32_t>(sizeof(out.actions) /
                                          sizeof(out.actions[0]))) {
        out.truncated = true;
        return false;
    }

    out.actions[out.count++] = action;
    return true;
}

void ApplyPadStartComAudioGlobalResetContract(State& state) {
    state.word_800943A8 = 0;
    state.word_800943AC = kClosedVoiceSentinel;
    state.dword_800943B4 = 0;
    state.dword_80094410 = kDword80094410;
}

int32_t Apply80027120(State& state, ActionList* out) {
    const State before = state;
    Action wrapper =
        MakeWrapperAction(ActionKind::DirectWrapper80027120, kFn80027120,
                          before);

    if (state.word_800943A8 >= 0) {
        Action lower{};
        lower.kind = ActionKind::HalSsVabClose8002DF80;
        lower.wrapperFunction = kFn80027120;
        lower.lowerFunction = kFnSsVabClose8002DF80;
        lower.arg0 = state.word_800943A8;
        lower.vabIdBefore = state.word_800943A8;
        lower.vabIdAfter = state.word_800943A8;
        lower.voiceBefore = state.word_800943AC;
        lower.voiceAfter = state.word_800943AC;
        lower.flushBusyFlag = state.dword_800943B4;
        AppendIfPresent(out, lower);

        state.word_800943A8 = kClosedVabSentinel80027120;
    } else {
        wrapper.skippedByGuard = true;
    }

    wrapper.vabIdAfter = state.word_800943A8;
    wrapper.result = 0;
    AppendIfPresent(out, wrapper);
    return 0;
}

int32_t Apply80027078(State& state,
                     uint32_t vhPtr,
                     int16_t lowerVabId,
                     ActionList* out) {
    const State before = state;
    Action wrapper =
        MakeWrapperAction(ActionKind::DirectWrapper80027078, kFn80027078,
                          before);

    Action lower{};
    lower.kind = ActionKind::HalVabOpen8002E3D8;
    lower.wrapperFunction = kFn80027078;
    lower.lowerFunction = kFn8002E3D8;
    lower.pointerArg = vhPtr;
    lower.arg0 = static_cast<int32_t>(vhPtr);
    lower.arg1 = kOpenVabAnySlotArg80027078;
    lower.result = lowerVabId;
    lower.vabIdBefore = before.word_800943A8;
    lower.vabIdAfter = lowerVabId;
    lower.voiceBefore = before.word_800943AC;
    lower.voiceAfter = before.word_800943AC;
    lower.flushBusyFlag = before.dword_800943B4;
    lower.lowerResultProvided = true;
    AppendIfPresent(out, lower);

    state.word_800943A8 = lowerVabId;

    wrapper.pointerArg = vhPtr;
    wrapper.result = (state.word_800943A8 >= 0) ? 1 : 0;
    wrapper.vabIdAfter = state.word_800943A8;
    AppendIfPresent(out, wrapper);
    return wrapper.result;
}

int32_t Apply800270D4(State& state,
                     uint32_t vbPtr,
                     int32_t lowerTransferResult,
                     ActionList* out) {
    const State before = state;
    Action wrapper =
        MakeWrapperAction(ActionKind::DirectWrapper800270D4, kFn800270D4,
                          before);

    Action lower{};
    lower.kind = ActionKind::HalVabTransfer8002EB80;
    lower.wrapperFunction = kFn800270D4;
    lower.lowerFunction = kFn8002EB80;
    lower.pointerArg = vbPtr;
    lower.arg0 = static_cast<int32_t>(vbPtr);
    lower.arg1 = state.word_800943A8;
    lower.result = lowerTransferResult;
    lower.vabIdBefore = before.word_800943A8;
    lower.vabIdAfter = before.word_800943A8;
    lower.voiceBefore = before.word_800943AC;
    lower.voiceAfter = before.word_800943AC;
    lower.flushBusyFlag = before.dword_800943B4;
    lower.lowerResultProvided = true;
    AppendIfPresent(out, lower);

    wrapper.pointerArg = vbPtr;
    wrapper.result = lowerTransferResult;
    AppendIfPresent(out, wrapper);
    return lowerTransferResult;
}

int32_t Apply800270FC(State& state,
                     int32_t arg0,
                     int32_t lowerEnableResult,
                     ActionList* out) {
    const State before = state;
    Action wrapper =
        MakeWrapperAction(ActionKind::DirectWrapper800270FC, kFn800270FC,
                          before);

    const int32_t enabled = (arg0 == 1) ? 1 : 0;
    Action lower{};
    lower.kind = ActionKind::HalVabEnable8002EEFC;
    lower.wrapperFunction = kFn800270FC;
    lower.lowerFunction = kFn8002EEFC;
    lower.arg0 = enabled;
    lower.result = lowerEnableResult;
    lower.vabIdBefore = before.word_800943A8;
    lower.vabIdAfter = before.word_800943A8;
    lower.voiceBefore = before.word_800943AC;
    lower.voiceAfter = before.word_800943AC;
    lower.flushBusyFlag = before.dword_800943B4;
    lower.lowerResultProvided = true;
    AppendIfPresent(out, lower);

    wrapper.arg0 = arg0;
    wrapper.arg1 = enabled;
    wrapper.result = lowerEnableResult;
    AppendIfPresent(out, wrapper);
    return lowerEnableResult;
}

int32_t Apply80026FA4(State& state,
                     int32_t ignoredCallerArg,
                     int32_t lowerResetResult,
                     ActionList* out) {
    const State before = state;
    Action wrapper =
        MakeWrapperAction(ActionKind::DirectWrapper80026FA4, kFn80026FA4,
                          before);

    Action lower{};
    lower.kind = ActionKind::HalSfxReset800351B8;
    lower.wrapperFunction = kFn80026FA4;
    lower.lowerFunction = kFn800351B8;
    lower.arg0 = kResetArg80026FA4;
    lower.result = lowerResetResult;
    lower.vabIdBefore = before.word_800943A8;
    lower.vabIdAfter = before.word_800943A8;
    lower.voiceBefore = before.word_800943AC;
    lower.voiceAfter = before.word_800943AC;
    lower.flushBusyFlag = before.dword_800943B4;
    lower.lowerResultProvided = true;
    AppendIfPresent(out, lower);

    wrapper.arg0 = ignoredCallerArg;
    wrapper.arg1 = kResetArg80026FA4;
    wrapper.result = lowerResetResult;
    AppendIfPresent(out, wrapper);
    return lowerResetResult;
}

int32_t Apply80026EF8(State& state,
                     Cue4& cue,
                     int16_t lowerVoiceId,
                     uint32_t cuePointer,
                     ActionList* out) {
    const State before = state;
    const Cue4 cueBefore = cue;
    cue.pitch = static_cast<uint8_t>(cue.note + kCuePitchAdd80026EF8);

    Action lower{};
    lower.kind = ActionKind::HalSfxCue80034240;
    lower.wrapperFunction = kFn80026EF8;
    lower.lowerFunction = kFn80034240;
    lower.pointerArg = cuePointer;
    lower.arg0 = state.word_800943A8;
    lower.arg1 = cue.program;
    lower.arg2 = cue.note;
    lower.arg3 = cue.pitch;
    lower.arg4 = 0;
    lower.arg5 = cue.volume;
    lower.arg6 = cue.volume;
    lower.result = lowerVoiceId;
    lower.vabIdBefore = before.word_800943A8;
    lower.vabIdAfter = before.word_800943A8;
    lower.voiceBefore = before.word_800943AC;
    lower.voiceAfter = lowerVoiceId;
    lower.flushBusyFlag = before.dword_800943B4;
    lower.cueBefore = cueBefore;
    lower.cueAfter = cue;
    lower.lowerResultProvided = true;
    AppendIfPresent(out, lower);

    state.word_800943AC = lowerVoiceId;

    Action wrapper =
        MakeWrapperAction(ActionKind::DirectWrapper80026EF8, kFn80026EF8,
                          before);
    wrapper.pointerArg = cuePointer;
    wrapper.result = lowerVoiceId;
    wrapper.vabIdAfter = state.word_800943A8;
    wrapper.voiceAfter = state.word_800943AC;
    wrapper.cueBefore = cueBefore;
    wrapper.cueAfter = cue;
    AppendIfPresent(out, wrapper);
    return lowerVoiceId;
}

int32_t Apply80026ECC(State& state,
                     int32_t lowerFlushResult,
                     ActionList* out) {
    const State before = state;
    Action wrapper =
        MakeWrapperAction(ActionKind::DirectWrapper80026ECC, kFn80026ECC,
                          before);

    if (state.dword_800943B4 == 0) {
        Action lower{};
        lower.kind = ActionKind::HalSfxFlush8002EFF4;
        lower.wrapperFunction = kFn80026ECC;
        lower.lowerFunction = kFn8002EFF4;
        lower.result = lowerFlushResult;
        lower.vabIdBefore = before.word_800943A8;
        lower.vabIdAfter = before.word_800943A8;
        lower.voiceBefore = before.word_800943AC;
        lower.voiceAfter = before.word_800943AC;
        lower.flushBusyFlag = before.dword_800943B4;
        lower.lowerResultProvided = true;
        AppendIfPresent(out, lower);

        wrapper.result = lowerFlushResult;
    } else {
        wrapper.result = state.dword_800943B4;
        wrapper.skippedByGuard = true;
    }

    AppendIfPresent(out, wrapper);
    return wrapper.result;
}

bool ApplyLiveActionResult(State& state,
                           const LiveActionInput& input,
                           LiveActionResult& out) {
    out = LiveActionResult{};
    out.actionKind = input.actionKind;
    if (!input.lowerResultKnown) {
        out.incomplete = true;
        return false;
    }

    out.lowerResult = input.lowerResult;
    out.lowerResultKnown = true;

    switch (input.actionKind) {
    case ActionKind::DirectWrapper80027120:
        (void)Apply80027120(state, &out.actionList);
        break;
    case ActionKind::DirectWrapper80027078:
        if (!input.vhPtrKnown) {
            out.incomplete = true;
            return false;
        }
        (void)Apply80027078(
            state,
            input.vhPtr,
            static_cast<int16_t>(input.lowerResult),
            &out.actionList);
        break;
    case ActionKind::DirectWrapper800270D4:
        if (!input.vbPtrKnown) {
            out.incomplete = true;
            return false;
        }
        (void)Apply800270D4(
            state,
            input.vbPtr,
            input.lowerResult,
            &out.actionList);
        break;
    case ActionKind::DirectWrapper800270FC:
        (void)Apply800270FC(state, 1, input.lowerResult, &out.actionList);
        break;
    case ActionKind::DirectWrapper80026FA4:
        (void)Apply80026FA4(state, 0, input.lowerResult, &out.actionList);
        break;
    case ActionKind::DirectWrapper80026EF8: {
        if (!input.cueKnown) {
            out.incomplete = true;
            return false;
        }
        Cue4 cue = input.cue;
        (void)Apply80026EF8(
            state,
            cue,
            static_cast<int16_t>(input.lowerResult),
            input.cuePointer,
            &out.actionList);
        break;
    }
    case ActionKind::DirectWrapper80026ECC:
        (void)Apply80026ECC(state, input.lowerResult, &out.actionList);
        break;
    default:
        out.incomplete = true;
        return false;
    }

    out.produced = true;
    return true;
}

}  // namespace PrStage1LoaderSpuHal
