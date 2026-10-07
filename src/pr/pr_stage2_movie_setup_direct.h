#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2MovieSetupDirect {
using Services=PrStage2LifecycleDirect::Services;
void Volume8001A478(Services&,uint32_t volume);
void Mute8001A4A4(Services&,uint32_t mode);
int32_t Allocate80027238(Services&,uint32_t bytes,uint32_t unusedLabel);
int32_t Decoder80027288(Services&,uint32_t kind);
int32_t OutputCallback80027220(Services&);
void RingSlots8003954C(Services&,uint32_t first,uint32_t count);
void ClearRing80039260(Services&);
void SetRing8003624C(Services&,uint32_t buffer,uint32_t count);
int32_t SetOutputCallback80047658(Services&,uint32_t callback);
int32_t StopReset80027664(Services&);
int32_t Reset800473EC(Services&,uint32_t mode);
int32_t ResetLower8004767C(Services&,uint32_t mode);
int32_t InputWait8004789C(Services&);
int32_t Input80047778(Services&,uint32_t packet,uint32_t words);
bool TryCall(Services&,uint32_t,std::initializer_list<uint32_t>,int32_t&);
bool TryCallVoid(Services&,uint32_t,std::initializer_list<uint32_t>);
}
