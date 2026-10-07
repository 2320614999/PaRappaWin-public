#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2TransitionDirect {
using Services=PrStage2LifecycleDirect::Services;
int32_t Reset8001FFD4(Services&,uint32_t mode);
int32_t Pending8001F518(Services&);
int32_t FourFrames80020090(Services&,uint32_t work,uint32_t mode,uint32_t prepare,uint32_t present);
int32_t FourFramesWithCue80020008(Services&,uint32_t work,uint32_t mode,uint32_t prepare,uint32_t present);
int32_t Transition800201AC(Services&,uint32_t work,uint32_t mode,uint32_t before,uint32_t after);
int32_t TransitionWithCue80020110(Services&,uint32_t work,uint32_t mode,uint32_t before,uint32_t after);
int32_t MovieEnter80020248(Services&,uint32_t subtitles);
int32_t MovieLeave80020308(Services&,uint32_t subtitles);
int32_t Border8001C864(Services&,uint32_t priority);
int32_t CaptionBorder8001CE30(Services&,uint32_t priority);
int32_t CommonBorder8001F230(Services&,uint32_t priority);
int32_t CommonTiles8001FEB4(Services&,uint32_t priority);
void SetPatternTile8001F698(Services&,uint32_t pattern,uint32_t value);
int32_t RevealTiles8001FC40(Services&,uint32_t pattern,uint32_t count);
int32_t CoverTiles8001FCBC(Services&,uint32_t count,uint32_t pattern);
int32_t DrawMaskedTiles8001FDC0(Services&,uint32_t priority);
int32_t TransitionCue800271E4(Services&,uint32_t index);
int32_t PeriodicCue80027194(Services&,uint32_t unused);
bool TryCall(Services&,uint32_t function,std::initializer_list<uint32_t> args,int32_t& result);
bool TryCallVoid(Services&,uint32_t function,std::initializer_list<uint32_t> args);
}
