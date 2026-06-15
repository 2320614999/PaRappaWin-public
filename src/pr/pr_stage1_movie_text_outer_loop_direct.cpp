#include "pr_stage1_movie_text_outer_loop_direct.h"

#include "pr_stage1_lifecycle_executor_direct.h"
#include "pr_stage1_movie_segment_direct.h"

#include <system_error>

namespace PrStage1MovieTextOuterLoopDirect {
namespace {

using PrStage1MovieTextDirect::AdvanceCommonLyricsSub80024CF8;
using PrStage1MovieTextDirect::BeginPlayAndWaitSub801C77C0;
using PrStage1MovieTextDirect::EndPlayAndWaitSub801C77C0;
using PrStage1MovieTextDirect::GetCommonLyricsDescAddrSub80024C84;
using PrStage1MovieTextDirect::kMovie1CommonLyricsModeCount;

static MovieTextOuterLoopActionSub801C455C* PushAction(
    MovieTextOuterLoopStepResultSub801C455C& result,
    MovieTextOuterLoopActionKindSub801C455C kind,
    uint32_t psxFunctionAddr,
    uint32_t arg0 = 0,
    uint32_t arg1 = 0,
    uint32_t value = 0,
    bool appliedToDirectRuntime = false,
    bool gap = false,
    bool valueKnown = false) {
    if (result.actionCount >= result.actions.size()) {
        return nullptr;
    }

    MovieTextOuterLoopActionSub801C455C& action =
        result.actions[result.actionCount++];
    action.kind = kind;
    action.psxFunctionAddr = psxFunctionAddr;
    action.arg0 = arg0;
    action.arg1 = arg1;
    action.value = value;
    action.appliedToDirectRuntime = appliedToDirectRuntime;
    action.gap = gap;
    action.valueKnown = valueKnown;
    return &action;
}

static void EnterCleanup(MovieTextOuterLoopRuntimeSub801C455C& outer,
                         MovieTextOuterLoopExitReasonSub801C455C reason) {
    outer.exitReason = reason;
    outer.phase = MovieTextOuterLoopPhaseSub801C455C::Cleanup;
    outer.cleanupEntered = true;
}

static void RunCleanupSub801C455C(
    MovieTextOuterLoopRuntimeSub801C455C& outer,
    PrStage1MovieTextDirect::Movie1TextRuntime& textRuntime,
    MovieTextOuterLoopStepResultSub801C455C& result) {
    PushAction(result,
               MovieTextOuterLoopActionKindSub801C455C::CleanupSub80027664,
               0x80027664u);
    PushAction(result,
               MovieTextOuterLoopActionKindSub801C455C::CleanupSub8001A4A4Arg1,
               0x8001A4A4u,
               1);
    PushAction(result,
               MovieTextOuterLoopActionKindSub801C455C::CleanupSub8001A694,
               0x8001A694u);
    PushAction(result,
               MovieTextOuterLoopActionKindSub801C455C::CleanupNullsub4Ctx,
               0x80024CF0u);
    PushAction(result,
               MovieTextOuterLoopActionKindSub801C455C::CleanupSub8001B120Arg1,
               0x8001B120u,
               1);
    EndPlayAndWaitSub801C77C0(textRuntime);
    outer.phase = MovieTextOuterLoopPhaseSub801C455C::Complete;
    outer.complete = true;
    result.cleanupComplete = true;
}

static bool IsKnownDescMode(uint8_t mode) {
    return mode < kMovie1CommonLyricsModeCount;
}

static bool TryResolveStage1MovieSegmentIso9660BinPath801C455C(
    const std::filesystem::path& dataRoot,
    std::filesystem::path& outPath) {
    outPath.clear();
    std::error_code ec;
    std::filesystem::path p = dataRoot;
    for (int i = 0; i < 8 && !p.empty(); ++i) {
        const std::filesystem::path candidate =
            p / "PaRappa the Rapper.bin";
        if (std::filesystem::exists(candidate, ec)) {
            outPath = candidate;
            return true;
        }
        const std::filesystem::path parent = p.parent_path();
        if (parent == p) {
            break;
        }
        p = parent;
    }

    p = std::filesystem::current_path(ec);
    for (int i = 0; i < 8 && !p.empty(); ++i) {
        const std::filesystem::path candidate =
            p / "PaRappa the Rapper.bin";
        if (std::filesystem::exists(candidate, ec)) {
            outPath = candidate;
            return true;
        }
        const std::filesystem::path parent = p.parent_path();
        if (parent == p) {
            break;
        }
        p = parent;
    }
    return false;
}

static int32_t PsxCall80036A78_MsfToLba(uint8_t mm, uint8_t ss, uint8_t ff) {
    PrStage1MovieSegmentDirect::MsfBcd80036A78 msf{};
    msf.minute = mm;
    msf.second = ss;
    msf.frame = ff;
    return PrStage1MovieSegmentDirect::PsxCall80036A78_MsfToLba(msf).lba;
}

static PrMovieSegmentDirect::StreamClockPollResult8001A3C8
PushSub801C448CWrapperActions(
    MovieTextOuterLoopStepResultSub801C455C& result,
    PrStage1XaCdDirectState* xaCdState) {
    PrMovieSegmentDirect::StreamClockPollResult8001A3C8 clockPoll{};
    PushAction(result,
               MovieTextOuterLoopActionKindSub801C455C::WaitGpuSub801C448CArg2,
               0x80035560u,
               2);
    MovieTextOuterLoopActionSub801C455C* snapshotAction =
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::
                       SnapshotWorkBaseSub8001A3C8,
                   0x8001A3C8u);
    if (xaCdState != nullptr) {
        clockPoll = PrStage1XaCdDirectApplySub8001A3C8ClockPollFromLowerState(
            *xaCdState);
        if (snapshotAction != nullptr) {
            snapshotAction->appliedToDirectRuntime = true;
            snapshotAction->gap = !clockPoll.dword80049428Known;
            snapshotAction->valueKnown = clockPoll.dword80049428Known;
            snapshotAction->value = clockPoll.dword80049428Known
                                        ? static_cast<uint32_t>(
                                              clockPoll.dword80049428)
                                        : 0u;
        }
    }
    PushAction(result,
               MovieTextOuterLoopActionKindSub801C455C::InitSub801C448CSub8001A280,
               0x8001A280u);
    return clockPoll;
}

static bool StepSub801C4350(
    MovieTextOuterLoopRuntimeSub801C455C& outer,
    MovieTextOuterLoopStepResultSub801C455C& result,
    const MovieTextOuterLoopInputSub801C455C& input,
    bool& value) {
    const bool looseSegmentWindowFactsKnown =
        input.segmentTimeBaseA1Plus40Known ||
        input.segmentEndA1Plus44Known ||
        input.segmentEndBiasA1Plus8Known;
    const bool scanCarrierAuthority =
        input.segmentWindowFieldsFromDirectCarrier &&
        input.outerLoopInputAuthorityFromScanCarrier;
    if (looseSegmentWindowFactsKnown && !scanCarrierAuthority) {
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::
                       GapHostDerivedSegmentWindowFields,
                   kSub801C4350,
                   0,
                   0,
                   0,
                   false,
                   true);
    }

