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
    bool ioResultKnown = false;
    int32_t ioResult = 0;
    bool cardIoStateBeforeKnown80017594 = false;
    bool cardIoStateAfterKnown80017594 = false;
    PrStage1SaveUiCardIoState80017594 cardIoStateBefore80017594{};
    PrStage1SaveUiCardIoState80017594 cardIoStateAfter80017594{};
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

SaveUi19148HostTickAttempt RunSaveUi19148HostTickAttempt(
    PrGameContext& ctx,
    const PrStage1SaveUi19148LowerFeedback* lowerFeedback = nullptr);

const PrPsxEventFrameDirect::EventFrameState8001E750*
GetActiveSaveUiEventFrameState8001E750();

} // namespace PrStage1SaveUiHostBridgeDirect
