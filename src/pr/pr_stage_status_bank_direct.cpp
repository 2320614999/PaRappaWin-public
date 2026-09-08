#include "pr_stage_status_bank_direct.h"

namespace {

constexpr uint32_t kFn800143F0 = 0x800143F0u;
constexpr uint32_t kFn80015590 = 0x80015590u;
constexpr uint32_t kFn80015CC4 = 0x80015CC4u;
constexpr uint32_t kFn8001628C = 0x8001628Cu;
constexpr uint32_t kFn8001635C = 0x8001635Cu;
constexpr uint32_t kFn800166AC = 0x800166ACu;
constexpr uint32_t kFn8001670C = 0x8001670Cu;
constexpr uint32_t kFn80016758 = 0x80016758u;
constexpr uint32_t kFn8001681C = 0x8001681Cu;
constexpr uint32_t kFn800169E0 = 0x800169E0u;
constexpr uint32_t kFn80019148 = 0x80019148u;
constexpr uint32_t kFn80024E54 = 0x80024E54u;
constexpr uint32_t kFn800259C0 = 0x800259C0u;
constexpr uint32_t kFn80094440 = 0x80094440u;
constexpr uint32_t kAddrSavePayloadBank80092F10 = 0x80092F10u;
constexpr uint32_t kRunnerFlag80 = 0x80u;

void Emit(PrStageStatusBankActionTrace& trace,
          PrStageStatusBankActionKind kind,
          uint32_t psxAddress = 0u,
          bool arg0Known = false,
          int32_t arg0 = 0,
          bool arg1Known = false,
          int32_t arg1 = 0,
          bool arg2Known = false,
          int32_t arg2 = 0,
          bool arg3Known = false,
          int32_t arg3 = 0) {
    if (trace.count >= trace.actions.size()) {
        trace.overflow = true;
        return;
    }
    PrStageStatusBankAction& action = trace.actions[trace.count++];
    action.kind = kind;
    action.psxAddress = psxAddress;
    action.arg0Known = arg0Known;
    action.arg0 = arg0;
    action.arg1Known = arg1Known;
    action.arg1 = arg1;
    action.arg2Known = arg2Known;
    action.arg2 = arg2;
    action.arg3Known = arg3Known;
    action.arg3 = arg3;
}

bool IsTerminalComod7(PrStageStatusBankComod comod) {
    return comod == PrStageStatusBankComod::Comod7;
}

uint32_t LocalStashAddress(PrStageStatusBankComod comod) {
    switch (comod) {
    case PrStageStatusBankComod::Comod1:
        return 0x801D3040u;
    case PrStageStatusBankComod::Comod2:
        return 0x801D2988u;
    case PrStageStatusBankComod::Comod3:
        return 0x801D42C0u;
    case PrStageStatusBankComod::Comod5:
        return 0x801D81E0u;
    case PrStageStatusBankComod::Comod6:
        return 0x801D67A8u;
    case PrStageStatusBankComod::Comod7:
        return 0x801D8A58u;
    }
    return 0u;
}

uint16_t ClearProducerD0ExitDaValue(PrStageStatusBankComod comod,
                                    uint16_t localStashValue) {
    return comod == PrStageStatusBankComod::Comod1 ? localStashValue : 0u;
}

bool ActionHasRequiredArgs(const PrStageStatusBankAction& action,
                           bool arg0,
                           bool arg1 = false,
                           bool arg2 = false,
                           bool arg3 = false) {
    return (!arg0 || action.arg0Known) && (!arg1 || action.arg1Known) &&
           (!arg2 || action.arg2Known) && (!arg3 || action.arg3Known);
}

uint32_t ExpectedPsxAddressForActionKind(PrStageStatusBankActionKind kind) {
    switch (kind) {
    case PrStageStatusBankActionKind::Call80024E54:
        return kFn80024E54;
    case PrStageStatusBankActionKind::Call80094440:
        return kFn80094440;
    case PrStageStatusBankActionKind::Call800143F0:
        return kFn800143F0;
    case PrStageStatusBankActionKind::Call8001681C:
        return kFn8001681C;
    case PrStageStatusBankActionKind::Call80016758:
        return kFn80016758;
    case PrStageStatusBankActionKind::Call8001670C:
        return kFn8001670C;
    case PrStageStatusBankActionKind::Call800259C0:
        return kFn800259C0;
    case PrStageStatusBankActionKind::Call800166AC:
        return kFn800166AC;
    case PrStageStatusBankActionKind::Call8001635C:
        return kFn8001635C;
    case PrStageStatusBankActionKind::Call8001628C:
        return kFn8001628C;
    case PrStageStatusBankActionKind::Call80015590:
        return kFn80015590;
    case PrStageStatusBankActionKind::Call80019148:
        return kFn80019148;
    case PrStageStatusBankActionKind::Call80015CC4:
        return kFn80015CC4;
    case PrStageStatusBankActionKind::Call800169E0:
        return kFn800169E0;
    case PrStageStatusBankActionKind::None:
    case PrStageStatusBankActionKind::StoreWord800916D0:
    case PrStageStatusBankActionKind::StoreWord800916DA:
    case PrStageStatusBankActionKind::StoreLocalStash:
    case PrStageStatusBankActionKind::StoreWord800916E0:
        return 0u;
    }
    return 0u;
}

PrStageStatusBankDirectCallRequest BuildCallRequest(
    const PrStageStatusBankAction& action) {
    PrStageStatusBankDirectCallRequest request{};
    const uint32_t expectedPsxAddress =
        ExpectedPsxAddressForActionKind(action.kind);
    request.valid =
        expectedPsxAddress != 0u && action.psxAddress == expectedPsxAddress;
    request.kind = action.kind;
    request.psxFunction = action.psxAddress;
    request.arg0Known = action.arg0Known;
    request.arg0 = action.arg0;
    request.arg1Known = action.arg1Known;
    request.arg1 = action.arg1;
    request.arg2Known = action.arg2Known;
    request.arg2 = action.arg2;
    request.arg3Known = action.arg3Known;
    request.arg3 = action.arg3;
    return request;
}

void AppendDirectMemoryRequest(PrStageStatusBankTraceExecutionResult& out,
                               const PrStageStatusBankDirectCallRequest& request) {
    if (!request.valid) {
        return;
    }
    if (out.directMemoryRequestCount >= out.directMemoryRequests.size()) {
        out.traceOverflow = true;
        out.ok = false;
        return;
    }
    out.directMemoryRequests[out.directMemoryRequestCount++] = request;
}

bool RejectInvalidCallRequest(PrStageStatusBankTraceExecutionResult& out,
                              const PrStageStatusBankDirectCallRequest& request) {
    if (request.valid) {
        return false;
    }
    ++out.psxAddressMismatchActions;
    out.ok = false;
    return true;
}

} // namespace