    const bool clockComputed = input.byte800493F4Known;
    const int32_t clockValue = clockComputed ?
        PsxCall80036A78_MsfToLba(input.byte800493F4_00,
                                 input.byte800493F4_01,
                                 input.byte800493F4_02) :
        0;
    if (clockComputed) {
        result.sub80036A78Computed = true;
        result.sub80036A78Result = clockValue;
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::
                       ResolveMsfToLbaSub80036A78,
                   0x80036A78u,
                   input.byte800493F4_00,
                   input.byte800493F4_01,
                   static_cast<uint32_t>(clockValue),
                   true,
                   false,
                   true);
    }

    const bool clockKnown = clockComputed;
    if (!clockKnown) {
        result.gapMissingByte800493F4ClockProducer = true;
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::
                       GapByte800493F4ClockProducer,
                   0x8001A7A4u,
                   0x800493F4u,
                   0x80036A78u,
                   0,
                   false,
                   true,
                   false);
    }
    const bool directSegmentWindowFieldsKnown = scanCarrierAuthority;
    const bool counterComputed =
        clockKnown && directSegmentWindowFieldsKnown &&
        input.segmentTimeBaseA1Plus40Known;
    const int32_t counterValue = counterComputed ?
        clockValue - input.segmentTimeBaseA1Plus40 :
        input.sub8001A7A4Result;
    bool watchdogKnownThisCall = false;
    int32_t watchdogValueThisCall = 0;
    if (clockKnown) {
        outer.dword80049404 = clockValue + 150;
        outer.dword80049404Known = true;
        watchdogKnownThisCall = true;
        watchdogValueThisCall = outer.dword80049404;
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::
                       WriteWatchdogDword80049404Sub8001A7A4,
                   0x8001A7A4u,
                   0,
                   0,
                   static_cast<uint32_t>(outer.dword80049404),
                   clockComputed,
                   false,
                   clockComputed);
    } else if (input.dword80049404Known) {
        outer.dword80049404 = input.dword80049404;
        outer.dword80049404Known = true;
        watchdogKnownThisCall = true;
        watchdogValueThisCall = outer.dword80049404;
    }

    if (counterComputed) {
        result.sub8001A7A4Computed = true;
        result.sub8001A7A4Result = counterValue;
    } else if (input.sub8001A7A4Known) {
        result.sub8001A7A4Result = input.sub8001A7A4Result;
    }

    PushAction(result,
               MovieTextOuterLoopActionKindSub801C455C::QueryMovieCounterSub8001A7A4,
               0x8001A7A4u,
               0,
               0,
               counterComputed || input.sub8001A7A4Known ?
                   static_cast<uint32_t>(counterValue) : 0u,
               counterComputed || input.sub8001A7A4Known,
               !(counterComputed || input.sub8001A7A4Known),
               counterComputed || input.sub8001A7A4Known);
    if (!(counterComputed || input.sub8001A7A4Known)) {
        outer.sub801C448CGap = true;
        return false;
    }

    const int32_t counter = counterValue;
    value = true;
    if (counter < 0) {
        return true;
    }

    const int32_t forwardDelta = counter - outer.dword801C954C;
    if (forwardDelta <= 0) {
        if (outer.dword801C954C - counter >= 301) {
            return true;
        }
    } else if (forwardDelta >= 301) {
        return true;
    }

    outer.dword801C954C = counter;
    const int32_t frameUnit = counter / 5;
    outer.ctxWord04 = static_cast<uint16_t>(frameUnit / 1800);
    outer.ctxByte07 = static_cast<uint8_t>(frameUnit % 30);
    outer.ctxByte06 = static_cast<uint8_t>((frameUnit % 1800) / 30);
    PushAction(result,
               MovieTextOuterLoopActionKindSub801C455C::UpdateCtxTimecodeSub801C4350,
               kSub801C4350,
               outer.ctxWord04,
               outer.ctxByte06,
               outer.ctxByte07,
               true,
               false,
               true);

    const bool endedComputed =
        directSegmentWindowFieldsKnown &&
        input.segmentEndA1Plus44Known &&
        input.segmentEndBiasA1Plus8Known &&
        watchdogKnownThisCall;
    const uint32_t endedValue = endedComputed ?
        (input.segmentEndA1Plus44 + input.segmentEndBiasA1Plus8 <=
                 watchdogValueThisCall ?
             1u :
             0u) :
        input.sub8001A7F8Result;
    result.sub8001A7F8Computed = endedComputed;
    result.sub8001A7F8Known = endedComputed || input.sub8001A7F8Known;
    result.sub8001A7F8Result = endedValue;
    PushAction(result,
               MovieTextOuterLoopActionKindSub801C455C::QueryMovieEndedSub8001A7F8,
               0x8001A7F8u,
               0,
               0,
               endedComputed || input.sub8001A7F8Known ? endedValue : 0u,
               endedComputed || input.sub8001A7F8Known,
               !(endedComputed || input.sub8001A7F8Known),
               endedComputed || input.sub8001A7F8Known);
    if (!(endedComputed || input.sub8001A7F8Known)) {
        outer.sub801C448CGap = true;
        return false;
    }

    value = endedValue != 1u;
    return true;
}

}  // namespace

