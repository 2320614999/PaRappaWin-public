#include "pr_stage1_lifecycle_direct.h"

#include "pr_stage_status_bank_direct.h"

#include <cstddef>

namespace PrStage1LifecycleDirect {

namespace {

constexpr uint32_t kTransitionWork801C3640 = 0x801C3640u;
constexpr uint32_t kSavePayloadBank80092F10 = 0x80092F10u;
constexpr uint32_t kCue80094410 = 0x80094410u;
constexpr uint32_t kFn80014344 = 0x80014344u;
constexpr uint32_t kFn80015660 = 0x80015660u;
constexpr uint32_t kFn8001AC18 = 0x8001AC18u;
constexpr uint32_t kFn8001A324 = 0x8001A324u;
constexpr uint32_t kFn80024E98 = 0x80024E98u;
constexpr uint32_t kFn80025A00 = 0x80025A00u;
constexpr uint32_t kFn80025A34 = 0x80025A34u;
constexpr uint32_t kFn80026FA4 = 0x80026FA4u;
constexpr uint32_t kFn800201AC = 0x800201ACu;
constexpr uint32_t kFn80020110 = 0x80020110u;
constexpr uint32_t kFn80020090 = 0x80020090u;
constexpr uint32_t kFn80020008 = 0x80020008u;
constexpr uint32_t kFn8001A4D0 = 0x8001A4D0u;
constexpr uint32_t kFn8001EF14 = 0x8001EF14u;
constexpr uint32_t kFn801C7A60 = 0x801C7A60u;
constexpr uint32_t kFn800166AC = 0x800166ACu;
constexpr uint32_t kFn80015744 = 0x80015744u;
constexpr uint32_t kFn80026B94 = 0x80026B94u;
constexpr uint32_t kFn80026EF8 = 0x80026EF8u;
constexpr uint32_t kFn80026ECC = 0x80026ECCu;
constexpr uint32_t kFn8001635C = 0x8001635Cu;
constexpr uint32_t kFn8001628C = 0x8001628Cu;
constexpr uint32_t kFn80015590 = 0x80015590u;
constexpr uint32_t kFn80019148 = 0x80019148u;
constexpr uint32_t kFn801C44E0 = 0x801C44E0u;
constexpr uint32_t kFn801C455C = 0x801C455Cu;
constexpr uint32_t kFn801CB67C = 0x801CB67Cu;
constexpr uint32_t kFn801C85DC = 0x801C85DCu;
constexpr uint32_t kSceneEntryFirstLoaderOffset801C7284 = 0x0Cu;
constexpr uint32_t kSceneEntryCompoLoaderOffset801C7284 = 0x3Cu;
constexpr uint8_t kSceneEntryCompoLoaderRowIndex801C7284 = 1;
constexpr int32_t kSceneEntryCompoLoaderMode801C7284 = 0;
constexpr uint8_t kSub8001AC18MaxAttempts801C7284 = 4;
constexpr int32_t kSub8001AC18FirstAttemptParserFlag801C7284 = 1;
constexpr int32_t kSub8001AC18RetryParserFlag801C7284 = 0;
constexpr uint8_t kSub8001AC18CopiedDescriptorFirstWord801C7284 = 4;
constexpr uint8_t kSub8001AC18CopiedDescriptorWordCount801C7284 = 6;
constexpr uint32_t kInitialMovie1LoaderOffset = 0x6Cu;
constexpr uint32_t kStageLoopLoaderOffset = 0x9Cu;
constexpr uint32_t kClearTailGoodLoaderOffset = 0xCCu;
constexpr uint32_t kClearTailCoolLoaderOffset = 0xFCu;
constexpr uint32_t kBootstrapZcompoLoaderOffset = 0x12Cu;

bool IsStageLoopBypassD0(uint16_t word800916D0) {
    return word800916D0 == 1 || word800916D0 == 2;
}

PrStageStatusBankComod ResolveStatusBankComod801C81EC(
    const FrameInput801C81EC& input) {
    (void)input;
    return PrStageStatusBankComod::Comod1;
}

Action801C7284& EmitInitAction801C7284(SceneEntry801C7284& out,
                                       ActionKind801C7284 kind,
                                       uint32_t psxFunction) {
    Action801C7284 action{};
    action.kind = kind;
    action.psxOrder = static_cast<uint32_t>(out.initActions801C7284.size());
    action.psxFunction = psxFunction;
    action.psxFunctionKnown = psxFunction != 0u;
    action.sceneEntryPtrKnown = out.sceneEntryPtr8006EDB8Known;
    action.sceneEntryPtr8006EDB8 = out.sceneEntryPtr8006EDB8;
    action.sceneIndex = out.sceneIndex;
    out.initActions801C7284.push_back(action);
    return out.initActions801C7284.back();
}

void SetRawArgs(Action801C7284& action,
                uint32_t rawArg0 = 0,
                uint32_t rawArg1 = 0,
                uint32_t rawArg2 = 0,
                uint32_t rawArg3 = 0) {
    action.rawArg0 = rawArg0;
    action.rawArg1 = rawArg1;
    action.rawArg2 = rawArg2;
    action.rawArg3 = rawArg3;
}

void AttachSceneLoaderSlot801C7284(Action801C7284& action,
                                   const SceneEntry801C7284& sceneEntry,
                                   const Scene1LoaderRecord801C7284& loader) {
    action.loaderOffsetFromSceneEntryKnown = true;
    action.loaderOffsetFromSceneEntry = loader.psxOffsetFromSceneEntry;
    action.sceneLoaderSlotKnown = true;
    action.sceneLoaderSlotPresent = loader.present;
    action.sceneLoaderSlot = loader.slotIndex;
    if (loader.psxAddrKnown) {
        action.rawArg0 = loader.psxAddr;
    } else if (sceneEntry.sceneEntryPtr8006EDB8Known) {
        action.rawArg0 =
            sceneEntry.sceneEntryPtr8006EDB8 + loader.psxOffsetFromSceneEntry;
    }
}

void RefreshSceneLoadersFromMovieScan801C7284(SceneEntry801C7284& sceneEntry) {
    for (uint8_t i = 0; i < sceneEntry.loaders.size(); i++) {
        Scene1LoaderRecord801C7284& loader = sceneEntry.loaders[i];
        loader.slotIndex = i;
        loader.psxOffsetFromSceneEntry =
            kSceneEntryFirstLoaderOffset801C7284 +
            PrStage1MovieSegmentDirect::kMovieSegmentRecordSize8001A324 *
                static_cast<uint32_t>(i);

        if (sceneEntry.movieSegmentScan801C4780.called) {
            loader.movieSegmentRecord =
                sceneEntry.movieSegmentScan801C4780.table.rows[i];
            loader.rowInit8001A324 =
                sceneEntry.movieSegmentScan801C4780.rowInit[i];
            loader.psxAddrKnown = loader.movieSegmentRecord.psxAddr != 0u;
            loader.psxAddr = loader.movieSegmentRecord.psxAddr;
            if (loader.movieSegmentRecord.known) {
                loader.present = true;
                if (loader.movieSegmentRecord.pathPtrA1Plus00Known) {
                    loader.present =
                        loader.movieSegmentRecord.pathPtrA1Plus00 != 0u;
                }
            }
            if (sceneEntry.sceneEntryPtr8006EDB8Known &&
                loader.psxAddrKnown) {
                loader.psxOffsetFromSceneEntry =
                    loader.psxAddr - sceneEntry.sceneEntryPtr8006EDB8;
            }
        }
    }
}

uint32_t PackCdlLocWord801C7284(
    const PrStage1MovieSegmentDirect::MsfBcd80036A78& msf) {
    return static_cast<uint32_t>(msf.minute) |
           (static_cast<uint32_t>(msf.second) << 8u) |
           (static_cast<uint32_t>(msf.frame) << 16u);
}

uint32_t PackNameWord801C7284(
    const std::array<uint8_t, 16>& name,
    std::size_t firstByte) {
    return static_cast<uint32_t>(name[firstByte]) |
           (static_cast<uint32_t>(name[firstByte + 1u]) << 8u) |
           (static_cast<uint32_t>(name[firstByte + 2u]) << 16u) |
           (static_cast<uint32_t>(name[firstByte + 3u]) << 24u);
}

void AttachSceneEntryCompoDescriptorCopy801C7284(
    SceneEntryCompoLoadFeedback801C7284& out,
    const PrStage1MovieSegmentDirect::MovieSegmentRecord48& row) {
    if (row.cdlFileNameA1Plus18Known) {
        for (std::size_t i = 0; i < 4u; ++i) {
            out.descriptorCopyWordKnown[i + 2u] = true;
            out.descriptorCopyWords[i + 2u] =
                PackNameWord801C7284(row.cdlFileNameA1Plus18, i * 4u);
        }
    }

    if (row.startMsfKnown) {
        out.descriptorCopyWordKnown[0] =
            true;
        out.descriptorCopyWords[0] =
            PackCdlLocWord801C7284(row.startMsf);
    }

    if (row.lengthSourceA1Plus20Known) {
        out.descriptorCopyWordKnown[1] = true;
        out.descriptorCopyWords[1] = row.lengthSourceA1Plus20;
    }

    out.descriptorCopyWordsKnown = true;
    for (bool wordKnown : out.descriptorCopyWordKnown) {
        out.descriptorCopyWordsKnown =
            out.descriptorCopyWordsKnown && wordKnown;
    }
}

SceneEntryCompoLoadFeedback801C7284
BuildSceneEntryCompoLoadFeedbackCarrier801C7284(
    const SceneEntry801C7284& sceneEntry,
    const InitSceneInput801C7284& input) {
    SceneEntryCompoLoadFeedback801C7284 out{};
    if (input.row1CompoLoadFeedbackKnown) {
        out = input.row1CompoLoadFeedback;
    }

    out.known = true;
    out.pending = !out.completed;
    out.loaderFunction = kFn8001AC18;
    out.loaderOffsetFromSceneEntry = kSceneEntryCompoLoaderOffset801C7284;
    out.rowIndex = kSceneEntryCompoLoaderRowIndex801C7284;
    out.mode = kSceneEntryCompoLoaderMode801C7284;
    out.maxAttempts = kSub8001AC18MaxAttempts801C7284;
    out.firstAttemptParserFlag =
        kSub8001AC18FirstAttemptParserFlag801C7284;
    out.retryParserFlag = kSub8001AC18RetryParserFlag801C7284;
    out.descriptorCopyFirstWord =
        kSub8001AC18CopiedDescriptorFirstWord801C7284;
    out.descriptorCopyWordCount =
        kSub8001AC18CopiedDescriptorWordCount801C7284;

    if (sceneEntry.sceneEntryPtr8006EDB8Known) {
        out.loaderPsxAddrKnown = true;
        out.loaderPsxAddr =
            sceneEntry.sceneEntryPtr8006EDB8 +
            kSceneEntryCompoLoaderOffset801C7284;
    }

    if (kSceneEntryCompoLoaderRowIndex801C7284 < sceneEntry.loaders.size()) {
        AttachSceneEntryCompoDescriptorCopy801C7284(
            out,
            sceneEntry.loaders[kSceneEntryCompoLoaderRowIndex801C7284]
                .movieSegmentRecord);
    }

    if (out.completed && out.success) {
        out.intHeaderRecordProducerPending = false;
        out.cdPayloadFeedbackPending = false;
        out.memoryRegistrationFeedbackPending = false;
        out.gpuTimUploadFeedbackPending = false;
        out.spuVabFeedbackPending = false;
    }
    return out;
}

Action801C81EC& Emit(Runtime801C81EC& runtime,
                     StepResult801C81EC& out,
                     ActionKind801C81EC kind,
                     uint32_t psxFunction) {
    Action801C81EC action{};
    action.kind = kind;
    action.psxOrder = runtime.nextPsxActionOrder++;
    action.psxFunction = psxFunction;
    action.psxFunctionKnown = psxFunction != 0u;
    out.actions.push_back(action);
    return out.actions.back();
}

void SetRawArgs(Action801C81EC& action,
                uint32_t rawArg0 = 0,
                uint32_t rawArg1 = 0,
                uint32_t rawArg2 = 0,
                uint32_t rawArg3 = 0) {
    action.rawArg0 = rawArg0;
    action.rawArg1 = rawArg1;
    action.rawArg2 = rawArg2;
    action.rawArg3 = rawArg3;
}

void SetTransitionArgs801C81EC(Action801C81EC& action,
                               uint32_t a1,
                               int32_t modeA2,
                               int32_t preFfd4ArgA3,
                               int32_t postFfd4ArgA4,
                               bool hasExtraDelay27194,
                               int32_t extraDelayFrames27194,
                               uint32_t finishFunction) {
    action.transitionWorkPtr = a1;
    action.transitionA1_801C3640 = a1;
    action.transitionModeA2 = modeA2;
    action.transitionPreFfd4ArgA3 = preFfd4ArgA3;
    action.transitionPostFfd4ArgA4 = postFfd4ArgA4;
    action.transitionHasExtraDelay27194 = hasExtraDelay27194;
    action.transitionExtraDelayFrames27194 = extraDelayFrames27194;
    action.transitionFinishFunction = finishFunction;
}

void AttachSceneLoaderSlot801C81EC(Action801C81EC& action,
                                   const SceneEntry801C7284& sceneEntry,
                                   uint32_t loaderOffset,
                                   StrBlockKind801C81EC strBlockKind =
                                       StrBlockKind801C81EC::None) {
    action.loaderOffsetFromSceneEntry = loaderOffset;
    action.strBlockKind = strBlockKind;
    action.sceneLoaderSlotKnown = false;
    action.sceneLoaderSlotPresent = false;
    action.sceneLoaderSlot = 0;
    action.sceneLoaderRecordKnown = false;
    action.sceneLoaderRowIndex = 0;
    action.sceneLoaderPsxAddrKnown = false;
    action.sceneLoaderPsxAddr = 0;
    action.sceneLoaderMovieSegmentRecord =
        PrStage1MovieSegmentDirect::MovieSegmentRecord48{};
    action.sceneLoaderRowInit8001A324 =
        PrStage1MovieSegmentDirect::SegmentInitResult8001A324{};
    for (const Scene1LoaderRecord801C7284& loader : sceneEntry.loaders) {
        if (loader.psxOffsetFromSceneEntry != loaderOffset) {
            continue;
        }
        action.sceneLoaderSlotKnown = true;
        action.sceneLoaderSlotPresent = loader.present;
        action.sceneLoaderSlot = loader.slotIndex;
        action.sceneLoaderRecordKnown = loader.movieSegmentRecord.known;
        action.sceneLoaderRowIndex = loader.slotIndex;
        action.sceneLoaderPsxAddrKnown = loader.psxAddrKnown;
        action.sceneLoaderPsxAddr = loader.psxAddr;
        action.sceneLoaderMovieSegmentRecord = loader.movieSegmentRecord;
        action.sceneLoaderRowInit8001A324 = loader.rowInit8001A324;
        return;
    }
}

void EmitEntryPrologue(Runtime801C81EC& runtime, StepResult801C81EC& out) {
    if (runtime.entryPrologueRequested) {
        return;
    }

    Emit(runtime, out, ActionKind801C81EC::ConfigMovieViewport, kFn801CB67C);
    Emit(runtime, out, ActionKind801C81EC::AudioReset26FA4, kFn80026FA4);
    runtime.entryPrologueRequested = true;
}

void EmitInitialMovie1Request(Runtime801C81EC& runtime,
                              StepResult801C81EC& out,
                              const SceneEntry801C7284& sceneEntry) {
    Action801C81EC& open =
        Emit(runtime, out, ActionKind801C81EC::Transition201AC, kFn800201AC);
    open.strBlockKind = StrBlockKind801C81EC::InitialMovie1;
    SetTransitionArgs801C81EC(open,
                              kTransitionWork801C3640,
                              6,
                              2,
                              1,
                              false,
                              0,
                              kFn80020090);

    Action801C81EC& init =
        Emit(runtime, out, ActionKind801C81EC::StrInit, kFn801C44E0);
    AttachSceneLoaderSlot801C81EC(
        init,
        sceneEntry,
        kInitialMovie1LoaderOffset,
        StrBlockKind801C81EC::InitialMovie1);
    init.arg0 = 0;
    SetRawArgs(init, kInitialMovie1LoaderOffset, 0);

    Action801C81EC& play =
        Emit(runtime, out, ActionKind801C81EC::StrPlayAndWait, kFn801C455C);
    AttachSceneLoaderSlot801C81EC(
        play,
        sceneEntry,
        kInitialMovie1LoaderOffset,
        StrBlockKind801C81EC::InitialMovie1);
    play.transitionWorkPtr = kTransitionWork801C3640;
    play.arg0 = 0;
    SetRawArgs(play, kInitialMovie1LoaderOffset, kTransitionWork801C3640, 0);
    play.blocksUntilResult = true;
}

void EmitStageRunnerRunRequest(Runtime801C81EC& runtime,
                               StepResult801C81EC& out,
                               const SceneEntry801C7284& sceneEntry,
                               uint8_t sceneId) {
    Action801C81EC& run =
        Emit(runtime, out, ActionKind801C81EC::StageRunnerRun7A60,
             kFn801C7A60);
    AttachSceneLoaderSlot801C81EC(run, sceneEntry, kStageLoopLoaderOffset);
    run.transitionWorkPtr = kTransitionWork801C3640;
    run.sceneId = sceneId;
    run.arg0 = static_cast<int32_t>(kStageLoopLoaderOffset);
    run.arg1 = static_cast<int32_t>(kTransitionWork801C3640);
    run.arg2 = sceneId;
    SetRawArgs(run, kStageLoopLoaderOffset, kTransitionWork801C3640, sceneId);
    run.blocksUntilResult = true;
}

void EmitStageRecordTickRequest(Runtime801C81EC& runtime,
                                StepResult801C81EC& out,
                                const SceneEntry801C7284& sceneEntry) {
    Action801C81EC& record =
        Emit(runtime, out, ActionKind801C81EC::StageRecordTick1A4D0,
             kFn8001A4D0);
    AttachSceneLoaderSlot801C81EC(record, sceneEntry, kStageLoopLoaderOffset);
    record.arg0 = 0;
    SetRawArgs(record, kStageLoopLoaderOffset, 0);
}

void EmitStageLoopRequest(Runtime801C81EC& runtime,
                          StepResult801C81EC& out,
                          const SceneEntry801C7284& sceneEntry,
                          uint8_t sceneId) {
    EmitStageRecordTickRequest(runtime, out, sceneEntry);
    EmitStageRunnerRunRequest(runtime, out, sceneEntry, sceneId);
}

void EmitTransition201AC(Runtime801C81EC& runtime,
                         StepResult801C81EC& out,
                         int32_t modeA2,
                         int32_t preFfd4ArgA3,
                         int32_t postFfd4ArgA4,
                         StrBlockKind801C81EC owner =
                             StrBlockKind801C81EC::None) {
    Action801C81EC& action =
        Emit(runtime, out, ActionKind801C81EC::Transition201AC, kFn800201AC);
    SetTransitionArgs801C81EC(action,
                              kTransitionWork801C3640,
                              modeA2,
                              preFfd4ArgA3,
                              postFfd4ArgA4,
                              false,
                              0,
                              kFn80020090);
    action.strBlockKind = owner;
}

void EmitTransition20110(Runtime801C81EC& runtime,
                         StepResult801C81EC& out,
                         int32_t modeA2 = 2,
                         int32_t preFfd4ArgA3 = 1,
                         int32_t postFfd4ArgA4 = 2) {
    Action801C81EC& action =
        Emit(runtime, out, ActionKind801C81EC::Transition20110, kFn80020110);
    SetTransitionArgs801C81EC(action,
                              kTransitionWork801C3640,
                              modeA2,
                              preFfd4ArgA3,
                              postFfd4ArgA4,
                              true,
                              30,
                              kFn80020008);
}

void EmitResetHoldTiles(Runtime801C81EC& runtime, StepResult801C81EC& out) {
    Emit(runtime, out, ActionKind801C81EC::ResetHoldTiles1EF14, kFn8001EF14);
}

void EmitClearTailPreludeRequest(Runtime801C81EC& runtime,
                                 StepResult801C81EC& out,
                                 uint8_t sceneId) {
    EmitTransition20110(runtime, out);

    Action801C81EC& status =
        Emit(runtime, out, ActionKind801C81EC::QueryStageStatus166AC,
             kFn800166AC);
    status.sceneId = sceneId;
    status.arg0 = sceneId;
    SetRawArgs(status, sceneId);
    status.blocksUntilResult = true;
}

void EmitSceneResult(Runtime801C81EC& runtime,
                     StepResult801C81EC& out,
                     int32_t result) {
    out.sceneResultKnown = true;
    out.sceneResult = result;
    runtime.entryPrologueRequested = false;
}

void EmitAbortExit(Runtime801C81EC& runtime,
                   StepResult801C81EC& out,
                   uint16_t sceneExitReason) {
    EmitTransition20110(runtime, out);
    EmitResetHoldTiles(runtime, out);
    out.write800916E0 = true;
    out.word800916E0 = sceneExitReason;
    EmitSceneResult(runtime, out, -1);
}

void EmitClearTailMovieRequest(Runtime801C81EC& runtime,
                               StepResult801C81EC& out,
                               const SceneEntry801C7284& sceneEntry,
                               const FrameInput801C81EC& input) {
    const bool coolClear = input.byte801C368E;
    const uint32_t loaderOffset =
        coolClear ? kClearTailCoolLoaderOffset : kClearTailGoodLoaderOffset;
    const int32_t playMode = coolClear ? 2 : 1;

    Action801C81EC& init =
        Emit(runtime, out, ActionKind801C81EC::StrInit, kFn801C44E0);
    AttachSceneLoaderSlot801C81EC(
        init,
        sceneEntry,
        loaderOffset,
        StrBlockKind801C81EC::ClearTailMovie);
    init.arg0 = 0;
    SetRawArgs(init, loaderOffset, 0);

    Emit(runtime, out, ActionKind801C81EC::AudioReset26FA4, kFn80026FA4);
    EmitTransition201AC(runtime,
                        out,
                        6,
                        2,
                        1,
                        StrBlockKind801C81EC::ClearTailMovie);

    Action801C81EC& play =
        Emit(runtime, out, ActionKind801C81EC::StrPlayAndWait, kFn801C455C);
    AttachSceneLoaderSlot801C81EC(
        play,
        sceneEntry,
        loaderOffset,
        StrBlockKind801C81EC::ClearTailMovie);
    play.transitionWorkPtr = kTransitionWork801C3640;
    play.arg0 = playMode;
    SetRawArgs(play,
               loaderOffset,
               kTransitionWork801C3640,
               static_cast<uint32_t>(playMode));
    play.blocksUntilResult = true;
}

PrStageStatusBankClearProducerResult
EmitClearTailPostMovieAndStatus(Runtime801C81EC& runtime,
                                StepResult801C81EC& out,
                                const SceneEntry801C7284& sceneEntry,
                                const FrameInput801C81EC& input,
                                int32_t stageStatus166AC) {
    const bool coolClear = input.byte801C368E;

    Emit(runtime, out, ActionKind801C81EC::AudioReset26FA4, kFn80026FA4);
    EmitTransition201AC(runtime,
                        out,
                        5,
                        1,
                        2,
                        StrBlockKind801C81EC::ClearTailMovie);
    EmitResetHoldTiles(runtime, out);

    Action801C81EC& cue =
        Emit(runtime, out, ActionKind801C81EC::SfxCue26EF8, kFn80026EF8);
    cue.arg0 = static_cast<int32_t>(kCue80094410);
    SetRawArgs(cue, kCue80094410);
    Emit(runtime, out, ActionKind801C81EC::AudioFlush26ECC, kFn80026ECC);

    const PrStageStatusBankClearProducerInput statusInput{
        ResolveStatusBankComod801C81EC(input),
        input.sceneId,
        input.word800916D0,
        input.word800916DA,
        input.word800916F0Known,
        input.word800916F0,
        input.word80091816,
        true,
        stageStatus166AC,
        !coolClear,
        static_cast<uint16_t>(input.dword801D3040),
    };
    const PrStageStatusBankClearProducerResult statusResult =
        PrStageStatusBankDirectClearProducer(statusInput);
    for (size_t i = 0; i < statusResult.trace.count; ++i) {
        const PrStageStatusBankAction& statusAction =
            statusResult.trace.actions[i];
        switch (statusAction.kind) {
        case PrStageStatusBankActionKind::StoreWord800916D0:
            if (statusAction.arg0Known) {
                out.write800916D0 = true;
                out.word800916D0 = static_cast<uint16_t>(statusAction.arg0);
            }
            break;
        case PrStageStatusBankActionKind::StoreWord800916DA:
            if (statusAction.arg0Known) {
                out.write800916DA = true;
                out.word800916DA = static_cast<uint16_t>(statusAction.arg0);
            }
            break;
        case PrStageStatusBankActionKind::StoreWord800916E0:
            if (statusAction.arg0Known) {
                out.write800916E0 = true;
                out.word800916E0 = static_cast<uint16_t>(statusAction.arg0);
            }
            break;
        case PrStageStatusBankActionKind::Call8001635C:
        {
            Action801C81EC& save =
                Emit(runtime, out, ActionKind801C81EC::SaveStatus1635C,
                     kFn8001635C);
            save.sceneId = static_cast<uint8_t>(
                statusAction.arg0Known ? statusAction.arg0 : input.sceneId);
            save.arg0 = statusAction.arg1Known ? statusAction.arg1 : 0;
            save.arg1 = statusAction.arg2Known ? statusAction.arg2
                                               : stageStatus166AC;
            save.arg2 = statusAction.arg3Known ? statusAction.arg3
                                               : input.word80091816;
            SetRawArgs(save,
                       static_cast<uint32_t>(save.sceneId),
                       static_cast<uint32_t>(save.arg0),
                       static_cast<uint32_t>(save.arg1),
                       static_cast<uint32_t>(save.arg2));
            break;
        }
        case PrStageStatusBankActionKind::Call8001628C:
        {
            Action801C81EC& unlock =
                Emit(runtime, out, ActionKind801C81EC::UnlockNextStage1628C,
                     kFn8001628C);
            unlock.sceneId = static_cast<uint8_t>(
                statusAction.arg0Known ? statusAction.arg0 : input.sceneId + 1);
            SetRawArgs(unlock, static_cast<uint32_t>(unlock.sceneId));
            break;
        }
        case PrStageStatusBankActionKind::Call80015590:
        {
            Action801C81EC& save =
                Emit(runtime, out, ActionKind801C81EC::Bootstrap15590,
                     kFn80015590);
            AttachSceneLoaderSlot801C81EC(
                save,
                sceneEntry,
                kBootstrapZcompoLoaderOffset);
            save.bootstrapZcompoLoader = true;
            save.sceneId = static_cast<uint8_t>(
                statusAction.arg0Known ? statusAction.arg0 : input.sceneId);
            SetRawArgs(save, static_cast<uint32_t>(save.sceneId));
            break;
        }
        case PrStageStatusBankActionKind::Call80019148:
        {
            Action801C81EC& saveUi =
                Emit(runtime, out, ActionKind801C81EC::SaveUi19148,
                     kFn80019148);
            saveUi.arg0 = statusAction.arg0Known
                              ? statusAction.arg0
                              : static_cast<int32_t>(kSavePayloadBank80092F10);
            SetRawArgs(saveUi, static_cast<uint32_t>(saveUi.arg0));
            break;
        }
        case PrStageStatusBankActionKind::None:
        case PrStageStatusBankActionKind::Call800166AC:
        case PrStageStatusBankActionKind::Call80015CC4:
        case PrStageStatusBankActionKind::Call800169E0:
        case PrStageStatusBankActionKind::StoreLocalStash:
        case PrStageStatusBankActionKind::Call80024E54:
        case PrStageStatusBankActionKind::Call80094440:
        case PrStageStatusBankActionKind::Call800143F0:
        case PrStageStatusBankActionKind::Call8001681C:
        case PrStageStatusBankActionKind::Call80016758:
        case PrStageStatusBankActionKind::Call8001670C:
        case PrStageStatusBankActionKind::Call800259C0:
            break;
        }
    }

    EmitResetHoldTiles(runtime, out);
    return statusResult;
}

void FinalizeClearTailResult(Runtime801C81EC& runtime,
                             StepResult801C81EC& out,
                             const PrStageStatusBankClearProducerResult&
                                 statusResult) {
    if (statusResult.returnValueKnown) {
        EmitSceneResult(runtime, out, statusResult.returnValue);
    } else if (statusResult.blockedByUnknownWord800916F0) {
        out.blockedByUnknownWord800916F0 = true;
    }
}

} // namespace

Globals801CA3BC InitGlobals801CA3BC() {
    Globals801CA3BC out{};
    out.sourceFunction = 0x801CA3BCu;
    out.returnValueKnown = true;
    out.returnValue = 1;
    out.ptr800943C0 = 0x801D2F04u;
    out.ptr800943C4 = 0x801CD38Cu;
    out.value800943C8 = 66u;
    out.ptr800943CC = 0x801CE080u;
    out.ptr800943D0 = 0x801CFA54u;
    out.ptr800943D4 = 0x801D0084u;
    out.ptr800943D8 = 0x801CCF34u;
    out.ptr800943DC = 0x801CD2B8u;
    out.ptr800943E0 = 0x801CD2BCu;
    out.ptr800943E4 = 0x801D2E2Cu;
    out.ptr800943E8 = 0x801D2F00u;
    out.ptr800943EC_F8 = {
        0x801CCC00u,
        0x801CCC08u,
        0x801CCC10u,
        0x801CCC18u,
    };
    out.ptr800943FC = 0x801CCC2Cu;
    out.ptr80094400 = 0x801CCC20u;
    out.ptr80094404_10 = {
        0x801CCC38u,
        0x801CCC40u,
        0x801CCC48u,
        0x801CCC50u,
    };
    out.ptr80094414_2C = {
        0x801CCC58u,
        0x801CCC60u,
        0x801CCC68u,
        0x801CCC74u,
        0x801CCC7Cu,
        0x801CCC84u,
        0x801CCC8Cu,
    };
    out.fn80094430 = 0x801CA0E0u;
    out.fn80094434 = 0x801CA1ECu;
    out.fn80094438 = 0x801CA25Cu;
    out.fn8009443C = 0x801CA264u;
    out.fn80094440 = 0x801C8660u;
    return out;
}

SceneEntry801C7284 InitScene801C7284(const InitSceneInput801C7284& input) {
    SceneEntry801C7284 out{};
    out.sceneIndex = input.sceneIndex;

    out.movieSegmentTable801C4780 =
        PrStage1MovieSegmentDirect::
            MaterializeStage1SceneEntryMovieSegmentsFromStaticTable801C4780(
                input.sceneIndex);
    const bool hasMovieSegmentBatchFeedback =
        input.movieSegmentCdLookupBatchFeedbackKnown &&
        input.movieSegmentCdLookupBatchFeedback.matchedCount != 0u;
    const bool hasMovieSegmentFeedback =
        input.movieSegmentCdSeamProbeFeedbackKnown &&
        input.movieSegmentCdSeamProbeFeedback.probeAccepted &&
        input.movieSegmentCdSeamProbeFeedback.matched;
    if (hasMovieSegmentBatchFeedback) {
        out.movieSegmentFeedbackSource =
            MovieSegmentFeedbackSource801C7284::CdLookupBatch;
        out.movieSegmentScan801C4780 =
            PrStage1MovieSegmentDirect::
                PsxCall801C4780_ScanMovieSegmentsWithFeedback(
                    out.movieSegmentTable801C4780,
                    input.movieSegmentCdLookupBatchFeedback.rowFeedback.data(),
                    PrStage1MovieSegmentDirect::
                        kSceneEntryMovieSegmentCount801C4780);
    } else if (hasMovieSegmentFeedback) {
        out.movieSegmentFeedbackSource =
            MovieSegmentFeedbackSource801C7284::SingleCdSeamProbe;
        out.movieSegmentScan801C4780 =
            PrStage1MovieSegmentDirect::
                PsxCall801C4780_ScanMovieSegmentsWithFeedback(
                    out.movieSegmentTable801C4780,
                    input.movieSegmentCdSeamProbeFeedback.rowFeedback.data(),
                    PrStage1MovieSegmentDirect::
                        kSceneEntryMovieSegmentCount801C4780);
    } else {
        out.movieSegmentScan801C4780 =
            PrStage1MovieSegmentDirect::PsxCall801C4780_ScanMovieSegments(
                out.movieSegmentTable801C4780);
    }
    // 801C7284 shares the seven-row 8001A324 pass, not 801C4780's
    // post-scan COMPO loader tail.
    out.movieSegmentScan801C4780.loadCompo8001AC18Action = false;
    out.movieSegmentScan801C4780.loadCompoArg0 = 0;
    out.movieSegmentScan801C4780.loadCompoArg1 = 0;
    out.sceneEntryPtr8006EDB8Known =
        out.movieSegmentTable801C4780.sceneEntryBaseKnown;
    out.sceneEntryPtr8006EDB8 = out.movieSegmentTable801C4780.sceneEntryBase;

    const int32_t v14 =
        96 * static_cast<int32_t>(input.sceneEntryField06_Bpm100);
    out.field348_TicksPerMinute = v14 / 100;
    out.field356_BaseTick =
        static_cast<int32_t>(input.sceneEntryField08_TickOffset) +
        static_cast<int32_t>(input.sceneEntryField0A_ExtraTickOffset);
    out.field352_TicksPerFrame60 = (v14 / 3600 + 50) / 100;
    out.field360 = 16;

    for (uint8_t i = 0; i < out.loaders.size(); i++) {
        Scene1LoaderRecord801C7284& loader = out.loaders[i];
        loader.slotIndex = i;
        loader.psxOffsetFromSceneEntry =
            kSceneEntryFirstLoaderOffset801C7284 +
            PrStage1MovieSegmentDirect::kMovieSegmentRecordSize8001A324 *
                static_cast<uint32_t>(i);
        loader.present = input.loaderPresent[i];
    }
    RefreshSceneLoadersFromMovieScan801C7284(out);
    out.row1CompoLoadFeedback =
        BuildSceneEntryCompoLoadFeedbackCarrier801C7284(out, input);

    out.compoLoadTableValue801CCBBCKnown =
        input.compoLoadTableValue801CCBBCKnown;
    out.compoLoadTableValue801CCBBC = input.compoLoadTableValue801CCBBC;

    Action801C7284& resetPool =
        EmitInitAction801C7284(out,
                               ActionKind801C7284::ResetResourcePool25A34,
                               kFn80025A34);
    SetRawArgs(resetPool,
               out.sceneEntryPtr8006EDB8,
               input.sceneIndex,
               input.rawArgA3,
               input.rawArgA4);

    EmitInitAction801C7284(out,
                           ActionKind801C7284::StageRunnerResetState,
                           kFn801C85DC);
    EmitInitAction801C7284(out,
                           ActionKind801C7284::ResetEventRuntime24E98,
                           kFn80024E98);
    EmitInitAction801C7284(out,
                           ActionKind801C7284::ResetScorer14344,
                           kFn80014344);

    for (const Scene1LoaderRecord801C7284& loader : out.loaders) {
        Action801C7284& initSegment =
            EmitInitAction801C7284(out,
                                   ActionKind801C7284::InitSegmentRecord1A324,
                                   kFn8001A324);
        AttachSceneLoaderSlot801C7284(initSegment, out, loader);
    }

    EmitInitAction801C7284(out,
                           ActionKind801C7284::ResetResourceCursor25A00,
                           kFn80025A00);
    EmitInitAction801C7284(out,
                           ActionKind801C7284::ResetHoldTiles1EF14,
                           kFn8001EF14);

    Action801C7284& loadCompo =
        EmitInitAction801C7284(out,
                               ActionKind801C7284::LoadCompo15660,
                               kFn80015660);
    loadCompo.compoLoadTableValue801CCBBCKnown =
        out.compoLoadTableValue801CCBBCKnown;
    loadCompo.compoLoadTableValue801CCBBC =
        out.compoLoadTableValue801CCBBC;
    SetRawArgs(loadCompo,
               input.sceneIndex,
               out.compoLoadTableValue801CCBBC,
               1);
    return out;
}

void Reset801C81ECRuntime(Runtime801C81EC& runtime) {
    runtime = Runtime801C81EC{};
}

StepResult801C81EC Step801C81EC(Runtime801C81EC& runtime,
                                const SceneEntry801C7284& sceneEntry,
                                const FrameInput801C81EC& input) {
    (void)sceneEntry;

    StepResult801C81EC out{};
    out.phaseBefore = runtime.phase;

    EmitEntryPrologue(runtime, out);

    if (runtime.phase == Phase801C81EC::InitialMovie1Requested) {
        if (!input.movie1PlayAndWaitResultKnown) {
            out.phaseAfter = runtime.phase;
            return out;
        }

        Emit(runtime, out, ActionKind801C81EC::AudioReset26FA4, kFn80026FA4);
        EmitTransition201AC(runtime,
                            out,
                            5,
                            1,
                            2,
                            StrBlockKind801C81EC::InitialMovie1);

        if (input.movie1PlayAndWaitResult == 1) {
            EmitResetHoldTiles(runtime, out);
            out.write800916E0 = true;
            out.word800916E0 = 3;
            EmitSceneResult(runtime, out, -1);
            out.phaseAfter = runtime.phase;
            return out;
        }

        runtime.phase = Phase801C81EC::StageLoop;
    }

    if (runtime.phase == Phase801C81EC::StageRunRequested) {
        if (!input.stageResultKnown) {
            EmitStageRunnerRunRequest(runtime, out, sceneEntry, input.sceneId);
            out.phaseAfter = runtime.phase;
            return out;
        }

        if (input.word800916D0 == 1) {
            out.write800916D0 = true;
            out.word800916D0 = 0;
            out.write800916DA = true;
            out.word800916DA = static_cast<uint16_t>(input.dword801D3040);
            EmitTransition20110(runtime, out);
            EmitResetHoldTiles(runtime, out);
            EmitSceneResult(runtime, out, 0);
            out.phaseAfter = runtime.phase;
            return out;
        }

        if (input.word800916D0 == 2) {
            Action801C81EC& restore =
                Emit(runtime, out,
                     ActionKind801C81EC::RestoreTransitionPayload15744,
                     kFn80015744);
            restore.arg0 = static_cast<int32_t>(kSavePayloadBank80092F10);
            SetRawArgs(restore, kSavePayloadBank80092F10);
            out.write800916D0 = true;
            out.word800916D0 = 0;
            EmitTransition20110(runtime, out);
            EmitResetHoldTiles(runtime, out);
            out.write800916E0 = true;
            out.word800916E0 = 3;
            out.write800916DA = true;
            out.word800916DA = static_cast<uint16_t>(input.dword801D3040);
            EmitSceneResult(runtime, out, -1);
            out.phaseAfter = runtime.phase;
            return out;
        }

        if (input.stageResult801C7A60 == 1) {
            EmitClearTailPreludeRequest(runtime, out, input.sceneId);
            runtime.clearTailStageStatusKnown = false;
            runtime.clearTailStageStatus166AC = 0;
            runtime.phase = Phase801C81EC::ClearTailStatusRequested;
            out.phaseAfter = runtime.phase;
            return out;
        }

        // 801C7A60 is a blocking PSX stage runner. The Win host currently
        // advances it one frame at a time, so result 0 means "keep running",
        // not "the PSX call returned and abort poll should run now".
        if (input.stageResult801C7A60 == 0) {
            runtime.phase = Phase801C81EC::StageLoop;
            out.phaseAfter = runtime.phase;
            return out;
        }

        Action801C81EC& abortPoll =
            Emit(runtime, out, ActionKind801C81EC::QueryAbort26B94,
                 kFn80026B94);
        abortPoll.arg0 = 4;
        abortPoll.arg1 = 0;
        SetRawArgs(abortPoll, 4, 0);
        abortPoll.blocksUntilResult = true;

        if (input.abortPollResultKnown) {
            if (input.abortPollResult26B94 == 2) {
                EmitAbortExit(runtime, out, 2);
                out.phaseAfter = runtime.phase;
                return out;
            }
            EmitResetHoldTiles(runtime, out);
            runtime.phase = Phase801C81EC::StageLoop;
        } else {
            runtime.phase = Phase801C81EC::AbortPollRequested;
        }

        out.phaseAfter = runtime.phase;
        return out;
    }

    if (runtime.phase == Phase801C81EC::AbortPollRequested) {
        if (!input.abortPollResultKnown) {
            out.phaseAfter = runtime.phase;
            return out;
        }

        if (input.abortPollResult26B94 == 2) {
            EmitAbortExit(runtime, out, 2);
            out.phaseAfter = runtime.phase;
            return out;
        }

        EmitResetHoldTiles(runtime, out);
        runtime.phase = Phase801C81EC::StageLoop;
    }

    if (runtime.phase == Phase801C81EC::ClearTailStatusRequested) {
        if (!input.stageStatus166ACKnown) {
            out.phaseAfter = runtime.phase;
            return out;
        }

        runtime.clearTailStageStatusKnown = true;
        runtime.clearTailStageStatus166AC = input.stageStatus166AC;
        EmitClearTailMovieRequest(runtime, out, sceneEntry, input);
        runtime.phase = Phase801C81EC::ClearTailMovieRequested;
        out.phaseAfter = runtime.phase;
        return out;
    }

    if (runtime.phase == Phase801C81EC::ClearTailMovieRequested) {
        if (!input.clearTailPlayAndWaitResultKnown ||
            !runtime.clearTailStageStatusKnown) {
            out.phaseAfter = runtime.phase;
            return out;
        }

        const PrStageStatusBankClearProducerResult statusResult =
            EmitClearTailPostMovieAndStatus(
                runtime,
                out,
                sceneEntry,
                input,
                runtime.clearTailStageStatus166AC);
        FinalizeClearTailResult(runtime, out, statusResult);
        runtime.clearTailStageStatusKnown = false;
        runtime.clearTailStageStatus166AC = 0;
        out.phaseAfter = runtime.phase;
        return out;
    }

    if (runtime.phase == Phase801C81EC::Entry) {
        if (IsStageLoopBypassD0(input.word800916D0)) {
            runtime.phase = Phase801C81EC::StageLoop;
        } else {
            EmitInitialMovie1Request(runtime, out, sceneEntry);
            runtime.phase = Phase801C81EC::InitialMovie1Requested;
            out.phaseAfter = runtime.phase;
            return out;
        }
    }

    if (runtime.phase == Phase801C81EC::StageLoop) {
        EmitStageLoopRequest(runtime, out, sceneEntry, input.sceneId);
        runtime.phase = Phase801C81EC::StageRunRequested;
        out.phaseAfter = runtime.phase;
        return out;
    }

    out.phaseAfter = runtime.phase;
    return out;
}

} // namespace PrStage1LifecycleDirect