uint32_t PrStageStatusBankComodClearProducerAddress(
    PrStageStatusBankComod comod) {
    switch (comod) {
    case PrStageStatusBankComod::Comod1:
        return 0x801C81ECu;
    case PrStageStatusBankComod::Comod2:
        return 0x801C74E4u;
    case PrStageStatusBankComod::Comod3:
        return 0x801C7880u;
    case PrStageStatusBankComod::Comod5:
        return 0x801C8C88u;
    case PrStageStatusBankComod::Comod6:
        return 0x801C7030u;
    case PrStageStatusBankComod::Comod7:
        return 0x801C7C98u;
    }
    return 0u;
}

uint32_t PrStageStatusBankComodRestoreInitAddress(
    PrStageStatusBankComod comod) {
    switch (comod) {
    case PrStageStatusBankComod::Comod1:
        return 0x801C7A60u;
    case PrStageStatusBankComod::Comod2:
        return 0x801C6D58u;
    case PrStageStatusBankComod::Comod3:
        return 0x801C70F4u;
    case PrStageStatusBankComod::Comod5:
        return 0x801C84FCu;
    case PrStageStatusBankComod::Comod6:
        return 0x801C6894u;
    case PrStageStatusBankComod::Comod7:
        return 0x801C72A0u;
    }
    return 0u;
}

