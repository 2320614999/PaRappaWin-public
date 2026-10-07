#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2TimDirect {
using PrStage2LifecycleDirect::Services;
int32_t GetTimInfo80040EAC(Services& s, uint32_t flags, uint32_t output);
int32_t UploadTim8001AE7C(Services& s, uint32_t tim);
int32_t GetClut80043EBC(uint32_t x, uint32_t y);
int32_t UploadClut800431E0(Services& s, uint32_t source, uint32_t x, uint32_t y);
int32_t UploadRuntimeTim8001ADEC(Services& s, uint32_t tim, uint32_t uploadClut);
}
