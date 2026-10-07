#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// Presentation frames are 30 Hz units, with a fractional part at 60 Hz.
// Shared by both stages; no input, score or source clock is changed here.
namespace PrModernRailAnimation {
struct Pose { float x=1, y=1, glow=0, glowFrames=1; };
inline Pose Sample(float age, int popFrames, float popScale, int flipFrames, int glowFrames) {
    const float impact=float((std::max)(1,popFrames));
    const float flip=float((std::max)(1,flipFrames));
    Pose p;
    p.glowFrames=float((std::max)(1,(std::min)(glowFrames,int(impact))));
    age=(std::max)(0.0f,age);
    const float light=std::clamp(1.0f-age/p.glowFrames,0.0f,1.0f);
    p.glow=light*light*light;
    if(age<impact){
        const float tail=1.0f-age/impact;
        p.x=p.y=1.0f+((std::max)(1.0f,popScale)-1.0f)*tail*tail*tail;
    }else if(age<impact+flip){
        p.x=std::cos((age-impact)/flip*6.28318530717958647692f);
    }
    return p;
}

// Additive light stores its radial falloff in RGB, with opaque alpha. Both the
// Stage1 sprite shader and Stage2's native textured triangles preserve RGB;
// the latter intentionally treats texture alpha as a PSX color key only.
inline std::array<uint32_t, 64u * 64u> GlowPixels() {
    std::array<uint32_t, 64u * 64u> pixels{};
    for (int y = 0; y < 64; ++y) for (int x = 0; x < 64; ++x) {
        const float dx = (x + 0.5f - 32) / 32, dy = (y + 0.5f - 32) / 32;
        const float edge = std::clamp(1.0f - std::sqrt(dx * dx + dy * dy), 0.0f, 1.0f);
        const float smooth = edge * edge * (3.0f - 2.0f * edge);
        const auto light = uint32_t(std::pow(smooth, 1.35f) * 255);
        pixels[size_t(y) * 64 + x] = 0xFF000000u | light | (light << 8) | (light << 16);
    }
    return pixels;
}

struct GlowLayer { float size, r, g, b, alpha; };
inline std::array<GlowLayer, 2> GlowLayers(float noteSize, float scaleX, float scaleY,
                                        float glowScale, float alpha) {
    const float size = noteSize * (std::max)(std::abs(scaleX), std::abs(scaleY)) *
        (std::max)(1.0f, glowScale);
    alpha = std::clamp(alpha, 0.0f, 1.0f);
    return {{{size * 1.35f, 1.0f, 0.76f, 0.18f, alpha * 0.70f},
             {size * 0.78f, 1.0f, 0.96f, 0.58f, alpha * 0.55f}}};
}
}