static MovieTextOuterLoopCdCallbackActionApplyResultSub801C455C
ApplyMovieTextOuterLoopCdCallbackActionsSub801C455C(
    const MovieTextOuterLoopStepResultSub801C455C& step,
    PrStage1XaCdDirectState& xaCdState);

void BeginMovieTextOuterLoopSub801C455C(
    MovieTextOuterLoopRuntimeSub801C455C& outer,
    PrStage1MovieTextDirect::Movie1TextRuntime& textRuntime,
    uint8_t mode,
    uint8_t languageIndex) {
    if (textRuntime.playAndWaitActiveSub801C77C0 &&
        textRuntime.playAndWaitModeSub801C77C0 == mode) {
        return;
    }

    outer = {};
    outer.phase = MovieTextOuterLoopPhaseSub801C455C::Warmup;
    outer.mode = mode;
    outer.languageIndex = languageIndex;
    outer.warmupFramesRemaining =
        kMovieTextOuterLoopWarmupFramesSub801C455C;

    if (IsKnownDescMode(mode)) {
        BeginPlayAndWaitSub801C77C0(textRuntime, mode, languageIndex);
        outer.selectedDescAddr =
            GetCommonLyricsDescAddrSub80024C84(textRuntime, mode);
        outer.descSelectedSub80024C84 =
            textRuntime.playAndWaitDescSelectedSub80024C84;
        outer.modeDescGap = !outer.descSelectedSub80024C84;
    } else {
        outer.selectedDescAddr = 0;
        outer.descSelectedSub80024C84 = false;
        outer.modeDescGap = mode < 4u;
        EndPlayAndWaitSub801C77C0(textRuntime);
    }
}

void ResetMovieTextOuterLoopSub801C455C(
    MovieTextOuterLoopRuntimeSub801C455C& outer) {
    outer = MovieTextOuterLoopRuntimeSub801C455C{};
}

void EndMovieTextOuterLoopSub801C455C(
    MovieTextOuterLoopRuntimeSub801C455C& outer,
    PrStage1MovieTextDirect::Movie1TextRuntime& textRuntime) {
    EndPlayAndWaitSub801C77C0(textRuntime);
    ResetMovieTextOuterLoopSub801C455C(outer);
}

void CompleteMovieTextOuterLoopFromHostMovieEndSub801C455C(
    MovieTextOuterLoopRuntimeSub801C455C& outer,
    PrStage1MovieTextDirect::Movie1TextRuntime& textRuntime) {
    if (outer.complete) {
        return;
    }
    EndPlayAndWaitSub801C77C0(textRuntime);
    outer.exitReason =
        MovieTextOuterLoopExitReasonSub801C455C::MovieStepReturnedFalse;
    outer.phase = MovieTextOuterLoopPhaseSub801C455C::Complete;
    outer.cleanupEntered = true;
    outer.complete = true;
    outer.psxReturnValue = 0;
}

