#pragma once

#include "pr_psx_event_frame_direct.h"
#include "pr_stage1_save_ui_direct.h"

#include <cstdint>

struct PrGameContext;

namespace PrStage1SaveUiHostBridgeDirect {

struct SaveUi19148HostTickAttempt {
    bool attempted = false;
    bool active = false;
    bool done = false;
    bool saveSucceeded = false;
    int32_t saveResult = 0;
    int32_t psxState = 0;
    int32_t psxEventId = 0;
    bool inputStateConsumes80018FB0 = false;
    bool inputDispatcherResultDrive80018FB0 = false;
    int32_t inputDispatcherResultPendingBefore80018FB0 = 0;
    int32_t inputMaskRaw80035510 = 0;
    int32_t inputMaskBeforeDedup80018FB0 = 0;
    int32_t inputMaskAfterDedup80018FB0 = 0;
    int32_t gp708LastInputBefore80018FB0 = 0;
    int32_t gp708LastInputAfter80018FB0 = 0;
    bool inputDuplicateSuppressed80018FB0 = false;
    bool inputHandled800185D0 = false;
    int32_t inputStateBefore800185D0 = 0;
    int32_t inputStateAfter800185D0 = 0;
    bool ioResultKnown = false;
    int32_t ioResult = 0;
    bool cardIoStateBeforeKnown80017594 = false;
    bool cardIoStateAfterKnown80017594 = false;
    PrStage1SaveUiCardIoState80017594 cardIoStateBefore80017594{};
    PrStage1SaveUiCardIoState80017594 cardIoStateAfter80017594{};
    bool consumedBy80019458State15Known = false;
    bool consumedBy80019458State15 = false;
    uint32_t state15PrefixAddress = 0;
    bool state15CopyTo8007ADE8Known = false;
    bool state15CopyTo8007ADE8 = false;
    bool saveWriteResultKnown80017A10 = false;
    int32_t saveWriteResult80017A10 = 0;
    bool saveWriteSucceeded80019458 = false;
    bool gp716After80019458Known = false;
    int32_t gp716After80019458 = 0;
    bool gp720After80019458Known = false;
    int32_t gp720After80019458 = 0;
    PrStage1SaveUi19148LowerFeedbackRequestList lowerFeedbackRequests{};
};

void ExecuteSaveUi19148HostActionRequests(
    const PrStage1SaveUi19148HostActionRequestList& requests);

bool StartSaveUiWithEntry19148(PrGameContext& ctx,
    const PrStage1SaveStatusPrefix80092F10& seed);
void ResetSaveUiPresentation19148();
bool IsSaveEntryTransitionActive19148();
bool HasPendingSaveUiPresentation19148();
void PumpSaveUiPresentation19148(PrGameContext& ctx);

SaveUi19148HostTickAttempt RunSaveUi19148HostTickAttempt(
    PrGameContext& ctx,
    const PrStage1SaveUi19148LowerFeedback* lowerFeedback = nullptr);

const PrPsxEventFrameDirect::EventFrameState8001E750*
GetActiveSaveUiEventFrameState8001E750();

} // namespace PrStage1SaveUiHostBridgeDirect
