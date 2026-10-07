#pragma once
#include "pr/pr_presentation_settings.h"

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

    PrPresentationSettings presentation;

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
