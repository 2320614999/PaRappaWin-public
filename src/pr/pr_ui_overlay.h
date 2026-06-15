#pragma once

#include <cstdint>

class D3D11Renderer;
struct ID3D11ShaderResourceView;
struct PrGameContext;

namespace PrUiOverlay {

void Init(D3D11Renderer* renderer);
void Shutdown();

void Render(PrGameContext& ctx);

void DrawTextUi(float x, float y, float s, const char* text, float r, float g, float b, float a);
bool RenderEvent4PromptNative(PrGameContext& ctx, int selectionState);

float MeasureSubtitleTextNative(float s, const char* text);
bool DrawSubtitleTextNative(PrGameContext& ctx, float x, float y, float s, const char* text, float r, float g, float b, float a);

bool SubmitSystemTextBlockW(PrGameContext& ctx,
                            float vx, float vy, float vs,
                            const char* cacheKey,
                            float x, float y, float baseW,
                            const wchar_t* line1, const wchar_t* line2,
                            float r, float g, float b, float a,
                            int layer);

void ClearSystemTextCache();

}