MovieTextOuterLoopStepResultSub801C455C StepMovieTextOuterLoopSub801C455C(
    MovieTextOuterLoopRuntimeSub801C455C& outer,
    PrStage1MovieTextDirect::Movie1TextRuntime& textRuntime,
    const MovieTextOuterLoopInputSub801C455C& input,
    PrStage1XaCdDirectState* xaCdState) {
    MovieTextOuterLoopStepResultSub801C455C result{};
    result.phaseBefore = outer.phase;

    switch (outer.phase) {
    case MovieTextOuterLoopPhaseSub801C455C::Idle:
        break;

    case MovieTextOuterLoopPhaseSub801C455C::Warmup: {
        if (!outer.prologueRecorded) {
            PushAction(result,
                       MovieTextOuterLoopActionKindSub801C455C::SelectDescSub80024C84,
                       0x80024C84u,
                       outer.mode,
                       0,
                       outer.selectedDescAddr,
                       outer.descSelectedSub80024C84,
                       outer.modeDescGap,
                       outer.descSelectedSub80024C84);
            PushAction(result,
                       MovieTextOuterLoopActionKindSub801C455C::ClearDword801C954C,
                       kSub801C455C,
                       0,
                       0,
                       0,
                       true);
            outer.dword801C954C = 0;
            outer.prologueRecorded = true;
        }
        bool sub8001A750Known = input.sub8001A750Known;
        uint32_t sub8001A750Result = input.sub8001A750Result;
        bool sub8001A750SideEffectGap = false;
        if (!sub8001A750Known && xaCdState != nullptr) {
            const PrMovieSegmentDirect::StreamStatusPollResult8001A750
                statusPoll =
                    PrStage1XaCdDirectApplySub8001A750StatusPollFromLowerState(
                        *xaCdState);
            sub8001A750Known = statusPoll.resultKnown;
            sub8001A750Result =
                statusPoll.resultKnown
                    ? static_cast<uint32_t>(statusPoll.psxReturn)
                    : 0u;
            sub8001A750SideEffectGap =
                statusPoll.gapMissingCommandWrapper80036678;
        }
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::PollSub8001A750,
                   0x8001A750u,
                   0,
                   0,
                   sub8001A750Result,
                   sub8001A750Known,
                   !sub8001A750Known || sub8001A750SideEffectGap,
                   sub8001A750Known);
        result.gapMissingSub8001A750StatusProducer =
            !sub8001A750Known;
        if ((sub8001A750Known && sub8001A750Result == 1u) ||
            outer.warmupFramesRemaining == 0u) {
            outer.phase = MovieTextOuterLoopPhaseSub801C455C::InitMain;
            break;
        }

        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::DisplaySub8001EC54Ctx7,
                   0x8001EC54u,
                   0,
                   7);
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::WaitGpuSub80035560Arg2,
                   0x80035560u,
                   2);
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::FlushSub8001ED3C,
                   0x8001ED3Cu);
        --outer.warmupFramesRemaining;
        if (outer.warmupFramesRemaining == 0u) {
            outer.phase = MovieTextOuterLoopPhaseSub801C455C::InitMain;
        }
        break;
    }

    case MovieTextOuterLoopPhaseSub801C455C::InitMain:
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::InitSub8001A280,
                   0x8001A280u);
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::CopyMemory164ToCtxC,
                   kSub801C455C,
                   0x0Cu,
                   0x164u,
                   input.memory164,
                   input.memory164Known,
                   !input.memory164Known ||
                       !input.memory164FromSceneEntrySource,
                   input.memory164Known &&
                       input.memory164FromSceneEntrySource);
        result.gapMissingMemory164SceneEntrySource =
            !input.memory164Known || !input.memory164FromSceneEntrySource;
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::CopyWord800916D8ToCtx66,
                   kSub801C455C,
                   0x66u,
                   0x800916D8u,
                   input.word800916D8,
                   true,
                   false,
                   true);
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::CopyWord800916DCToCtx68And54,
                   kSub801C455C,
                   0x68u,
                   0x54u,
                   input.word800916DC,
                   true,
                   false,
                   true);
        outer.phase = MovieTextOuterLoopPhaseSub801C455C::Main;
        break;

    case MovieTextOuterLoopPhaseSub801C455C::Main: {
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::ClearCtxWord0,
                   kSub801C455C,
                   0,
                   0,
                   0,
                   true);
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::PollInputSub80035510,
                   0x80035510u,
                   1,
                   0,
                   input.inputMaskSub80035510,
                   input.inputMaskSub80035510Known,
                   !input.inputMaskSub80035510Known,
                   input.inputMaskSub80035510Known);
        result.gapMissingInputMaskSub80035510Producer =
            !input.inputMaskSub80035510Known;
        if (input.inputMaskSub80035510Known &&
            input.inputMaskSub80035510 != 0u) {
            if (input.inputMaskSub80035510 ==
                kMovieTextOuterLoopInputConfirmMaskSub801C455C) {
                outer.psxReturnValue = 1;
                PushAction(result,
                           MovieTextOuterLoopActionKindSub801C455C::ExitInput0100ReturnOne,
                           kSub801C455C,
                           input.inputMaskSub80035510,
                           0,
                           1,
                           true,
                           false,
                           true);
                EnterCleanup(outer,
                             MovieTextOuterLoopExitReasonSub801C455C::
                                 InputConfirm0100);
                break;
            }
            if (input.inputMaskSub80035510 ==
                    kMovieTextOuterLoopInputAbortExactSub801C455C ||
                (input.inputMaskSub80035510 &
                 kMovieTextOuterLoopInputAbortMaskSub801C455C) != 0u) {
                PushAction(result,
                           MovieTextOuterLoopActionKindSub801C455C::ExitInput0800Or0840,
                           kSub801C455C,
                           input.inputMaskSub80035510,
                           0,
                           0,
                           true,
                           false,
                           true);
                EnterCleanup(outer,
                             MovieTextOuterLoopExitReasonSub801C455C::
                                 InputAbort0800Or0840);
                break;
            }
        }

        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::AdvanceTextSub80024CF8,
                   0x80024CF8u,
                   outer.mode,
                   input.languageIndex,
                   input.movieFrame30,
                   true,
                   false,
                   true);
        AdvanceCommonLyricsSub80024CF8(textRuntime,
                                      outer.mode,
                                      input.languageIndex,
                                      input.movieFrame30);
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::WorkbaseStepSub80027528,
                   0x80027528u);
        if (!input.dword801C9554Known) {
            PushAction(result,
                       MovieTextOuterLoopActionKindSub801C455C::
                           GapDword801C9554Writer,
                       0x801C44E0u,
                       0x801C9554u,
                       0,
                       0,
                       false,
                       true,
                       false);
            result.gapMissingDword801C9554Writer = true;
        } else if (input.dword801C9554EqualsOne) {
            PushAction(result,
                       MovieTextOuterLoopActionKindSub801C455C::DisplaySub8001EC54Ctx7,
                       0x8001EC54u,
                       0,
                       7);
        }
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::WorkbaseStepSub80027528,
                   0x80027528u);
        const PrMovieSegmentDirect::StreamClockPollResult8001A3C8
            sameTickWorkBasePoll =
                PushSub801C448CWrapperActions(result, xaCdState);
        bool gsGetWorkBaseKnown = input.gsGetWorkBaseKnown;
        uint32_t gsGetWorkBaseResult = input.gsGetWorkBaseResult;
        if (sameTickWorkBasePoll.dword80049428Known) {
            gsGetWorkBaseKnown = true;
            gsGetWorkBaseResult =
                static_cast<uint32_t>(sameTickWorkBasePoll.dword80049428);
        }
        {
            bool movieStepResult = input.sub801C448CResult;
            bool movieStepKnown = input.sub801C448CKnown;
            if (!movieStepKnown) {
                movieStepResult = true;
                movieStepKnown =
                    StepSub801C4350(outer, result, input, movieStepResult);
            }
            result.movieStepSub801C448CKnown = movieStepKnown;
            result.movieStepSub801C448CResult = movieStepResult;
            PushAction(result,
                       MovieTextOuterLoopActionKindSub801C455C::
                           ReturnMovieStepSub801C448C,
                       kSub801C448C,
                       0,
                       0,
                       movieStepKnown ? (movieStepResult ? 1u : 0u) : 0u,
                       movieStepKnown,
                       !movieStepKnown,
                       movieStepKnown);
            if (!movieStepKnown) {
                PushAction(result,
                           MovieTextOuterLoopActionKindSub801C455C::
                               GapMovieStepSub801C448C,
                           kSub801C448C,
                           0,
                           0,
                           0,
                           false,
                           true,
                           false);
                outer.sub801C448CGap = true;
            }
        }
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::EndTextFrameSub8001ED74,
                   0x8001ED74u);
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::DrawTextWorkSub8002756C,
                   0x8002756Cu);
        if (input.dword801C9554Known && input.dword801C9554EqualsOne) {
            PushAction(result,
                       MovieTextOuterLoopActionKindSub801C455C::FlushSub8001ED3C,
                       0x8001ED3Cu);
        }
        PushAction(result,
                   MovieTextOuterLoopActionKindSub801C455C::CheckGsGetWorkBase,
                   0x8001A3B8u,
                   0,
                   0,
                   gsGetWorkBaseResult,
                   gsGetWorkBaseKnown,
                   !gsGetWorkBaseKnown,
                   gsGetWorkBaseKnown);
        result.gapMissingGsGetWorkBaseProducer =
            !gsGetWorkBaseKnown;
        ++outer.mainIteration;
        if (gsGetWorkBaseKnown && gsGetWorkBaseResult == 1u) {
            PushAction(result,
                       MovieTextOuterLoopActionKindSub801C455C::
                           StopCallbacksSub8001A694StopCallback,
                       0x8001A694u);
            EnterCleanup(outer,
                         MovieTextOuterLoopExitReasonSub801C455C::GsWorkBaseOne);
        } else if (result.movieStepSub801C448CKnown &&
                   !result.movieStepSub801C448CResult) {
            EnterCleanup(
                outer,
                MovieTextOuterLoopExitReasonSub801C455C::MovieStepReturnedFalse);
        }
        break;
    }

    case MovieTextOuterLoopPhaseSub801C455C::Cleanup:
        RunCleanupSub801C455C(outer, textRuntime, result);
        break;

    case MovieTextOuterLoopPhaseSub801C455C::Complete:
        break;
    }

    if (outer.phase == MovieTextOuterLoopPhaseSub801C455C::Cleanup) {
        RunCleanupSub801C455C(outer, textRuntime, result);
    }

    result.phaseAfter = outer.phase;
    result.exitReason = outer.exitReason;
    result.psxReturnValue = outer.psxReturnValue;
    return result;
}

