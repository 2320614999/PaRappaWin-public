#pragma once

#include "pr_stage1_lifecycle_executor_direct.h"
#include "pr_stage_status_bank_direct.h"

namespace PrStageStatusBankHostBridgeDirect {

PrStage1LifecycleExecutorDirect::StatusBankDirectMemoryFeedback801C81EC
ExecuteStatusBankDirectMemoryRequest801C81EC(
    const PrStageStatusBankDirectCallRequest& request);

} // namespace PrStageStatusBankHostBridgeDirect