uint32_t PrStageStatusBankComodTerminalConsumerAddress(
    PrStageStatusBankComod comod) {
    switch (comod) {
    case PrStageStatusBankComod::Comod1:
        return 0x801C9094u;
    case PrStageStatusBankComod::Comod2:
        return 0x801C8714u;
    case PrStageStatusBankComod::Comod3:
        return 0x801C8918u;
    case PrStageStatusBankComod::Comod5:
        return 0x801C9FBCu;
    case PrStageStatusBankComod::Comod6:
        return 0x801C8238u;
    case PrStageStatusBankComod::Comod7:
        return 0x801C8D9Cu;
    }
    return 0u;
}

PrStageStatusBankRestoreInitResult
PrStageStatusBankDirectRestoreInit(
    const PrStageStatusBankRestoreInitInput& input) {
    PrStageStatusBankRestoreInitResult out{};
    uint16_t recordsMode916DAAfterSetup = input.word800916DA;

    if (input.word800916D0 == 1u) {
        out.storeLocalStash = true;
        out.localStashPsxAddress = LocalStashAddress(input.comod);
        out.localStashValue = input.word800916DA;
        out.clearWord800916DA = true;
        out.word800916DAAfter = 0u;
        out.writeCtxWord41 = true;
        out.ctxWord41Value = 1u;
        recordsMode916DAAfterSetup = 0u;
        Emit(out.trace,
             PrStageStatusBankActionKind::StoreLocalStash,
             out.localStashPsxAddress,
             true,
             input.word800916DA);
        Emit(out.trace,
             PrStageStatusBankActionKind::StoreWord800916DA,
             0u,
             true,
             0);
        Emit(out.trace,
             PrStageStatusBankActionKind::Call80024E54,
             kFn80024E54,
             true,
             0);
        Emit(out.trace,
             PrStageStatusBankActionKind::Call80094440,
             kFn80094440);
        Emit(out.trace,
             PrStageStatusBankActionKind::Call800143F0,
             kFn800143F0,
             true,
             0);
        Emit(out.trace,
             PrStageStatusBankActionKind::Call800259C0,
             kFn800259C0,
             true,
             0);
    } else if (input.word800916D0 == 2u) {
        out.storeLocalStash = true;
        out.localStashPsxAddress = LocalStashAddress(input.comod);
        out.localStashValue = input.word800916DA;
        out.clearWord800916DA = true;
        out.word800916DAAfter = 0u;
        out.writeCtxWord41 = true;
        out.ctxWord41Value = 1u;
        recordsMode916DAAfterSetup = 0u;
        Emit(out.trace,
             PrStageStatusBankActionKind::StoreLocalStash,
             out.localStashPsxAddress,
             true,
             input.word800916DA);
        Emit(out.trace,
             PrStageStatusBankActionKind::StoreWord800916DA,
             0u,
             true,
             0);
        Emit(out.trace,
             PrStageStatusBankActionKind::Call80024E54,
             kFn80024E54,
             true,
             0);
        Emit(out.trace,
             PrStageStatusBankActionKind::Call8001681C,
             kFn8001681C);
        Emit(out.trace,
             PrStageStatusBankActionKind::Call80016758,
             kFn80016758,
             true,
             static_cast<int32_t>(input.comod));
        Emit(out.trace,
             PrStageStatusBankActionKind::Call800143F0,
             kFn800143F0,
             input.restoreSetupReturn80016758Known,
             input.restoreSetupReturn80016758);
        Emit(out.trace,
             PrStageStatusBankActionKind::Call800259C0,
             kFn800259C0,
             input.restoreSetupReturn80016758Known,
             input.restoreSetupReturn80016758);
    } else {
        out.writeCtxWord41 = true;
        out.ctxWord41Value = 0u;
        Emit(out.trace,
             PrStageStatusBankActionKind::Call80024E54,
             kFn80024E54,
             true,
             0);
        Emit(out.trace,
             PrStageStatusBankActionKind::Call8001670C,
             kFn8001670C,
             true,
             static_cast<int32_t>(input.comod));
        Emit(out.trace,
             PrStageStatusBankActionKind::Call800143F0,
             kFn800143F0,
             input.defaultSetupReturn8001670CKnown,
             input.defaultSetupReturn8001670C);
        Emit(out.trace,
             PrStageStatusBankActionKind::Call800259C0,
             kFn800259C0,
             input.defaultSetupReturn8001670CKnown,
             input.defaultSetupReturn8001670C);
    }

    out.writeCtxDword34 = true;
    out.ctxDword34Value =
        recordsMode916DAAfterSetup != 0u
            ? 24
            : input.sceneEntryField360HalfSource / 2;
    out.writeCtxWord55 = true;
    out.ctxWord55Value = input.word800916F6;
    if (input.word800916F6 == 1u) {
        out.writeCtxWord54 = true;
        out.ctxWord54Value = 1u;
    }
    if (recordsMode916DAAfterSetup == 1u) {
        Emit(out.trace,
             PrStageStatusBankActionKind::Call800143F0,
             kFn800143F0,
             true,
             0);
        Emit(out.trace,
             PrStageStatusBankActionKind::Call800259C0,
             kFn800259C0,
             true,
             0);
    }
    return out;
}

