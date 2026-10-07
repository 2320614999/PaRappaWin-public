#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "pr_stage1_loader_gpu_hal.h"

class ResourceManager;
class PsxVramAtlas;
struct PrGameContext;
struct PrStage1RuntimeSlotsSnapshot;

namespace PrSS0Scene0IntLoadDirect {
struct Transaction8001AC18;
}

namespace PrPsxFastSpriteSubmitDirect {
struct RuntimeState8003FA20;
}

namespace PrPsxEventFrameDirect {
struct EventFrameState8001E750;
}

namespace PrStageSceneSubmitBackend {

bool LoadStage1Resources(ResourceManager* resources,
    const PrSS0Scene0IntLoadDirect::Transaction8001AC18* startupCommon = nullptr);

void ClearStage1Resources();

bool ApplyStage1NativeTimUploads8001A8F0(
    const std::vector<PrStage1LoaderGpuHal::TimRecordUpload8001A8F0>& uploads,
    PsxVramAtlas* residentAtlas = nullptr);

// Borrow the native COMMON + subsequent INT upload projection for the
// resident 80015788 directory. Ownership stays here; no scene/model reset.
PsxVramAtlas& GetNativeDirectoryAtlasProjection80015590();

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
