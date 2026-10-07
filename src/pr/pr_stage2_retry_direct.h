#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2RetryDirect {
using Services = PrStage2LifecycleDirect::Services;
int32_t Initialize800267C8(Services&, uint32_t table);
int32_t ChangedPad80026744(Services&);
void InputSound80025C8C(Services&, uint32_t mask);
int32_t Choose80025F0C(Services&, uint32_t pad, uint32_t context);
int32_t TickCue80025E6C(Services&);
int32_t DrawPrompt800203D4(Services&, int32_t choice);
int32_t DrawRetryFrame8001E750(Services&, uint32_t context);
int32_t EndFrame8001EA00(Services&, uint32_t event);
int32_t PadVsync80026E2C(Services&);
int32_t FlushDebugText800436F0();
// The event-4 branch of the original common dispatcher. Other event owners
// remain separate; this loop retains the source release gate and result tail.
int32_t RunRetry80026B94(Services&, uint32_t argument);
}
