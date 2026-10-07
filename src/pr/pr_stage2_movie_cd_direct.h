#pragma once
#include "pr_stage2_lifecycle_direct.h"
#include "pr_stage2_cd_parameters.h"

namespace PrStage2MovieCdDirect {
using Services=PrStage2LifecycleDirect::Services;
int32_t WaitCleanup8001A694(Services&);
int32_t OpenStream8001A4D0(Services&,uint32_t record,uint32_t mode);
int32_t SelectAudioChannel8001A654(Services&,uint32_t channel);
int32_t Control800367A4(Services&,uint32_t command,uint32_t params,uint32_t result);
int32_t ControlCommand80036540(Services&,uint32_t command,uint32_t params,uint32_t result);
int32_t ControlPrivate80036540(Services&,uint32_t command,const PrStage2CdParameters::View&,uint32_t result);
int32_t ControlNonblocking80036678(Services&,uint32_t command,uint32_t params);
int32_t Sync800364D0(Services&,uint32_t mode,uint32_t result);
bool TryCall(Services&,uint32_t,std::initializer_list<uint32_t>,int32_t&);
}
