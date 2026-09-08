#include "pr/pr_stage1_scene1_movie1_direct.h"
#include "pr/pr_stage1_movie_text_direct.h"
#include <cstdio>
#include <cstdlib>
#include <memory>

// This test runs transition state/presentation boundaries, not text submission.
// Fail loudly if that unrelated host boundary is ever reached.
namespace PrStage1MovieTextDirect {
Movie1TextFastSpriteSequenceApplyResultSub8001B954 ApplyMovieTextWindowSubmitSub801C77C0(
    Movie1TextWindowTickResult&, PrPsxGraphOwnerDirect::PsxGraphState&,
    PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20*) {
    std::abort();
}
}

namespace Movie = PrStage1Scene1Movie1Direct;

static bool HasAction(const Movie::TransitionSub800201ACStep& step,
                      Movie::TransitionSub800201ACAction action) {
    for (unsigned i = 0; i < step.actionCount; ++i)
        if (step.actions[i] == action) return true;
    return false;
}

int main() {
    int failures = 0;
    const auto check = [&](bool value, const char* message) {
        if (!value) {
            if (failures < 24) std::printf("FAIL: %s\n", message);
            ++failures;
        }
    };
    // Current 80020110 mode 2, 800201AC modes 5/6, with DC on/off.
    // Counts follow 8001FCBC(+8), 80020308/80020248(16/31), and the
    // four 80020008/80020090 tail iterations. No renderer or clock overrides.
    for (unsigned mode : {2u, 5u, 6u}) {
        for (bool subtitles : {false, true}) {
            for (bool completesMovie : {false, true}) {
                auto state = std::make_unique<Movie::Movie1RuntimeState>();
                Movie::Movie1HostFeedback host{};
                host.subtitleEnabled = subtitles;
                PrMovieSubtitles::MovieSubtitleTrack track{};
                if (mode == 2)
                    Movie::BeginTransitionSub80020110(*state, 0x801C3640u, mode, 1, 2, 10, completesMovie);
                else
                    Movie::BeginTransitionSub800201AC(*state, 0x801C3640u, mode,
                        mode == 6 ? 2 : 1, mode == 6 ? 1 : 2, 10, completesMovie);
                unsigned loopFrames = 0, tailFrames = 0, calls = 0;
                while (state->transitionSub800201ACActive && calls++ < 256) {
                    const auto result = Movie::AdvanceRuntimePure(*state, host, track);
                    if (!state->transitionSub800201ACActive) {
                        check(completesMovie ? result.completedToStage1 : state->transitionSub800201ACCompleted,
                              "completion delivered after final presentation");
                        break;
                    }
                    const auto& step = state->transitionDrawStep;
                    check(state->transitionDrawStepValid, "active transition retains drawable frame");
                    const bool presents = HasAction(step, Movie::TransitionSub800201ACAction::Sub80035560_WaitGpu2)
                        && HasAction(step, Movie::TransitionSub800201ACAction::Sub80040CA4_PresentGp872Buffer);
                    check(presents, "each tick exposes exactly one native wait/present, no empty init/end frame");
                    check(result.transitionFrameReadyForPresent == presents,
                          "host drain yield signal follows actual original presentation actions");
                    if (step.phase == Movie::TransitionSub800201ACPhase::LoopSub8001EA74) ++loopFrames;
                    if (step.phase == Movie::TransitionSub800201ACPhase::TailSub80020090) {
                        check(step.tailIteration == tailFrames, "tail presentation index");
                        ++tailFrames;
                    }
                    // Querying twice represents separate 60 Hz renders of one
                    // 30 Hz logical frame: it must not mutate the animation.
                    const auto gp = state->outroGp196;
                    check(Movie::QueryDrawableState(*state).drawableActive, "first render remains drawable");
                    check(Movie::QueryDrawableState(*state).drawableActive && state->outroGp196 == gp,
                          "second render does not advance original logic");
                }
                check(loopFrames == (mode == 2 ? 24u : (subtitles ? 31u : 16u)), "native loop presentation count");
                check(tailFrames == 4u, "all four tail frames survive until presentation");
                check(!state->transitionSub800201ACActive && calls < 256, "bounded completion");
            }
        }
    }
    // A physical input is interpreted by 801C455C exactly once. Its completed
    // owner (including a one-frame Start/Down tap) must end video as well as
    // subtitles; the host must not re-poll a Select-only condition.
    auto movie = std::make_unique<Movie::Movie1RuntimeState>();
    movie->strStarted = true;
    Movie::Movie1HostFeedback input{};
    input.strPlayerReady = true;
    for (const uint32_t raw : {0u, 0x0100u, 0x0800u, 0x0040u, 0x0840u, 0x0020u}) {
        input.inputMaskSub80035510Known = true;
        input.inputMaskSub80035510 = raw;
        input.nativePlayAndWaitComplete801C455C = false;
        check(!Movie::BuildHostStrPollPlan(*movie, input).skipAllowed,
              "no second raw PAD interpretation during native warmup/main");
        input.nativePlayAndWaitComplete801C455C = true;
        check(Movie::BuildHostStrPollPlan(*movie, input).skipAllowed,
              "native completion survives physical button release and ends video");
    }
    input.nativePlayAndWaitComplete801C455C = false;
    input.debugF1StrSkipRequested = true;
    check(Movie::BuildHostStrPollPlan(*movie, input).skipAllowed, "Win F1 skip remains available");
    movie->outroActive = true;
    check(!Movie::BuildHostStrPollPlan(*movie, input).skipAllowed, "do not skip native removal transition");
    std::printf("Stage1 transition presentation + movie completion: %s (%d failures)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
