#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <filesystem>

// All configurable settings for PaRappaWin.
// Loaded from / saved to config.ini in the EXE directory.
// If the file does not exist, it is auto-generated with defaults.

struct KeyBindings {
    int up        = 0x26;  // VK_UP
    int down      = 0x28;  // VK_DOWN
    int left      = 0x25;  // VK_LEFT
    int right     = 0x27;  // VK_RIGHT
    int up_alt    = 'W';
    int down_alt  = 'S';
    int left_alt  = 'A';
    int right_alt = 'D';

    int triangle     = 'V';
    int circle       = 'X';
    int cross        = 'Z';
    int cross_alt    = 0x20; // VK_SPACE
    int cross_alt2   = 0x0D; // VK_RETURN
    int square       = 'C';

    int l1 = 'Q';
    int r1 = 'E';
    int l2 = '1';
    int r2 = '3';

    int start  = 'P';
    int select = 'O';

    // Debug / Win-only
    int str_skip = 0x70; // VK_F1
};

struct AudioConfig {
    float master = 0.8f;
    float bgm    = 0.8f;
    float sfx    = 0.8f;
};

struct AppConfig {
    KeyBindings keys;
    AudioConfig audio;
    bool subtitlesEnabled = true;

    // Window
    int windowWidth  = 640;
    int windowHeight = 480;

    // Graphics
    bool render60fps = false;  // 60fps rendering mode (logic stays 30Hz)
    bool stage1ParappaRailAssist = true;  // Stage1 Parappa portrait assist marker
    int stage1RailMode = 0;  // 0=PSX compact rail, 1=PaRappa 2-style student rail
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
    bool stage1RestoreCeilingLights = false;  // Optional restoration, off for exact ROM behavior
    bool stage1HdGeometryCleanup = false;  // Optional HD cleanup for PSX scene-map strip artifacts
    bool stage1TextureReplacements = false;  // Optional PS4/PSPHD texture replacement layer
    std::string stage1TextureReplacementDir = "ex/image/texreplace";
    bool stage1HdSubtitles = true;  // Optional external HD subtitle overlay for Stage1
    std::string stage1HdSubtitleLanguage = "CN";
    std::string stage1HdSubtitleFile = "ex/subtitles/stage1_hd_zh.tsv";
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

    // Debug
    bool debugStage1TextureReplacementTrace = false;  // Verbose per-lookup texture replacement trace
    bool debugStage1ShowPsxFrame = false;  // Show Stage1 PSX logic frame in-game

    // Load from file. Returns true if file existed and was parsed.
    bool Load(const std::filesystem::path& path);

    // Save current settings to file.
    bool Save(const std::filesystem::path& path) const;

    // Load from EXE directory. If not found, generate default and save.
    static AppConfig LoadOrCreate(const std::filesystem::path& exeDir);

    // Get path to config.ini relative to EXE dir
    static std::filesystem::path GetConfigPath(const std::filesystem::path& exeDir);

private:
    // INI helpers
    static std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
        ParseIni(const std::filesystem::path& path);

    static std::string GetVal(
        const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& ini,
        const std::string& section, const std::string& key, const std::string& def);

    static int ParseVKey(const std::string& s);
    static std::string VKeyToString(int vk);
};
