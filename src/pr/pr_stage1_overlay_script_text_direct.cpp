#include "pr_stage1_overlay_script_text_direct.h"

#include "pr_scn1.h"

namespace {

struct PrStage1OverlayScriptTextDirectWindow {
    const PrStage1OverlayData* data = nullptr;
    const PrStage1EventStream* stream = nullptr;
    uint8_t languageIndex = 0;
    uint32_t queryFrame = 0;
    uint32_t scriptFrame = 0;
    bool directScriptBoxPermit4E = false;
    bool useLocalStreamClock = false;
    uint32_t localStreamStartFrame = 0;
    bool dueEventValid = false;
    uint8_t dueEventStreamId = 0;
    const PrStage1ScriptEvent* dueEvent = nullptr;
    uint32_t dueEventDueFrame = 0;
};

struct PrStage1OverlayScriptTextDirectAdvanceInput {
    const PrStage1OverlayData* data = nullptr;
    bool currentSceneIsStage1 = false;
    bool stageRunning = false;
    bool runnerPlayingStage1 = false;
    uint8_t selectedLanguageIndex = 0;
    uint32_t queryFrame30 = 0;
    uint32_t scriptFrame = 0;
    bool directScriptBoxPermit4E = false;
    uint8_t defaultStreamId = 1;
    uint8_t selectedStreamId = 0;
    uint8_t activeDispatchStreamId = 0;
    uint32_t activeDispatchStartScriptFrame = 0;
    bool flag2000SceneFamilyActive = false;
    uint8_t flag2000SceneFamilyStreamId = 0;
    uint32_t flag2000SceneFamilyStartFrame = 0;
    uint32_t flag2000SceneFamilyStartScriptFrame = 0;
    const PrStageEventDirectStage1FrameResult801C9094* frameResult801C9094 =
        nullptr;
};

struct PrStage1OverlayScriptTextDirectProducerWindowInput {
    const PrStage1OverlayData* data = nullptr;
    bool currentSceneIsStage1 = false;
    bool stageRunning = false;
    bool runnerPlayingStage1 = false;
    uint8_t selectedLanguageIndex = 0;
    uint32_t queryFrame30 = 0;
    uint32_t scriptFrame = 0;
    bool directScriptBoxPermit4E = false;
    uint8_t defaultStreamId = 1;
    uint8_t selectedStreamId = 0;
    uint8_t activeDispatchStreamId = 0;
    uint32_t activeDispatchStartScriptFrame = 0;
    bool flag2000SceneFamilyActive = false;
    uint8_t flag2000SceneFamilyStreamId = 0;
    uint32_t flag2000SceneFamilyStartFrame = 0;
    uint32_t flag2000SceneFamilyStartScriptFrame = 0;
};

void ClearStage1OverlayScriptTextActiveSinkSub801C9094(
    PrStage1OverlayScriptTextDirectRuntime& runtime) {
    runtime.activeTextId = 0u;
    // 801C9094 clears unk_8008ECE4 and ctx+0x10C when word_8008ECFA expires.
    runtime.activeTextPtr = nullptr;
    runtime.activeTextMirrorPtr = nullptr;
    runtime.activeTimeoutFramesRemaining = 0u;
}

void MirrorStage1OverlayScriptTextCtx10CSub801C8604(
    PrStage1OverlayScriptTextDirectRuntime& runtime) {
    runtime.activeTextMirrorPtr = runtime.activeTextPtr;
}

void TickStage1OverlayScriptTextFrameUpdateSub801C9094(
    PrStage1OverlayScriptTextDirectRuntime& runtime,
    uint32_t queryFrame) {
    if (!runtime.initialized || queryFrame <= runtime.lastQueryFrame ||
        runtime.activeTimeoutFramesRemaining == 0u) {
        return;
    }

    const uint32_t elapsed = queryFrame - runtime.lastQueryFrame;
    if (elapsed >= runtime.activeTimeoutFramesRemaining) {
        ClearStage1OverlayScriptTextActiveSinkSub801C9094(runtime);
        return;
    }
    runtime.activeTimeoutFramesRemaining =
        (uint16_t)(runtime.activeTimeoutFramesRemaining - elapsed);
}

bool IsStage1OverlayScriptTextDueEventNew(
    const PrStage1OverlayScriptTextDirectRuntime& runtime,
    const PrStage1OverlayScriptTextDirectWindow& window) {
    if (!window.dueEventValid || !window.dueEvent ||
        window.dueEventStreamId != window.stream->streamId) {
        return false;
    }

    return runtime.lastAppliedStreamId != window.dueEventStreamId ||
           runtime.lastAppliedEventPsxAddr != window.dueEvent->psxAddr ||
           runtime.lastAppliedEventDueFrame != window.dueEventDueFrame;
}

size_t ResolveStage1OverlayScriptTextEventIndex(
    const PrStage1EventStream& stream,
    const PrStage1ScriptEvent& event) {
    for (size_t i = 0; i < stream.events.size(); ++i) {
        if (stream.events[i].psxAddr == event.psxAddr) {
            return i;
        }
    }
    return 0u;
}

uint8_t ResolveStage1OverlayScriptTextWindowStreamId(
    const PrStage1OverlayScriptTextDirectProducerWindowInput& input) {
    uint8_t streamId = input.defaultStreamId;
    if (input.selectedStreamId != 0u) {
        streamId = input.selectedStreamId;
    }
    if (input.flag2000SceneFamilyActive &&
        input.flag2000SceneFamilyStreamId != 0u &&
        PrStageEventDirectStage1IsFlag2000Stream(
            input.flag2000SceneFamilyStreamId) &&
        (streamId == input.defaultStreamId ||
         input.activeDispatchStreamId == 0u)) {
        streamId = input.flag2000SceneFamilyStreamId;
    }
    return streamId;
}

bool ResolveStage1OverlayScriptTextLocalStreamClock(
    const PrStage1OverlayScriptTextDirectProducerWindowInput& input,
    uint8_t streamId,
    uint32_t& outStartFrame) {
    if (streamId == input.defaultStreamId) {
        return false;
    }
    if (input.activeDispatchStreamId == streamId) {
        outStartFrame = input.activeDispatchStartScriptFrame;
        return true;
    }
    if (input.flag2000SceneFamilyActive &&
        input.flag2000SceneFamilyStreamId == streamId) {
        outStartFrame = input.flag2000SceneFamilyStartScriptFrame != 0u
                            ? input.flag2000SceneFamilyStartScriptFrame
                            : input.flag2000SceneFamilyStartFrame;
        return true;
    }
    return false;
}

bool ResolveStage1OverlayScriptTextDirectProducerWindow(
    const PrStage1OverlayScriptTextDirectProducerWindowInput& input,
    PrStage1OverlayScriptTextDirectWindow& outWindow) {
    outWindow = PrStage1OverlayScriptTextDirectWindow{};
    if (!input.currentSceneIsStage1 ||
        !input.stageRunning ||
        !input.runnerPlayingStage1 ||
        input.data == nullptr ||
        !input.data->valid) {
        return false;
    }

    outWindow.data = input.data;
    outWindow.languageIndex = input.selectedLanguageIndex;
    outWindow.queryFrame = input.queryFrame30;
    outWindow.scriptFrame = input.scriptFrame;
    outWindow.directScriptBoxPermit4E = input.directScriptBoxPermit4E;

    const uint8_t streamId =
        ResolveStage1OverlayScriptTextWindowStreamId(input);

    outWindow.stream = input.data->FindStream(streamId);
    if (outWindow.stream == nullptr) {
        outWindow = PrStage1OverlayScriptTextDirectWindow{};
        return false;
    }

    uint32_t localStreamStartFrame = 0u;
    if (ResolveStage1OverlayScriptTextLocalStreamClock(
            input,
            streamId,
            localStreamStartFrame)) {
        outWindow.useLocalStreamClock = true;
        outWindow.localStreamStartFrame = localStreamStartFrame;
    }
    return true;
}

bool ApplyStage1OverlayScriptTextConsumedEvent801C9094(
    const PrStageEventDirectStage1ConsumedScriptEvent801C9094& consumed,
    PrStage1OverlayScriptTextDirectWindow& window) {
    if (!consumed.valid || !window.data || !window.stream ||
        consumed.streamId != window.stream->streamId) {
        return false;
    }

    if (consumed.eventIndex >= window.stream->events.size()) {
        return false;
    }

    const PrStage1ScriptEvent& ev =
        window.stream->events[static_cast<size_t>(consumed.eventIndex)];
    if (ev.psxAddr != consumed.psxAddr) {
        return false;
    }

    window.dueEventValid = true;
    window.dueEventStreamId = consumed.streamId;
    window.dueEvent = &ev;
    window.dueEventDueFrame = consumed.dueFrame;
    return true;
}

const PrStageEventDirectStage1ConsumedScriptEvent801C9094&
ResolveStage1OverlayScriptTextConsumedEvent801C9094(
    const PrStage1OverlayScriptTextDirectAdvanceInput& input) {
    static const PrStageEventDirectStage1ConsumedScriptEvent801C9094
        kNoConsumedEvent{};
    if (!input.frameResult801C9094 || !input.frameResult801C9094->valid) {
        return kNoConsumedEvent;
    }
    return input.frameResult801C9094->setTextEvent801C8604;
}

} // namespace