static MovieTextOuterLoopInputSub801C455C BuildMovieTextOuterLoopInputFromScan801C455C(
    const MovieTextOuterLoopInputSub801C455C& base,
    const PrStage1MovieSegmentDirect::MovieSegmentScanResult801C4780& scan) {
    const PrStage1MovieSegmentDirect::SceneEntrySegmentSelection801C4DC4
        selection =
            PrStage1MovieSegmentDirect::PsxSelectMovieSegment801C4DC4FromScan(
                scan);
    if (!selection.rowKnown) {
        return base;
    }
    MovieTextOuterLoopInputSub801C455C result = base;
    const PrStage1MovieSegmentDirect::MovieStepSegmentFields801C4350 fields =
        PrStage1MovieSegmentDirect::BuildMovieStepSegmentFields801C4350(
            selection.row);
    result.segmentTimeBaseA1Plus40Known =
        fields.segmentTimeBaseA1Plus40Known;
    result.segmentTimeBaseA1Plus40 = fields.segmentTimeBaseA1Plus40;
    result.segmentEndA1Plus44Known = fields.segmentEndA1Plus44Known;
    result.segmentEndA1Plus44 = fields.segmentEndA1Plus44;
    result.segmentEndBiasA1Plus8Known = fields.segmentEndBiasA1Plus8Known;
    result.segmentEndBiasA1Plus8 = fields.segmentEndBiasA1Plus8;
    result.segmentWindowFieldsFromDirectCarrier =
        fields.segmentTimeBaseA1Plus40Known ||
        fields.segmentEndA1Plus44Known ||
        fields.segmentEndBiasA1Plus8Known;
    result.outerLoopInputAuthorityFromScanCarrier = true;
    return result;
}

