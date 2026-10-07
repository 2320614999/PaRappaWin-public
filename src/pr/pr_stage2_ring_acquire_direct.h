#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2RingAcquireDirect {
using Services=PrStage2LifecycleDirect::Services;
struct Frame {uint32_t payload=0,metadata=0;bool available=false;};
Frame AcquirePrivate8003958C(Services&);
int32_t Acquire8003958C(Services&,uint32_t payloadOut,uint32_t metadataOut);
void SetStream80039408(Services&,uint32_t mode,uint32_t first,uint32_t last,uint32_t callback,uint32_t loop);
void Locate80039650(Services&,uint32_t seek,uint32_t first,uint32_t last);
int32_t VlcSize80047B00(Services&,uint32_t size);
int32_t DecodeNext800273A4(Services&);
int32_t InitializeDecoder800274D4(Services&);
bool TryCall(Services&,uint32_t,std::initializer_list<uint32_t>,int32_t&);
bool TryCallVoid(Services&,uint32_t,std::initializer_list<uint32_t>);
}