void PrStage1OverlayScriptTextDirectResetSub801C8604(
    PrStage1OverlayScriptTextDirectRuntime& runtime) {
    runtime = PrStage1OverlayScriptTextDirectRuntime{};
}

static bool PrStage1OverlayScriptTextDirectAdvanceSub801C8604(
    PrStage1OverlayScriptTextDirectRuntime& runtime,
    const PrStage1OverlayScriptTextDirectWindow& window) {
    if (!window.data || !window.stream) {
        PrStage1OverlayScriptTextDirectResetSub801C8604(runtime);
        return false;
    }
    if (runtime.initialized && window.queryFrame < runtime.lastQueryFrame) {
        PrStage1OverlayScriptTextDirectResetSub801C8604(runtime);
    }
    TickStage1OverlayScriptTextFrameUpdateSub801C9094(runtime,
                                                     window.queryFrame);

    runtime.producerActive = true;
    runtime.initialized = true;
    runtime.directScriptBoxPermit4E = window.directScriptBoxPermit4E;
    runtime.selectedLanguageIndex = window.languageIndex;
    runtime.streamId = window.stream->streamId;
    runtime.useLocalStreamClock = window.useLocalStreamClock;
    runtime.localStreamStartFrame = window.localStreamStartFrame;
    runtime.lastQueryFrame = window.queryFrame;
    runtime.lastScriptFrame = window.scriptFrame;

    if (!IsStage1OverlayScriptTextDueEventNew(runtime, window)) {
        MirrorStage1OverlayScriptTextCtx10CSub801C8604(runtime);
        return true;
    }

    runtime.lastAppliedStreamId = window.dueEventStreamId;
    runtime.lastAppliedEventPsxAddr = window.dueEvent->psxAddr;
    runtime.lastAppliedEventDueFrame = window.dueEventDueFrame;
    runtime.eventCursor =
        ResolveStage1OverlayScriptTextEventIndex(*window.stream,
                                                 *window.dueEvent);
    if (window.dueEvent->textId == 0u) {
        MirrorStage1OverlayScriptTextCtx10CSub801C8604(runtime);
        return true;
    }

    const std::string* activeText =
        window.data->ResolveText(window.languageIndex, window.dueEvent->textId);
    if (activeText == nullptr) {
        MirrorStage1OverlayScriptTextCtx10CSub801C8604(runtime);
        return true;
    }

    // 801C8604: textId != 0 writes word_8008ECFA=0x78, then unk_8008ECE4,
    // then mirrors the current pointer to ctx+0x10C.
    runtime.activeTextId = window.dueEvent->textId;
    runtime.activeTextPtr = activeText->c_str();
    runtime.activeTimeoutFramesRemaining =
        kPrStage1OverlayScriptTextDirectTimeoutFrames;
    MirrorStage1OverlayScriptTextCtx10CSub801C8604(runtime);
    return true;
}

