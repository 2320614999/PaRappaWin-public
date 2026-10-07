#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2ResourceSetupDirect {
using Services=PrStage2LifecycleDirect::Services;
int32_t SelectPortrait800246A8(Services&,uint32_t mode);
int32_t BindCamera800127C4(Services&,uint32_t camera);
int32_t LoadCamera800127F0(Services&,uint32_t source,uint32_t flags);
int32_t ResetCamera800128DC(Services&);
int32_t ResetFirstTrackState80024308(Services&);
int32_t ResetSecondTrackState80024390(Services&);
int32_t ParseVdfChannel80013994(Services&,uint32_t channel,uint32_t source);
int32_t InitializePrimaryMime800140E0(Services&,uint32_t dat,uint32_t vdf,uint32_t loop,uint32_t begin,uint32_t length);
int32_t InitializeMimeChannel80014050(Services&,uint32_t channel,uint32_t dat,uint32_t vdf,
                                      uint32_t datState,uint32_t datBuffer,uint32_t cursorOutput);
int32_t UpdateCameraFrame80012960(Services&,int32_t frame);
int32_t InitTracks80013D10(Services&,uint32_t object,uint32_t entries,uint32_t source,uint32_t mode);
int32_t ReadTrack80013DB8(Services&,uint32_t object,uint32_t index);
int32_t SetTrackFrame80013E04(Services&,uint32_t object,uint32_t frame);
int32_t ClearTrackWeights80013E40(Services&);
int32_t BindModelTable80013650(Services&,uint32_t source,uint32_t object);
void SaveModelVertices8001371C(Services&,uint32_t object,uint32_t output);
int32_t BindMorphTable800137BC(Services&,uint32_t source,uint32_t object);
int32_t InitMorphChannel8001385C(Services&,uint32_t channel,uint32_t model,uint32_t morph,uint32_t backup);
int32_t RestoreModelVertices800139F8(Services&,uint32_t channel);
int32_t ApplyMorphChannel80013AA8(Services&,uint32_t channel);
int32_t ApplyMorphVertices8003A5DC(Services&,uint32_t vertices,uint32_t deltas,uint32_t count,uint32_t weight);
int32_t ApplyTrackFrame80013EA8(Services&,uint32_t object,uint32_t frame,uint32_t channel);
int32_t InitMainMorph80014164(Services&,uint32_t tracks,uint32_t morph,uint32_t model,uint32_t mode);
int32_t AdvanceMainMorph800141D8(Services&);
int32_t RestoreMainMorph80014324(Services&);
bool TryCall(Services&,uint32_t,std::initializer_list<uint32_t>,int32_t&);
bool TryCallVoid(Services&,uint32_t,std::initializer_list<uint32_t>);
}
