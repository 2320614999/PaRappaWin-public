#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2LoadingWorkDirect {
// Original shared Loading/menu work initialization, called by 8001ED94.
// Invoke at the matching resident-scene boundary, NOT on every VBlank and
// NOT while a prepared frame or retained initializer owns these banks.
void Initialize8001E6D0(PrStage2LifecycleDirect::Services& memory);
void RestorePacketBanks8001E34C(PrStage2LifecycleDirect::Services& memory);
}
