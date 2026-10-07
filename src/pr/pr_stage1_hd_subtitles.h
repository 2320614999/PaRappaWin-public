#pragma once

#include <cstdint>
#include <string>
struct ID3D11ShaderResourceView;

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
struct NativeTextTexture { ID3D11ShaderResourceView* srv=nullptr; float width=0,height=0; };
// Literal UTF-8 or a native Latin-1 caption with an optional exact-text sidecar.
// Dimensions use 320x240 presentation units; the cache owns the returned SRV.
NativeTextTexture RasterizeNativeText(PrGameContext& ctx, const std::string& text,
    const std::string& translationFile={}, bool nativeLatin1=false, float fontSize=0);

}
