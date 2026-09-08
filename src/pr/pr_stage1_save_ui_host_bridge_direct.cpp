#include "pr_stage1_save_ui_host_bridge_direct.h"

#include "logger.h"
#include "pr_game_context.h"
#include "pr_sfx.h"
#include "pr_stage_scene_submit_runtime_private.h"
#include "pr_stage1_save_entry_direct.h"
#include "pr_stage1_save_presentation_direct.h"
#include "pr_ss0_card_image_storage_direct.h"

#include <cstring>

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
bool s_saveUi19148WaitFrameActive80035560 = false;
PrStage1SaveEntryDirect::State s_saveEntry19148{};
PrStage1SaveStatusPrefix80092F10 s_saveEntrySeed19148{};
PrStage1SavePresentationDirect::State s_savePresentation19148{};
PrStage1SaveUi19148TickResult s_savePresentationResult19148{};
bool s_savePresentationResultPending19148 = false;
constexpr uint32_t kFn80017E6C = 0x80017E6Cu;
constexpr uint32_t kFn800180D8 = 0x800180D8u;

void ResetSaveUi19148EventFrameState8001E750() {
    s_saveUi19148FrameState8001E750 =
        PrPsxEventFrameDirect::EventFrameState8001E750{};
    s_saveUi19148FrameStateActive8001E750 = false;
    s_saveUi19148WaitFrameActive80035560 = false;
}

bool SubmitSaveUi19148EventFrameDraw8001E750(int32_t eventId,
                                             int32_t contextWord0,
                                             int32_t contextWord1,
                                             int32_t contextWord2,
                                             int32_t languageIndex,
                                             const PrStage1SaveUiCardInfoRenderSnapshot80020BE4*
                                                 cardInfoSnapshot,
                                             const PrStage1SaveUiCardGridRenderSnapshot80020F94*
                                                 cardGridSnapshot) {
    if (!s_saveUi19148FrameState8001E750.initialized) {
        PrPsxEventFrameDirect::ResetEventFrameState8003FB9C(
            s_saveUi19148FrameState8001E750, 320u, 240u);
        // 80019148 -> 80018FB0 -> 8001E750 does not reinitialize Gs.
        // Retain the active 8001C470 screen center/page, as native Event4 does.
        s_saveUi19148FrameState8001E750.graph =
            PrStageSceneSubmitDirect::GetOwnedStage1GraphOwner801CBFDC();
    }
    PrPsxEventFrameDirect::SaveUiCardInfoDrawInput80020BE4 cardInfoInput{};
    const PrPsxEventFrameDirect::SaveUiCardInfoDrawInput80020BE4*
        cardInfoInputPtr = nullptr;
    if (eventId == 5) {
        const PrStage1SaveUiCardInfoRenderSnapshot80020BE4 snapshot =
            cardInfoSnapshot != nullptr
                ? *cardInfoSnapshot
                : PrStage1SaveUiCardInfoRenderSnapshot80020BE4{};
        cardInfoInput.requestBound = snapshot.requestBound;
        cardInfoInput.argAddress = snapshot.argAddress;
        cardInfoInput.languageKnown =
            languageIndex >= 0 && languageIndex <= 4;
        cardInfoInput.languageIndex = languageIndex;
        cardInfoInput.topFlagKnown = true;
        cardInfoInput.topFlag = static_cast<uint32_t>(contextWord0);
        cardInfoInput.ioFlagKnown = true;
        cardInfoInput.ioFlag = static_cast<uint32_t>(contextWord2);
        cardInfoInput.selectedMarkerKnown = snapshot.selectedMarkerKnown;
        cardInfoInput.selectedMarker = snapshot.selectedMarker;
        cardInfoInput.lowerModeKnown = snapshot.lowerModeKnown;
        cardInfoInput.lowerMode = snapshot.lowerMode;
        cardInfoInput.topIconTemplateSlotsKnown =
            snapshot.topIconTemplateSlotsKnown;
        cardInfoInput.topIconOffTemplate = snapshot.topIconOffTemplate;
        cardInfoInput.topIconOnTemplate = snapshot.topIconOnTemplate;
        cardInfoInput.encodedPreviewKnown = snapshot.encodedPreviewKnown;
        cardInfoInput.encodedPreviewByteCount =
            snapshot.encodedPreviewByteCount;
        cardInfoInput.lowRamDescriptorKnown =
            snapshot.lowRamDescriptorKnown;
        cardInfoInput.lowRamAttr = snapshot.lowRamAttr;
        cardInfoInput.lowRamTexX = snapshot.lowRamTexX;
        cardInfoInput.lowRamTexY = snapshot.lowRamTexY;
        cardInfoInput.lowRamWidth = snapshot.lowRamWidth;
        cardInfoInput.lowRamHeight = snapshot.lowRamHeight;
        cardInfoInput.lowRamClutX = snapshot.lowRamClutX;
        cardInfoInput.lowRamClutY = snapshot.lowRamClutY;
        for (std::size_t i = 0u;
             i < snapshot.encodedPreviewByteCount &&
             i < PrPsxEventFrameDirect::
                     kSaveUiCardInfoPreviewCapacity80020BE4;
             ++i) {
            cardInfoInput.encodedPreview[i] =
                static_cast<uint8_t>(snapshot.encodedPreview[i]);
        }
        cardInfoInputPtr = &cardInfoInput;
    }
    PrPsxEventFrameDirect::SaveUiCardGridDrawInput80020F94 cardGridInput{};
    const PrPsxEventFrameDirect::SaveUiCardGridDrawInput80020F94*
        cardGridInputPtr = nullptr;
    if (eventId >= 7 && eventId <= 9) {
        const PrStage1SaveUiCardGridRenderSnapshot80020F94 snapshot =
            cardGridSnapshot != nullptr
                ? *cardGridSnapshot
                : PrStage1SaveUiCardGridRenderSnapshot80020F94{};
        cardGridInput.requestBound = snapshot.requestBound;
        cardGridInput.argAddress = snapshot.argAddress;
        cardGridInput.rows = snapshot.rows;
        cardGridInput.columns = snapshot.columns;
        cardGridInput.itemCount = snapshot.itemCount;
        cardGridInput.selected = snapshot.selected;
        for (std::size_t i = 0u;
             i < PrPsxEventFrameDirect::
                     kSaveUiCardGridItemCapacity80020F94;
             ++i) {
            cardGridInput.enabled[i] = snapshot.enabled[i];
            std::memcpy(
                cardGridInput.slotText[i],
                snapshot.slotText[i],
                PrPsxEventFrameDirect::
                    kSaveUiCardGridTextCapacity80020F94);
        }
        cardGridInputPtr = &cardGridInput;
    }
    const bool frameCloseBlocked =
        PrPsxEventFrameDirect::PsxCall8001E750_SaveUiEventFrame(
            s_saveUi19148FrameState8001E750,
            eventId,
            contextWord0,
            contextWord1,
            contextWord2,
            languageIndex,
            cardInfoInputPtr,
            cardGridInputPtr);
    s_saveUi19148FrameStateActive8001E750 = !frameCloseBlocked;
    return !frameCloseBlocked;
}

