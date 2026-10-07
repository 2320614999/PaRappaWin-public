#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2SpuBootDirect {
using Services=PrStage2LifecycleDirect::Services;
int32_t InitializeSound80026E4C(Services&);
int32_t SsInitHot8002AC20(Services&);
int32_t SpuInit8002AC50(Services&);
int32_t SpuInitMode8002AC70(Services&,uint32_t warm);
int32_t ResetSpu8002961C(Services&,uint32_t warm);
int32_t StartEvents8002AD38(Services&);
int32_t SetDmaCallback8002ADBC(Services&,uint32_t callback);
int32_t InitializeLibrary8002ADE0(Services&);
int32_t InitializeVoices8003226C(Services&,uint32_t count);
void SetTickMode8002DA78(Services&,uint32_t mode);
int32_t StartTick8002B130(Services&);
int32_t StartTickMode8002AEC8(Services&,uint32_t alternate);
void CommonAttributes8002A6FC(Services&,uint32_t attributes);
void MasterVolume8002A6AC(Services&,uint32_t left,uint32_t right);
void SetMix8002AA90(Services&,uint32_t device,uint32_t kind,uint32_t enabled);
void ExternalVolume8002AB24(Services&,uint32_t device,uint32_t left,uint32_t right);
int32_t TransferCommand8002A1EC(Services&,uint32_t command,uint32_t first,uint32_t second);
int32_t TransferWrite8002A494(Services&,uint32_t source,uint32_t bytes);
void WriteFifo80029B38(Services&,uint32_t source,uint32_t bytes);
void DmaInterrupt80029E6C(Services&);
bool TryCall(Services&,uint32_t,std::initializer_list<uint32_t>,int32_t&);
bool TryCallVoid(Services&,uint32_t,std::initializer_list<uint32_t>);
}