static void CaptureOuterLoopSourceGapState801C455C(
    MovieTextOuterLoopScanInputCarrierSub801C455C& carrier) {
    carrier.memory164Known = carrier.input.memory164Known;
    carrier.memory164FromSceneEntrySource =
        carrier.input.memory164FromSceneEntrySource;
    carrier.gapMissingMemory164SceneEntrySource =
        !carrier.memory164Known || !carrier.memory164FromSceneEntrySource;
    carrier.sub8001A750Known = carrier.input.sub8001A750Known;
    carrier.gapMissingSub8001A750StatusProducer =
        !carrier.input.sub8001A750Known;
    carrier.inputMaskSub80035510Known =
        carrier.input.inputMaskSub80035510Known;
    carrier.gapMissingInputMaskSub80035510Producer =
        !carrier.input.inputMaskSub80035510Known;
    carrier.dword801C9554Known = carrier.input.dword801C9554Known;
    carrier.gapMissingDword801C9554Writer =
        !carrier.input.dword801C9554Known;
    carrier.gsGetWorkBaseKnown = carrier.input.gsGetWorkBaseKnown;
    carrier.gapMissingGsGetWorkBaseProducer =
        !carrier.input.gsGetWorkBaseKnown;
}

static MovieTextOuterLoopScanInputCarrierSub801C455C
BuildMovieTextOuterLoopScanInputCarrier801C455C(
    const MovieTextOuterLoopInputSub801C455C& base,
    const PrStage1MovieSegmentDirect::MovieSegmentScanResult801C4780& scan) {
    MovieTextOuterLoopScanInputCarrierSub801C455C carrier{};
    carrier.called = true;
    carrier.scan = scan;
    carrier.input = base;
    CaptureOuterLoopSourceGapState801C455C(carrier);
    carrier.scanRowsNeedingCdLookupFeedback =
        scan.rowNeedsCdLookupCount;
    carrier.scanRowsMissingCdLookupFeedback =
        scan.rowMissingCdLookupFeedbackCount;
    carrier.scanRowsMissingCdlFileNameFeedback =
        scan.rowMissingCdlFileNameFeedbackCount;
    carrier.gapMissingCdLookupFeedback =
        scan.gapMissingCdLookupFeedback;
    carrier.gapMissingCdlFileNameFeedback =
        scan.gapMissingCdlFileNameFeedback;
    carrier.byte800493F4ClockProducerKnown = base.byte800493F4Known;
    carrier.streamClockCarrierKnown = base.streamClockCarrierKnown;
    carrier.streamClockProducerGap =
        base.streamClockProducerGap ||
        !carrier.byte800493F4ClockProducerKnown;
    carrier.gapMissingByte800493F4ClockProducer =
        !carrier.byte800493F4ClockProducerKnown;
    carrier.liveCdLookupFeedbackCarrierKnown =
        base.cdLookupFeedbackCarrierKnown;
    carrier.liveCdLookupProducerGap =
        base.cdLookupFeedbackCarrierGap ||
        scan.rowMissingCdLookupFeedbackCount != 0u ||
        scan.rowMissingCdlFileNameFeedbackCount != 0u;
    carrier.liveCdLookupProducerKnown =
        carrier.liveCdLookupFeedbackCarrierKnown &&
        !carrier.liveCdLookupProducerGap;
    if (!scan.called) {
        carrier.gapMissingScanResult = true;
        return carrier;
    }

    carrier.selection =
        PrStage1MovieSegmentDirect::PsxSelectMovieSegment801C4DC4FromScan(scan);
    carrier.selectedMovieRowKnown = carrier.selection.rowKnown;
    if (!carrier.selection.rowKnown) {
        carrier.gapMissingSelectedMovieRow = true;
        return carrier;
    }

    carrier.input =
        BuildMovieTextOuterLoopInputFromScan801C455C(base, scan);
    CaptureOuterLoopSourceGapState801C455C(carrier);
    if (carrier.selection.rowIndex <
        PrStage1MovieSegmentDirect::kSceneEntryMovieSegmentCount801C4780) {
        carrier.selectedMovieRowNeedsCdLookupFeedback =
            scan.rowNeedsCdLookupMask[carrier.selection.rowIndex];
        carrier.selectedMovieRowCdLookupReady =
            scan.rowCdLookupReadyMask[carrier.selection.rowIndex];
        carrier.selectedMovieRowMissingCdLookupFeedback =
            scan.rowMissingCdLookupFeedbackMask[carrier.selection.rowIndex];
        carrier.selectedMovieRowCdlFileNameReady =
            scan.rowCdlFileNameReadyMask[carrier.selection.rowIndex];
        carrier.selectedMovieRowMissingCdlFileNameFeedback =
            scan.rowMissingCdlFileNameFeedbackMask[carrier.selection.rowIndex];
    }
    carrier.gapMissingCdLookupFeedback =
        carrier.gapMissingCdLookupFeedback ||
        carrier.selectedMovieRowMissingCdLookupFeedback;
    carrier.gapMissingCdlFileNameFeedback =
        carrier.gapMissingCdlFileNameFeedback ||
        carrier.selectedMovieRowMissingCdlFileNameFeedback;
    carrier.directSegmentWindowFieldsKnown =
        carrier.input.segmentWindowFieldsFromDirectCarrier;
    carrier.directSegmentWindowFieldsComplete =
        carrier.input.segmentTimeBaseA1Plus40Known &&
        carrier.input.segmentEndA1Plus44Known &&
        carrier.input.segmentEndBiasA1Plus8Known;
    carrier.gapMissingSegmentWindowFields =
        !carrier.directSegmentWindowFieldsComplete;
    return carrier;
}