bool SubmitSaveUi19148WaitFrame80035560(int32_t arg0) {
    if (!s_saveUi19148FrameStateActive8001E750) {
        return false;
    }
    PrPsxVSyncDirect::PsxVSyncState80035560& sharedVSync =
        PrPsxVSyncDirect::ProcessVSyncState80035560();
    const PrPsxVSyncDirect::PsxVSyncResult80035560 wait =
        PrPsxEventFrameDirect::PsxCall80035560_WaitFrameDetailed(
            s_saveUi19148FrameState8001E750,
            sharedVSync,
            arg0);
    s_saveUi19148WaitFrameActive80035560 =
        wait.sourceKnown && wait.waitPending &&
        wait.mode ==
            PrPsxVSyncDirect::PsxVSyncMode80035560::WaitForVblanks;
    return s_saveUi19148WaitFrameActive80035560;
}

bool SubmitSaveUi19148EndFrame8001EA00(int32_t eventId) {
    if (!s_saveUi19148FrameStateActive8001E750 ||
        !s_saveUi19148WaitFrameActive80035560) {
        return false;
    }
    PrPsxVSyncDirect::PsxVSyncState80035560& sharedVSync =
        PrPsxVSyncDirect::ProcessVSyncState80035560();
    const PrPsxVSyncDirect::PsxVSyncResult80035560 wait =
        PrPsxEventFrameDirect::
            PsxConsume80035560_WaitFrameHostVblankDetailed(
                s_saveUi19148FrameState8001E750,
                sharedVSync,
                1);
    const bool waitComplete =
        wait.sourceKnown && wait.softwareStateCommitted &&
        wait.softwareWaitComplete &&
        wait.mode ==
            PrPsxVSyncDirect::PsxVSyncMode80035560::WaitForVblanks;
    if (!waitComplete) {
        return false;
    }
    s_saveUi19148WaitFrameActive80035560 = false;
    PrPsxEventFrameDirect::PsxCall8001EA00_EndFrame(
        s_saveUi19148FrameState8001E750,
        eventId);
    return true;
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

bool ConsumeSaveUi19148EventFrameAction(
    PrGameContext& ctx,
    const PrStage1SaveUi19148Action& action,
    const PrStage1SaveUiCardGridRenderSnapshot80020F94*
        cardGridSnapshot) {
    switch (action.kind) {
    case PrStage1SaveUi19148ActionKind::Call80017E58InitEventArg:
        s_saveUi19148EventFrameContext =
            SaveUi19148EventFrameContext{};
        s_saveUi19148EventFrameContext.eventId = action.arg0;
        ResetSaveUi19148EventFrameState8001E750();
        break;
    case PrStage1SaveUi19148ActionKind::Call80017E6CSetEventResult:
        if (action.psxFunction != kFn80017E6C &&
            action.psxFunction != kFn800180D8) {
            LogSaveUi19148ActionGap(
                action, "unknown event-result action producer");
            return false;
        }
        if (action.psxFunction == kFn80017E6C &&
            (action.arg3 != kSaveUiPromptFlashFrameCount80017E6C ||
             (action.arg0 == 5 && !action.cardInfoSnapshot.requestBound) ||
             (action.arg0 >= 7 && action.arg0 <= 9 &&
              (!action.usesCardGridSnapshot80020F94 ||
               cardGridSnapshot == nullptr ||
               !cardGridSnapshot->requestBound)))) {
            ResetSaveUi19148EventFrameState8001E750();
            LogSaveUi19148ActionGap(action,
                                    "invalid 80017E6C prompt flash payload");
            return false;
        }
        s_saveUi19148EventFrameContext.eventId = action.arg0;
        s_saveUi19148EventFrameContext.word0 = 1;
        if (action.arg1 >= 0) {
            s_saveUi19148EventFrameContext.word1 = action.arg1;
        }
        s_saveUi19148EventFrameContext.word2 = action.arg2;
        if (action.psxFunction == kFn800180D8) {
            break;
        }
        // Exactly one transaction. The presentation continuation owns the
        // native 20 VBlank loop and prevents later actions from overtaking it.
        {
            if (!SubmitSaveUi19148EventFrameDraw8001E750(
                    action.arg0,
                    s_saveUi19148EventFrameContext.word0,
                    s_saveUi19148EventFrameContext.word1,
                    s_saveUi19148EventFrameContext.word2,
                    ctx.languageIndex,
                    action.arg0 == 5 ? &action.cardInfoSnapshot : nullptr,
                    action.arg0 >= 7 && action.arg0 <= 9
                        ? cardGridSnapshot
                        : nullptr)) {
                ResetSaveUi19148EventFrameState8001E750();
                LogSaveUi19148ActionGap(action,
                                        "80017E6C prompt flash draw blocked");
                return false;
            }
            if (!SubmitSaveUi19148WaitFrame80035560(0) ||
                !SubmitSaveUi19148EndFrame8001EA00(0)) {
                ResetSaveUi19148EventFrameState8001E750();
                LogSaveUi19148ActionGap(
                    action,
                    "80017E6C typed VSync/end-frame transaction blocked");
                return false;
            }
        }
        break;
    case PrStage1SaveUi19148ActionKind::Call8001E750DrawEvent:
        if (s_saveUi19148EventFrameContext.eventId != action.arg0) {
            s_saveUi19148EventFrameContext.eventId = action.arg0;
        }
        switch (static_cast<PrStage1SaveUiEventArgUpdate80018FB0>(
            action.arg1)) {
        case PrStage1SaveUiEventArgUpdate80018FB0::None:
            break;
        case PrStage1SaveUiEventArgUpdate80018FB0::ToggleWord0:
            s_saveUi19148EventFrameContext.word0 =
                s_saveUi19148EventFrameContext.word0 == 1 ? 0 : 1;
            break;
        case PrStage1SaveUiEventArgUpdate80018FB0::
            ForceWord0AndWord2One:
            s_saveUi19148EventFrameContext.word0 = 1;
            s_saveUi19148EventFrameContext.word2 = 1;
            break;
        default:
            return true;
        }
        if (!SubmitSaveUi19148EventFrameDraw8001E750(
            action.arg0,
            s_saveUi19148EventFrameContext.word0,
            s_saveUi19148EventFrameContext.word1,
            s_saveUi19148EventFrameContext.word2,
            ctx.languageIndex,
            action.arg0 == 5 ? &action.cardInfoSnapshot : nullptr,
            action.arg0 >= 7 && action.arg0 <= 9
                ? cardGridSnapshot
                : nullptr)) {
            if (action.arg0 >= 7 && action.arg0 <= 9) {
                ResetSaveUi19148EventFrameState8001E750();
                LogSaveUi19148ActionGap(
                    action, "80020F94 card-grid draw blocked");
                return false;
            }
        }
        break;
    case PrStage1SaveUi19148ActionKind::Call80035560ResetInput:
        if (!SubmitSaveUi19148WaitFrame80035560(action.arg0)) {
            LogSaveUi19148ActionGap(
                action, "typed 80035560 wait request blocked");
            return false;
        }
        break;
    case PrStage1SaveUi19148ActionKind::Call8001EA00EndFrame:
        if (!SubmitSaveUi19148EndFrame8001EA00(action.arg0)) {
            LogSaveUi19148ActionGap(
                action, "typed VSync completion blocked end frame");
            return false;
        }
        break;
    default:
        break;
    }
    return true;
}

bool ConsumeSaveUi19148OrderedAction(
    PrGameContext& ctx,
    const PrStage1SaveUi19148Action& action,
    const PrStage1SaveUi19148ActionList& actions) {
    const auto* cardGridSnapshot = action.usesCardGridSnapshot80020F94
        ? &actions.cardGridSnapshot80020F94 : nullptr;
    if (!ConsumeSaveUi19148EventFrameAction(ctx, action, cardGridSnapshot)) return false;
    // SFX and gap reports belong at their original position, not after all
    // draw/flash actions. No input or card HAL is called from this consumer.
    PrStage1SaveUi19148ActionList one{};
    one.count = 1;
    one.actions[0] = action;
    ExecuteSaveUi19148HostActionRequests(
        PrStage1SaveUiDirect::BuildHostActionRequests19148(one));
    return true;
}

bool ConsumeSaveUi19148EventFrameActions(
    PrGameContext& ctx,
    const PrStage1SaveUi19148ActionList& actions) {
    if (actions.truncated || actions.count > 96u) return false;
    for (uint32_t i = 0; i < actions.count; ++i) {
        if (!ConsumeSaveUi19148OrderedAction(ctx, actions.actions[i], actions)) return false;
    }
    return true;
}

} // namespace

