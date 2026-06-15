#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "pr_scene.h"
#include "pr_stage1_xa_cd_direct.h"

class D3D11Renderer;
class ResourceManager;
class StrPlayer;
class Xa1Player;
struct PrStage1OverlayData;
struct PrSceneDef;

struct PrGameContext {
    D3D11Renderer* renderer = nullptr;
    ResourceManager* resources = nullptr;
    StrPlayer* strPlayer = nullptr;
    Xa1Player* xa1Player = nullptr;
    PrStage1XaCdDirectState stage1XaCdDirect{};
    std::filesystem::path dataRoot{};
    std::filesystem::path currentXaPath{};
    std::filesystem::path currentComodPath{};
    std::filesystem::path currentCompoPath{};
    std::filesystem::path currentZcompoPath{};
    std::vector<uint8_t> currentComodBytes;
    std::shared_ptr<PrStage1OverlayData> stage1OverlayData;
    const PrSceneDef* currentSceneDef = nullptr;
    PrSceneId currentScene = PrSceneId::Scene0;
    uint32_t frame = 0;

    // PSX 关键全局变量 (对应主程序 word_800916D0/DA/E0)
    int16_t transitionState = 0;      // word_800916D0: 0=默认, 1=随机选关, 2=replay/manual-load 特殊入口
    int16_t transitionStateDA = 0;    // word_800916DA: 关联标志
    int16_t languageIndex = 0;        // word_800916D8: 语言索引 (0=EN,1=FR,2=DE,3=ES,4=IT)
    int16_t subtitleFlag = 0;         // word_800916DC: 字幕开关 (0=关闭, 1=开启; PSX global)
    int16_t sceneExitReason = 0;      // word_800916E0: 退出原因 (2=ev4退出, 3=收尾退出)

    // 调试输入
    bool debugNextScene = false;
    bool debugGenericSwitch = false;
    int debugGenericEvent = 0;
    int debugGenericEventArg = 0;
    bool debugStage1DirectBoot = false; // debug-only: skip MOVIE1 and jump directly into Stage1 loop
    int debugStage1TextStream = 0;      // debug-only: 0=auto/main stream1, 1..8=force parser text stream
    uint16_t debugPadInput = 0;       // 直接注入的 PSX pad mask
    bool debugShowBgTexture = false;  // 是否渲染背景纹理（默认关闭避免误选字库）
    int debugBgTexIndex = -1;

    bool debugScn0Mode = false;
    int debugScn0BgIndex = -1;
    int debugScn0UiBaseIndex = -1;
    int debugScn0UiStride = 1;
    bool debugScn0UiUseMenuIndex = false;
    int scn0SubFrameOffset = 0;

    // 调试热键标志 (min runnable)
    bool debugF1_StrSkip = false;     // F1: 模拟 sub_80035510()==256，触发 STR 跳过
    bool debugEsc_Ev4Exit = false;    // Esc: 触发 ev=4 并走 CIRCLE->2 退出
    bool debugF5_StageClear = false;  // F5: non-Stage1 debug clear shortcut

    // 关卡状态
    bool stageRunning = false;        // 是否在关卡循环中
    int stageRunnerResult = 0;        // StageRunner 的模拟返回值

    // 脚本事件调试（最近一次 Emit）
    uint32_t lastSqevsEventId = 0;
    uint32_t lastSqevsEventFrame = 0;

    // 程序退出请求（例如 Scene0 的 EXIT）
    bool requestQuit = false;

    int debugScn0Cursor = 0;
    bool debugScn0CursorOverride = false;

    int scn0HiliteCursor = 0;
    int scn0HiliteLastCursor = 0;
    int scn0HiliteBank = 0;
    int scn0HiliteCooldown = 0;
    int scn0HiliteCooldown2 = 0;
    int scn0HiliteOverlayEnabled = 0;
    int scn0HiliteHudSlot = 0;
    int scn0HiliteResIndex = 0;
    int scn0HudState = 4;

    // Menu panel slide-in animation (PSX: dx=-163, dy=192, frames=204)
    int scn0PanelAnimFrame = 0;       // current frame (counts up from 0 to scn0PanelAnimTotal)
    int scn0PanelAnimTotal = 204;     // total frames for slide-in
    float scn0PanelOffsetX = 0.0f;    // current X offset in PSX pixels (starts at dx, ends at 0)
    float scn0PanelOffsetY = 0.0f;    // current Y offset in PSX pixels (starts at dy, ends at 0)
    bool scn0PanelAnimActive = false; // animation in progress
    int scn0DanceEndDelayTicks = 21;

    // 60fps rendering: render-only frame flag (no logic tick on this frame)
    bool renderOnlyFrame = false;
    uint8_t renderSubFrame8 = 0;     // 0..255: in-between render phase within current 30Hz logic frame
    bool stage1ParappaRailAssist = true;
    int stage1RailMode = 0;
    float stage1RailParappa2Darken = 0.42f;
    bool stage1RailParappa2CoreAlign = false;
    int stage1RailParappa2PopFrames = 5;
    float stage1RailParappa2PopScale = 2.0f;
    int stage1RailParappa2FlipFrames = 8;
    int stage1RailParappa2GlowFadeFrames = 16;
    float stage1RailParappa2GlowAlpha = 0.55f;
    float stage1RailParappa2GlowScale = 1.75f;
    float stage1RailParappa2LeadSlots = 0.0f;
    bool stage1RailParappa2TraceAlign = false;
    bool stage1RailParappa2ScorerHud = true;
    bool stage1RailParappa2CreativePrompt = true;
    std::string stage1RailParappa2CreativePromptLanguage = "EN";
    bool stage1RestoreCeilingLights = false;
    bool stage1HdGeometryCleanup = false;
    bool stage1TextureReplacements = false;
    std::filesystem::path stage1TextureReplacementDir{};
    bool stage1HdSubtitles = true;
    std::string stage1HdSubtitleLanguage = "CN";
    std::filesystem::path stage1HdSubtitleFile{};
    std::string stage1HdSubtitleFont = "Microsoft YaHei";
    float stage1HdSubtitleFontSizePsx = 11.5f;
    float stage1HdSubtitleY = 184.0f;
    float stage1HdSubtitleMovieY = 180.0f;
    float stage1HdSubtitleGameplayY = 182.0f;
    float stage1HdSubtitleWidth = 288.0f;
    bool stage1HdSubtitleDrawBox = false;
    std::string stage1HdSubtitleFillColor = "#FFFFFF";
    std::string stage1HdSubtitleOutlineColor = "#000000";
    std::string stage1HdSubtitleShadowColor = "#6F6F6F";
    float stage1HdSubtitleOutlinePsx = 1.25f;
    float stage1HdSubtitleShadowOffsetXPsx = 1.4f;
    float stage1HdSubtitleShadowOffsetYPsx = 1.7f;
    bool debugStage1TextureReplacementTrace = false;
    bool debugStage1ShowPsxFrame = false;

    bool debugFaceAutoShots = false;
    bool debugFaceAutoShotPending = false;
    std::string debugFaceAutoShotTag;
};
