#pragma once
#include "pr/pr_stage2_vram_device.h"
void RunStage2VramAtlasChecks(PrStage2LifecycleDirect::Services& memory,
                              const PrStage2VramDevice::Device& vram,
                              const std::vector<uint32_t>& sizes, uint32_t pass);
void RunStage2RuntimeTimChecks(PrStage2LifecycleDirect::Services& memory,
                              const PrStage2VramDevice::Device& vram,
                              const std::vector<uint32_t>& sizes, const char* modulePath);