void ResetSaveUiPresentation19148() {
    s_savePresentation19148 = {};
    s_savePresentationResult19148 = {};
    s_savePresentationResultPending19148 = false;
    s_saveEntry19148 = {};
    s_saveEntrySeed19148 = {};
    s_saveUi19148EventFrameContext = {};
    ResetSaveUi19148EventFrameState8001E750();
}

bool IsSaveEntryTransitionActive19148() {
    return s_saveEntry19148.pending;
}

bool HasPendingSaveUiPresentation19148() {
    // Retain ownership after the last interval until the lifecycle consumes
    // its result. In particular it must not start/commit card I/O beforehand.
    return s_savePresentationResultPending19148;
}

void PumpSaveUiPresentation19148(PrGameContext& ctx) {
    if (!s_savePresentationResultPending19148 || !s_savePresentation19148.pending) return;
    const bool complete = PrStage1SavePresentationDirect::Advance(
        s_savePresentation19148, s_savePresentationResult19148.actions,
        ctx.hostPresentationVblank60,
        [&](const PrStage1SaveUi19148Action& action, uint64_t vblank) {
            const bool accepted = ConsumeSaveUi19148OrderedAction(
                ctx, action, s_savePresentationResult19148.actions);
            if (accepted && PrStage1SavePresentationDirect::IsFlash(action) &&
                (s_savePresentation19148.flashFrames == 0u ||
                 s_savePresentation19148.flashFrames == 19u)) {
                Log::Printf("Stage1 SaveUi19148: feedback event=%d choice=%d frame=%u vblank=%llu nativeFrame=%u/20",
                    action.arg0, action.arg1, ctx.frame,
                    static_cast<unsigned long long>(vblank),
                    s_savePresentation19148.flashFrames + 1u);
            }
            return accepted;
        });
    if (complete) {
        Log::Printf("Stage1 SaveUi19148: feedback intervals complete frame=%u vblank=%llu presentations=%u done=%d",
            ctx.frame, static_cast<unsigned long long>(s_savePresentation19148.nextVblank),
            s_savePresentation19148.presentations, s_savePresentationResult19148.done ? 1 : 0);
    }
}