bool PrStage1OverlayScriptTextDirectResolveSnapshot(
    const PrStage1OverlayScriptTextDirectRuntime& runtime,
    PrStage1OverlayScriptTextSnapshot& out) {
    out = PrStage1OverlayScriptTextSnapshot{};
    out.producerActive = runtime.producerActive;
    if (!runtime.producerActive) {
        return false;
    }

    out.directScriptBoxPermit4E = runtime.directScriptBoxPermit4E;
    out.selectedLanguageIndex = runtime.selectedLanguageIndex;
    out.activeTextPtr = runtime.activeTextPtr;
    out.activeTextMirrorPtr = runtime.activeTextMirrorPtr;
    out.activeTextId = runtime.activeTextId;
    out.activeTimeoutFramesRemaining =
        runtime.activeTimeoutFramesRemaining;
    return true;
}

PrStage1OverlayScriptTextDirectScene1FrameInputSub801C8604
PrStage1OverlayScriptTextDirectBuildScene1FrameInputSub801C8604(
    const PrStage1OverlayData* data,
    bool currentSceneIsStage1,
    bool stageRunning,
    bool runnerPlayingStage1,
    uint8_t selectedLanguageIndex,
    uint32_t queryFrame30,
    uint32_t scriptFrame,
    bool directScriptBoxPermit4E,
    const PrStageEventDirectStage1FrameResult801C9094* frameResult801C9094) {
    PrStage1OverlayScriptTextDirectScene1FrameInputSub801C8604 input{};
    input.data = data;
    input.currentSceneIsStage1 = currentSceneIsStage1;
    input.stageRunning = stageRunning;
    input.runnerPlayingStage1 = runnerPlayingStage1;
    input.selectedLanguageIndex = selectedLanguageIndex;
    input.queryFrame30 = queryFrame30;
    input.scriptFrame = scriptFrame;
    input.directScriptBoxPermit4E = directScriptBoxPermit4E;
    input.defaultStreamId =
        kPrStage1OverlayScriptTextDefaultStreamIdSub801C8604;
    input.frameResult801C9094 = frameResult801C9094;
    return input;
}

