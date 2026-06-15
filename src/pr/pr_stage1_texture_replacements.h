#pragma once

#include <cstdint>

struct ID3D11ShaderResourceView;
struct PrGameContext;

namespace PrStage1TextureReplacements {

struct ReplacementTexture {
    ID3D11ShaderResourceView* srv = nullptr;
    int sourceWidth = 0;
    int sourceHeight = 0;
    int imageWidth = 0;
    int imageHeight = 0;
    bool hasTint = false;
    float tintR = 1.0f;
    float tintG = 1.0f;
    float tintB = 1.0f;
};

uint64_t HashRgba(const uint32_t* rgba, int width, int height, int stridePixels);

bool TryFindReplacement(PrGameContext& ctx,
                        const uint32_t* rgba,
                        int width,
                        int height,
                        int stridePixels,
                        uint64_t sourceKey,
                        ReplacementTexture& out);

bool TryFindReplacementBySourceKey(PrGameContext& ctx,
                                   uint64_t sourceKey,
                                   ReplacementTexture& out);

bool TryFindReplacementByAtlasRect(PrGameContext& ctx,
                                   uint16_t tpage,
                                   uint16_t clut,
                                   int minU,
                                   int minV,
                                   int maxU,
                                   int maxV,
                                   ReplacementTexture& out,
                                   int& outSourceX,
                                   int& outSourceY,
                                   int& outSourceW,
                                   int& outSourceH);

void DebugLogSourceKeyLookup(const PrGameContext& ctx,
                             uint64_t sourceKey,
                             const char* kind,
                             const char* detail,
                             bool hit);

void Prime(PrGameContext& ctx);

void Reset();

}  // namespace PrStage1TextureReplacements
