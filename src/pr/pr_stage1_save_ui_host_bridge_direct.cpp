#include "pr_stage1_save_ui_host_bridge_direct.h"

#include "logger.h"
#include "pr_game_context.h"
#include "pr_sfx.h"

namespace PrStage1SaveUiHostBridgeDirect {
namespace {

struct SaveUi19148EventFrameContext {
    int32_t eventId = 0;
    int32_t word0 = 0;
    int32_t word1 = 0;
    int32_t word2 = 0;
};

SaveUi19148EventFrameContext s_saveUi19148EventFrameContext{};
PrPsxEventFrameDirect::EventFrameState8001E750
    s_saveUi19148FrameState8001E750{};
bool s_saveUi19148FrameStateActive8001E750 = false;

void ResetSaveUi19148EventFrameState8001E750() {
    s_saveUi19148FrameState8001E750 =
        PrPsxEventFrameDirect::EventFrameState8001E750{};
    s_saveUi19148FrameStateActive8001E750 = false;
}

void SubmitSaveUi19148EventFrameDraw8001E750(int32_t eventId,
                                             int32_t contextWord0,
                                             int32_t contextWord1,
                                             int32_t contextWord2,
                                             int32_t languageIndex) {
    const bool frameCloseBlocked =
        PrPsxEventFrameDirect::PsxCall8001E750_SaveUiEventFrame(
            s_saveUi19148FrameState8001E750,
            eventId,
            contextWord0,
            contextWord1,
            contextWord2,
            languageIndex);
    s_saveUi19148FrameStateActive8001E750 = !frameCloseBlocked;
}

void SubmitSaveUi19148WaitFrame80035560(int32_t arg0) {
    if (!s_saveUi19148FrameStateActive8001E750) {
        return;
    }
    PrPsxEventFrameDirect::PsxCall80035560_WaitFrame(
        s_saveUi19148FrameState8001E750,
        arg0);
}

void SubmitSaveUi19148EndFrame8001EA00(int32_t eventId) {
    if (!s_saveUi19148FrameStateActive8001E750) {
        return;
    }
    PrPsxEventFrameDirect::PsxCall8001EA00_EndFrame(
        s_saveUi19148FrameState8001E750,
        eventId);
    PrPsxEventFrameDirect::PsxConsume80035560_WaitFrameHostVblank(
        s_saveUi19148FrameState8001E750,
        1);
}

bool ShouldLogSaveUi19148ActionGap(
    const PrStage1SaveUi19148Action& action) {
    struct LoggedActionGap {
        PrStage1SaveUi19148ActionKind kind;
        uint32_t psxFunction;
        int32_t arg0;
    };
    static LoggedActionGap logged[128]{};
    static uint32_t loggedCount = 0;

    for (uint32_t i = 0; i < loggedCount; ++i) {
        if (logged[i].kind == action.kind &&
            logged[i].psxFunction == action.psxFunction &&
            logged[i].arg0 == action.arg0) {
            return false;
        }
    }
    if (loggedCount <
        static_cast<uint32_t>(sizeof(logged) / sizeof(logged[0]))) {
        logged[loggedCount++] =
            LoggedActionGap{action.kind, action.psxFunction, action.arg0};
    }
    return true;
}

void LogSaveUi19148ActionGap(
    const PrStage1SaveUi19148Action& action,
    const char* reason) {
    if (!ShouldLogSaveUi19148ActionGap(action)) {
        return;
    }
    Log::Printf(
        "Scene1 801C81EC save-ui 19148 action gap: kind=%s boundary=%s "
        "fn=%08X state=%d->%d args=%d,%d,%d,%d reason=%s",
        PrStage1SaveUiDirect::ActionKindName19148(action.kind),
        PrStage1SaveUiDirect::ActionHostBoundaryName19148(
            action.hostBoundary),
        action.psxFunction,
        action.stateBefore,
        action.stateAfter,
        action.arg0,
        action.arg1,
        action.arg2,
        action.arg3,
        reason ? reason : "");
}

void ExecuteSaveUi19148HostActionRequest(
    const PrStage1SaveUi19148HostActionRequest& request) {
    switch (request.kind) {
    case PrStage1SaveUi19148HostActionRequestKind::None:
        break;
    case PrStage1SaveUi19148HostActionRequestKind::PlayInputSfx:
        PrSfx::PlayStage1UiCue80025C8CRaw(request.sfxCue);
        PrSfx::ApplySharedAudioDriverFlushBarrier26ECC();
        break;
    case PrStage1SaveUi19148HostActionRequestKind::GapReport:
        LogSaveUi19148ActionGap(
            request.action,
            PrStage1SaveUiDirect::ActionGapReasonName19148(
                request.gapReason));
        break;
    }
}

void ConsumeSaveUi19148EventFrameAction(
    PrGameContext& ctx,
    const PrStage1SaveUi19148Action& action) {
    switch (action.kind) {
    case PrStage1SaveUi19148ActionKind::Call80017E58InitEventArg:
        s_saveUi19148EventFrameContext =
            SaveUi19148EventFrameContext{};
        s_saveUi19148EventFrameContext.eventId = action.arg0;
        ResetSaveUi19148EventFrameState8001E750();
        break;
    case PrStage1SaveUi19148ActionKind::Call80017E6CSetEventResult:
        s_saveUi19148EventFrameContext.eventId = action.arg0;
        s_saveUi19148EventFrameContext.word0 = 1;
        if (action.arg1 >= 0) {
            s_saveUi19148EventFrameContext.word1 = action.arg1;
        }
        s_saveUi19148EventFrameContext.word2 = action.arg2;
        break;
    case PrStage1SaveUi19148ActionKind::Call8001E750DrawEvent:
        if (s_saveUi19148EventFrameContext.eventId != action.arg0) {
            s_saveUi19148EventFrameContext.eventId = action.arg0;
        }
        SubmitSaveUi19148EventFrameDraw8001E750(
            action.arg0,
            s_saveUi19148EventFrameContext.word0,
            s_saveUi19148EventFrameContext.word1,
            s_saveUi19148EventFrameContext.word2,
            ctx.languageIndex);
        break;
    case PrStage1SaveUi19148ActionKind::Call80035560ResetInput:
        SubmitSaveUi19148WaitFrame80035560(action.arg0);
        break;
    case PrStage1SaveUi19148ActionKind::Call8001EA00EndFrame:
        SubmitSaveUi19148EndFrame8001EA00(action.arg0);
        break;
    default:
        break;
    }
}

void ConsumeSaveUi19148EventFrameActions(
    PrGameContext& ctx,
    const PrStage1SaveUi19148ActionList& actions) {
    for (uint32_t i = 0; i < actions.count; ++i) {
        ConsumeSaveUi19148EventFrameAction(ctx, actions.actions[i]);
    }
}

} // namespace

void ExecuteSaveUi19148HostActionRequests(
    const PrStage1SaveUi19148HostActionRequestList& requests) {
    for (uint32_t i = 0; i < requests.count; ++i) {
        ExecuteSaveUi19148HostActionRequest(requests.requests[i]);
    }
    if (requests.sourceActionListTruncated || requests.truncated) {
        Log::Printf(
            "Scene1 801C81EC save-ui 19148 action gap: action list "
            "truncated");
    }
}

SaveUi19148HostTickAttempt RunSaveUi19148HostTickAttempt(
    PrGameContext& ctx,
    const PrStage1SaveUi19148LowerFeedback* lowerFeedback) {
    SaveUi19148HostTickAttempt out{};
    const PrStage1SaveUi19148TickResult saveUiResult =
        PrStage1SaveUiDirect::Tick19148(ctx, lowerFeedback);
    ConsumeSaveUi19148EventFrameActions(ctx, saveUiResult.actions);
    if (saveUiResult.done) {
        ResetSaveUi19148EventFrameState8001E750();
    }
    ExecuteSaveUi19148HostActionRequests(
        PrStage1SaveUiDirect::BuildHostActionRequests19148(
            saveUiResult.actions));
    out.lowerFeedbackRequests =
        PrStage1SaveUiDirect::BuildLowerFeedbackRequests19148(
            saveUiResult.actions);
    out.attempted = true;
    out.active = saveUiResult.active;
    out.done = saveUiResult.done;
    out.saveSucceeded = saveUiResult.saveSucceeded;
    out.saveResult = saveUiResult.saveResult;
    out.psxState = saveUiResult.psxState;
    out.psxEventId = saveUiResult.psxEventId;
    out.ioResultKnown = saveUiResult.ioResultKnown;
    out.ioResult = saveUiResult.ioResult;
    out.cardIoStateBeforeKnown80017594 =
        saveUiResult.cardIoStateBeforeKnown80017594;
    out.cardIoStateAfterKnown80017594 =
        saveUiResult.cardIoStateAfterKnown80017594;
    out.cardIoStateBefore80017594 =
        saveUiResult.cardIoStateBefore80017594;
    out.cardIoStateAfter80017594 =
        saveUiResult.cardIoStateAfter80017594;
    out.saveWriteResultKnown80017A10 =
        saveUiResult.saveWriteResultKnown80017A10;
    out.saveWriteResult80017A10 = saveUiResult.saveWriteResult80017A10;
    out.saveWriteSucceeded80019458 =
        saveUiResult.saveWriteSucceeded80019458;
    out.gp716After80019458Known = saveUiResult.gp716After80019458Known;
    out.gp716After80019458 = saveUiResult.gp716After80019458;
    out.gp720After80019458Known = saveUiResult.gp720After80019458Known;
    out.gp720After80019458 = saveUiResult.gp720After80019458;
    return out;
}

const PrPsxEventFrameDirect::EventFrameState8001E750*
GetActiveSaveUiEventFrameState8001E750() {
    return s_saveUi19148FrameStateActive8001E750
               ? &s_saveUi19148FrameState8001E750
               : nullptr;
}

} // namespace PrStage1SaveUiHostBridgeDirect
