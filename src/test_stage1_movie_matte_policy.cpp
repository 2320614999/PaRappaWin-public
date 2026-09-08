#include "pr/pr_stage1_scene1_movie1_direct.h"
#include <cstdio>

int main() {
    using namespace PrStage1Scene1Movie1Direct;
    int failures = 0;
    // The policy must be independent of the Windows resolution multiplier.
    for (const float scale : {1.0f, 2.0f, 4.0f}) {
        for (const auto helper : {
                 Movie1PsxDrawHelper::Sub8001CE30_NoSubtitleFrame,
                 Movie1PsxDrawHelper::Sub8001C864_SubtitleFrame,
                 Movie1PsxDrawHelper::Sub8001F230_OutroNoSubboxFrame,
                 Movie1PsxDrawHelper::Sub8001FEB4_FinalNoVideoFrame,
                 Movie1PsxDrawHelper::Sub800201AC_TransitionFrame}) {
            Movie1DrawPlan plan{};
            plan.frame.psxDrawHelper = helper;
            plan.frame.useFinalNoVideoLayout =
                helper == Movie1PsxDrawHelper::Sub8001FEB4_FinalNoVideoFrame;
            plan.video = {35.0f * scale, 25.0f * scale,
                          256.0f * scale, 144.0f * scale};
            const bool hasVideoWindow =
                helper == Movie1PsxDrawHelper::Sub8001CE30_NoSubtitleFrame ||
                helper == Movie1PsxDrawHelper::Sub8001C864_SubtitleFrame ||
                helper == Movie1PsxDrawHelper::Sub8001F230_OutroNoSubboxFrame;
            for (const bool video : {false, true}) {
                plan.drawVideo = video;
                if (ShouldSubmitEmptyVideoMatte(plan) != (hasVideoWindow && !video)) {
                    ++failures;
                }
            }
        }
    }
    std::printf("stage1_movie_matte_policy: %s (%d failures)\n",
                failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
