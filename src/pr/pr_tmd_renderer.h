#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

class D3D11Renderer;
class ResourceManager;
struct PrGameContext;

namespace PrTmdRenderer {
    // Boundary: Scene0/title TMD and menu overlay renderer only. Stage1
    // scene-submit/state semantics stay in the direct/backend command path.

    // Load TMD models from COMPO00.INT MEM resources (HP/LO/PA/PA_KAGE.TMD)
    // Also loads paired VDF/DAT/TOD animation resources
    void LoadModels(ResourceManager* resources);

    // Render loaded TMD models at PSX-style projection for Scene0 menu
    void Render(PrGameContext& ctx, float viewX, float viewY, float viewScale);

    // Check if models are loaded
    bool HasModels();

    // Stage runtime path: load every original stage TMD found in the active
    // COMPO archive and render the PSX textured geometry directly.  This is
    // intentionally separate from the Scene0/title model set above; stages
    // have a different model directory and must not fall back to the old
    // procedural stage shell when their original resources are available.
    void LoadStageModels(ResourceManager* resources, int sceneId);
    bool RenderStage(PrGameContext& ctx, float viewX, float viewY,
                     float viewScale);
    bool HasStageModels();

    // Clear legacy Scene0/title renderer state when SS0 direct owns Scene0.
    void Clear();

    // Initialize camera animation from COMOD*.BIN event table + INT resources
    void InitCameraEvents(const uint8_t* comodData, size_t comodSize,
                          ResourceManager* resources,
                          int sceneId = -1);

    // Initialize PSX face event system from COMOD*.BIN data
    void InitFaceEvents(const uint8_t* comodData, size_t comodSize,
                        const std::string& jsonPath, ResourceManager* resources);

    void SetScn0SqevFrame(uint32_t stageFrame);
    void ResetScn0FaceEvents();

    // === Scene0 menu overlay quad rendering ===
    // Replicates PSX's sub_80013D10/sub_80014050: create flat quads from TIM texture
    // dimensions and render through the same camera projection as TMD models.
    // screenX/Y = desired PSX screen position (320x240 coords); the quad is reverse-
    // projected through the current camera to 3D space, then rendered alongside models.
    struct OverlayQuadParams {
        const char* textureName;   // TIM texture name in ResourceManager
        float       screenX;       // PSX screen X (0-320), -1 = auto-center
        float       screenY;       // PSX screen Y (0-240)
        int         layer;         // sprite layer for ordering
    };
    void SubmitOverlayQuad(PrGameContext& ctx, const OverlayQuadParams& params,
                           float viewX, float viewY, float viewScale);

    // Get diagnostic info string for DebugServer tmddump
    std::string DumpStats();
}
