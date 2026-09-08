#include "pr_scene_entry_executor_direct.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <system_error>

#include "logger.h"
#include "pr_event.h"
#include "pr_game_context.h"
#include "pr_pad.h"
#include "pr_psx_pad_direct.h"
#include "pr_scene1_entry_original_disc_direct.h"
#include "pr_ss0_scene0_runtime_direct.h"
#include "pr_ss0_stage_progress_bank_direct.h"
#include "pr_stage1_loader_cd_hal.h"
#include "pr_stage1_loader_memory_direct.h"
#include "pr_stage1_movie_segment_direct.h"
#include "pr_stage1_save_ui_direct.h"
#include "pr_stage1_xa_cd_direct.h"
#include "pr_scn1.h"
#include "pr_sfx.h"
#include "pr_transition.h"
#include "../resource_manager.h"

namespace PrSceneEntryExecutorDirect {

namespace {

static int ClampSceneIndex80015788(int scene) {
    if (scene < 0 || scene >= static_cast<int>(kPrSceneCount)) {
        return 0;
    }
    return scene;
}

static bool SS0DirectOwnsGenericSwitchScene80015788(PrSceneId scene) {
    return PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
           (scene == PrSceneId::Scene0 || scene == PrSceneId::Scene1);
}

static bool SS0DirectOwnsGenericSwitchContext80015788(
    const PrGameContext& ctx) {
    return SS0DirectOwnsGenericSwitchScene80015788(ctx.currentScene);
}

enum class GenericSwitchPendingHostCall80015788 : uint8_t {
    None = 0,
    Event26B94,
    Replay193F4,
    Practice2776C,
    Save193B0,
};

struct GenericSwitchHostState80015788 {
    bool active = false;
    PrSceneEntryDirect::GenericSwitchState80015788 direct{};
    GenericSwitchPendingHostCall80015788 pending =
        GenericSwitchPendingHostCall80015788::None;
    uint32_t pendingEventId = 0;
    int16_t pendingD0 = 0;
    int16_t pendingOutArg = -1;
    int32_t pendingEvent6ArgToken = 0;
    void* pendingEvent6ArgPtr = nullptr;
    bool call19414FeedbackLatched = false;
    PrSceneEntryDirect::Call80019414Feedback80015788 call19414Feedback{};
    uint32_t call19414TablePsxAddress = 0;
    size_t call19414TableByteCount = 0;
    uint8_t call19414TableStorage
        [PrSceneEntryDirect::kHiScoreTableSize80019284]{};
    bool call15700GapLogged = false;
    bool call161A8GapLogged = false;
    bool call19414GapLogged = false;
};

static GenericSwitchHostState80015788 s_genericSwitch80015788;
static constexpr uint32_t kFn80025A34 = 0x80025A34u;
static constexpr uint32_t kFn80025A00 = 0x80025A00u;
static constexpr size_t kDirectScene1InitTraceCount80015D18 = 7u;

static bool AppendRow0OverlayTransferCdSeamRequest80015D18(
    MainSceneCallbackExecution80015D18& out,
    const PrStage1LoaderCdHal::Action& action,
    const PrMovieSegmentDirect::OverlayTransferAttemptProvenance8001ACF8&
        provenance) {
    if (out.row0OverlayTransferCdSeamRequestCount >=
        kRow0OverlayTransferMaxCdSeamRequests80015D18) {
        out.row0OverlayTransferCdSeamRequestsTruncated = true;
        return false;
    }

    PrStage1LoaderDirect::CdSeamResult cd{};
    cd.present = true;
    cd.feedback.kind = action.kind;
    cd.feedback.handled = false;
    cd.lowerRequest =
        PrStage1LoaderCdHal::BuildLowerActionRequestMetadata(action);
    if (provenance.destKnown) {
        cd.dstPtrKnown = true;
        cd.dstPtr = provenance.dest;
    }
    if (provenance.sectorCountKnown) {
        cd.sectorCountKnown = true;
        cd.sectorCount = static_cast<int32_t>(provenance.sectorCount);
    }
    PrStage1MovieSegmentDirect::TagOverlayTransferAttemptCdSeam8001ACF8(
        cd,
        provenance);
    out.row0OverlayTransferCdSeamRequests
        [out.row0OverlayTransferCdSeamRequestCount++] = cd;
    return true;
}

static void CollectRow0OverlayTransferCdSeamRequests80015D18(
    MainSceneCallbackExecution80015D18& out) {
    const PrMovieSegmentDirect::OverlayTransferResult8001ACF8& transfer =
        out.overlayTransfer800154B0.sub8001ACF8;
    const uint32_t attemptCount =
        transfer.attemptCount <
                PrMovieSegmentDirect::kOverlayTransferMaxAttempts8001ACF8
            ? transfer.attemptCount
            : PrMovieSegmentDirect::kOverlayTransferMaxAttempts8001ACF8;

    for (uint32_t index = 0; index < attemptCount; ++index) {
        const PrMovieSegmentDirect::OverlayTransferAttempt8001ACF8& attempt =
            transfer.attempts[index];
        if (!attempt.executed) {
            continue;
        }

        const PrMovieSegmentDirect::MovieSegmentRecord48& attemptRow =
            attempt.rowInit.record;
        const PrMovieSegmentDirect::OverlayTransferAttemptProvenance8001ACF8
            provenance =
                PrMovieSegmentDirect::
                    BuildOverlayTransferAttemptProvenance8001ACF8(
                        attemptRow.psxAddr != 0u,
                        attemptRow.psxAddr,
                        true,
                        attempt.attemptIndex,
                        attempt.destKnown,
                        attempt.dest,
                        attempt.sectorCountKnown,
                        attempt.sectorCount);

        if (attempt.seekAction &&
            attempt.seek8001A89C.targetLbaKnown) {
            const uint32_t msfTargetPtr =
                attempt.seek8001A89C.syncArg1CdlPtrKnown
                    ? attempt.seek8001A89C.syncArg1CdlPtr
                    : 0u;
            AppendRow0OverlayTransferCdSeamRequest80015D18(
                out,
                PrStage1LoaderCdHal::MakeSeekSyncAction800367A4(
                    msfTargetPtr,
                    attempt.seek8001A89C.targetLba),
                provenance);
        }

        if (!attempt.readAction ||
            !attempt.destKnown ||
            !attempt.sectorCountKnown) {
            continue;
        }

        const int32_t sectorCount =
            static_cast<int32_t>(attempt.sectorCount);
        const int32_t modeFlag =
            PrStage1LoaderCdHal::BuildReadModeFlag8001A818(
                attempt.firstAttemptFlag);
        AppendRow0OverlayTransferCdSeamRequest80015D18(
            out,
            PrStage1LoaderCdHal::MakeReadStartAction80038FC0(
                sectorCount,
                attempt.dest,
                modeFlag),
            provenance);
        AppendRow0OverlayTransferCdSeamRequest80015D18(
            out,
            PrStage1LoaderCdHal::MakeReadSyncAction800390C8(),
            provenance);
    }
}

static bool Row0OverlayTransferHasGap80015D18(
    const PrMovieSegmentDirect::OverlayTransferResult800154B0& result) {
    return result.sub8001ACF8.gapMissingCdLookupFeedback ||
           result.sub8001ACF8.gapMissingSeekFeedback ||
           result.sub8001ACF8.gapMissingReadFeedback ||
           !result.sub8001ACF8.resultKnown;
}

static bool HasExplicitRow0OverlayTransferFeedback80015D18(
    const PrMovieSegmentDirect::OverlayTransferSeekFeedback8001A89C*
        seekFeedback,
    uint32_t seekFeedbackCount,
    const PrMovieSegmentDirect::OverlayTransferReadFeedback8001A818*
        readFeedback,
    uint32_t readFeedbackCount) {
    return (seekFeedback != nullptr && seekFeedbackCount != 0u) ||
           (readFeedback != nullptr && readFeedbackCount != 0u);
}

static bool FindOverlayTransferAttemptCarrier80015D18(
    const PrStage1LoaderDirect::CdSeamResult* seams,
    uint32_t seamCount,
    uint32_t attemptIndex,
    const PrStage1LoaderDirect::CdSeamResult*& outCarrier) {
    outCarrier = nullptr;
    if (seams == nullptr) {
        return false;
    }
    for (uint32_t index = 0; index < seamCount; ++index) {
        const PrStage1LoaderDirect::CdSeamResult& cd = seams[index];
        if (!cd.present || !cd.overlayTransferAttempt.known ||
            !cd.overlayTransferAttempt.attemptIndexKnown ||
            cd.overlayTransferAttempt.attemptIndex != attemptIndex) {
            continue;
        }
        outCarrier = &cd;
        return true;
    }
    return false;
}

enum class GenericSwitchTraceExecStatus80015788 : uint8_t {
    Completed = 0,
    Pending,
    Failed,
    DirectGap,
};

static void LoadGenericSwitchZCompo15590(PrGameContext& ctx,
                                         PrSceneTable& table,
                                         int32_t sceneIndex) {
    if (!ctx.resources ||
        sceneIndex < 0 ||
        sceneIndex >= static_cast<int32_t>(kPrSceneCount)) {
        return;
    }
    const PrSceneDef& def = table.Get(static_cast<PrSceneId>(
        static_cast<uint8_t>(sceneIndex)));
    if (def.zcompo.path.empty()) {
        return;
    }
    const std::filesystem::path zcompoPath = ctx.dataRoot / def.zcompo.path;
    std::error_code ec;
    if (!std::filesystem::exists(zcompoPath, ec)) {
        Log::Printf("80015788 bootstrap missing ZCOMPO path=%s",
                    zcompoPath.u8string().c_str());
        return;
    }
    const bool ok = ctx.resources->LoadIntArchive(zcompoPath.u8string());
    Log::Printf("80015788 bootstrap ZCOMPO path=%s ok=%d",
                zcompoPath.u8string().c_str(),
                ok ? 1 : 0);
}

static bool StartGenericSwitchDispatcher80015788(
    PrGameContext& ctx,
    const PrSceneEntryDirect::GenericSwitchRequest80015788& request,
    GenericSwitchHostState80015788& host) {
    if (SS0DirectOwnsGenericSwitchContext80015788(ctx)) {
        host = {};
        PrEvent::ClearDispatcherResidueForDirectCutover();
        Log::Printf(
            "80015788 dispatcher start rejected for direct SS0 scene=%u",
            static_cast<unsigned>(ctx.currentScene));
        return false;
    }
    if (!request.arg0Known ||
        request.arg0 < 0) {
        return false;
    }

    host.pendingEventId = static_cast<uint32_t>(request.arg0);
    host.pendingOutArg = -1;
    void* argPtr = nullptr;
    if (host.pendingEventId == 2u) {
        PrEvent::SetRecordsModePtr(&ctx.transitionStateDA);
        argPtr = &host.pendingOutArg;
    } else if (host.pendingEventId == 3u ||
               host.pendingEventId == 17u) {
        host.pendingD0 = static_cast<int16_t>(
            host.direct.word800916D0Known
                ? host.direct.word800916D0
                : static_cast<uint16_t>(ctx.transitionState));
        argPtr = &host.pendingD0;
    } else if (host.pendingEventId == 6u) {
        if (request.arg1Known && request.arg1 != 0) {
            if (request.arg1 != host.pendingEvent6ArgToken ||
                host.pendingEvent6ArgPtr == nullptr) {
                host.pendingEvent6ArgToken = 0;
                host.pendingEvent6ArgPtr = nullptr;
                Log::Printf("80015788 ev6 missing stable arg token=%d",
                            request.arg1);
                return false;
            }
            argPtr = host.pendingEvent6ArgPtr;
        }
    }

    if (!PrEvent::StartDispatcherEx(host.pendingEventId, argPtr, ctx)) {
        if (host.pendingEventId == 6u) {
            host.pendingEvent6ArgToken = 0;
            host.pendingEvent6ArgPtr = nullptr;
        }
        Log::Printf("80015788 dispatcher start blocked ev=%u",
                    host.pendingEventId);
        return false;
    }
    host.pending = GenericSwitchPendingHostCall80015788::Event26B94;
    return true;
}

static bool BuildPendingGenericSwitchFeedback80015788(
    PrGameContext& ctx,
    GenericSwitchHostState80015788& host,
    PrSceneEntryDirect::GenericSwitchFeedback80015788& feedback) {
    if (SS0DirectOwnsGenericSwitchContext80015788(ctx)) {
        host = {};
        PrEvent::ClearDispatcherResidueForDirectCutover();
        Log::Printf(
            "80015788 pending legacy feedback rejected for direct SS0 scene=%u",
            static_cast<unsigned>(ctx.currentScene));
        return false;
    }
    if (host.pending == GenericSwitchPendingHostCall80015788::None) {
        return true;
    }
    if (PrEvent::IsDispatcherRunning()) {
        return false;
    }

    const int result = PrEvent::ConsumeDispatcherResult();
    if (result == -1) {
        Log::Printf("80015788 pending host call has no dispatcher result ev=%u",
                    host.pendingEventId);
        return false;
    }
    switch (host.pending) {
    case GenericSwitchPendingHostCall80015788::Event26B94:
        feedback.event26B94ResultKnown = true;
        feedback.event26B94Result = result;
        if (host.pendingEventId == 2u) {
            feedback.event26B94OutArgKnown = true;
            feedback.event26B94OutArg = host.pendingOutArg;
        }
        if (host.pendingEventId == 3u || host.pendingEventId == 17u) {
            ctx.transitionState = host.pendingD0;
            feedback.word800916D0Written = true;
            feedback.word800916D0 = static_cast<uint16_t>(host.pendingD0);
        }
        if (host.pendingEventId == 6u) {
            host.pendingEvent6ArgToken = 0;
            host.pendingEvent6ArgPtr = nullptr;
        }
        break;
    case GenericSwitchPendingHostCall80015788::Replay193F4:
        feedback.call193F4ResultKnown = true;
        feedback.call193F4Result =
            result == 1 ? PrEvent::GetMemCardReplayResolvedSlot() : -1;
        break;
    case GenericSwitchPendingHostCall80015788::Practice2776C:
        feedback.call2776CDoneKnown = true;
        feedback.call2776CDone = true;
        break;
    case GenericSwitchPendingHostCall80015788::Save193B0:
        feedback.call193B0DoneKnown = true;
        feedback.call193B0Done = true;
        break;
    case GenericSwitchPendingHostCall80015788::None:
        break;
    }

    host.pending = GenericSwitchPendingHostCall80015788::None;
    host.pendingEventId = 0;
    return true;
}

static bool StartGenericSwitchLegacyDispatcher80015788(
    PrGameContext& ctx,
    GenericSwitchHostState80015788& host,
    uint32_t eventId,
    GenericSwitchPendingHostCall80015788 pending,
    const char* label) {
    if (SS0DirectOwnsGenericSwitchContext80015788(ctx)) {
        host = {};
        PrEvent::ClearDispatcherResidueForDirectCutover();
        Log::Printf(
            "80015788 %s legacy dispatcher rejected for direct SS0 scene=%u",
            label ? label : "unknown",
            static_cast<unsigned>(ctx.currentScene));
        return false;
    }
    if (!PrEvent::StartDispatcher(eventId, nullptr, ctx)) {
        Log::Printf("80015788 %s dispatcher start blocked",
                    label ? label : "unknown");
        return false;
    }
    host.pending = pending;
    host.pendingEventId = eventId;
    return true;
}

CompletedCall80019414Input80015788
BuildCompletedCall80019414InputFromFeedbackAdapterResult80019414(
    const PrSceneEntryFeedbackAdapterDirect::FeedbackAdapterResult80019414&
        adapter) {
    CompletedCall80019414Input80015788 out{};
    if (!adapter.completed ||
        adapter.gap ||
        adapter.gapReason !=
            PrSceneEntryFeedbackAdapterDirect::FeedbackAdapterGap80019414::
                None ||
        !adapter.feedback.gp720Known ||
        !adapter.feedback.call80019284ResultKnown ||
        adapter.tablePsxAddress !=
            static_cast<uint32_t>(adapter.feedback.call80019284Result) ||
        adapter.tableByteCount !=
            PrSceneEntryDirect::kHiScoreTableSize80019284) {
        return out;
    }

    out.completed = true;
    out.feedback = adapter.feedback;
    out.tablePsxAddress = adapter.tablePsxAddress;
    out.tableByteCount = adapter.tableByteCount;
    std::memcpy(out.tableStorage.data(),
                adapter.tableStorage,
                out.tableStorage.size());
    out.tableBytes = out.tableStorage.data();
    return out;
}

static bool LatchCompletedCall80019414Input80015788(
    GenericSwitchHostState80015788& host,
    const CompletedCall80019414Input80015788* input) {
    host.call19414FeedbackLatched = false;
    host.call19414Feedback =
        PrSceneEntryDirect::Call80019414Feedback80015788{};
    host.call19414TablePsxAddress = 0;
    host.call19414TableByteCount = 0;
    if (!input || !input->completed ||
        !input->feedback.gp720Known ||
        !input->feedback.call80019284ResultKnown ||
        input->tablePsxAddress !=
            static_cast<uint32_t>(input->feedback.call80019284Result) ||
        input->tableBytes == nullptr ||
        input->tableByteCount !=
            PrSceneEntryDirect::kHiScoreTableSize80019284) {
        return false;
    }

    std::memcpy(host.call19414TableStorage,
                input->tableBytes,
                PrSceneEntryDirect::kHiScoreTableSize80019284);
    host.call19414Feedback = input->feedback;
    host.call19414Feedback.call80019284HostArgPtr =
        host.call19414TableStorage;
    host.call19414TablePsxAddress = input->tablePsxAddress;
    host.call19414TableByteCount = input->tableByteCount;
    host.call19414FeedbackLatched = true;
    return true;
}

static GenericSwitchTraceExecStatus80015788 ExecuteGenericSwitchTrace80015788(
    PrGameContext& ctx,
    PrSceneTable& table,
    GenericSwitchHostState80015788& host,
    const PrSceneEntryDirect::GenericSwitchRequestTrace80015788& trace,
    PrSceneEntryDirect::GenericSwitchFeedback80015788& feedback,
    const CompletedCall80019414Input80015788* completedCall19414,
    bool& producedFeedback) {
    producedFeedback = false;
    if (trace.overflow) {
        Log::Printf("80015788 request trace overflow");
        return GenericSwitchTraceExecStatus80015788::Failed;
    }
    for (size_t i = 0; i < trace.count; ++i) {
        const PrSceneEntryDirect::GenericSwitchRequest80015788& request =
            trace.requests[i];
        using RequestKind =
            PrSceneEntryDirect::GenericSwitchRequestKind80015788;
        switch (request.kind) {
        case RequestKind::None:
            break;
        case RequestKind::Call80026FA4:
            PrSfx::ApplySharedAudioResetBarrier26FA4();
            break;
        case RequestKind::Call80026EF8:
            PrSfx::PlaySceneTransitionCue94410();
            break;
        case RequestKind::Call80026ECC:
            PrSfx::ApplySharedAudioDriverFlushBarrier26ECC();
            break;
        case RequestKind::Call80015590:
            if (request.arg0Known) {
                LoadGenericSwitchZCompo15590(ctx, table, request.arg0);
            }
            break;
        case RequestKind::Call80035510:
        {
            const PrPadState pad = PrPad::GetState(0);
            const uint16_t padReturnedMask =
                PrPsxPadDirect::BuildReturnedMask80035510FromLocalAndDebugPad(
                    pad.held,
                    ctx.debugPadInput);
            const PrPsxPadDirect::PadReadResult80035510 pad80035510 =
                PrPsxPadDirect::PsxReadPadMask80035510(padReturnedMask);
            feedback.poll35510Known = true;
            feedback.poll35510Result = pad80035510.psxReturnMask;
            producedFeedback = true;
            break;
        }
        case RequestKind::Call80026B94:
            if (!StartGenericSwitchDispatcher80015788(ctx, request, host)) {
                return GenericSwitchTraceExecStatus80015788::Failed;
            }
            return GenericSwitchTraceExecStatus80015788::Pending;
        case RequestKind::Call80019414:
        {
            if (!LatchCompletedCall80019414Input80015788(
                    host, completedCall19414)) {
                if (!host.call19414GapLogged) {
                    Log::Printf(
                        "80019414 direct gap: completed feedback missing");
                    host.call19414GapLogged = true;
                }
                return GenericSwitchTraceExecStatus80015788::DirectGap;
            }
            const PrSceneEntryDirect::Call80019414Result80015788 direct =
                PrSceneEntryDirect::PsxCall80019414_HiScoreEntry80015788(
                    request.arg0,
                    request.arg0Known,
                    host.call19414Feedback);
            if (!direct.resultKnown) {
                if (!host.call19414GapLogged) {
                    Log::Printf(
                        "80019414 direct gap gp720=%d sub80019284=%d",
                        direct.missingGp720 ? 0 : 1,
                        direct.missing80019284Result ? 0 : 1);
                    host.call19414GapLogged = true;
                }
                return GenericSwitchTraceExecStatus80015788::DirectGap;
            }
            feedback.call19414ResultKnown = true;
            feedback.call19414Result = direct.result;
            if (direct.result != 0) {
                host.pendingEvent6ArgToken = direct.result;
                host.pendingEvent6ArgPtr = direct.event6HostArgPtr;
            }
            producedFeedback = true;
            break;
        }
        case RequestKind::Call80015700:
        {
            const PrStage1SaveStatusBackupResult80015700 backup =
                PrStage1SaveUiDirect::Sub80015700(
                    PrStage1SaveStatusPrefix80092F10::kPsxAddress);
            if (!backup.ok) {
                if (!host.call15700GapLogged) {
                    Log::Printf(
                        "80015700 direct gap backupKnown=%d fault=%08X",
                        backup.backupKnown ? 1 : 0,
                        backup.lastFaultAddress);
                    host.call15700GapLogged = true;
                }
                return GenericSwitchTraceExecStatus80015788::DirectGap;
            }
            break;
        }
        case RequestKind::Call800193F4:
            if (!StartGenericSwitchLegacyDispatcher80015788(
                    ctx,
                    host,
                    9u,
                    GenericSwitchPendingHostCall80015788::Replay193F4,
                    "replay")) {
                return GenericSwitchTraceExecStatus80015788::Failed;
            }
            return GenericSwitchTraceExecStatus80015788::Pending;
        case RequestKind::Call800161A8:
            if (request.arg0Known) {
                int32_t scene = 0;
                if (!PrSS0StageProgressBankDirect::TryMapReplayScene800161A8(
                        request.arg0, &scene)) {
                    if (!host.call161A8GapLogged) {
                        Log::Printf(
                            "800161A8 direct gap selector=%d",
                            request.arg0);
                        host.call161A8GapLogged = true;
                    }
                    return GenericSwitchTraceExecStatus80015788::DirectGap;
                }
                feedback.call161A8ResultKnown = true;
                feedback.call161A8Result = scene;
                producedFeedback = true;
            }
            break;
        case RequestKind::Call8002776C:
            if (!StartGenericSwitchLegacyDispatcher80015788(
                    ctx,
                    host,
                    5u,
                    GenericSwitchPendingHostCall80015788::Practice2776C,
                    "practice")) {
                return GenericSwitchTraceExecStatus80015788::Failed;
            }
            return GenericSwitchTraceExecStatus80015788::Pending;
        case RequestKind::Call800193B0:
            if (!StartGenericSwitchLegacyDispatcher80015788(
                    ctx,
                    host,
                    8u,
                    GenericSwitchPendingHostCall80015788::Save193B0,
                    "load")) {
                return GenericSwitchTraceExecStatus80015788::Failed;
            }
            return GenericSwitchTraceExecStatus80015788::Pending;
        }
    }
    return GenericSwitchTraceExecStatus80015788::Completed;
}

static PrMovieSegmentDirect::MovieSegmentRecord48
BuildMovieSegmentRecordFromSceneEntryRawRow80015D18(
    const PrSceneEntryDirect::SceneEntryRawRow& raw,
    uint32_t rowIndex,
    uint32_t rowAddr,
    bool rowAddrKnown) {
    PrMovieSegmentDirect::MovieSegmentRecord48 out{};
    out.known = raw.known || rowAddrKnown;
    out.tableIndex = rowIndex;
    if (rowAddrKnown) {
        out.psxAddr = rowAddr;
    }
    out.pathPtrA1Plus00Known = raw.pathPtrKnown;
    out.pathPtrA1Plus00 = raw.pathPtr;
    out.opaqueA1Plus04Known = raw.opaque04Known;
    out.opaqueA1Plus04 = raw.opaque04;
    out.endBiasA1Plus8Known = raw.endBias08Known;
    out.endBiasA1Plus8 = raw.endBias08;
    out.loadedStateA1Plus0CKnown = raw.loadedState0CKnown;
    out.loadedStateA1Plus0C = raw.loadedState0C;
    return out;
}

}  // namespace

CompletedCall80019414Input80015788
BuildCompletedCall80019414InputFromFeedbackAdapterResult80019414(
    const PrSceneEntryFeedbackAdapterDirect::FeedbackAdapterResult80019414&
        adapter) {
    CompletedCall80019414Input80015788 out{};
    if (!adapter.completed ||
        adapter.gap ||
        adapter.gapReason !=
            PrSceneEntryFeedbackAdapterDirect::FeedbackAdapterGap80019414::
                None ||
        !adapter.feedback.gp720Known ||
        !adapter.feedback.call80019284ResultKnown ||
        adapter.tablePsxAddress !=
            static_cast<uint32_t>(adapter.feedback.call80019284Result) ||
        adapter.tableByteCount !=
            PrSceneEntryDirect::kHiScoreTableSize80019284) {
        return out;
    }

    out.completed = true;
    out.feedback = adapter.feedback;
    out.tablePsxAddress = adapter.tablePsxAddress;
    out.tableByteCount = adapter.tableByteCount;
    std::memcpy(out.tableStorage.data(),
                adapter.tableStorage,
                out.tableStorage.size());
    out.tableBytes = out.tableStorage.data();
    return out;
}

MainSceneTraceExecution80015D18
ExecuteMainSceneHostTrace80015D18(
    const PrSceneEntryDirect::MainSceneRequestTrace80015D18& trace,
    bool executeSceneSwitchRequests,
    bool executeSceneCallbackRequests) {
    MainSceneTraceExecution80015D18 out{};
    out.traceOverflow = trace.overflow;
    using Kind = PrSceneEntryDirect::MainSceneRequestKind80015D18;

    for (size_t i = 0; i < trace.count; ++i) {
        const PrSceneEntryDirect::MainSceneRequest80015D18& request =
            trace.requests[i];
        if (!request.valid) {
            continue;
        }
        switch (request.kind) {
        case Kind::None:
            break;
        case Kind::Call8001EF14:
            PrTransition::ResetHoldOverlayState1EF14();
            out.call8001EF14Executed = true;
            break;
        case Kind::Call80015CC4:
        {
            const PrStage1SavePayloadProducerResult result =
                PrStage1SaveUiDirect::Sub80015CC4();
            out.call80015CC4Executed = true;
            out.call80015CC4Ok = result.ok;
            Log::Printf(
                "MainScene 80015D18 Call80015CC4 executed ok=%d payloadKnown=%d helperGap=%d result=%d",
                result.ok ? 1 : 0,
                result.payloadKnown ? 1 : 0,
                result.helperGap ? 1 : 0,
                result.result);
            break;
        }
        case Kind::Call8001E34C:
            out.drawBuffers8001E34C =
                PrSceneDrawBufferDirect::PsxCall8001E34C_SetMainDrawBuffers();
            out.call8001E34CExecuted = out.drawBuffers8001E34C.known;
            out.call8001E34CGap = !out.drawBuffers8001E34C.known;
            break;
        case Kind::Call80015788:
            if (executeSceneSwitchRequests) {
                out.unsupportedRequest = true;
            }
            break;
        case Kind::Call80025A34:
        case Kind::Call8001A324:
        case Kind::Call800154B0:
        case Kind::CallSceneFn0:
        case Kind::CallSceneFn1:
        case Kind::CallSceneFn2:
            if (executeSceneCallbackRequests) {
                out.unsupportedRequest = true;
            }
            break;
        }
    }
    return out;
}

bool BuildRow0OverlayTransferFeedbackFromCdSeams80015D18(
    const PrStage1LoaderDirect::CdSeamResult* seams,
    uint32_t seamCount,
    Row0OverlayTransferFeedbackFromCdSeams80015D18& out) {
    out = Row0OverlayTransferFeedbackFromCdSeams80015D18{};
    if (seams == nullptr || seamCount == 0u) {
        return false;
    }

    bool anyKnown = false;
    for (uint32_t attemptIndex = 0;
         attemptIndex <
             PrMovieSegmentDirect::kOverlayTransferMaxAttempts8001ACF8;
         ++attemptIndex) {
        const PrStage1LoaderDirect::CdSeamResult* carrier = nullptr;
        if (!FindOverlayTransferAttemptCarrier80015D18(
                seams,
                seamCount,
                attemptIndex,
                carrier) ||
            carrier == nullptr) {
            continue;
        }

        std::array<PrStage1LoaderDirect::CdSeamResult,
                   kRow0OverlayTransferMaxCdSeamRequests80015D18>
            attemptSeams{};
        uint32_t attemptSeamCount = 0;
        for (uint32_t seamIndex = 0; seamIndex < seamCount; ++seamIndex) {
            const PrStage1LoaderDirect::CdSeamResult& cd = seams[seamIndex];
            if (!cd.present || !cd.overlayTransferAttempt.known ||
                !cd.overlayTransferAttempt.attemptIndexKnown ||
                cd.overlayTransferAttempt.attemptIndex != attemptIndex) {
                continue;
            }
            if (attemptSeamCount >= attemptSeams.size()) {
                break;
            }
            attemptSeams[attemptSeamCount++] = cd;
        }
        if (attemptSeamCount == 0u) {
            continue;
        }

        PrMovieSegmentDirect::OverlayTransferSeekFeedback8001A89C seek{};
        PrMovieSegmentDirect::OverlayTransferReadFeedback8001A818 read{};
        const bool built =
            PrStage1MovieSegmentDirect::
                BuildOverlayTransferAttemptFeedbackFromCdSeams8001ACF8(
                    attemptSeams.data(),
                    attemptSeamCount,
                    carrier->overlayTransferAttempt.dst,
                    carrier->overlayTransferAttempt.dstKnown,
                    carrier->overlayTransferAttempt.sectorCount,
                    carrier->overlayTransferAttempt.sectorCountKnown,
                    attemptIndex == 0u ? 1 : 0,
                    seek,
                    read);
        if (!built) {
            continue;
        }
        const PrMovieSegmentDirect::OverlayTransferAttemptProvenance8001ACF8
            provenance =
                PrMovieSegmentDirect::
                    BuildOverlayTransferAttemptProvenance8001ACF8(
                        carrier->overlayTransferAttempt.rowAddrKnown,
                        carrier->overlayTransferAttempt.rowAddr,
                        true,
                        attemptIndex,
                        carrier->overlayTransferAttempt.dstKnown,
                        carrier->overlayTransferAttempt.dst,
                        carrier->overlayTransferAttempt.sectorCountKnown,
                        carrier->overlayTransferAttempt.sectorCount);
        if (seek.known) {
            PrMovieSegmentDirect::
                AttachOverlayTransferAttemptProvenance8001ACF8(
                    provenance,
                    seek);
            out.seekFeedback[attemptIndex] = seek;
            if (out.seekFeedbackCount <= attemptIndex) {
                out.seekFeedbackCount = attemptIndex + 1u;
            }
            anyKnown = true;
        }
        if (read.known) {
            PrMovieSegmentDirect::
                AttachOverlayTransferAttemptProvenance8001ACF8(
                    provenance,
                    read);
            out.readFeedback[attemptIndex] = read;
            if (out.readFeedbackCount <= attemptIndex) {
                out.readFeedbackCount = attemptIndex + 1u;
            }
            anyKnown = true;
        }
    }
    return anyKnown;
}

bool BuildScene1Row0OverlayTransferFeedback80015D18(
    PrGameContext& ctx,
    PrSceneId scene,
    PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324& out) {
    return scene == PrSceneId::Scene1 &&
           PrScn1::BuildScene1Row0OverlayTransferFeedback800154B0(ctx, out);
}

static Row0OverlayTransferCdSeamRejectReason80015D18
GetRow0OverlayTransferCdSeamRejectReason80015D18(
    const PrStage1LoaderDirect::CdSeamResult& cd);

static bool HasAcceptedRow0OverlayTransferCdSeamFields80015D18(
    const PrStage1LoaderDirect::CdSeamResult& cd);

static bool IsCompletedRow0OverlayTransferCdSeam80015D18(
    const PrStage1LoaderDirect::CdSeamResult& cd);

static bool RunRow0OverlayTransferCdSeamProducer80015D18(
    const Row0OverlayTransferCdSeamProducer80015D18* producer,
    PrGameContext& ctx,
    bool sceneIndexKnown,
    uint32_t sceneIndex,
    const PrMovieSegmentDirect::MovieSegmentRecord48& row,
    const PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324&
        rowFeedback,
    MainSceneCallbackExecution80015D18& out) {
    if (producer == nullptr || producer->produce == nullptr ||
        out.row0OverlayTransferCdSeamRequestCount == 0u) {
        return false;
    }

    Row0OverlayTransferCdSeamProducerInput80015D18 input{};
    input.ctx = &ctx;
    input.sceneIndexKnown = sceneIndexKnown;
    input.sceneIndex = sceneIndex;
    input.row = row;
    input.rowFeedback = rowFeedback;
    input.discovery = out.overlayTransfer800154B0;
    input.requests = out.row0OverlayTransferCdSeamRequests.data();
    input.requestCount = out.row0OverlayTransferCdSeamRequestCount;
    input.requestsTruncated =
        out.row0OverlayTransferCdSeamRequestsTruncated;

    Row0OverlayTransferCdSeamProducerOutput80015D18 output{};
    out.row0OverlayTransferCdSeamProducerCalled = true;
    if (!producer->produce(input, output, producer->user) ||
        output.seamCount == 0u) {
        return false;
    }

    const uint32_t outputSeamCount =
        output.seamCount <
                kRow0OverlayTransferMaxCdSeamRequests80015D18
            ? output.seamCount
            : kRow0OverlayTransferMaxCdSeamRequests80015D18;
    for (uint32_t index = 0; index < outputSeamCount; ++index) {
        const PrStage1LoaderDirect::CdSeamResult& cd =
            output.seams[index];
        const Row0OverlayTransferCdSeamRejectReason80015D18 rejectReason =
            GetRow0OverlayTransferCdSeamRejectReason80015D18(cd);
        if (rejectReason !=
            Row0OverlayTransferCdSeamRejectReason80015D18::None) {
            ++out.row0OverlayTransferCompletedCdSeamRejectedCount;
            out.row0OverlayTransferLastRejectedCdSeamIndex = index;
            out.row0OverlayTransferLastRejectedCdSeamReason = rejectReason;
            out.row0OverlayTransferLastRejectedCdSeam = cd;
            continue;
        }
        if (out.row0OverlayTransferCompletedCdSeamCount >=
            kRow0OverlayTransferMaxCdSeamRequests80015D18) {
            out.row0OverlayTransferCompletedCdSeamsTruncated = true;
            break;
        }
        out.row0OverlayTransferCompletedCdSeams
            [out.row0OverlayTransferCompletedCdSeamCount++] = cd;
    }
    out.row0OverlayTransferCompletedCdSeamsTruncated =
        out.row0OverlayTransferCompletedCdSeamsTruncated ||
        output.seamsTruncated ||
        output.seamCount >
            kRow0OverlayTransferMaxCdSeamRequests80015D18;
    out.row0OverlayTransferCdSeamProducerProduced =
        out.row0OverlayTransferCompletedCdSeamCount != 0u;
    return out.row0OverlayTransferCdSeamProducerProduced;
}

static Row0OverlayTransferCdSeamRejectReason80015D18
GetRow0OverlayTransferCdSeamRejectReason80015D18(
    const PrStage1LoaderDirect::CdSeamResult& cd) {
    if (HasAcceptedRow0OverlayTransferCdSeamFields80015D18(cd)) {
        return Row0OverlayTransferCdSeamRejectReason80015D18::None;
    }
    if (!cd.present) {
        return Row0OverlayTransferCdSeamRejectReason80015D18::NotPresent;
    }
    if (!cd.feedback.handled) {
        return Row0OverlayTransferCdSeamRejectReason80015D18::Unhandled;
    }
    if (!cd.overlayTransferAttempt.known ||
        !cd.overlayTransferAttempt.attemptIndexKnown) {
        if (!cd.overlayTransferAttempt.known) {
            return Row0OverlayTransferCdSeamRejectReason80015D18::
                MissingAttemptProvenance;
        }
        return Row0OverlayTransferCdSeamRejectReason80015D18::
            MissingAttemptIndex;
    }
    if (cd.overlayTransferAttempt.sourceFunction !=
            PrMovieSegmentDirect::kSub800154B0OverlayTransferWrapper ||
        cd.overlayTransferAttempt.transferFunction !=
            PrMovieSegmentDirect::kSub8001ACF8OverlayTransfer) {
        return Row0OverlayTransferCdSeamRejectReason80015D18::
            AttemptRouteMismatch;
    }
    if (!cd.lowerRequest.known) {
        return Row0OverlayTransferCdSeamRejectReason80015D18::
            MissingLowerRequest;
    }
    switch (cd.feedback.kind) {
    case PrStage1LoaderCdHal::ActionKind::SeekSync800367A4:
        if (cd.lowerRequest.actionKind !=
                PrStage1LoaderCdHal::ActionKind::SeekSync800367A4) {
            return Row0OverlayTransferCdSeamRejectReason80015D18::
                LowerRequestKindMismatch;
        }
        if (cd.lowerRequest.callerFunction !=
                PrStage1LoaderCdHal::kFn8001A89C ||
            cd.lowerRequest.directHelperFunction !=
                PrStage1LoaderCdHal::kFn80036974 ||
            cd.lowerRequest.lowerFunction !=
                PrStage1LoaderCdHal::kFn800367A4) {
            return Row0OverlayTransferCdSeamRejectReason80015D18::
                LowerRequestRouteMismatch;
        }
        if (!cd.lowerRequest.seekRequestKnown ||
            !cd.lowerRequest.seekMsfTargetPtrKnown ||
            !cd.lowerRequest.seekLbaKnown ||
            !cd.lowerRequest.seekMsfTargetKnown ||
            cd.lowerRequest.seekArg0 !=
                PrStage1LoaderCdHal::kSeek8001A89CSyncMode ||
            cd.lowerRequest.seekArg1 !=
                static_cast<int32_t>(cd.lowerRequest.seekMsfTargetPtr) ||
            cd.lowerRequest.seekArg2 !=
                PrStage1LoaderCdHal::kSeek8001A89CSyncArg2) {
            return Row0OverlayTransferCdSeamRejectReason80015D18::
                LowerRequestIncomplete;
        }
        if (cd.dstPtrKnown &&
            cd.lowerRequest.seekMsfTargetPtrKnown &&
            cd.dstPtr != cd.lowerRequest.seekMsfTargetPtr) {
            return Row0OverlayTransferCdSeamRejectReason80015D18::
                AttemptPayloadMismatch;
        }
        return Row0OverlayTransferCdSeamRejectReason80015D18::None;
    case PrStage1LoaderCdHal::ActionKind::ReadStart80038FC0:
        if (cd.lowerRequest.actionKind !=
                PrStage1LoaderCdHal::ActionKind::ReadStart80038FC0) {
            return Row0OverlayTransferCdSeamRejectReason80015D18::
                LowerRequestKindMismatch;
        }
        if (cd.lowerRequest.callerFunction !=
                PrStage1LoaderCdHal::kFn8001A818 ||
            cd.lowerRequest.lowerFunction !=
                PrStage1LoaderCdHal::kFn80038FC0 ||
            cd.lowerRequest.finalFunction !=
                PrStage1LoaderCdHal::kFn800390C8) {
            return Row0OverlayTransferCdSeamRejectReason80015D18::
                LowerRequestRouteMismatch;
        }
        if (!cd.lowerRequest.readStartRequestKnown ||
            !cd.lowerRequest.readStartDstPtrKnown ||
            !cd.lowerRequest.readStartSectorCountKnown ||
            !cd.lowerRequest.readStartModeFlagKnown ||
            !cd.lowerRequest.readSyncRequestKnown ||
            cd.lowerRequest.readSyncFunction !=
                PrStage1LoaderCdHal::kFn800390C8 ||
            cd.lowerRequest.readSyncArg0 !=
                PrStage1LoaderCdHal::kRead8001A818SyncArg0 ||
            cd.lowerRequest.readSyncArg1 !=
                PrStage1LoaderCdHal::kRead8001A818SyncArg1) {
            return Row0OverlayTransferCdSeamRejectReason80015D18::
                LowerRequestIncomplete;
        }
        if (cd.dstPtrKnown &&
            cd.lowerRequest.readStartDstPtrKnown &&
            cd.dstPtr != cd.lowerRequest.readStartDstPtr) {
            return Row0OverlayTransferCdSeamRejectReason80015D18::
                AttemptPayloadMismatch;
        }
        if (cd.sectorCountKnown &&
            cd.lowerRequest.readStartSectorCountKnown &&
            cd.sectorCount != cd.lowerRequest.readStartSectorCount) {
            return Row0OverlayTransferCdSeamRejectReason80015D18::
                AttemptPayloadMismatch;
        }
        return Row0OverlayTransferCdSeamRejectReason80015D18::None;
    case PrStage1LoaderCdHal::ActionKind::ReadSync800390C8:
        if (cd.lowerRequest.actionKind !=
                PrStage1LoaderCdHal::ActionKind::ReadSync800390C8) {
            return Row0OverlayTransferCdSeamRejectReason80015D18::
                LowerRequestKindMismatch;
        }
        if (cd.lowerRequest.callerFunction !=
                PrStage1LoaderCdHal::kFn8001A818 ||
            cd.lowerRequest.lowerFunction !=
                PrStage1LoaderCdHal::kFn800390C8 ||
            cd.lowerRequest.finalFunction !=
                PrStage1LoaderCdHal::kFn800364F0) {
            return Row0OverlayTransferCdSeamRejectReason80015D18::
                LowerRequestRouteMismatch;
        }
        if (!cd.lowerRequest.readSyncRequestKnown ||
            cd.lowerRequest.readSyncFunction !=
                PrStage1LoaderCdHal::kFn800390C8 ||
            cd.lowerRequest.readSyncArg0 !=
                PrStage1LoaderCdHal::kRead8001A818SyncArg0 ||
            cd.lowerRequest.readSyncArg1 !=
                PrStage1LoaderCdHal::kRead8001A818SyncArg1) {
            return Row0OverlayTransferCdSeamRejectReason80015D18::
                LowerRequestIncomplete;
        }
        return Row0OverlayTransferCdSeamRejectReason80015D18::None;
    default:
        return Row0OverlayTransferCdSeamRejectReason80015D18::UnsupportedKind;
    }
}

static bool IsCompletedRow0OverlayTransferCdSeam80015D18(
    const PrStage1LoaderDirect::CdSeamResult& cd) {
    return HasAcceptedRow0OverlayTransferCdSeamFields80015D18(cd);
}

static bool HasAcceptedRow0OverlayTransferCdSeamFields80015D18(
    const PrStage1LoaderDirect::CdSeamResult& cd) {
    if (!cd.present || !cd.feedback.handled ||
        !cd.overlayTransferAttempt.known ||
        !cd.overlayTransferAttempt.attemptIndexKnown) {
        return false;
    }
    switch (cd.feedback.kind) {
    case PrStage1LoaderCdHal::ActionKind::SeekSync800367A4:
    case PrStage1LoaderCdHal::ActionKind::ReadStart80038FC0:
    case PrStage1LoaderCdHal::ActionKind::ReadSync800390C8:
        return true;
    default:
        return false;
    }
}

static bool SceneCallbackFunctionMatches80015D18(
    const PrSceneEntryDirect::MainSceneRequest80015D18& request,
    uint32_t hostPsxFunction) {
    return request.psxFunction != 0u &&
           hostPsxFunction != 0u &&
           request.psxFunction == hostPsxFunction;
}

static bool MainSceneRequestExactlyMatches80015D18(
    const PrSceneEntryDirect::MainSceneRequest80015D18& actual,
    const PrSceneEntryDirect::MainSceneRequest80015D18& expected) {
    return actual.valid == expected.valid &&
           actual.kind == expected.kind &&
           actual.psxFunction == expected.psxFunction &&
           actual.psxFunctionSlotKnown == expected.psxFunctionSlotKnown &&
           actual.psxFunctionSlot == expected.psxFunctionSlot &&
           actual.arg0Known == expected.arg0Known &&
           actual.arg0 == expected.arg0 &&
           actual.arg1Known == expected.arg1Known &&
           actual.arg1 == expected.arg1;
}

static bool IsExactDirectScene1InitTrace80015D18(
    PrSceneId scene,
    const PrSceneEntryDirect::MainSceneStepResult80015D18& step,
    const PrSceneDef& def) {
    if (scene != PrSceneId::Scene1 ||
        !step.state.currentSceneV0Known ||
        step.state.currentSceneV0 !=
            static_cast<int32_t>(PrSceneId::Scene1) ||
        step.trace.overflow ||
        step.trace.count != kDirectScene1InitTraceCount80015D18 ||
        step.invalidCurrentSceneIndex ||
        step.missingWord800916D0For15CC4Gate ||
        !step.waitingForSceneFn2Result ||
        def.fn0 == nullptr ||
        def.fn1 == nullptr ||
        def.fn2 == nullptr) {
        return false;
    }

    const PrSceneEntryDirect::MainSceneStepResult80015D18 expected =
        PrSceneEntryDirect::PrepareMainSceneCallbacks80015D18(step.state);
    if (expected.trace.overflow ||
        expected.trace.count != kDirectScene1InitTraceCount80015D18 ||
        !expected.waitingForSceneFn2Result) {
        return false;
    }
    for (size_t index = 0; index < expected.trace.count; ++index) {
        if (!MainSceneRequestExactlyMatches80015D18(
                step.trace.requests[index],
                expected.trace.requests[index])) {
            return false;
        }
    }

    const PrSceneEntryDirect::SceneCallbackTriplet80048D28 callbacks =
        PrSceneEntryDirect::GetSceneCallbackTriplet80048D28(
            static_cast<uint32_t>(PrSceneId::Scene1));
    return callbacks.known &&
           def.psxFn0 == callbacks.fn0 &&
           def.psxFn1 == callbacks.fn1 &&
           def.psxFn2 == callbacks.fn2;
}

static bool IsExactDirectScene1Row0Feedback80015D18(
    const PrSceneEntryDirect::MainSceneStepResult80015D18& step,
    const PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324*
        feedback) {
    if (feedback == nullptr ||
        !PrScene1EntryOriginalDiscDirect::
            IsExactInitRow0Feedback80015D18(*feedback)) {
        return false;
    }

    const auto& request = step.trace.requests[1];
    const PrSceneEntryDirect::SceneEntryRawRow raw =
        PrSceneEntryDirect::GetSceneEntryStaticRawRow(
            static_cast<uint32_t>(PrSceneId::Scene1),
            0u);
    if (!request.arg0Known ||
        request.arg0 != static_cast<int32_t>(
                            PrScene1EntryOriginalDiscDirect::
                                kRowAddress80015D18) ||
        !raw.known || !raw.pathPtrKnown || !raw.loadedState0CKnown) {
        return false;
    }
    const uint32_t rowAddr = static_cast<uint32_t>(request.arg0);
    const uint32_t cdlFilePtr = rowAddr + 0x10u;
    return feedback->lookupRequestCdlFilePtr == cdlFilePtr &&
           feedback->lookupRequestPathPtr == raw.pathPtr &&
           feedback->lookupResultPtr == cdlFilePtr &&
           feedback->loadedStateA1Plus0C == raw.loadedState0C;
}

static int CompletedCdSeamKindIndex80015D18(
    PrStage1LoaderCdHal::ActionKind kind) {
    switch (kind) {
    case PrStage1LoaderCdHal::ActionKind::SeekSync800367A4:
        return 0;
    case PrStage1LoaderCdHal::ActionKind::ReadStart80038FC0:
        return 1;
    case PrStage1LoaderCdHal::ActionKind::ReadSync800390C8:
        return 2;
    default:
        return -1;
    }
}

static bool BuildExactDirectScene1Row0TransferFeedback80015D18(
    const PrSceneEntryDirect::MainSceneStepResult80015D18& step,
    const PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324&
        rowFeedback,
    const PrStage1LoaderDirect::CdSeamResult* seams,
    uint32_t seamCount,
    Row0OverlayTransferFeedbackFromCdSeams80015D18& out) {
    if (seams == nullptr || seamCount == 0u ||
        seamCount > kRow0OverlayTransferMaxCdSeamRequests80015D18) {
        return false;
    }

    const uint32_t rowAddr =
        static_cast<uint32_t>(step.trace.requests[1].arg0);
    const uint32_t sectorCount =
        (rowFeedback.cdlFileSize + 2047u) >> 11;
    const PrMovieSegmentDirect::MsfToLbaResult80036A78 startLba =
        PrMovieSegmentDirect::PsxCall80036A78_MsfToLba(
            rowFeedback.cdlFilePos);
    if (!startLba.known) {
        return false;
    }
    bool seen[PrMovieSegmentDirect::kOverlayTransferMaxAttempts8001ACF8][3]{};
    for (uint32_t index = 0; index < seamCount; ++index) {
        const PrStage1LoaderDirect::CdSeamResult& cd = seams[index];
        const int kindIndex = CompletedCdSeamKindIndex80015D18(
            cd.feedback.kind);
        bool exactLowerRoute = false;
        if (cd.feedback.kind ==
            PrStage1LoaderCdHal::ActionKind::SeekSync800367A4) {
            PrMovieSegmentDirect::OverlayTransferSeekFeedback8001A89C
                seek{};
            exactLowerRoute =
                PrStage1MovieSegmentDirect::
                    BuildOverlayTransferSeekFeedbackFromCdSeam8001A89C(
                        cd,
                        PrMovieSegmentDirect::
                            kOverlayTransferDest801C3870,
                        true,
                        sectorCount,
                        true,
                        seek);
            exactLowerRoute =
                exactLowerRoute &&
                cd.lowerRequest.seekMsfTargetPtr == rowAddr + 0x10u &&
                cd.lowerRequest.seekLba == startLba.lba &&
                cd.lowerRequest.seekMsfTarget.minute ==
                    rowFeedback.cdlFilePos.minute &&
                cd.lowerRequest.seekMsfTarget.second ==
                    rowFeedback.cdlFilePos.second &&
                cd.lowerRequest.seekMsfTarget.frame ==
                    rowFeedback.cdlFilePos.frame;
        } else {
            PrMovieSegmentDirect::OverlayTransferReadLowerFeedback8001A818
                read{};
            exactLowerRoute =
                PrStage1MovieSegmentDirect::
                    BuildOverlayTransferReadLowerFeedbackFromCdSeams8001A818(
                        &cd,
                        1u,
                        PrMovieSegmentDirect::
                            kOverlayTransferDest801C3870,
                        true,
                        sectorCount,
                        true,
                        read);
        }
        if (kindIndex < 0 ||
            !exactLowerRoute ||
            GetRow0OverlayTransferCdSeamRejectReason80015D18(cd) !=
                Row0OverlayTransferCdSeamRejectReason80015D18::None ||
            !cd.overlayTransferAttempt.rowAddrKnown ||
            cd.overlayTransferAttempt.rowAddr != rowAddr ||
            !cd.overlayTransferAttempt.dstKnown ||
            cd.overlayTransferAttempt.dst !=
                PrMovieSegmentDirect::kOverlayTransferDest801C3870 ||
            !cd.overlayTransferAttempt.sectorCountKnown ||
            cd.overlayTransferAttempt.sectorCount != sectorCount ||
            cd.overlayTransferAttempt.attemptIndex >=
                PrMovieSegmentDirect::kOverlayTransferMaxAttempts8001ACF8 ||
            seen[cd.overlayTransferAttempt.attemptIndex][kindIndex]) {
            return false;
        }
        if (cd.feedback.kind ==
                PrStage1LoaderCdHal::ActionKind::ReadStart80038FC0 &&
            cd.lowerRequest.readStartModeFlag !=
                static_cast<int32_t>(
                    PrStage1LoaderCdHal::BuildReadModeFlag8001A818(
                        cd.overlayTransferAttempt.attemptIndex == 0u
                            ? 1
                            : 0))) {
            return false;
        }
        seen[cd.overlayTransferAttempt.attemptIndex][kindIndex] = true;
    }
    for (uint32_t attempt = 0;
         attempt < PrMovieSegmentDirect::kOverlayTransferMaxAttempts8001ACF8;
         ++attempt) {
        const bool any = seen[attempt][0] || seen[attempt][1] ||
                         seen[attempt][2];
        if (any &&
            (!seen[attempt][0] || !seen[attempt][1] ||
             !seen[attempt][2])) {
            return false;
        }
    }
    if (!seen[0][0] || !seen[0][1] || !seen[0][2]) {
        return false;
    }
    return BuildRow0OverlayTransferFeedbackFromCdSeams80015D18(
        seams,
        seamCount,
        out);
}

static bool HasCompletedDirectScene1Row0Prefix80015D18(
    const MainSceneCallbackExecution80015D18& out) {
    const auto& transfer = out.overlayTransfer800154B0;
    return !out.traceOverflow &&
           !out.unsupportedRequest &&
           !out.invalidCurrentSceneIndex &&
           !out.missingWord800916D0For15CC4Gate &&
           !out.call80025A34Gap &&
           !out.call80025A34StateMissing &&
           out.call80025A34Executed &&
           out.call80025A34StateApplied &&
           out.call80025A34Count == 2u &&
           out.call80025A34Function == kFn80025A34 &&
           out.call80025A34TailFunction == kFn80025A00 &&
           out.call8001A324Executed &&
           out.call8001A324Count == 1u &&
           !out.call8001A324Gap &&
           out.row0Init8001A324.result == 0 &&
           out.row0Init8001A324.cdLookupSucceeded &&
           out.call800154B0Executed &&
           out.call800154B0Count == 1u &&
           !out.call800154B0Gap &&
           !out.row0OverlayTransferCdSeamRequestsTruncated &&
           transfer.called &&
           transfer.sourceFunction ==
               PrMovieSegmentDirect::kSub800154B0OverlayTransferWrapper &&
           transfer.rowArgKnown &&
           transfer.ignoredArg1Known &&
           transfer.ignoredArg1 == 0 &&
           transfer.destKnown &&
           transfer.dest ==
               PrMovieSegmentDirect::kOverlayTransferDest801C3870 &&
           transfer.sub8001ACF8.called &&
           transfer.sub8001ACF8.sourceFunction ==
               PrMovieSegmentDirect::kSub8001ACF8OverlayTransfer &&
           transfer.sub8001ACF8.resultKnown &&
           transfer.sub8001ACF8.psxReturn;
}

bool BuildRow0OverlayTransferCdSeamProducerOutputFromExplicitLowerResult80015D18(
    const PrStage1XaCdDirectLowerCdProducerResult& lower,
    Row0OverlayTransferCdSeamProducerOutput80015D18& out) {
    out = Row0OverlayTransferCdSeamProducerOutput80015D18{};
    if (!lower.called ||
        !lower.cdSeamResultAccepted ||
        lower.cdSeamResultRejected ||
        !IsCompletedRow0OverlayTransferCdSeam80015D18(
            lower.cdSeamResult)) {
        return false;
    }

    out.seamCount = 1u;
    out.seams[0] = lower.cdSeamResult;
    return true;
}

static MainSceneCallbackExecution80015D18
ExecuteMainSceneCallbacksImpl80015D18(
    const PrSceneEntryDirect::MainSceneStepResult80015D18& step,
    PrGameContext& ctx,
    const PrSceneDef& def,
    bool executeFn0Fn1,
    bool executeFn2,
    const PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324*
        row0Feedback8001A324,
    const PrMovieSegmentDirect::OverlayTransferSeekFeedback8001A89C*
        row0SeekFeedback8001A89C,
    uint32_t row0SeekFeedbackCount8001A89C,
    const PrMovieSegmentDirect::OverlayTransferReadFeedback8001A818*
        row0ReadFeedback8001A818,
    uint32_t row0ReadFeedbackCount8001A818,
    PrStage1LoaderMemoryDirectState* loaderMemoryState80025A34,
    const Row0OverlayTransferCdSeamProducer80015D18*
        row0CdSeamProducer8001ACF8,
    bool requireDirectScene1Row0PrefixBeforeFn0Fn1) {
    MainSceneCallbackExecution80015D18 out{};
    const PrSceneEntryDirect::MainSceneRequestTrace80015D18& trace =
        step.trace;
    out.traceOverflow = trace.overflow;
    out.invalidCurrentSceneIndex = step.invalidCurrentSceneIndex;
    out.missingWord800916D0For15CC4Gate =
        step.missingWord800916D0For15CC4Gate;
    out.waitingForSceneFn2Result = step.waitingForSceneFn2Result;
    using Kind = PrSceneEntryDirect::MainSceneRequestKind80015D18;

    for (size_t i = 0; i < trace.count; ++i) {
        const PrSceneEntryDirect::MainSceneRequest80015D18& request =
            trace.requests[i];
        if (!request.valid) {
            continue;
        }
        switch (request.kind) {
        case Kind::None:
            break;
        case Kind::Call80025A34:
            if (executeFn0Fn1) {
                out.call80025A34Executed = true;
                out.call80025A34Function =
                    request.psxFunction != 0u ? request.psxFunction
                                               : kFn80025A34;
                out.call80025A34TailFunction = kFn80025A00;
                ++out.call80025A34Count;
                if (loaderMemoryState80025A34 != nullptr) {
                    PrStage1LoaderMemoryDirectReset(
                        *loaderMemoryState80025A34);
                    out.call80025A34StateApplied = true;
                } else {
                    out.call80025A34Gap = true;
                    out.call80025A34StateMissing = true;
                }
            }
            break;
        case Kind::Call8001A324:
            if (executeFn0Fn1) {
                ++out.call8001A324Count;
                const bool sceneIndexKnown =
                    step.state.currentSceneV0Known &&
                    step.state.currentSceneV0 >= 0;
                const uint32_t sceneIndex =
                    sceneIndexKnown
                        ? static_cast<uint32_t>(step.state.currentSceneV0)
                        : 0u;
                const PrSceneEntryDirect::SceneEntryRawRow raw =
                    sceneIndexKnown
                        ? PrSceneEntryDirect::GetSceneEntryStaticRawRow(
                              sceneIndex,
                              0u)
                        : PrSceneEntryDirect::SceneEntryRawRow{};
                const bool rowAddrKnown = request.arg0Known;
                const uint32_t rowAddr =
                    rowAddrKnown ? static_cast<uint32_t>(request.arg0) : 0u;
                const PrMovieSegmentDirect::MovieSegmentRecord48 row =
                    BuildMovieSegmentRecordFromSceneEntryRawRow80015D18(
                        raw,
                        0u,
                        rowAddr,
                        rowAddrKnown);
                const PrMovieSegmentDirect::MovieSegmentRecord48 rowWithFeedback =
                    row0Feedback8001A324 != nullptr
                        ? PrMovieSegmentDirect::ApplyMovieSegmentRowFeedback8001A324(
                              row,
                              *row0Feedback8001A324)
                        : row;
                out.row0Init8001A324 =
                    PrMovieSegmentDirect::PsxCall8001A324_InitSegmentRecord(
                        rowWithFeedback);
                out.call8001A324Executed = true;
                out.call8001A324Gap =
                    out.row0Init8001A324.result < 0 ||
                    !out.row0Init8001A324.record.timeBaseA1Plus40Known ||
                    !out.row0Init8001A324.record.endA1Plus44Known;
            }
            break;
        case Kind::Call800154B0:
        {
            if (!executeFn0Fn1) {
                break;
            }
            ++out.call800154B0Count;
            const bool sceneIndexKnown =
                step.state.currentSceneV0Known &&
                step.state.currentSceneV0 >= 0;
            const uint32_t sceneIndex =
                sceneIndexKnown
                    ? static_cast<uint32_t>(step.state.currentSceneV0)
                    : 0u;
            const PrSceneEntryDirect::SceneEntryRawRow raw =
                sceneIndexKnown
                    ? PrSceneEntryDirect::GetSceneEntryStaticRawRow(
                          sceneIndex,
                          0u)
                    : PrSceneEntryDirect::SceneEntryRawRow{};
            const bool rowAddrKnown = request.arg0Known;
            const uint32_t rowAddr =
                rowAddrKnown ? static_cast<uint32_t>(request.arg0) : 0u;
            PrMovieSegmentDirect::MovieSegmentRecord48 row =
                BuildMovieSegmentRecordFromSceneEntryRawRow80015D18(
                    raw,
                    0u,
                    rowAddr,
                    rowAddrKnown);
            if (out.call8001A324Executed) {
                row = out.row0Init8001A324.record;
            }
            const PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324
                rowFeedback =
                    out.call8001A324Executed
                        ? PrMovieSegmentDirect::
                              MovieSegmentRowInitFeedback8001A324{}
                    : row0Feedback8001A324 != nullptr
                        ? *row0Feedback8001A324
                        : PrMovieSegmentDirect::
                              MovieSegmentRowInitFeedback8001A324{};
            out.overlayTransfer800154B0 =
                PrMovieSegmentDirect::PsxCall800154B0_Row0OverlayTransfer(
                    row,
                    rowFeedback,
                    row0SeekFeedback8001A89C,
                    row0SeekFeedbackCount8001A89C,
                    row0ReadFeedback8001A818,
                    row0ReadFeedbackCount8001A818);
            CollectRow0OverlayTransferCdSeamRequests80015D18(out);
            out.call800154B0Executed = true;
            out.call800154B0Gap =
                Row0OverlayTransferHasGap80015D18(
                    out.overlayTransfer800154B0);
            const bool explicitRow0OverlayTransferFeedback =
                HasExplicitRow0OverlayTransferFeedback80015D18(
                    row0SeekFeedback8001A89C,
                    row0SeekFeedbackCount8001A89C,
                    row0ReadFeedback8001A818,
                    row0ReadFeedbackCount8001A818);
            out.row0OverlayTransferCdSeamProducerSkippedForExplicitFeedback =
                explicitRow0OverlayTransferFeedback &&
                row0CdSeamProducer8001ACF8 != nullptr &&
                row0CdSeamProducer8001ACF8->produce != nullptr;
            if (!explicitRow0OverlayTransferFeedback &&
                RunRow0OverlayTransferCdSeamProducer80015D18(
                    row0CdSeamProducer8001ACF8,
                    ctx,
                    sceneIndexKnown,
                    sceneIndex,
                    row,
                    rowFeedback,
                    out)) {
                Row0OverlayTransferFeedbackFromCdSeams80015D18
                    row0TransferFeedback{};
                const bool feedbackKnown =
                    BuildRow0OverlayTransferFeedbackFromCdSeams80015D18(
                        out.row0OverlayTransferCompletedCdSeams.data(),
                        out.row0OverlayTransferCompletedCdSeamCount,
                        row0TransferFeedback);
                if (feedbackKnown) {
                    out.overlayTransfer800154B0 =
                        PrMovieSegmentDirect::
                            PsxCall800154B0_Row0OverlayTransfer(
                                row,
                                rowFeedback,
                                row0TransferFeedback.seekFeedback.data(),
                                row0TransferFeedback.seekFeedbackCount,
                                row0TransferFeedback.readFeedback.data(),
                                row0TransferFeedback.readFeedbackCount);
                    out.row0OverlayTransferCompletedFeedbackApplied = true;
                    out.call800154B0Gap =
                        Row0OverlayTransferHasGap80015D18(
                            out.overlayTransfer800154B0);
                }
            }
            break;
        }
        case Kind::CallSceneFn0:
            ++out.fn0RequestCount;
            out.fn0FunctionMatched =
                SceneCallbackFunctionMatches80015D18(request, def.psxFn0);
            if (requireDirectScene1Row0PrefixBeforeFn0Fn1) {
                out.directScene1InitRow0PrefixComplete =
                    HasCompletedDirectScene1Row0Prefix80015D18(out);
            }
            if (executeFn0Fn1 && def.fn0 && out.fn0FunctionMatched &&
                (!requireDirectScene1Row0PrefixBeforeFn0Fn1 ||
                 out.directScene1InitRow0PrefixComplete)) {
                out.fn0Result = def.fn0(ctx);
                out.fn0ResultKnown = true;
                out.fn0Executed = true;
            }
            break;
        case Kind::CallSceneFn1:
            ++out.fn1RequestCount;
            out.fn1FunctionMatched =
                SceneCallbackFunctionMatches80015D18(request, def.psxFn1);
            if (executeFn0Fn1 && def.fn1 && out.fn1FunctionMatched &&
                (!requireDirectScene1Row0PrefixBeforeFn0Fn1 ||
                 (out.directScene1InitRow0PrefixComplete &&
                  out.fn0Executed && out.fn0ResultKnown &&
                  out.fn0Result ==
                      static_cast<int32_t>(PrSceneId::Scene1)))) {
                def.fn1(ctx);
                out.fn1Executed = true;
            }
            break;
        case Kind::CallSceneFn2:
            ++out.fn2RequestCount;
            out.fn2FunctionMatched =
                SceneCallbackFunctionMatches80015D18(request, def.psxFn2);
            if (executeFn2 && def.fn2 && out.fn2FunctionMatched) {
                out.fn2Result = def.fn2(ctx);
                out.fn2ResultKnown = true;
                out.fn2Executed = true;
            }
            break;
        case Kind::Call8001EF14:
        case Kind::Call80015788:
        case Kind::Call80015CC4:
        case Kind::Call8001E34C:
            out.unsupportedRequest = true;
            break;
        }
    }
    return out;
}

MainSceneCallbackExecution80015D18
ExecuteMainSceneCallbacks80015D18(
    const PrSceneEntryDirect::MainSceneStepResult80015D18& step,
    PrGameContext& ctx,
    const PrSceneDef& def,
    bool executeFn0Fn1,
    bool executeFn2,
    const PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324*
        row0Feedback8001A324,
    const PrMovieSegmentDirect::OverlayTransferSeekFeedback8001A89C*
        row0SeekFeedback8001A89C,
    uint32_t row0SeekFeedbackCount8001A89C,
    const PrMovieSegmentDirect::OverlayTransferReadFeedback8001A818*
        row0ReadFeedback8001A818,
    uint32_t row0ReadFeedbackCount8001A818,
    PrStage1LoaderMemoryDirectState* loaderMemoryState80025A34,
    const Row0OverlayTransferCdSeamProducer80015D18*
        row0CdSeamProducer8001ACF8) {
    return ExecuteMainSceneCallbacksImpl80015D18(
        step,
        ctx,
        def,
        executeFn0Fn1,
        executeFn2,
        row0Feedback8001A324,
        row0SeekFeedback8001A89C,
        row0SeekFeedbackCount8001A89C,
        row0ReadFeedback8001A818,
        row0ReadFeedbackCount8001A818,
        loaderMemoryState80025A34,
        row0CdSeamProducer8001ACF8,
        false);
}

MainSceneCallbackExecution80015D18
ExecuteMainSceneCallbacks80015D18(
    const PrSceneEntryDirect::MainSceneStepResult80015D18& step,
    PrGameContext& ctx,
    const PrSceneDef& def,
    bool executeFn0Fn1,
    bool executeFn2,
    const PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324*
        row0Feedback8001A324,
    const PrStage1LoaderDirect::CdSeamResult* row0CdSeams8001ACF8,
    uint32_t row0CdSeamCount8001ACF8,
    PrStage1LoaderMemoryDirectState* loaderMemoryState80025A34) {
    Row0OverlayTransferFeedbackFromCdSeams80015D18 row0TransferFeedback{};
    const bool feedbackKnown =
        BuildRow0OverlayTransferFeedbackFromCdSeams80015D18(
            row0CdSeams8001ACF8,
            row0CdSeamCount8001ACF8,
            row0TransferFeedback);
    return ExecuteMainSceneCallbacks80015D18(
        step,
        ctx,
        def,
        executeFn0Fn1,
        executeFn2,
        row0Feedback8001A324,
        feedbackKnown ? row0TransferFeedback.seekFeedback.data() : nullptr,
        feedbackKnown ? row0TransferFeedback.seekFeedbackCount : 0u,
        feedbackKnown ? row0TransferFeedback.readFeedback.data() : nullptr,
        feedbackKnown ? row0TransferFeedback.readFeedbackCount : 0u,
        loaderMemoryState80025A34);
}

MainSceneCallbackExecution80015D18
ExecuteMainSceneCallbacks80015D18(
    const PrSceneEntryDirect::MainSceneStepResult80015D18& step,
    PrGameContext& ctx,
    const PrSceneDef& def,
    bool executeFn0Fn1,
    bool executeFn2,
    const PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324*
        row0Feedback8001A324,
    const Row0OverlayTransferCdSeamProducer80015D18*
        row0CdSeamProducer8001ACF8,
    PrStage1LoaderMemoryDirectState* loaderMemoryState80025A34) {
    return ExecuteMainSceneCallbacks80015D18(
        step,
        ctx,
        def,
        executeFn0Fn1,
        executeFn2,
        row0Feedback8001A324,
        nullptr,
        0u,
        nullptr,
        0u,
        loaderMemoryState80025A34,
        row0CdSeamProducer8001ACF8);
}

void ResetMainSceneInitGate80015D18(
    MainSceneInitGateState80015D18& state) {
    state = MainSceneInitGateState80015D18{};
}

bool IsExactDirectScene1InitCompletion80015D18(
    const MainSceneCallbackExecution80015D18& execution) {
    return execution.directScene1InitSceneMatched &&
           execution.directScene1InitTraceMatched &&
           execution.directScene1InitRow0FeedbackAccepted &&
           execution.directScene1InitCompletedCdSeamsAccepted &&
           execution.directScene1InitRow0PrefixComplete &&
           execution.row0OverlayTransferCompletedFeedbackApplied &&
           execution.row0OverlayTransferCompletedCdSeamCount != 0u &&
           execution.row0OverlayTransferCompletedCdSeamRejectedCount == 0u &&
           !execution.row0OverlayTransferCompletedCdSeamsTruncated &&
           execution.fn0RequestCount == 1u &&
           execution.fn0FunctionMatched &&
           execution.fn0Executed &&
           execution.fn0ResultKnown &&
           execution.fn0Result ==
               static_cast<int32_t>(PrSceneId::Scene1) &&
           execution.fn1RequestCount == 1u &&
           execution.fn1FunctionMatched &&
           execution.fn1Executed &&
           execution.fn2RequestCount == 1u &&
           execution.fn2FunctionMatched &&
           !execution.fn2Executed &&
           !execution.fn2ResultKnown;
}

MainSceneCallbackExecution80015D18
ExecuteDirectScene1InitCallbacksOnce80015D18(
    MainSceneInitGateState80015D18& gate,
    PrSceneId scene,
    const PrSceneEntryDirect::MainSceneStepResult80015D18& step,
    PrGameContext& ctx,
    const PrSceneDef& def,
    const PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324*
        row0Feedback8001A324,
    const PrStage1LoaderDirect::CdSeamResult*
        completedRow0CdSeams8001ACF8,
    uint32_t completedRow0CdSeamCount8001ACF8,
    PrStage1LoaderMemoryDirectState* loaderMemoryState80025A34) {
    MainSceneCallbackExecution80015D18 out{};

    if (gate.initializedSceneKnown && gate.initializedScene == scene) {
        return out;
    }
    if (gate.directScene1InitAttempted) {
        return out;
    }

    out.traceOverflow = step.trace.overflow;
    out.invalidCurrentSceneIndex = step.invalidCurrentSceneIndex;
    out.missingWord800916D0For15CC4Gate =
        step.missingWord800916D0For15CC4Gate;
    out.waitingForSceneFn2Result = step.waitingForSceneFn2Result;
    out.directScene1InitSceneMatched =
        scene == PrSceneId::Scene1 &&
        step.state.currentSceneV0Known &&
        step.state.currentSceneV0 ==
            static_cast<int32_t>(PrSceneId::Scene1);

    out.directScene1InitTraceMatched =
        IsExactDirectScene1InitTrace80015D18(scene, step, def);
    out.directScene1InitRow0FeedbackAccepted =
        out.directScene1InitTraceMatched &&
        IsExactDirectScene1Row0Feedback80015D18(
            step,
            row0Feedback8001A324);

    Row0OverlayTransferFeedbackFromCdSeams80015D18 transferFeedback{};
    out.directScene1InitCompletedCdSeamsAccepted =
        out.directScene1InitRow0FeedbackAccepted &&
        BuildExactDirectScene1Row0TransferFeedback80015D18(
            step,
            *row0Feedback8001A324,
            completedRow0CdSeams8001ACF8,
            completedRow0CdSeamCount8001ACF8,
            transferFeedback);
    if (!out.directScene1InitCompletedCdSeamsAccepted) {
        return out;
    }
    if (loaderMemoryState80025A34 == nullptr) {
        out.call80025A34Gap = true;
        out.call80025A34StateMissing = true;
        return out;
    }

    gate.directScene1InitAttempted = true;
    out = ExecuteMainSceneCallbacksImpl80015D18(
        step,
        ctx,
        def,
        true,
        false,
        row0Feedback8001A324,
        transferFeedback.seekFeedback.data(),
        transferFeedback.seekFeedbackCount,
        transferFeedback.readFeedback.data(),
        transferFeedback.readFeedbackCount,
        loaderMemoryState80025A34,
        nullptr,
        true);
    out.directScene1InitSceneMatched = true;
    out.directScene1InitTraceMatched = true;
    out.directScene1InitRow0FeedbackAccepted = true;
    out.directScene1InitCompletedCdSeamsAccepted = true;
    out.row0OverlayTransferCompletedFeedbackApplied = true;
    out.row0OverlayTransferCompletedCdSeamCount =
        completedRow0CdSeamCount8001ACF8;
    for (uint32_t index = 0;
         index < completedRow0CdSeamCount8001ACF8;
         ++index) {
        out.row0OverlayTransferCompletedCdSeams[index] =
            completedRow0CdSeams8001ACF8[index];
    }
    out.directScene1InitRow0PrefixComplete =
        HasCompletedDirectScene1Row0Prefix80015D18(out);

    if (IsExactDirectScene1InitCompletion80015D18(out)) {
        gate.initializedSceneKnown = true;
        gate.initializedScene = scene;
        gate.directScene1InitFailed = false;
    } else {
        gate.directScene1InitFailed = true;
    }
    return out;
}

MainSceneCallbackExecution80015D18
ExecuteMainSceneInitCallbacksOnce80015D18(
    MainSceneInitGateState80015D18& gate,
    PrSceneId scene,
    const PrSceneEntryDirect::MainSceneStepResult80015D18& step,
    PrGameContext& ctx,
    const PrSceneDef& def,
    const PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324*
        row0Feedback8001A324,
    PrStage1LoaderMemoryDirectState* loaderMemoryState80025A34) {
    if (gate.initializedSceneKnown && gate.initializedScene == scene) {
        return MainSceneCallbackExecution80015D18{};
    }

    MainSceneCallbackExecution80015D18 out =
        ExecuteMainSceneCallbacks80015D18(
            step,
            ctx,
            def,
            true,
            false,
            row0Feedback8001A324,
            nullptr,
            0u,
            nullptr,
            0u,
            loaderMemoryState80025A34);
    gate.initializedSceneKnown = true;
    gate.initializedScene = scene;
    return out;
}

MainSceneCallbackExecution80015D18
ExecuteMainSceneInitCallbacksOnce80015D18(
    MainSceneInitGateState80015D18& gate,
    PrSceneId scene,
    const PrSceneEntryDirect::MainSceneStepResult80015D18& step,
    PrGameContext& ctx,
    const PrSceneDef& def,
    PrStage1LoaderMemoryDirectState* loaderMemoryState80025A34) {
    if (gate.initializedSceneKnown && gate.initializedScene == scene) {
        return MainSceneCallbackExecution80015D18{};
    }

    PrMovieSegmentDirect::MovieSegmentRowInitFeedback8001A324 row0Feedback{};
    const bool row0FeedbackKnown =
        BuildScene1Row0OverlayTransferFeedback80015D18(
            ctx,
            scene,
            row0Feedback);
    return ExecuteMainSceneInitCallbacksOnce80015D18(
        gate,
        scene,
        step,
        ctx,
        def,
        row0FeedbackKnown ? &row0Feedback : nullptr,
        loaderMemoryState80025A34);
}

GenericSwitchRunResult80015788
RunGenericSwitch80015788(PrGameContext& ctx,
                         PrSceneTable& table,
                         PrSceneId prevScene) {
    return RunGenericSwitch80015788(ctx, table, prevScene, nullptr);
}

GenericSwitchRunResult80015788
RunGenericSwitch80015788(
    PrGameContext& ctx,
    PrSceneTable& table,
    PrSceneId prevScene,
    const CompletedCall80019414Input80015788* completedCall19414) {
    GenericSwitchRunResult80015788 out{};
    out.scene = prevScene;
    out.decided = false;
    if (SS0DirectOwnsGenericSwitchContext80015788(ctx) ||
        SS0DirectOwnsGenericSwitchScene80015788(prevScene)) {
        if (s_genericSwitch80015788.active) {
            s_genericSwitch80015788 = {};
        }
        PrEvent::ClearDispatcherResidueForDirectCutover();
        Log::Printf("80015788 suppressed for direct SS0 current=%u prev=%u",
                    static_cast<unsigned>(ctx.currentScene),
                    static_cast<unsigned>(prevScene));
        return out;
    }
    if (!s_genericSwitch80015788.active) {
        s_genericSwitch80015788 = {};
        s_genericSwitch80015788.active = true;
        s_genericSwitch80015788.direct =
            PrSceneEntryDirect::InitGenericSwitchState80015788(
                static_cast<int32_t>(prevScene));
        s_genericSwitch80015788.direct.word800916D0Known = true;
        s_genericSwitch80015788.direct.word800916D0 =
            static_cast<uint16_t>(ctx.transitionState);
    }

    PrSceneEntryDirect::GenericSwitchFeedback80015788 feedback{};
    if (!BuildPendingGenericSwitchFeedback80015788(
            ctx,
            s_genericSwitch80015788,
            feedback)) {
        return out;
    }

    for (int step = 0; step < 32; ++step) {
        const PrSceneEntryDirect::GenericSwitchStepResult80015788 result =
            PrSceneEntryDirect::StepGenericSwitch80015788(
                s_genericSwitch80015788.direct,
                feedback);
        feedback = {};

        if (result.done && result.state.returnValueKnown) {
            const int next = ClampSceneIndex80015788(result.state.returnValue);
            s_genericSwitch80015788 = {};
            out.scene = static_cast<PrSceneId>(static_cast<uint8_t>(next));
            out.decided = true;
            Log::Printf("80015788 decided next=%d", next);
            return out;
        }

        bool producedFeedback = false;
        const GenericSwitchTraceExecStatus80015788 traceStatus =
            ExecuteGenericSwitchTrace80015788(
                ctx,
                table,
                s_genericSwitch80015788,
                result.trace,
                feedback,
                completedCall19414,
                producedFeedback);
        if (traceStatus == GenericSwitchTraceExecStatus80015788::Failed) {
            s_genericSwitch80015788 = {};
            return out;
        }
        if (traceStatus == GenericSwitchTraceExecStatus80015788::DirectGap) {
            s_genericSwitch80015788 = {};
            return out;
        }
        s_genericSwitch80015788.direct = result.state;
        ctx.transitionState =
            static_cast<int16_t>(s_genericSwitch80015788.direct.word800916D0);
        if (traceStatus == GenericSwitchTraceExecStatus80015788::Pending) {
            return out;
        }
        if (producedFeedback || result.trace.count > 0u) {
            continue;
        }
        if (result.waitingForFeedback) {
            return out;
        }
    }

    Log::Printf("80015788 adapter step limit reached");
    return out;
}

GenericSwitchRunResult80015788
ExecuteMainSceneSwitchTrace80015D18(
    const PrSceneEntryDirect::MainSceneRequestTrace80015D18& trace,
    PrGameContext& ctx,
    PrSceneTable& table,
    PrSceneId prevScene,
    const CompletedCall80019414Input80015788* completedCall19414) {
    GenericSwitchRunResult80015788 out{};
    out.scene = prevScene;
    if (trace.overflow) {
        Log::Printf("80015D18 switch trace overflow");
        return out;
    }

    bool hasSwitchRequest = false;
    PrSceneId requestPrevScene = prevScene;
    using Kind = PrSceneEntryDirect::MainSceneRequestKind80015D18;
    for (size_t i = 0; i < trace.count; ++i) {
        const PrSceneEntryDirect::MainSceneRequest80015D18& request =
            trace.requests[i];
        if (!request.valid ||
            request.kind != Kind::Call80015788) {
            continue;
        }
        if (!request.arg0Known ||
            request.arg0 < 0 ||
            request.arg0 >= static_cast<int32_t>(kPrSceneCount)) {
            Log::Printf(
                "80015D18 switch trace gap: 80015788 arg0 missing/invalid");
            return out;
        }
        hasSwitchRequest = true;
        requestPrevScene =
            static_cast<PrSceneId>(static_cast<uint8_t>(request.arg0));
        break;
    }

    if (!hasSwitchRequest && !s_genericSwitch80015788.active) {
        return out;
    }
    return RunGenericSwitch80015788(
        ctx,
        table,
        hasSwitchRequest ? requestPrevScene : prevScene,
        completedCall19414);
}

}  // namespace PrSceneEntryExecutorDirect
