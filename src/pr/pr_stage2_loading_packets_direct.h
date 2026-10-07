#pragma once
#include "pr_stage2_lifecycle_direct.h"
#include <initializer_list>

namespace PrStage2LoadingPacketsDirect {
using Services = PrStage2LifecycleDirect::Services;
// Native carrier for a source-private Gs sprite. No fabricated RAM/stack
// address, no implicit RGB producer, and no extra graphics state owner.
struct PrivateSprite {
    uint32_t attributes = 0;
    uint16_t x = 0, y = 0, width = 0, height = 0, page = 0;
    uint8_t u = 0, v = 0;
    uint16_t clutX = 0, clutY = 0;
};
int32_t SortPrivateSprite8003FA20(Services&,const PrivateSprite&,uint32_t table,
                                 uint32_t priority,uint32_t incomingV0);
// SCUS-backed packet writes. Private sprite/box locals stay native objects:
// they are never assigned invented PSX stack addresses.
int32_t GpuType80044A24(Services&);
int32_t TexturePage80043DF4(Services&,uint32_t depth,uint32_t blend,uint32_t x,uint32_t y);
int32_t SpritePrefix8001B25C(Services&,uint32_t local,uint32_t source,uint32_t frame,uint32_t clutAdvance);
int32_t SelectionPrefix8001B428(Services&,uint32_t local,uint32_t source,uint32_t selected);
int32_t SelectionSprite8001B5F4(Services&,uint32_t x,uint32_t y,uint32_t source,uint32_t selected,uint32_t priority,uint32_t table);
int32_t Link8003EF5C(Services&,uint32_t packet,uint32_t table,uint32_t priority,uint32_t words);
// The original skip branch leaves V0 untouched: callers with raw RAM inputs
// must provide its actual incoming value rather than claiming it returns zero.
int32_t SortSprite8003FA20(Services&,uint32_t local,uint32_t table,uint32_t priority,uint32_t incomingV0);
int32_t SortBox8003EE84(Services&,uint32_t local,uint32_t table,uint32_t priority,uint32_t incomingV0);
int32_t Sprite8001B590(Services&,uint32_t x,uint32_t y,uint32_t source,uint32_t frame,uint32_t clutAdvance,uint32_t priority,uint32_t table);
// 8001BEE4 -> 8001BE34 private fast-sprite wrapper. The source descriptor is
// read by value; no PSX stack address is exposed to the host.
int32_t FastSprite8001BEE4(Services&,uint32_t x,uint32_t y,uint32_t source,
                           uint32_t alternateClut,uint32_t table);
int32_t Box8001B6C4(Services&,uint32_t x,uint32_t y,uint32_t width,uint32_t height,uint32_t color,uint32_t priority,uint32_t table);
int32_t WorkSprite8001C550(Services&,uint32_t x,uint32_t y,uint32_t source,uint32_t priority);
int32_t WorkBox8001C4EC(Services&,uint32_t x,uint32_t y,uint32_t width,uint32_t height,uint32_t color,uint32_t priority);
bool TryCall(Services&,uint32_t function,std::initializer_list<uint32_t> args,int32_t& result);
}