PrStageStatusBankTraceExecutionResult
PrStageStatusBankExecuteActionTrace(
    const PrStageStatusBankActionTrace& trace,
    PrStageStatusBankDirectState* state) {
    PrStageStatusBankTraceExecutionResult out{};
    out.traceOverflow = trace.overflow;
    out.ok = !trace.overflow;

    for (size_t i = 0u; i < trace.count && i < trace.actions.size(); ++i) {
        const PrStageStatusBankAction& action = trace.actions[i];
        ++out.actionsVisited;

        switch (action.kind) {
        case PrStageStatusBankActionKind::None:
            break;
        case PrStageStatusBankActionKind::StoreWord800916D0:
            if (!action.arg0Known) {
                ++out.missingArgumentActions;
                out.ok = false;
                break;
            }
            if (state) {
                state->word800916D0Known = true;
                state->word800916D0 = static_cast<uint16_t>(action.arg0);
            }
            ++out.stateStoresApplied;
            break;
        case PrStageStatusBankActionKind::StoreWord800916DA:
            if (!action.arg0Known) {
                ++out.missingArgumentActions;
                out.ok = false;
                break;
            }
            if (state) {
                state->word800916DAKnown = true;
                state->word800916DA = static_cast<uint16_t>(action.arg0);
            }
            ++out.stateStoresApplied;
            break;
        case PrStageStatusBankActionKind::StoreWord800916E0:
            if (!action.arg0Known) {
                ++out.missingArgumentActions;
                out.ok = false;
                break;
            }
            if (state) {
                state->word800916E0Known = true;
                state->word800916E0 = static_cast<uint16_t>(action.arg0);
            }
            ++out.stateStoresApplied;
            break;
        case PrStageStatusBankActionKind::StoreLocalStash:
            if (!action.arg0Known) {
                ++out.missingArgumentActions;
                out.ok = false;
                break;
            }
            if (state) {
                state->localStashKnown = true;
                state->localStashPsxAddress = action.psxAddress;
                state->localStashValue = static_cast<uint16_t>(action.arg0);
            }
            ++out.stateStoresApplied;
            break;
        case PrStageStatusBankActionKind::Call800166AC:
            if (!ActionHasRequiredArgs(action, true)) {
                ++out.missingArgumentActions;
                out.ok = false;
                break;
            }
            out.lastStatusQuery800166AC = BuildCallRequest(action);
            if (RejectInvalidCallRequest(out, out.lastStatusQuery800166AC)) {
                break;
            }
            AppendDirectMemoryRequest(out, out.lastStatusQuery800166AC);
            ++out.directMemoryRequestsReady;
            break;
        case PrStageStatusBankActionKind::Call8001635C:
            if (!ActionHasRequiredArgs(action, true, true, false, true)) {
                ++out.missingArgumentActions;
                out.ok = false;
                break;
            }
            out.lastSavePayload8001635C = BuildCallRequest(action);
            if (RejectInvalidCallRequest(out, out.lastSavePayload8001635C)) {
                break;
            }
            if (!action.arg2Known) {
                if (!out.lastStatusQuery800166AC.valid) {
                    ++out.missingArgumentActions;
                    out.ok = false;
                    out.lastSavePayload8001635C.valid = false;
                    break;
                }
                out.lastSavePayload8001635C.arg2DependsOnPrevious800166AC =
                    true;
            }
            AppendDirectMemoryRequest(out, out.lastSavePayload8001635C);
            ++out.directMemoryRequestsReady;
            break;
        case PrStageStatusBankActionKind::Call8001628C:
            if (!ActionHasRequiredArgs(action, true)) {
                ++out.missingArgumentActions;
                out.ok = false;
                break;
            }
            out.lastUnlock8001628C = BuildCallRequest(action);
            if (RejectInvalidCallRequest(out, out.lastUnlock8001628C)) {
                break;
            }
            AppendDirectMemoryRequest(out, out.lastUnlock8001628C);
            ++out.directMemoryRequestsReady;
            break;
        case PrStageStatusBankActionKind::Call80015CC4:
        {
            const PrStageStatusBankDirectCallRequest request =
                BuildCallRequest(action);
            if (RejectInvalidCallRequest(out, request)) {
                break;
            }
            AppendDirectMemoryRequest(out, request);
            ++out.directMemoryRequestsReady;
            break;
        }
        case PrStageStatusBankActionKind::Call800169E0:
            if (!state || !state->word800916D0Known ||
                !state->word800916E2Known) {
                ++out.missingArgumentActions;
                out.ok = false;
                break;
            }
            out.lastScoreSync800169E0 = BuildCallRequest(action);
            if (RejectInvalidCallRequest(out, out.lastScoreSync800169E0)) {
                break;
            }
            out.lastScoreSync800169E0.arg0Known = true;
            out.lastScoreSync800169E0.arg0 = state->word800916D0;
            out.lastScoreSync800169E0.arg1Known = true;
            out.lastScoreSync800169E0.arg1 = state->word800916E2;
            AppendDirectMemoryRequest(out, out.lastScoreSync800169E0);
            ++out.directMemoryRequestsReady;
            break;
        case PrStageStatusBankActionKind::Call80015590:
        case PrStageStatusBankActionKind::Call80019148:
            ++out.hostBoundaryActions;
            out.ok = false;
            break;
        case PrStageStatusBankActionKind::Call80024E54:
        case PrStageStatusBankActionKind::Call80094440:
        case PrStageStatusBankActionKind::Call800143F0:
        case PrStageStatusBankActionKind::Call8001681C:
        case PrStageStatusBankActionKind::Call80016758:
        case PrStageStatusBankActionKind::Call8001670C:
        case PrStageStatusBankActionKind::Call800259C0:
            ++out.unsupportedDirectActions;
            out.ok = false;
            break;
        }
    }

    return out;
}

