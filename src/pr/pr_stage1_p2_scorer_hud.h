#pragma once

#include <cstdint>
struct ID3D11ShaderResourceView;

struct PrGameContext;
struct PrStage1HudPresentationDirectTemplates;

namespace PrStage1P2ScorerHud {

struct LabelTexture { ID3D11ShaderResourceView* srv=nullptr; float u0=0,v0=0,u1=1,v1=1,width=0,height=0; };
LabelTexture SharedLabel(PrGameContext& ctx, unsigned index);
LabelTexture SharedCreativePrompt(PrGameContext& ctx);
void Draw(PrGameContext& ctx,
          float vx,
          float vy,
          float vs,
          bool highLayoutMode,
          float scorePanelOffsetX,
          int32_t scoreDisplayValue,
          const PrStage1HudPresentationDirectTemplates& templates);

}  // namespace PrStage1P2ScorerHud
