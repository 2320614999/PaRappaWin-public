#pragma once

#include "pr_stage1_lifecycle_executor_direct.h"
#include "pr_stage1_loader_direct.h"

namespace PrStage1LifecycleExecutorDirect {

bool IsSameBootstrap15590CdLowerRuntimeKey801C81EC(
    const Bootstrap15590CdLowerProducerRuntime801C81EC& runtime,
    const PrStage1LoaderDirect::ProducerStep& step);

bool TryCarryBootstrap15590CdLowerSameKeyPendingRequest801C81EC(
    Bootstrap15590CdLowerProducerRuntime801C81EC& runtime,
    const PrStage1LoaderDirect::ProducerStep& step,
    Bootstrap15590CdLowerHostRequest801C81EC& out);

void MarkBootstrap15590CdLowerRequestPending801C81EC(
    Bootstrap15590CdLowerProducerRuntime801C81EC& runtime,
    const PrStage1LoaderDirect::ProducerStep& step,
    const Bootstrap15590CdLowerHostRequest801C81EC& request);

bool MarkBootstrap15590FinalReadyHalFactsRequired801C81EC(
    Bootstrap15590CdLowerProducerRuntime801C81EC& runtime,
    const Bootstrap15590CdLowerHostRequest801C81EC& request,
    uint32_t currentReadS27Serial);

bool IsBootstrap15590FinalReadyHalFactsRequirementCurrent801C81EC(
    const Bootstrap15590CdLowerProducerRuntime801C81EC& runtime,
    const Bootstrap15590CdLowerHostRequest801C81EC& request,
    uint32_t currentReadS27Serial);

bool IsReadSyncCompletionForPendingReadStart801C81EC(
    const Bootstrap15590CdLowerHostRequest801C81EC& expected,
    const PrStage1LoaderCdHal::LowerActionRequestMetadata& actual);

bool IsFinalReadyProgressGapForPendingReadStart801C81EC(
    const Bootstrap15590CdLowerHostRequest801C81EC& request,
    uint32_t currentReadS27Serial);
bool IsFinalReadyProgressGapForPendingReadStart801C81EC(
    const Bootstrap15590CdLowerObservableState801C81EC& observable,
    uint32_t currentReadS27Serial);

} // namespace PrStage1LifecycleExecutorDirect
