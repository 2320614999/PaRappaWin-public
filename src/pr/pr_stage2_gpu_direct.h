#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2GpuDirect {
using PrStage2LifecycleDirect::Services;
// Original SCUS queue and DMA register programming. Services owns actual
// MMIO status, interrupt-mask changes, DMA callbacks and VBlank query.
// No host completion, timing clamp or renderer callback is invented here.
int32_t ResetTimeout80047144(Services& s);
int32_t CheckTimeout80047178(Services& s);
int32_t StartLinkedDma80046840(Services& s,uint32_t head);
int32_t DrainQueue80046BC4(Services& s);
int32_t Enqueue800468E0(Services& s,uint32_t function,uint32_t source,
                       uint32_t copyBytes,uint32_t argument);
int32_t DrawOrderingTable800450A0(Services& s,uint32_t head);
int32_t DrawSync80044B3C(Services& s,uint32_t mode);
int32_t SyncQueue80046FFC(Services& s,uint32_t mode);
int32_t ClearOrderingTable80044FA8(Services& s,uint32_t table,uint32_t count);
int32_t StartOtcDma80045FC4(Services& s,uint32_t table,uint32_t count);
// Original RECT diagnostics are advisory; even a bad RECT is still submitted.
// This entry accepts actual public PSX RAM, not a fabricated private-stack slot.
int32_t CheckImageRect80044BA8(Services& s,uint32_t operation,uint32_t rect);
int32_t ClearImage80044CD0(Services& s,uint32_t rect,uint32_t red,
                         uint32_t green,uint32_t blue);
int32_t ClearImage800460AC(Services& s,uint32_t rect,uint32_t color);
// Native private RECT path; use ImageSourceServices for pointer-field storage.
int32_t ClearLocalImage80044CD0(Services& s,PrStage2LifecycleDirect::ImageRect rect,
                              uint32_t red,uint32_t green,uint32_t blue);
int32_t ClearFramebuffer8001B1B0(Services& s,uint32_t red,uint32_t green,uint32_t blue);
// 原版 MoveImage：保留诊断、读取次序、共享包和驱动队列返回值。
int32_t MoveImage80044E2C(Services& s,uint32_t rect,uint32_t x,uint32_t y);
// 原始栈 RECT 用本机值传递；最终提交的 GPU 包仍在原版公共 RAM 中。
int32_t MoveFramebuffer8001B120(Services& s,uint32_t argument);
int32_t ReadGpuInfo8004688C(Services& s,uint32_t command);
int32_t CurrentDrawBuffer8004019C(Services& s);
void SetWorkBase80040F90(Services& s,uint32_t packet);
void SetGeomScreen80040C94(Services& s,uint32_t screen);
void SetProjection80040C74(Services& s,uint32_t screen);
void SetGeomOffset800402C0(Services& s,uint32_t x,uint32_t y);
int32_t VideoMode8003623C(Services& s);
int32_t WriteGpuControl800467B4(Services& s,uint32_t command);
int32_t FillBytes800473C0(Services& s,uint32_t destination,uint32_t value,uint32_t count);
int32_t DisplayX80045EF0(Services& s,uint32_t environment);
int32_t SetDisplayMask80044AA0(Services& s,uint32_t enabled);
int32_t PutDisplayEnvironment800452EC(Services& s,uint32_t environment);
int32_t PackDrawAreaStart80045C8C(Services& s,uint32_t x,uint32_t y);
int32_t PackDrawAreaEnd80045D58(Services& s,uint32_t x,uint32_t y);
int32_t PackDrawOffset80045E24(Services& s,uint32_t x,uint32_t y);
int32_t PackDrawMode80045C30(Services& s,uint32_t drawToDisplay,uint32_t dither,uint32_t tpage);
int32_t PackTextureWindow80045E6C(Services& s,uint32_t rectangle);
int32_t BuildDrawEnvironment8004598C(Services& s,uint32_t packet,uint32_t environment);
int32_t PutDrawEnvironment80045114(Services& s,uint32_t environment);
int32_t ApplyDrawClip800402E0(Services& s);
int32_t ApplyFrameOffset800401AC(Services& s);
int32_t SwapBuffers80040370(Services& s);
}
