#pragma once

#include "pr_stage_event_direct.h"
#include "pr_stage1_overlay_parser.h"

#include <cstddef>
#include <cstdint>

struct PrStage1OverlayScriptTextSnapshot;

static constexpr uint16_t kPrStage1OverlayScriptTextDirectTimeoutFrames = 0x78u;
static constexpr uint8_t kPrStage1OverlayScriptTextDefaultStreamIdSub801C8604 =
    1u;

struct PrStage1OverlayScriptTextDirectRuntime {
    bool producerActive = false;
    bool initialized = false;
    bool directScriptBoxPermit4E = false;
    uint8_t selectedLanguageIndex = 0;
    // PSX 801C8604/801C9094 sink: unk_8008ECE4 and ctx+0x10C mirror.
    const char* activeTextPtr = nullptr;
    const char* activeTextMirrorPtr = nullptr;
    // PSX word_8008ECFA lifetime for unk_8008ECE4.
    uint16_t activeTimeoutFramesRemaining = 0;
    // Debug-only source id; display consumers must use the resolved sink.
    uint8_t activeTextId = 0;
    uint8_t streamId = 0;
    bool useLocalStreamClock = false;
    uint32_t localStreamStartFrame = 0;
    uint32_t lastQueryFrame = 0;
    uint32_t lastScriptFrame = 0;
    uint8_t lastAppliedStreamId = 0;
    uint32_t lastAppliedEventPsxAddr = 0;
    uint32_t lastAppliedEventDueFrame = 0;
    size_t eventCursor = 0;
};

struct PrStage1OverlayScriptTextDirectScene1FrameInputSub801C8604 {
    const PrStage1OverlayData* data = nullptr;
    bool currentSceneIsStage1 = false;
    bool stageRunning = false;
    bool runnerPlayingStage1 = false;
    uint8_t selectedLanguageIndex = 0;
    uint32_t queryFrame30 = 0;
    uint32_t scriptFrame = 0;
    bool directScriptBoxPermit4E = false;
    uint8_t defaultStreamId =
        kPrStage1OverlayScriptTextDefaultStreamIdSub801C8604;
    const PrStageEventDirectStage1FrameResult801C9094* frameResult801C9094 =
        nullptr;
};

void PrStage1OverlayScriptTextDirectResetSub801C8604(
    PrStage1OverlayScriptTextDirectRuntime& runtime);

bool PrStage1OverlayScriptTextDirectResolveSnapshot(
    const PrStage1OverlayScriptTextDirectRuntime& runtime,
    PrStage1OverlayScriptTextSnapshot& out);

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
    const PrStageEventDirectStage1FrameResult801C9094* frameResult801C9094);

bool PrStage1OverlayScriptTextDirectAdvanceScene1FrameSub801C8604(
    PrStage1OverlayScriptTextDirectRuntime& runtime,
    const PrStage1OverlayScriptTextDirectScene1FrameInputSub801C8604& input);