static bool PrStage1OverlayScriptTextDirectAdvanceFromRuntimeSub801C8604(
    PrStage1OverlayScriptTextDirectRuntime& runtime,
    const PrStage1OverlayScriptTextDirectAdvanceInput& input) {
    PrStage1OverlayScriptTextDirectWindow window{};
    if (!ResolveStage1OverlayScriptTextDirectProducerWindow(
            PrStage1OverlayScriptTextDirectProducerWindowInput{
                input.data,
                input.currentSceneIsStage1,
                input.stageRunning,
                input.runnerPlayingStage1,
                input.selectedLanguageIndex,
                input.queryFrame30,
                input.scriptFrame,
                input.directScriptBoxPermit4E,
                input.defaultStreamId,
                input.selectedStreamId,
                input.activeDispatchStreamId,
                input.activeDispatchStartScriptFrame,
                input.flag2000SceneFamilyActive,
                input.flag2000SceneFamilyStreamId,
                input.flag2000SceneFamilyStartFrame,
                input.flag2000SceneFamilyStartScriptFrame,
            },
            window)) {
        PrStage1OverlayScriptTextDirectResetSub801C8604(runtime);
        return false;
    }

    ApplyStage1OverlayScriptTextConsumedEvent801C9094(
        ResolveStage1OverlayScriptTextConsumedEvent801C9094(input),
        window);

    return PrStage1OverlayScriptTextDirectAdvanceSub801C8604(runtime, window);
}

bool PrStage1OverlayScriptTextDirectAdvanceScene1FrameSub801C8604(
    PrStage1OverlayScriptTextDirectRuntime& runtime,
    const PrStage1OverlayScriptTextDirectScene1FrameInputSub801C8604& input) {
    PrStage1OverlayScriptTextDirectAdvanceInput advance{};
    advance.data = input.data;
    advance.currentSceneIsStage1 = input.currentSceneIsStage1;
    advance.stageRunning = input.stageRunning;
    advance.runnerPlayingStage1 = input.runnerPlayingStage1;
    advance.selectedLanguageIndex = input.selectedLanguageIndex;
    advance.queryFrame30 = input.queryFrame30;
    advance.scriptFrame = input.scriptFrame;
    advance.directScriptBoxPermit4E = input.directScriptBoxPermit4E;
    advance.defaultStreamId = input.defaultStreamId;

    if (input.frameResult801C9094 != nullptr) {
        advance.frameResult801C9094 = input.frameResult801C9094;
        const auto& setTextEvent =
            input.frameResult801C9094->setTextEvent801C8604;
        advance.selectedStreamId =
            setTextEvent.valid ? setTextEvent.streamId
                               : input.frameResult801C9094->selectedStream;
        advance.activeDispatchStreamId =
            input.frameResult801C9094->activeDispatchStream;
        advance.activeDispatchStartScriptFrame =
            input.frameResult801C9094->activeDispatchStartScriptFrame;
        advance.flag2000SceneFamilyActive =
            input.frameResult801C9094->flag2000SceneFamilyActive;
        advance.flag2000SceneFamilyStreamId =
            input.frameResult801C9094->flag2000SceneFamilyStream;
        advance.flag2000SceneFamilyStartFrame =
            input.frameResult801C9094->flag2000SceneFamilyStartFrame;
        advance.flag2000SceneFamilyStartScriptFrame =
            input.frameResult801C9094->flag2000SceneFamilyStartScriptFrame;
    }

    return PrStage1OverlayScriptTextDirectAdvanceFromRuntimeSub801C8604(
        runtime,
        advance);
}
