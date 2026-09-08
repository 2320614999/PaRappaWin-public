#pragma once

#include <cstdint>

#include "pr_movie_subtitles.h"

struct PrGameContext;

namespace PrScn0 {
    using MovieSubtitleLine = PrMovieSubtitles::MovieSubtitleLine;
    using MovieSubtitleTrack = PrMovieSubtitles::MovieSubtitleTrack;

    int Fn0(PrGameContext& ctx);
    void Fn1(PrGameContext& ctx);
    int Fn2(PrGameContext& ctx);
    void Main(PrGameContext& ctx);
    void Render(PrGameContext& ctx);
    void RenderLateSubtitles(PrGameContext& ctx);

    uint32_t ComputeSubtitleStageFrameFromStr(const PrGameContext& ctx, int frameOffset);

    int GetPhaseDebug();
}
