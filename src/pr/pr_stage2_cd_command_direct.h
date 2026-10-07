#pragma once
#include "pr_stage2_lifecycle_direct.h"
#include "pr_stage2_cd_parameters.h"
#include <array>

namespace PrStage2CdCommandDirect {
using Services=PrStage2LifecycleDirect::Services;
int32_t Command800375BC(Services&,uint32_t command,uint32_t parameters,uint32_t result,uint32_t nonblocking);
int32_t CommandParameters800375BC(Services&,uint32_t command,const PrStage2CdParameters::View&,uint32_t result,uint32_t nonblocking);
int32_t Sync80037070(Services&,uint32_t nonblocking,uint32_t result);
// Same source operation for the movie callers' private result buffers. No
// synthetic PSX stack address; bytes are valid only after the original copy.
struct Reply { int32_t status=0; std::array<uint8_t,8> bytes{}; bool copied=false; };
Reply SyncReply80037070(Services&,uint32_t nonblocking);
int32_t Interrupt80036AF8(Services&);
int32_t Dispatch80038118(Services&);
int32_t CheckCallback80035898(Services&);
int32_t Reset80037A8C(Services&);
int32_t Reset80036430(Services&);
int32_t ReadyCallback80036528(Services&,uint32_t callback);
int32_t SyncCallback80036510(Services&,uint32_t callback);
int32_t DmaCallback80036930(Services&,uint32_t callback);
int32_t Rebind80037C60(Services&);
int32_t Initialize80037CB0(Services&);
bool TryCall(Services&,uint32_t,std::initializer_list<uint32_t>,int32_t&);
}
