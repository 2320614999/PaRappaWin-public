#pragma once
#include "pr_stage2_lifecycle_direct.h"
namespace PrStage2CdStreamDirect {
using Services=PrStage2LifecycleDirect::Services;
int32_t Start800391AC(Services&,uint32_t mode);
void Stop800392C0(Services&);
int32_t Ready80039240(Services&);
int32_t Dma80039318(Services&);
bool TryCall(Services&,uint32_t,std::initializer_list<uint32_t>,int32_t&);
bool TryCallVoid(Services&,uint32_t,std::initializer_list<uint32_t>);
}