static bool AdvanceSaveEntry19148(PrGameContext& ctx) {
    auto next = s_saveEntry19148;
    if (!PrStage1SaveEntryDirect::Advance(next, ctx.frame)) return false;
    if (!s_saveUi19148FrameState8001E750.initialized) {
        PrPsxEventFrameDirect::ResetEventFrameState8003FB9C(s_saveUi19148FrameState8001E750, 320u, 240u);
        s_saveUi19148FrameState8001E750.graph = PrStageSceneSubmitDirect::GetOwnedStage1GraphOwner801CBFDC();
    }
    // No selected choice during 80020110(0,3,2,1); dispatcher input and
    // its event argument initialization are deferred until the tail returns.
    if (PrPsxEventFrameDirect::PsxCall8001E750_SaveUiEventFrame(
            s_saveUi19148FrameState8001E750, 11, 0, 0, 0, ctx.languageIndex,
            nullptr, nullptr, &next.frame.visualFrame)) return false;
    (void)PrSfx::ApplyMovieTransitionCueCadence80027194();
    auto& vsync = PrPsxVSyncDirect::ProcessVSyncState80035560();
    const auto wait = PrPsxEventFrameDirect::PsxCall80035560_WaitFrameDetailed(
        s_saveUi19148FrameState8001E750, vsync, 2);
    if (!wait.sourceKnown || !wait.waitPending) return false;
    const auto consumed = PrPsxEventFrameDirect::PsxConsume80035560_WaitFrameHostVblankDetailed(
        s_saveUi19148FrameState8001E750, vsync, 2);
    if (!consumed.softwareStateCommitted || !consumed.softwareWaitComplete) return false;
    const auto end = PrPsxEventFrameDirect::PsxCall8001EBF4_SaveEntryEndFrame(s_saveUi19148FrameState8001E750);
    if (!end.completeWithinLimits) return false;
    s_saveEntry19148 = next;
    s_saveUi19148FrameStateActive8001E750 = true;
    Log::Printf("Stage1 SaveUi19148: entry mode3 frame=%u present=%u body=%u tail=%u tiles=%u complete=%d",
        ctx.frame, next.presentations, next.runtime.loopIterationsCompleted,
        next.runtime.tailIterationsCompleted, next.frame.visualFrame.activeCount, next.frame.complete ? 1 : 0);
    return true;
}