static MovieTextOuterLoopInputSub801C455C
BuildMovieTextOuterLoopInputFromSceneFrame801C455C(
    const PrStage1MovieTextDirect::Movie1TextScene1FrameInputSub801C77C0&
        frameInput,
    uint8_t languageIndex,
    const PrStage1MovieSegmentDirect::MovieSegmentCdLookupBatchFeedback801C4780*
        cdLookupFeedback) {
    MovieTextOuterLoopInputSub801C455C out{};
    out.languageIndex = languageIndex;
    out.word800916D8 =
        PrStage1MovieTextDirect::ResolveLanguageIndex(languageIndex);
    out.word800916DC = frameInput.globalOptions.word800916DCKnown
                           ? frameInput.globalOptions.word800916DC
                           : (frameInput.globalOptions.subtitleEnabled ? 1u
                                                                        : 0u);
    out.memory164Known = frameInput.sceneEntryMemory164Known;
    out.memory164 = frameInput.sceneEntryMemory164;
    out.memory164FromSceneEntrySource = frameInput.sceneEntryMemory164Known;
    out.movieFrame30 = frameInput.movieFrame30 > 0
                           ? static_cast<uint32_t>(frameInput.movieFrame30)
                           : 0u;
    out.queryFrame60 =
        frameInput.stageRunning
            ? (frameInput.runnerFrame30 > 0
                   ? static_cast<uint32_t>(frameInput.runnerFrame30) * 2u
                   : 0u) +
                  (frameInput.renderHalfStep ? 1u : 0u)
            : out.movieFrame30 * 2u;
    out.currentGp872SlotKnown = frameInput.currentGp872SlotKnown;
    out.currentGp872Slot = frameInput.currentGp872Slot;
    out.inputMaskSub80035510Known =
        frameInput.inputMaskSub80035510Known;
    out.inputMaskSub80035510 = frameInput.inputMaskSub80035510;
    out.sub8001A750Known = frameInput.sub8001A750Known;
    out.sub8001A750Result = frameInput.sub8001A750Result;
    out.sub801C448CKnown = frameInput.sub801C448CKnown;
    out.sub801C448CResult = frameInput.sub801C448CResult;
    out.cdLookupFeedbackCarrierKnown = cdLookupFeedback != nullptr;
    out.cdLookupFeedbackCarrierGap = cdLookupFeedback == nullptr;

    if (cdLookupFeedback != nullptr &&
        cdLookupFeedback->streamClock800493F4Known) {
        const PrStage1MovieSegmentDirect::StreamClockProducerCarrier800493F4&
            clock = cdLookupFeedback->streamClock800493F4;
        if (clock.acceptedByte800493F4) {
            out.byte800493F4Known = true;
            out.byte800493F4_00 = clock.feedback.byte800493F4.minute;
            out.byte800493F4_01 = clock.feedback.byte800493F4.second;
            out.byte800493F4_02 = clock.feedback.byte800493F4.frame;
        }
        out.streamClockCarrierKnown = true;
        out.streamClockProducerGap =
            clock.gapMissingByte800493F4ClockProducer ||
            !out.byte800493F4Known;
    } else {
        out.streamClockProducerGap = true;
    }
    return out;
}

static MovieTextOuterLoopScanInputCarrierSub801C455C
BuildStage1MovieTextOuterLoopScanInputCarrierFromSceneFrame801C455C(
    const PrStage1MovieTextDirect::Movie1TextScene1FrameInputSub801C77C0&
        frameInput,
    uint8_t languageIndex,
    uint32_t sceneIndex,
    const PrStage1MovieSegmentDirect::MovieSegmentCdLookupBatchFeedback801C4780*
        cdLookupFeedback) {
    const MovieTextOuterLoopInputSub801C455C base =
        BuildMovieTextOuterLoopInputFromSceneFrame801C455C(frameInput,
                                                           languageIndex,
                                                           cdLookupFeedback);
    const PrStage1MovieSegmentDirect::SceneEntryMovieSegmentTable801C4780
        table = PrStage1MovieSegmentDirect::
            MaterializeStage1SceneEntryMovieSegmentsFromStaticTable801C4780(
                sceneIndex);
    const PrStage1MovieSegmentDirect::MovieSegmentScanResult801C4780 scan =
        PrStage1MovieSegmentDirect::PsxCall801C4780_ScanMovieSegmentsWithFeedback(
            table,
            cdLookupFeedback != nullptr ? cdLookupFeedback->rowFeedback.data()
                                        : nullptr,
            cdLookupFeedback != nullptr
                ? PrStage1MovieSegmentDirect::kSceneEntryMovieSegmentCount801C4780
                : 0u);
    return BuildMovieTextOuterLoopScanInputCarrier801C455C(base, scan);
}

static bool ShouldDrainImmediateOuterLoopPhaseSub801C455C(
    const MovieTextOuterLoopStepResultSub801C455C& step) {
    return (step.phaseBefore == MovieTextOuterLoopPhaseSub801C455C::Warmup &&
            step.phaseAfter == MovieTextOuterLoopPhaseSub801C455C::InitMain) ||
           (step.phaseBefore == MovieTextOuterLoopPhaseSub801C455C::InitMain &&
            step.phaseAfter == MovieTextOuterLoopPhaseSub801C455C::Main);
}

