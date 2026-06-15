#pragma once

#include <cstdint>

struct PrGameContext;
enum class PrStage1HdSubtitleSourceKind : uint8_t;

namespace PrStage1HdSubtitles {

bool HasActiveExternalSubtitle(PrGameContext& ctx);
bool ShouldSuppressNativeSubtitleText(PrGameContext& ctx);
bool ShouldSuppressNativeSubtitleFrame(PrGameContext& ctx);
void Preload(PrGameContext& ctx);
void ObserveNativeSubtitleTextRect(PrGameContext& ctx,
                                   PrStage1HdSubtitleSourceKind kind,
                                   float x,
                                   float y,
                                   float w,
                                   float h);
void Render(PrGameContext& ctx);
void ClearCache();

}
