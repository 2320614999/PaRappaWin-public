#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2SaveUiRender {
using Services=PrStage2LifecycleDirect::Services;
int32_t CoordinateSprite8001C5A8(Services&,uint32_t xy,uint32_t source,uint32_t priority);
int32_t CoordinateFrame8001C7A8(Services&,uint32_t xy,uint32_t source,uint32_t frame,uint32_t priority);
int32_t NameText8001C668(Services&,uint32_t text,uint32_t style);
int32_t BannerText8001C6E0(Services&,uint32_t text);
int32_t Background8001D74C(Services&,uint32_t priority,uint32_t buffer);
int32_t CardBanner80020A3C(Services&,uint32_t kind);
int32_t Prompt80022CBC(Services&,uint32_t kind,uint32_t context);
int32_t NameEntry80020BE4(Services&,uint32_t context);
int32_t CardList80020F94(Services&,uint32_t event,uint32_t context);
int32_t EventFrame8001E750(Services&,uint32_t event,uint32_t context);
bool TryCall(Services&,uint32_t function,std::initializer_list<uint32_t> args,int32_t& result);
}
