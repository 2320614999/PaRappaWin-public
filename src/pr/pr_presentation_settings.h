#pragma once
#include <array>
#include <string>

// One presentation policy for every stage; stage-specific assets are data only.
struct PrPresentationSettings {
    bool render60fps = false;
    bool enabled = true;
    int aspectMode = 1;
    bool railAssist = true;
    int railMode = 0;
    float railDarken = 0.42f;
    bool railCoreAlign = false;
    int railPopFrames = 5;
    float railPopScale = 2.0f;
    int railFlipFrames = 8;
    int railGlowFadeFrames = 16;
    float railGlowAlpha = 0.55f;
    float railGlowScale = 1.75f;
    float railLeadSlots = 0.0f;
    bool railTraceAlign = false;
    bool railScorerHud = true;
    bool railCreativePrompt = true;
    std::string railCreativePromptLanguage = "EN";
    bool restoreSceneDetails = false;
    bool hdGeometryCleanup = false;
    bool textureReplacements = false;
    std::string textureReplacementDir = "ex/image/texreplace";
    bool hdSubtitles = true;
    std::string hdSubtitleLanguage = "CN";
    std::string hdSubtitleFont = "Microsoft YaHei";
    float hdSubtitleFontSizePsx = 11.5f;
    float hdSubtitleY = 184.0f;
    float hdSubtitleMovieY = 180.0f;
    float hdSubtitleGameplayY = 182.0f;
    float hdSubtitleWidth = 288.0f;
    bool hdSubtitleDrawBox = false;
    std::string hdSubtitleFillColor = "#FFFFFF";
    std::string hdSubtitleOutlineColor = "#000000";
    std::string hdSubtitleShadowColor = "#6F6F6F";
    float hdSubtitleOutlinePsx = 1.25f;
    float hdSubtitleShadowOffsetXPsx = 1.4f;
    float hdSubtitleShadowOffsetYPsx = 1.7f;
    std::array<std::string, 6> subtitleFiles{{
        "ex/subtitles/stage1_hd_zh.tsv", "ex/subtitles/stage2_hd_zh.tsv",
        "ex/subtitles/stage3_hd_zh.tsv", "ex/subtitles/stage4_hd_zh.tsv",
        "ex/subtitles/stage5_hd_zh.tsv", "ex/subtitles/stage6_hd_zh.tsv"}};

    // Derive runtime settings without erasing the user's stored preferences.
    PrPresentationSettings Effective() const {
        auto result = *this;
        if (!enabled) {
            result.aspectMode = 2; // Original 4:3 framing.
            result.render60fps = false;
            result.railMode = 0;
            result.railAssist = false;
            result.railScorerHud = false;
            result.railCreativePrompt = false;
            result.restoreSceneDetails = false;
            result.hdGeometryCleanup = false;
            result.textureReplacements = false;
            result.hdSubtitles = false;
        }
        return result;
    }
};
