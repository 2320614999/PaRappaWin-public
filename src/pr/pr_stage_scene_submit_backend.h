#pragma once

#include <cstdint>
#include <string>

class ResourceManager;
struct PrGameContext;
struct PrStage1RuntimeSlotsSnapshot;

namespace PrPsxFastSpriteSubmitDirect {
struct RuntimeState8003FA20;
}

namespace PrPsxEventFrameDirect {
struct EventFrameState8001E750;
}

namespace PrStageSceneSubmitBackend {

void LoadStage1Resources(ResourceManager* resources);

void ClearStage1Resources();

void ResetStage1SceneSubmitRuntimeForRender801CBFDC190();

bool AdvanceStage1SceneSubmitRuntimeForRender801CBFDC190(
    PrGameContext& ctx,
    const PrStage1RuntimeSlotsSnapshot& runtimeSlots,
    uint8_t renderSubFrame8);

void DrawStage1Scene801CBFDC190(PrGameContext& ctx);

void DrawStage1SceneGameplayBase801CBFDC190(PrGameContext& ctx);

void DrawStage1FastSpriteRuntime8003FA20(
    PrGameContext& ctx,
    const PrPsxFastSpriteSubmitDirect::RuntimeState8003FA20& runtime);

void DrawEventFrameBoxFillPackets8003EE84(
    PrGameContext& ctx,
    const PrPsxEventFrameDirect::EventFrameState8001E750& state);

void DrawEventFrameMoveImageBoxFill8001B120(
    PrGameContext& ctx,
    const PrPsxEventFrameDirect::EventFrameState8001E750& state);

void DrawEventFrameFastSpritePackets8003FA20(
    PrGameContext& ctx,
    const PrPsxEventFrameDirect::EventFrameState8001E750& state);

std::string DescribeStage1SceneSubmit428B0Debug();

} // namespace PrStageSceneSubmitBackend
