#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2GameHudDirect {
using Services = PrStage2LifecycleDirect::Services;
// Shared game HUD and rail: the live native work block owns all visibility,
// rating, flash and wobble state. Private note arguments travel by value.
void DrawBanner8001DB9C(Services&, int32_t mode, uint32_t workIndex);
void DrawRatingFeedback8001DE08(Services&, uint32_t restart, uint32_t direction, int32_t offset);
int32_t DrawRating8001DF24(Services&, uint32_t work, int32_t layout);
int32_t DrawStatus8001E2E4(Services&, uint32_t work, int32_t layout);
void AdvanceTeacher80023F20(Services&, int32_t count);
void AdvanceStudent80024114(Services&, int32_t count);
int32_t DrawRail80024744(Services&, uint32_t work);
void DrawNote80024418(Services&, uint16_t x, uint16_t y, uint16_t slot, uint16_t type);
}