void TickStage1MovieTextOuterLoopFromSceneFrame801C455C(
    MovieTextOuterLoopRuntimeSub801C455C& outer,
    PrStage1MovieTextDirect::Movie1TextRuntime& textRuntime,
    const PrStage1MovieTextDirect::Movie1TextScene1FrameInputSub801C77C0&
        frameInput,
    uint8_t languageIndex,
    uint32_t sceneIndex,
    PrStage1LifecycleExecutorDirect::State801C81EC& lifecycleExecutor,
    const std::filesystem::path* binPath,
    PrStage1XaCdDirectState* xaCdState,
    PrStage1MovieTextDirect::Movie1TextWindowTickResult& outWindowTick,
    MovieTextOuterLoopStepResultSub801C455C* outLastStep) {
    PrStage1MovieSegmentDirect::MovieSegmentCdLookupBatchFeedback801C4780
        cdLookupCarrier{};
    const bool cdLookupCarrierKnown =
        PrStage1LifecycleExecutorDirect::
            BuildBootstrap15590MovieSegmentCdLookupCarrier801C81EC(
                lifecycleExecutor,
                binPath != nullptr ? *binPath : std::filesystem::path{},
                binPath != nullptr,
                xaCdState,
                cdLookupCarrier);
    MovieTextOuterLoopScanInputCarrierSub801C455C scanInputCarrier =
        BuildStage1MovieTextOuterLoopScanInputCarrierFromSceneFrame801C455C(
            frameInput,
            languageIndex,
            sceneIndex,
            cdLookupCarrierKnown ? &cdLookupCarrier : nullptr);
    scanInputCarrier.input.dword801C9554Known =
        textRuntime.movie1DisplayFlushGate801D3044Known;
    scanInputCarrier.input.dword801C9554EqualsOne =
        textRuntime.movie1DisplayFlushGate801D3044;
    CaptureOuterLoopSourceGapState801C455C(scanInputCarrier);
    MovieTextOuterLoopStepResultSub801C455C step{};
    for (uint32_t guard = 0; guard < 3u; ++guard) {
        step = StepMovieTextOuterLoopSub801C455C(outer,
                                                textRuntime,
                                                scanInputCarrier.input,
                                                xaCdState);
        if (xaCdState != nullptr) {
            (void)ApplyMovieTextOuterLoopCdCallbackActionsSub801C455C(
                step,
                *xaCdState);
        }
        if (!ShouldDrainImmediateOuterLoopPhaseSub801C455C(step)) {
            break;
        }
    }
    if (outLastStep != nullptr) {
        *outLastStep = step;
    }
    outWindowTick =
        PrStage1MovieTextDirect::
            BuildMovie1TextWindowAfterOuterAdvanceSub801C455C(textRuntime,
                                                              frameInput,
                                                              languageIndex);
}

void TickStage1MovieTextOuterLoopFromHostFacts801C455C(
    MovieTextOuterLoopRuntimeSub801C455C& outer,
    PrStage1MovieTextDirect::Movie1TextRuntime& textRuntime,
    const Stage1MovieTextSceneFrameHostFacts801C455C& facts,
    PrStage1LifecycleExecutorDirect::State801C81EC& lifecycleExecutor,
    const std::filesystem::path& dataRoot,
    PrStage1XaCdDirectState* xaCdState,
    PrStage1MovieTextDirect::Movie1TextWindowTickResult& outWindowTick,
    MovieTextOuterLoopStepResultSub801C455C* outLastStep) {
    PrStage1MovieTextDirect::Movie1TextScene1FrameInputSub801C77C0
        frameInput{};
    frameInput.globalOptions = facts.globalOptions;
    frameInput.movieFrame30 = facts.movieFrame30;
    frameInput.stageRunning = facts.stageRunning;
    frameInput.runnerFrame30 = facts.runnerFrame30;
    frameInput.renderHalfStep = facts.renderHalfStep;
    frameInput.sceneEntryMemory164Known = facts.sceneEntryMemory164Known;
    frameInput.sceneEntryMemory164 = facts.sceneEntryMemory164;
    frameInput.currentGp872SlotKnown = facts.currentGp872SlotKnown;
    frameInput.currentGp872Slot = facts.currentGp872Slot;
    frameInput.inputMaskSub80035510Known = facts.inputMaskSub80035510Known;
    frameInput.inputMaskSub80035510 = facts.inputMaskSub80035510;
    frameInput.sub8001A750Known = facts.sub8001A750Known;
    frameInput.sub8001A750Result = facts.sub8001A750Result;
    frameInput.sub801C448CKnown = facts.sub801C448CKnown;
    frameInput.sub801C448CResult = facts.sub801C448CResult;

    std::filesystem::path binPath;
    const bool binPathKnown =
        TryResolveStage1MovieSegmentIso9660BinPath801C455C(dataRoot,
                                                           binPath);
    TickStage1MovieTextOuterLoopFromSceneFrame801C455C(
        outer,
        textRuntime,
        frameInput,
        facts.languageIndex,
        facts.sceneIndex,
        lifecycleExecutor,
        binPathKnown ? &binPath : nullptr,
        xaCdState,
        outWindowTick,
        outLastStep);
}

static MovieTextOuterLoopCdCallbackActionApplyResultSub801C455C
ApplyMovieTextOuterLoopCdCallbackActionsSub801C455C(
    const MovieTextOuterLoopStepResultSub801C455C& step,
    PrStage1XaCdDirectState& xaCdState) {
    MovieTextOuterLoopCdCallbackActionApplyResultSub801C455C out{};
    for (uint32_t i = 0; i < step.actionCount; ++i) {
        const MovieTextOuterLoopActionSub801C455C& action = step.actions[i];
        switch (action.kind) {
        case MovieTextOuterLoopActionKindSub801C455C::
            StopCallbacksSub8001A694StopCallback:
        case MovieTextOuterLoopActionKindSub801C455C::CleanupSub8001A694:
            out.sawActionSub8001A694 = true;
            out.lastClearCallback =
                PrStage1XaCdDirectApplySub8001A694ClearCdCallback(xaCdState);
            ++out.appliedClearCallbackCount;
            break;
        case MovieTextOuterLoopActionKindSub801C455C::
            SnapshotWorkBaseSub8001A3C8:
            if (action.appliedToDirectRuntime) {
                break;
            }
            out.sawActionSub8001A3C8 = true;
            out.lastStreamClockPoll =
                PrStage1XaCdDirectApplySub8001A3C8ClockPollFromLowerState(
                    xaCdState);
            ++out.appliedStreamClockPollCount;
            break;
        default:
            break;
        }
    }
    return out;
}

}  // namespace PrStage1MovieTextOuterLoopDirect