bool StartSaveUiWithEntry19148(PrGameContext& ctx,
    const PrStage1SaveStatusPrefix80092F10& seed) {
    if (s_saveEntry19148.pending || HasPendingSaveUiPresentation19148() ||
        PrStage1SaveUiDirect::IsActive19148()) return true;
    if (!seed.known || seed.helperGap || !seed.statusBankKnown80092F1D ||
        seed.psxAddress != PrStage1SaveStatusPrefix80092F10::kPsxAddress ||
        seed.byteCount != PrStage1SaveStatusPrefix80092F10::kByteCount) return false;
    ResetSaveUiPresentation19148();
    s_saveEntrySeed19148 = seed;
    if (!PrStage1SaveEntryDirect::Begin(s_saveEntry19148) || !AdvanceSaveEntry19148(ctx)) {
        ResetSaveUiPresentation19148();
        return false;
    }
    return true;
}

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
    if (s_saveEntry19148.pending) {
        out.active = true;
        if (!PrStage1SaveEntryDirect::ReadyForDispatcher(s_saveEntry19148, ctx.frame)) {
            if (!s_saveEntry19148.tickKnown || s_saveEntry19148.lastLogicTick != ctx.frame) {
                out.attempted = AdvanceSaveEntry19148(ctx);
            }
            return out;
        }
        if (!PrStage1SaveUiDirect::Start19148(ctx, &s_saveEntrySeed19148)) return out;
        // Native 19148 -> 17524 resets card communication, not the physical
        // medium. Start19148 resets UI-local buffers, including the imported
        // image view; restore it from the actual card HAL before card_info.
        // A missing/unreadable card remains missing: this never creates one.
        const auto card = PrSS0CardImageStorageDirect::BindAndLoadPrimaryCardImage8007A318();
        Log::Printf("Stage1 SaveUi19148: card HAL bound after UI init backend=%d found=%d read=%d imported=%d",
            card.backendBound ? 1 : 0, card.imageFileFound ? 1 : 0,
            card.imageRead ? 1 : 0, card.sinkImported ? 1 : 0);
        s_saveEntry19148.pending = false;
        // Publish the first dispatcher page in this SAME logic tick. The
        // final transition frame must never be cleared into an empty frame.
    }
    PrStage1SaveUi19148TickResult saveUiResult{};
    if (!s_savePresentationResultPending19148) {
        saveUiResult = PrStage1SaveUiDirect::Tick19148(ctx, lowerFeedback);
        if (PrStage1SavePresentationDirect::ContainsFlash(saveUiResult.actions)) {
            s_savePresentationResult19148 = saveUiResult;
            s_savePresentationResultPending19148 = true;
            PrStage1SavePresentationDirect::Begin(s_savePresentation19148,
                ctx.hostPresentationVblank60);
        } else if (!ConsumeSaveUi19148EventFrameActions(ctx, saveUiResult.actions)) {
            // A rejected submission must not publish completion/card requests.
            s_savePresentationResult19148 = saveUiResult;
            s_savePresentationResultPending19148 = true;
            PrStage1SavePresentationDirect::Begin(s_savePresentation19148,
                ctx.hostPresentationVblank60);
            s_savePresentation19148.blocked = true;
        }
    }
    if (s_savePresentationResultPending19148) {
        PumpSaveUiPresentation19148(ctx);
        if (s_savePresentation19148.pending) {
            out.attempted = true;
            out.active = true;
            out.psxState = s_savePresentationResult19148.psxState;
            out.psxEventId = s_savePresentationResult19148.psxEventId;
            return out;
        }
        saveUiResult = s_savePresentationResult19148;
        s_savePresentationResult19148 = {};
        s_savePresentationResultPending19148 = false;
    }
    if (saveUiResult.done) {
        // 80019148 runs 18F70 after 18FB0, including its final 17E6C wait.
        PrStage1SaveUiDirect::SnapshotDirectory80018F70();
        ResetSaveUi19148EventFrameState8001E750();
    }
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
    out.inputStateConsumes80018FB0 =
        saveUiResult.inputStateConsumes80018FB0;
    out.inputDispatcherResultDrive80018FB0 =
        saveUiResult.inputDispatcherResultDrive80018FB0;
    out.inputDispatcherResultPendingBefore80018FB0 =
        saveUiResult.inputDispatcherResultPendingBefore80018FB0;
    out.inputMaskRaw80035510 = saveUiResult.inputMaskRaw80035510;
    out.inputMaskBeforeDedup80018FB0 =
        saveUiResult.inputMaskBeforeDedup80018FB0;
    out.inputMaskAfterDedup80018FB0 =
        saveUiResult.inputMaskAfterDedup80018FB0;
    out.gp708LastInputBefore80018FB0 =
        saveUiResult.gp708LastInputBefore80018FB0;
    out.gp708LastInputAfter80018FB0 =
        saveUiResult.gp708LastInputAfter80018FB0;
    out.inputDuplicateSuppressed80018FB0 =
        saveUiResult.inputDuplicateSuppressed80018FB0;
    out.inputHandled800185D0 = saveUiResult.inputHandled800185D0;
    out.inputStateBefore800185D0 =
        saveUiResult.inputStateBefore800185D0;
    out.inputStateAfter800185D0 = saveUiResult.inputStateAfter800185D0;
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
    out.consumedBy80019458State15Known =
        saveUiResult.consumedBy80019458State15Known;
    out.consumedBy80019458State15 =
        saveUiResult.consumedBy80019458State15;
    out.state15PrefixAddress = saveUiResult.state15PrefixAddress;
    out.state15CopyTo8007ADE8Known =
        saveUiResult.state15CopyTo8007ADE8Known;
    out.state15CopyTo8007ADE8 = saveUiResult.state15CopyTo8007ADE8;
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
