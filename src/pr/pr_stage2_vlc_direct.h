#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2VlcDirect {
using Services=PrStage2LifecycleDirect::Services;
// 47E20..47E30 sets only SR.SwC, not interrupt enables or DMA completion.
// A native owner preserves this one known status-bit update; unrelated CPU
// state is neither seeded nor reconstructed by the VLC decoder.
struct CacheControl {
    virtual ~CacheControl()=default;
    virtual void SetSwapCacheBit80047E30()=0;
};
int32_t Decode80047B30(Services&,CacheControl&,uint32_t source,uint32_t destination);
int32_t ReleaseFrame80039490(Services&,uint32_t payload);
}
