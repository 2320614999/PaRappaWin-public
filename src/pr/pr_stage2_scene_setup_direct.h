#pragma once
#include "pr_stage2_lifecycle_direct.h"
namespace PrStage2SceneSetupDirect {
using Services=PrStage2LifecycleDirect::Services;
void ZeroBytes80025C44(Services& s,uint32_t destination,int32_t count);
int32_t ResetRating8001448C(Services& s);
int32_t ResetFeedback80014C1C(Services& s);
int32_t ResetInputRow80024F8C(Services& s,uint32_t work);
int32_t InitializeWorkRating80024FC0(Services& s,uint32_t work);
void ResetReplay80024E54(Services& s,uint32_t reserve);
int32_t LoadReplay8001681C(Services& s);
int32_t ReplayDifficulty80016758(Services& s,uint32_t unusedScene);
int32_t SaveStageIndex8001615C(Services& s,uint32_t scene);
int32_t SavedStageRating800166AC(Services& s,uint32_t scene);
int32_t StageDifficulty8001670C(Services& s,uint32_t scene);
int32_t AllStagesClear800161F4(Services& s,uint32_t ratings);
int32_t UnlockStage8001628C(Services& s,uint32_t scene);
int32_t RecordClear8001635C(Services& s,uint32_t scene,uint32_t rating,
                          uint32_t previousRating,uint32_t score);
int32_t BackupSave80015700(Services& s,uint32_t source);
int32_t RestoreSave80015744(Services& s,uint32_t destination);
int32_t LoadSave800164B4(Services& s,uint32_t source);
int32_t RestoreReplayScore800169E0(Services& s,uint32_t work);
void SetScorerDifficulty800143F0(Services& s,uint32_t difficulty);
void SetEventDifficulty800259C0(Services& s,uint32_t difficulty);
int32_t ResetEventState80024E98(Services& s);
int32_t ResetScorer80014344(Services& s);
int32_t ResetTransition8001EF14(Services& s);
int32_t DecodeDemoPad80024BC0(Services& s,uint32_t input);
int32_t ConfigureTextRect8001BC48(Services& s,uint32_t x,uint32_t y,
                               uint32_t width,uint32_t height);
// The S2 caller transports four argument registers. Original 1BC78 consumes
// only a0/a1/a2; its incoming a3 is intentionally not a fourth font setting.
int32_t ConfigureTextTexture8001BC78(Services& s,uint32_t u,uint32_t v,
                                  uint32_t palette,uint32_t unusedA3);
bool TryCall(Services& s,uint32_t function,std::initializer_list<uint32_t> args,int32_t& result);
bool TryCallVoid(Services& s,uint32_t function,std::initializer_list<uint32_t> args);
}
