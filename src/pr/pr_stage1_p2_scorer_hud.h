#pragma once

#include <cstdint>

struct PrGameContext;
struct PrStage1HudPresentationDirectTemplates;

namespace PrStage1P2ScorerHud {

void Draw(PrGameContext& ctx,
          float vx,
          float vy,
          float vs,
          bool highLayoutMode,
          float scorePanelOffsetX,
          int32_t scoreDisplayValue,
          const PrStage1HudPresentationDirectTemplates& templates);

}  // namespace PrStage1P2ScorerHud
