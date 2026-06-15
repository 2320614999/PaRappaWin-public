#include "pr_stage_status_bank_host_bridge_direct.h"

#include "pr_stage1_save_ui_direct.h"

namespace PrStageStatusBankHostBridgeDirect {
namespace {

struct StatusBankDirectMemoryApplyResult801C81EC {
    bool ok = false;
    PrStageClearStatusQueryResult status166AC{};
    PrStage1SavePayloadProducerResult savePayload{};
};

bool IsExecutableStatusBankRequest801C81EC(
    const PrStageStatusBankDirectCallRequest& request) {
    if (!request.valid) {
        return false;
    }

    switch (request.kind) {
    case PrStageStatusBankActionKind::Call800166AC:
        return request.psxFunction == 0x800166ACu;
    case PrStageStatusBankActionKind::Call8001635C:
        return request.psxFunction == 0x8001635Cu;
    case PrStageStatusBankActionKind::Call8001628C:
        return request.psxFunction == 0x8001628Cu;
    case PrStageStatusBankActionKind::None:
    case PrStageStatusBankActionKind::StoreWord800916D0:
    case PrStageStatusBankActionKind::StoreWord800916DA:
    case PrStageStatusBankActionKind::StoreLocalStash:
    case PrStageStatusBankActionKind::StoreWord800916E0:
    case PrStageStatusBankActionKind::Call80024E54:
    case PrStageStatusBankActionKind::Call80094440:
    case PrStageStatusBankActionKind::Call800143F0:
    case PrStageStatusBankActionKind::Call8001681C:
    case PrStageStatusBankActionKind::Call80016758:
    case PrStageStatusBankActionKind::Call8001670C:
    case PrStageStatusBankActionKind::Call800259C0:
    case PrStageStatusBankActionKind::Call80015590:
    case PrStageStatusBankActionKind::Call80019148:
    case PrStageStatusBankActionKind::Call80015CC4:
    case PrStageStatusBankActionKind::Call800169E0:
        return false;
    }
    return false;
}

StatusBankDirectMemoryApplyResult801C81EC
ApplyStatusBankDirectMemoryRequest801C81EC(
    const PrStageStatusBankDirectCallRequest& request) {
    StatusBankDirectMemoryApplyResult801C81EC out{};
    if (!IsExecutableStatusBankRequest801C81EC(request)) {
        return out;
    }

    switch (request.kind) {
    case PrStageStatusBankActionKind::Call800166AC:
        if (!request.arg0Known) {
            return out;
        }
        out.status166AC = PrStage1SaveUiDirect::Sub800166AC(request.arg0);
        out.ok = out.status166AC.ok;
        break;
    case PrStageStatusBankActionKind::Call8001635C:
        if (!request.arg0Known || !request.arg1Known ||
            !request.arg2Known || !request.arg3Known) {
            return out;
        }
        out.savePayload = PrStage1SaveUiDirect::Sub8001635C(
            request.arg0,
            request.arg1,
            request.arg2,
            request.arg3);
        out.ok = out.savePayload.ok;
        break;
    case PrStageStatusBankActionKind::Call8001628C:
        if (!request.arg0Known) {
            return out;
        }
        out.savePayload = PrStage1SaveUiDirect::Sub8001628C(request.arg0);
        out.ok = out.savePayload.ok;
        break;
    case PrStageStatusBankActionKind::None:
    case PrStageStatusBankActionKind::StoreWord800916D0:
    case PrStageStatusBankActionKind::StoreWord800916DA:
    case PrStageStatusBankActionKind::StoreLocalStash:
    case PrStageStatusBankActionKind::StoreWord800916E0:
    case PrStageStatusBankActionKind::Call80024E54:
    case PrStageStatusBankActionKind::Call80094440:
    case PrStageStatusBankActionKind::Call800143F0:
    case PrStageStatusBankActionKind::Call8001681C:
    case PrStageStatusBankActionKind::Call80016758:
    case PrStageStatusBankActionKind::Call8001670C:
    case PrStageStatusBankActionKind::Call800259C0:
    case PrStageStatusBankActionKind::Call80015590:
    case PrStageStatusBankActionKind::Call80019148:
    case PrStageStatusBankActionKind::Call80015CC4:
    case PrStageStatusBankActionKind::Call800169E0:
        break;
    }
    return out;
}

PrStage1LifecycleExecutorDirect::StatusBankDirectMemoryFeedback801C81EC
BuildStatusBankDirectMemoryFeedback801C81EC(
    const PrStageStatusBankDirectCallRequest& request,
    const StatusBankDirectMemoryApplyResult801C81EC& apply) {
    PrStage1LifecycleExecutorDirect::StatusBankDirectMemoryFeedback801C81EC
        feedback{};
    feedback.requestValid = request.valid;
    feedback.requestKind = request.kind;
    feedback.psxFunction = request.psxFunction;
    feedback.psxFunctionMatched =
        IsExecutableStatusBankRequest801C81EC(request);
    feedback.requestHandled = apply.ok;
    if (apply.status166AC.ok) {
        feedback.status166ACKnown = true;
        feedback.status166AC = apply.status166AC.status;
    }
    feedback.status166ACSceneId = apply.status166AC.sceneId;
    feedback.status166ACMapped = apply.status166AC.mapped;
    feedback.status166ACSlotIndex = apply.status166AC.slotIndex;
    feedback.status166ACStatusBankKnown =
        apply.status166AC.statusBankKnown;
    feedback.status166ACHelperGap = apply.status166AC.helperGap;
    feedback.payloadKnown = apply.savePayload.payloadKnown;
    feedback.payloadOk = apply.savePayload.ok;
    feedback.payloadHelperGap = apply.savePayload.helperGap;
    feedback.payloadResult = apply.savePayload.result;
    feedback.payloadLastFaultAddress = apply.savePayload.lastFaultAddress;
    const PrStage1SaveStatusPrefix80092F10 prefix =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    feedback.payloadPrefixKnown80092F10 = prefix.known;
    feedback.payloadPrefixStatusBankKnown80092F1D =
        prefix.statusBankKnown80092F1D;
    feedback.payloadLastWriterFunction = prefix.lastWriterFunction;
    feedback.payloadWrote8001635C = prefix.wrote8001635C;
    return feedback;
}

} // namespace

PrStage1LifecycleExecutorDirect::StatusBankDirectMemoryFeedback801C81EC
ExecuteStatusBankDirectMemoryRequest801C81EC(
    const PrStageStatusBankDirectCallRequest& request) {
    return BuildStatusBankDirectMemoryFeedback801C81EC(
        request,
        ApplyStatusBankDirectMemoryRequest801C81EC(request));
}

} // namespace PrStageStatusBankHostBridgeDirect
