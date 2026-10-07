#pragma once
#include "pr_stage2_gpu_direct.h"

namespace PrStage2GraphInitDirect {
using PrStage2LifecycleDirect::Services;
void InitGte80040D20(Services& s);

// Original SCUS graph setup, sharing the RAM/queue/environment implementation
// with Stage2. ResetCallback, BIOS GPU_cw, GTE/CP0 initialization and interrupt
// masks remain explicit Services dependencies; these are not success stubs.
int32_t DetectGpu800472E4(Services& s, uint32_t mode);
int32_t ResetGpu80046EC0(Services& s, uint32_t mode);
int32_t ResetGraph800446A0(Services& s, uint32_t mode);
int32_t InitGraphEnvironment8003FC14(Services& s, uint32_t width, uint32_t height,
    uint32_t flags, uint32_t dither, uint32_t rgb24);
int32_t InitClearPacket800442F4(Services& s, uint32_t packet);
int32_t InitGeometry8003FDE4(Services& s, uint32_t width, uint32_t height);
int32_t InitGraph8003FB9C(Services& s, uint32_t width, uint32_t height,
    uint32_t flags, uint32_t dither, uint32_t rgb24);
int32_t SetDoubleBufferOffsets80040AE4(Services& s, uint32_t x0, uint32_t y0,
    uint32_t x1, uint32_t y1);
int32_t SetScreenCenter80040B84(Services& s);
}