PrStageStatusBankClearProducerResult
PrStageStatusBankDirectClearProducer(
    const PrStageStatusBankClearProducerInput& input) {
    PrStageStatusBankClearProducerResult out{};

    if (IsTerminalComod7(input.comod)) {
        out.statusProducerCalled8001635C = true;
        out.statusA1 = input.stageArg;
        out.statusA2 = input.statusGateIsZero ? 3 : 2;
        out.statusA3 = input.previousStatus800166AC;
        out.statusA4 = input.word80091816;
        Emit(out.trace,
             PrStageStatusBankActionKind::Call800166AC,
             kFn800166AC,
             true,
             input.stageArg);
        Emit(out.trace,
             PrStageStatusBankActionKind::Call8001635C,
             kFn8001635C,
             true,
             out.statusA1,
             true,
             out.statusA2,
             input.previousStatusKnown800166AC,
             out.statusA3,
             true,
             out.statusA4);
        if (input.word800916F0Known) {
            if (input.word800916F0 != 1u) {
                out.saveMenuCalled = true;
                Emit(out.trace,
                     PrStageStatusBankActionKind::Call80015590,
                     kFn80015590,
                     true,
                     input.stageArg);
                Emit(out.trace,
                     PrStageStatusBankActionKind::Call80019148,
                     kFn80019148,
                     true,
                     static_cast<int32_t>(kAddrSavePayloadBank80092F10));
                out.returnValue = 1;
            }
        } else {
            out.blockedByUnknownWord800916F0 = true;
            out.returnValueKnown = false;
        }
        return out;
    }

    if (input.word800916D0 == 1u) {
        out.writeWord800916DA = true;
        out.word800916DAFromLocalStash =
            input.comod == PrStageStatusBankComod::Comod1;
        out.word800916DAValue =
            ClearProducerD0ExitDaValue(input.comod, input.localStashValue);
        out.localStashPsxAddress = LocalStashAddress(input.comod);
        Emit(out.trace,
             PrStageStatusBankActionKind::StoreWord800916D0,
             0u,
             true,
             0);
        Emit(out.trace,
             PrStageStatusBankActionKind::StoreWord800916DA,
             0u,
             true,
             out.word800916DAValue);
        out.returnValue = 0;
        return out;
    }
    if (input.word800916D0 == 2u) {
        out.writeWord800916DA = true;
        out.word800916DAFromLocalStash =
            input.comod == PrStageStatusBankComod::Comod1;
        out.word800916DAValue =
            ClearProducerD0ExitDaValue(input.comod, input.localStashValue);
        out.localStashPsxAddress = LocalStashAddress(input.comod);
        Emit(out.trace,
             PrStageStatusBankActionKind::StoreWord800916D0,
             0u,
             true,
             0);
        Emit(out.trace,
             PrStageStatusBankActionKind::StoreWord800916E0,
             0u,
             true,
             3);
        Emit(out.trace,
             PrStageStatusBankActionKind::StoreWord800916DA,
             0u,
             true,
             out.word800916DAValue);
        out.returnValue = -1;
        return out;
    }

    if (input.word800916DA == 0u) {
        out.statusProducerCalled8001635C = true;
        out.statusA1 = input.stageArg;
        out.statusA3 = input.previousStatus800166AC;
        out.statusA4 = input.word80091816;
        out.statusA2 = input.statusGateIsZero ? 3 : 2;
        Emit(out.trace,
             PrStageStatusBankActionKind::Call8001635C,
             kFn8001635C,
             true,
             out.statusA1,
             true,
             out.statusA2,
             input.previousStatusKnown800166AC,
             out.statusA3,
             true,
             out.statusA4);

        if (!IsTerminalComod7(input.comod) && input.stageArg < 6) {
            out.nextStageUnlockCalled8001628C = true;
            out.nextStageUnlockArg = input.stageArg + 1;
            Emit(out.trace,
                 PrStageStatusBankActionKind::Call8001628C,
                 kFn8001628C,
                 true,
                 out.nextStageUnlockArg);
        }
        if (input.word800916F0Known) {
            if (input.word800916F0 != 1u) {
                out.saveMenuCalled = true;
                Emit(out.trace,
                     PrStageStatusBankActionKind::Call80015590,
                     kFn80015590,
                     true,
                     input.stageArg);
                Emit(out.trace,
                     PrStageStatusBankActionKind::Call80019148,
                     kFn80019148,
                     true,
                     static_cast<int32_t>(kAddrSavePayloadBank80092F10));
            }
        } else {
            out.blockedByUnknownWord800916F0 = true;
            out.returnValueKnown = false;
        }
    }

    if (input.word800916DA == 1u) {
        out.returnValue = input.stageArg < 3 ? input.stageArg + 1 : 0;
    } else {
        out.returnValue = input.stageArg < 6 ? input.stageArg + 1 : 0;
    }
    if (IsTerminalComod7(input.comod)) {
        out.returnValue = 0;
    }
    return out;
}

PrStageStatusBankTerminalConsumerResult
PrStageStatusBankDirectTerminalConsumer(
    const PrStageStatusBankTerminalConsumerInput& input) {
    PrStageStatusBankTerminalConsumerResult out{};
    if ((input.eventFlags04 & kRunnerFlag80) == 0u) {
        return out;
    }
    out.setUnk8008ED1C = true;
    out.unk8008ED1CValue = 1u;
    out.setCtx54 = true;
    out.ctx54Value = 1u;
    out.call800169E0 = true;
    Emit(out.trace,
         PrStageStatusBankActionKind::Call800169E0,
         kFn800169E0);
    return out;
}
