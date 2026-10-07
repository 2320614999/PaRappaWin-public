#pragma once
#include <algorithm>

namespace PrDisplayViewport {
struct Fit { float x=0,y=0,width=0,height=0; };
inline Fit PreserveAspect(float width,float height,float sourceWidth,float sourceHeight) {
    if(width<=0 || height<=0 || sourceWidth<=0 || sourceHeight<=0)return {};
    const float scale=(std::min)(width/sourceWidth,height/sourceHeight);
    const float w=sourceWidth*scale,h=sourceHeight*scale;
    return {(width-w)*0.5f,(height-h)*0.5f,w,h};
}
// Transform already fitted legacy pixel coordinates into the requested output.
// Native Stage2 supplies its own packet viewport and does not use this mapping.
inline Fit LegacyOutput(float width, float height, int mode) {
    if (width <= 0 || height <= 0) return {};
    if (mode != 0) return {0, 0, width, height};
    const auto fit = PreserveAspect(width, height, 320, 240);
    const float sx = width / fit.width, sy = height / fit.height;
    return {-fit.x * sx, -fit.y * sy, width * sx, height * sy};
}
}
