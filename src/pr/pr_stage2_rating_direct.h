#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2RatingDirect {
using Services=PrStage2LifecycleDirect::Services;
int32_t ResetJudge80014400(Services&);
int32_t CountEmpty80014458(Services&);
int32_t ConsumeRecovery800144B8(Services&,uint32_t work);
int32_t FailedCool80014538(Services&);
int32_t CompareRating80014548(Services&,uint32_t work);
int32_t PatternBonus80014A80(Services&,uint32_t history,uint32_t first,uint32_t end);
void ResetHistory80014BDC(Services&,uint32_t bar);
int32_t CoolDelta80014C80(Services&);
int32_t PositiveDelta80014D28(Services&);
int32_t Score80014D58(Services&,uint32_t work);
int32_t ClearPatternHits800152D0(Services&);
int32_t AdvanceFeedbackDelay80015350(Services&,uint32_t unusedWork,uint32_t elapsed);
int32_t UpdateRating80024FD0(Services&,uint32_t work);
bool TryCall(Services&,uint32_t function,std::initializer_list<uint32_t> args,int32_t& result);
bool TryCallVoid(Services&,uint32_t function,std::initializer_list<uint32_t> args);
}
