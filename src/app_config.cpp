#include "app_config.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>

// ========== INI Parser ==========

static std::string Trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::string ToLower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return out;
}

std::unordered_map<std::string, std::unordered_map<std::string, std::string>>
AppConfig::ParseIni(const std::filesystem::path& path) {
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> result;
    std::ifstream f(path);
    if (!f.is_open()) return result;

    std::string currentSection = "general";
    std::string line;
    while (std::getline(f, line)) {
        line = Trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;

        if (line.front() == '[' && line.back() == ']') {
            currentSection = ToLower(Trim(line.substr(1, line.size() - 2)));
            continue;
        }

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = ToLower(Trim(line.substr(0, eq)));
        std::string val = Trim(line.substr(eq + 1));
        result[currentSection][key] = val;
    }
    return result;
}

std::string AppConfig::GetVal(
    const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& ini,
    const std::string& section, const std::string& key, const std::string& def)
{
    auto sit = ini.find(section);
    if (sit == ini.end()) return def;
    auto kit = sit->second.find(key);
    if (kit == sit->second.end()) return def;
    return kit->second;
}

// ========== VKey name <-> int ==========

struct VKeyEntry { const char* name; int code; };
static const VKeyEntry kVKeyTable[] = {
    {"VK_UP",       0x26}, {"VK_DOWN",     0x28}, {"VK_LEFT",     0x25}, {"VK_RIGHT",    0x27},
    {"VK_SPACE",    0x20}, {"VK_RETURN",   0x0D}, {"VK_ESCAPE",   0x1B}, {"VK_TAB",      0x09},
    {"VK_BACK",     0x08}, {"VK_DELETE",   0x2E}, {"VK_INSERT",   0x2D},
    {"VK_HOME",     0x24}, {"VK_END",      0x23}, {"VK_PRIOR",    0x21}, {"VK_NEXT",     0x22},
    {"VK_F1",       0x70}, {"VK_F2",       0x71}, {"VK_F3",       0x72}, {"VK_F4",       0x73},
    {"VK_F5",       0x74}, {"VK_F6",       0x75}, {"VK_F7",       0x76}, {"VK_F8",       0x77},
    {"VK_F9",       0x78}, {"VK_F10",      0x79}, {"VK_F11",      0x7A}, {"VK_F12",      0x7B},
    {"VK_LSHIFT",   0xA0}, {"VK_RSHIFT",   0xA1}, {"VK_LCONTROL", 0xA2}, {"VK_RCONTROL", 0xA3},
    {"VK_LMENU",    0xA4}, {"VK_RMENU",    0xA5},
    {"VK_SHIFT",    0x10}, {"VK_CONTROL",  0x11}, {"VK_MENU",     0x12},
    {"VK_OEM_1",    0xBA}, {"VK_OEM_PLUS", 0xBB}, {"VK_OEM_COMMA",0xBC},
    {"VK_OEM_MINUS",0xBD}, {"VK_OEM_PERIOD",0xBE},{"VK_OEM_2",   0xBF}, {"VK_OEM_3",   0xC0},
    {nullptr, 0}
};

int AppConfig::ParseVKey(const std::string& s) {
    std::string upper = s;
    std::transform(upper.begin(), upper.end(), upper.begin(),
                   [](unsigned char c) { return (char)std::toupper(c); });
    upper = Trim(upper);

    // Named VK_* constants
    for (const VKeyEntry* e = kVKeyTable; e->name; ++e) {
        if (upper == e->name) return e->code;
    }

    // Hex: 0x##
    if (upper.size() > 2 && upper[0] == '0' && (upper[1] == 'X' || upper[1] == 'x')) {
        return (int)std::strtol(upper.c_str(), nullptr, 16);
    }

    // Decimal number
    if (!upper.empty() && std::isdigit((unsigned char)upper[0])) {
        return (int)std::strtol(upper.c_str(), nullptr, 10);
    }

    // Single character -> use its ASCII value as VK
    if (upper.size() == 1 && std::isalnum((unsigned char)upper[0])) {
        return (int)(unsigned char)upper[0];
    }

    return -1; // invalid
}

std::string AppConfig::VKeyToString(int vk) {
    // Named VK_* constants
    for (const VKeyEntry* e = kVKeyTable; e->name; ++e) {
        if (vk == e->code) return e->name;
    }

    // Printable ASCII character
    if (vk >= 0x30 && vk <= 0x39) {
        return std::string(1, (char)vk); // 0-9
    }
    if (vk >= 0x41 && vk <= 0x5A) {
        return std::string(1, (char)vk); // A-Z
    }

    // Fallback to hex
    char buf[16];
    snprintf(buf, sizeof(buf), "0x%02X", vk);
    return buf;
}

// ========== Load ==========

bool AppConfig::Load(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) return false;

    auto ini = ParseIni(path);
    if (ini.empty()) return false;

    auto readKey = [&](const std::string& section, const std::string& key, int defaultVal) -> int {
        std::string val = GetVal(ini, section, key, "");
        if (val.empty()) return defaultVal;
        int vk = ParseVKey(val);
        return (vk >= 0) ? vk : defaultVal;
    };

    auto readFloat = [&](const std::string& section, const std::string& key, float defaultVal) -> float {
        std::string val = GetVal(ini, section, key, "");
        if (val.empty()) return defaultVal;
        return std::strtof(val.c_str(), nullptr);
    };

    auto readBool = [&](const std::string& section, const std::string& key, bool defaultVal) -> bool {
        std::string val = ToLower(GetVal(ini, section, key, ""));
        if (val.empty()) return defaultVal;
        return (val == "true" || val == "1" || val == "on" || val == "yes");
    };

    auto readInt = [&](const std::string& section, const std::string& key, int defaultVal) -> int {
        std::string val = GetVal(ini, section, key, "");
        if (val.empty()) return defaultVal;
        return (int)std::strtol(val.c_str(), nullptr, 10);
    };
    auto readString = [&](const std::string& section,
                          const std::string& key,
                          const std::string& defaultVal) -> std::string {
        std::string val = GetVal(ini, section, key, "");
        if (val.empty()) return defaultVal;
        return val;
    };

    auto readStage1RailMode = [&]() -> int {
        std::string val = ToLower(GetVal(ini, "graphics", "stage1_rail_mode", ""));
        val = Trim(val);
        if (val.empty()) return stage1RailMode;
        if (val == "parappa2" || val == "p2" || val == "2" ||
            val == "mode2") {
            return 1;
        }
        if (val == "psx" || val == "stage1" || val == "original" ||
            val == "1" || val == "0") {
            return 0;
        }
        return stage1RailMode;
    };
    auto normalizeP2PromptLanguage = [](std::string val) -> std::string {
        val = ToLower(Trim(val));
        if (val == "cn" || val == "zh" || val == "zh-cn" ||
            val == "chinese") {
            return "CN";
        }
        return "EN";
    };
    auto normalizeHdSubtitleLanguage = [](std::string val) -> std::string {
        val = ToLower(Trim(val));
        if (val == "auto") return "auto";
        if (val == "cn" || val == "zh" || val == "zh-cn" ||
            val == "chinese") {
            return "CN";
        }
        if (val == "fr" || val == "french") return "FR";
        if (val == "de" || val == "ger" || val == "german") return "DE";
        if (val == "es" || val == "sp" || val == "spanish") return "ES";
        if (val == "it" || val == "italian") return "IT";
        return "EN";
    };

    // [keys]
    keys.up         = readKey("keys", "up",         keys.up);
    keys.down       = readKey("keys", "down",       keys.down);
    keys.left       = readKey("keys", "left",       keys.left);
    keys.right      = readKey("keys", "right",      keys.right);
    keys.up_alt     = readKey("keys", "up_alt",     keys.up_alt);
    keys.down_alt   = readKey("keys", "down_alt",   keys.down_alt);
    keys.left_alt   = readKey("keys", "left_alt",   keys.left_alt);
    keys.right_alt  = readKey("keys", "right_alt",  keys.right_alt);

    keys.triangle   = readKey("keys", "triangle",   keys.triangle);
    keys.circle     = readKey("keys", "circle",     keys.circle);
    keys.cross      = readKey("keys", "cross",      keys.cross);
    keys.cross_alt  = readKey("keys", "cross_alt",  keys.cross_alt);
    keys.cross_alt2 = readKey("keys", "cross_alt2", keys.cross_alt2);
    keys.square     = readKey("keys", "square",     keys.square);

    keys.l1 = readKey("keys", "l1", readKey("keys", "l", keys.l1));
    keys.r1 = readKey("keys", "r1", readKey("keys", "r", keys.r1));
    keys.l2 = readKey("keys", "l2", keys.l2);
    keys.r2 = readKey("keys", "r2", keys.r2);

    keys.start    = readKey("keys", "start",    keys.start);
    keys.select   = readKey("keys", "select",   keys.select);
    keys.str_skip = readKey("keys", "str_skip", keys.str_skip);

    // [audio]
    audio.master = readFloat("audio", "master", audio.master);
    audio.bgm    = readFloat("audio", "bgm",   audio.bgm);
    audio.sfx    = readFloat("audio", "sfx",    audio.sfx);

    // Clamp volumes to [0, 1]
    auto clamp01 = [](float v) { return (v < 0.0f) ? 0.0f : (v > 1.0f) ? 1.0f : v; };
    audio.master = clamp01(audio.master);
    audio.bgm    = clamp01(audio.bgm);
    audio.sfx    = clamp01(audio.sfx);

    // [display]
    subtitlesEnabled = readBool("display", "subtitles", subtitlesEnabled);
    windowWidth      = readInt("display", "window_width",  windowWidth);
    windowHeight     = readInt("display", "window_height", windowHeight);

    // [graphics]
    render60fps = readBool("graphics", "render60fps", render60fps);
    stage1ParappaRailAssist =
        readBool("graphics", "stage1_parappa_rail_assist",
                 stage1ParappaRailAssist);
    stage1RailMode = readStage1RailMode();
    stage1RailParappa2Darken =
        clamp01(readFloat("stage1_rail_parappa2", "darken",
                          stage1RailParappa2Darken));
    stage1RailParappa2CoreAlign =
        readBool("stage1_rail_parappa2", "core_align",
                 stage1RailParappa2CoreAlign);
    stage1RailParappa2PopFrames =
        (std::max)(0, readInt("stage1_rail_parappa2", "pop_frames",
                              stage1RailParappa2PopFrames));
    stage1RailParappa2PopScale =
        (std::max)(1.0f, readFloat("stage1_rail_parappa2", "pop_scale",
                                   stage1RailParappa2PopScale));
    stage1RailParappa2FlipFrames =
        (std::max)(1, readInt("stage1_rail_parappa2", "flip_frames",
                              stage1RailParappa2FlipFrames));
    stage1RailParappa2GlowFadeFrames =
        (std::max)(0, readInt("stage1_rail_parappa2", "glow_fade_frames",
                              stage1RailParappa2GlowFadeFrames));
    stage1RailParappa2GlowAlpha =
        clamp01(readFloat("stage1_rail_parappa2", "glow_alpha",
                          stage1RailParappa2GlowAlpha));
    stage1RailParappa2GlowScale =
        (std::max)(1.0f, readFloat("stage1_rail_parappa2", "glow_scale",
                                   stage1RailParappa2GlowScale));
    stage1RailParappa2LeadSlots =
        readFloat("stage1_rail_parappa2", "lead_slots",
                  stage1RailParappa2LeadSlots);
    stage1RailParappa2TraceAlign =
        readBool("stage1_rail_parappa2", "trace_align",
                 stage1RailParappa2TraceAlign);
    stage1RailParappa2ScorerHud =
        readBool("stage1_rail_parappa2", "scorer_hud",
                 stage1RailParappa2ScorerHud);
    stage1RailParappa2CreativePrompt =
        readBool("stage1_rail_parappa2", "creative_prompt",
                 stage1RailParappa2CreativePrompt);
    stage1RailParappa2CreativePromptLanguage =
        normalizeP2PromptLanguage(
            readString("stage1_rail_parappa2",
                       "creative_prompt_language",
                       stage1RailParappa2CreativePromptLanguage));
    stage1RestoreCeilingLights =
        readBool("graphics", "stage1_restore_ceiling_lights",
                 stage1RestoreCeilingLights);
    stage1HdGeometryCleanup =
        readBool("graphics", "stage1_hd_geometry_cleanup",
                 stage1HdGeometryCleanup);
    stage1TextureReplacements =
        readBool("graphics", "stage1_texture_replacements",
                 stage1TextureReplacements);
    stage1TextureReplacementDir =
        readString("graphics", "stage1_texture_replacement_dir",
                   stage1TextureReplacementDir);
    stage1HdSubtitles =
        readBool("stage1_hd_subtitles", "enabled",
                 stage1HdSubtitles);
    stage1HdSubtitleLanguage =
        normalizeHdSubtitleLanguage(
            readString("stage1_hd_subtitles", "language",
                       stage1HdSubtitleLanguage));
    stage1HdSubtitleFile =
        readString("stage1_hd_subtitles", "file",
                   stage1HdSubtitleFile);
    stage1HdSubtitleFont =
        readString("stage1_hd_subtitles", "font",
                   stage1HdSubtitleFont);
    stage1HdSubtitleFontSizePsx =
        (std::max)(4.0f,
                   readFloat("stage1_hd_subtitles", "font_size_psx",
                             stage1HdSubtitleFontSizePsx));
    stage1HdSubtitleY =
        readFloat("stage1_hd_subtitles", "y_psx", stage1HdSubtitleY);
    stage1HdSubtitleMovieY =
        readFloat("stage1_hd_subtitles", "movie_y_psx",
                  stage1HdSubtitleMovieY);
    stage1HdSubtitleGameplayY =
        readFloat("stage1_hd_subtitles", "gameplay_y_psx",
                  stage1HdSubtitleGameplayY);
    stage1HdSubtitleWidth =
        (std::max)(80.0f,
                   readFloat("stage1_hd_subtitles", "width_psx",
                             stage1HdSubtitleWidth));
    stage1HdSubtitleDrawBox =
        readBool("stage1_hd_subtitles", "draw_box",
                 stage1HdSubtitleDrawBox);
    stage1HdSubtitleFillColor =
        readString("stage1_hd_subtitles", "fill_color",
                   stage1HdSubtitleFillColor);
    stage1HdSubtitleOutlineColor =
        readString("stage1_hd_subtitles", "outline_color",
                   stage1HdSubtitleOutlineColor);
    stage1HdSubtitleShadowColor =
        readString("stage1_hd_subtitles", "shadow_color",
                   stage1HdSubtitleShadowColor);
    stage1HdSubtitleOutlinePsx =
        (std::max)(0.0f,
                   readFloat("stage1_hd_subtitles", "outline_psx",
                             stage1HdSubtitleOutlinePsx));
    stage1HdSubtitleShadowOffsetXPsx =
        readFloat("stage1_hd_subtitles", "shadow_offset_x_psx",
                  stage1HdSubtitleShadowOffsetXPsx);
    stage1HdSubtitleShadowOffsetYPsx =
        readFloat("stage1_hd_subtitles", "shadow_offset_y_psx",
                  stage1HdSubtitleShadowOffsetYPsx);

    // [debug]
    debugStage1TextureReplacementTrace =
        readBool("debug", "stage1_texture_replacement_trace",
                 debugStage1TextureReplacementTrace);
    debugStage1ShowPsxFrame =
        readBool("debug", "stage1_show_psx_frame",
                 debugStage1ShowPsxFrame);

    return true;
}

// ========== Save ==========

bool AppConfig::Save(const std::filesystem::path& path) const {
    std::ofstream f(path);
    if (!f.is_open()) return false;

    f << "; PaRappa the Rapper - Win Edition Config\n";
    f << "; Auto-generated. Edit freely; delete to reset to defaults.\n";
    f << "; Key values: A-Z, 0-9, VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT,\n";
    f << ";   VK_SPACE, VK_RETURN, VK_F1..VK_F12, or hex 0x##.\n";
    f << "\n";

    f << "[keys]\n";
    f << "up         = " << VKeyToString(keys.up)         << "\n";
    f << "down       = " << VKeyToString(keys.down)       << "\n";
    f << "left       = " << VKeyToString(keys.left)       << "\n";
    f << "right      = " << VKeyToString(keys.right)      << "\n";
    f << "up_alt     = " << VKeyToString(keys.up_alt)     << "\n";
    f << "down_alt   = " << VKeyToString(keys.down_alt)   << "\n";
    f << "left_alt   = " << VKeyToString(keys.left_alt)   << "\n";
    f << "right_alt  = " << VKeyToString(keys.right_alt)  << "\n";
    f << "triangle   = " << VKeyToString(keys.triangle)   << "\n";
    f << "circle     = " << VKeyToString(keys.circle)     << "\n";
    f << "cross      = " << VKeyToString(keys.cross)      << "\n";
    f << "cross_alt  = " << VKeyToString(keys.cross_alt)  << "\n";
    f << "cross_alt2 = " << VKeyToString(keys.cross_alt2) << "\n";
    f << "square     = " << VKeyToString(keys.square)     << "\n";
    f << "l1         = " << VKeyToString(keys.l1)         << "\n";
    f << "r1         = " << VKeyToString(keys.r1)         << "\n";
    f << "l2         = " << VKeyToString(keys.l2)         << "\n";
    f << "r2         = " << VKeyToString(keys.r2)         << "\n";
    f << "start      = " << VKeyToString(keys.start)      << "\n";
    f << "select     = " << VKeyToString(keys.select)     << "\n";
    f << "str_skip   = " << VKeyToString(keys.str_skip)   << "\n";
    f << "\n";

    f << "[audio]\n";
    f << "master = " << audio.master << "\n";
    f << "bgm    = " << audio.bgm    << "\n";
    f << "sfx    = " << audio.sfx    << "\n";
    f << "\n";

    f << "[display]\n";
    f << "subtitles     = " << (subtitlesEnabled ? "true" : "false") << "\n";
    f << "window_width  = " << windowWidth  << "\n";
    f << "window_height = " << windowHeight << "\n";
    f << "\n";

    f << "[graphics]\n";
    f << "; render60fps: enable 60fps rendering (3D models benefit from smoother animation)\n";
    f << ";   Game logic remains at 30Hz (PSX-accurate). Only rendering is doubled.\n";
    f << "render60fps = " << (render60fps ? "true" : "false") << "\n";
    f << "; stage1_parappa_rail_assist: enable the Stage1 Parappa rail visual assist marker.\n";
    f << "stage1_parappa_rail_assist = "
      << (stage1ParappaRailAssist ? "true" : "false") << "\n";
    f << "; stage1_rail_mode: psx keeps the original compact rail; parappa2 enables the optional student-turn rail.\n";
    f << "stage1_rail_mode = "
      << (stage1RailMode == 1 ? "parappa2" : "psx") << "\n";
    f << "; stage1_restore_ceiling_lights: optional Stage1 DENKI ceiling-light restoration.\n";
    f << ";   Off preserves the loaded ROM's PSX behavior; on reuses TENNJOU.TMD + DENKI.TIM.\n";
    f << "stage1_restore_ceiling_lights = "
      << (stage1RestoreCeilingLights ? "true" : "false") << "\n";
    f << "; stage1_hd_geometry_cleanup: optional HD cleanup for PSX scene-map strip artifacts.\n";
    f << ";   Off preserves PSX high-resolution artifacts; on stabilizes thin scene-map strips.\n";
    f << "stage1_hd_geometry_cleanup = "
      << (stage1HdGeometryCleanup ? "true" : "false") << "\n";
    f << "; stage1_texture_replacements: optional PS4/PSPHD texture replacement layer for Stage1.\n";
    f << ";   Off keeps the direct PSX TIM/VRAM path; on reads files from stage1_texture_replacement_dir.\n";
    f << "stage1_texture_replacements = "
      << (stage1TextureReplacements ? "true" : "false") << "\n";
    f << "; Runtime learning is disabled; regenerate stage1_psx_texture_key_map.tsv offline.\n";
    f << "stage1_texture_replacement_dir = "
      << stage1TextureReplacementDir << "\n";
    f << "\n";

    f << "[stage1_rail_parappa2]\n";
    f << "; Tuning for graphics.stage1_rail_mode = parappa2.\n";
    f << "darken = " << stage1RailParappa2Darken << "\n";
    f << "; core_align: drive PaRappa2 rail portraits/stamps from the Stage1 scorer timing cursor.\n";
    f << "core_align = "
      << (stage1RailParappa2CoreAlign ? "true" : "false") << "\n";
    f << "pop_frames = " << stage1RailParappa2PopFrames << "\n";
    f << "pop_scale = " << stage1RailParappa2PopScale << "\n";
    f << "flip_frames = " << stage1RailParappa2FlipFrames << "\n";
    f << "glow_fade_frames = " << stage1RailParappa2GlowFadeFrames << "\n";
    f << "glow_alpha = " << stage1RailParappa2GlowAlpha << "\n";
    f << "glow_scale = " << stage1RailParappa2GlowScale << "\n";
    f << "; lead_slots: optional extra fine-tune in 15px PSX rail units; positive values move left.\n";
    f << "lead_slots = " << stage1RailParappa2LeadSlots << "\n";
    f << "; trace_align: write one log row per PaRappa2 rail stamp for visual-vs-core judgement calibration.\n";
    f << "trace_align = "
      << (stage1RailParappa2TraceAlign ? "true" : "false") << "\n";
    f << "; scorer_hud: show the optional PaRappa2 rail per-bar scorer breakdown above SCORE.\n";
    f << "scorer_hud = "
      << (stage1RailParappa2ScorerHud ? "true" : "false") << "\n";
    f << "; creative_prompt: show a small PaRappa2 rail creativity hint below the rail.\n";
    f << "creative_prompt = "
      << (stage1RailParappa2CreativePrompt ? "true" : "false") << "\n";
    f << "; creative_prompt_language: EN uses the original-language asset, CN uses the Chinese asset.\n";
    f << "creative_prompt_language = "
      << stage1RailParappa2CreativePromptLanguage << "\n";
    f << "\n";

    f << "[stage1_hd_subtitles]\n";
    f << "; External Stage1 HD subtitle overlay. It reads only the current original subtitle event/time.\n";
    f << "; Remove the file or set enabled=false to detach it without touching the direct PSX text path.\n";
    f << "enabled = " << (stage1HdSubtitles ? "true" : "false") << "\n";
    f << "; language: CN by default; auto follows the original 5-language in-game index (EN/FR/DE/ES/IT).\n";
    f << "language = " << stage1HdSubtitleLanguage << "\n";
    f << "; UTF-8 TSV relative to the data root or win directory.\n";
    f << "file = " << stage1HdSubtitleFile << "\n";
    f << "font = " << stage1HdSubtitleFont << "\n";
    f << "font_size_psx = " << stage1HdSubtitleFontSizePsx << "\n";
    f << "y_psx = " << stage1HdSubtitleY << "\n";
    f << "; Source-specific vertical placement in PSX pixels.\n";
    f << "movie_y_psx = " << stage1HdSubtitleMovieY << "\n";
    f << "gameplay_y_psx = " << stage1HdSubtitleGameplayY << "\n";
    f << "width_psx = " << stage1HdSubtitleWidth << "\n";
    f << "; draw_box=false keeps the original game subtitle frame and only replaces text.\n";
    f << "draw_box = " << (stage1HdSubtitleDrawBox ? "true" : "false") << "\n";
    f << "; Colors accept #RRGGBB or #AARRGGBB.\n";
    f << "fill_color = " << stage1HdSubtitleFillColor << "\n";
    f << "outline_color = " << stage1HdSubtitleOutlineColor << "\n";
    f << "shadow_color = " << stage1HdSubtitleShadowColor << "\n";
    f << "outline_psx = " << stage1HdSubtitleOutlinePsx << "\n";
    f << "shadow_offset_x_psx = " << stage1HdSubtitleShadowOffsetXPsx << "\n";
    f << "shadow_offset_y_psx = " << stage1HdSubtitleShadowOffsetYPsx << "\n";
    f << "\n";

    f << "[debug]\n";
    f << "; stage1_texture_replacement_trace: verbose per-lookup log for rebuilding the offline texture key map.\n";
    f << "stage1_texture_replacement_trace = "
      << (debugStage1TextureReplacementTrace ? "true" : "false") << "\n";
    f << "; stage1_show_psx_frame: show the Stage1 PSX logic frame in the top-left during gameplay.\n";
    f << ";   This is the 30Hz StageRunner frame, independent of render60fps.\n";
    f << "stage1_show_psx_frame = "
      << (debugStage1ShowPsxFrame ? "true" : "false") << "\n";

    return f.good();
}

// ========== LoadOrCreate ==========

std::filesystem::path AppConfig::GetConfigPath(const std::filesystem::path& exeDir) {
    return exeDir / "config.ini";
}

AppConfig AppConfig::LoadOrCreate(const std::filesystem::path& exeDir) {
    AppConfig cfg;
    std::filesystem::path path = GetConfigPath(exeDir);
    const bool primaryLoaded = cfg.Load(path);

    const std::filesystem::path overridePaths[] = {
        exeDir / "win" / "bin" / "config.ini",
        exeDir / "bin" / "config.ini",
    };
    for (const auto& overridePath : overridePaths) {
        if (!std::filesystem::exists(overridePath)) {
            continue;
        }
        auto ini = ParseIni(overridePath);
        if (ini.empty()) {
            continue;
        }
        auto readKey = [&](const std::string& key, int defaultVal) -> int {
            std::string val = GetVal(ini, "keys", key, "");
            if (val.empty()) return defaultVal;
            int vk = ParseVKey(val);
            return (vk >= 0) ? vk : defaultVal;
        };

        cfg.keys.up         = readKey("up", cfg.keys.up);
        cfg.keys.down       = readKey("down", cfg.keys.down);
        cfg.keys.left       = readKey("left", cfg.keys.left);
        cfg.keys.right      = readKey("right", cfg.keys.right);
        cfg.keys.up_alt     = readKey("up_alt", cfg.keys.up_alt);
        cfg.keys.down_alt   = readKey("down_alt", cfg.keys.down_alt);
        cfg.keys.left_alt   = readKey("left_alt", cfg.keys.left_alt);
        cfg.keys.right_alt  = readKey("right_alt", cfg.keys.right_alt);
        cfg.keys.triangle   = readKey("triangle", cfg.keys.triangle);
        cfg.keys.circle     = readKey("circle", cfg.keys.circle);
        cfg.keys.cross      = readKey("cross", cfg.keys.cross);
        cfg.keys.cross_alt  = readKey("cross_alt", cfg.keys.cross_alt);
        cfg.keys.cross_alt2 = readKey("cross_alt2", cfg.keys.cross_alt2);
        cfg.keys.square     = readKey("square", cfg.keys.square);
        cfg.keys.l1         = readKey("l1", readKey("l", cfg.keys.l1));
        cfg.keys.r1         = readKey("r1", readKey("r", cfg.keys.r1));
        cfg.keys.l2         = readKey("l2", cfg.keys.l2);
        cfg.keys.r2         = readKey("r2", cfg.keys.r2);
        cfg.keys.start      = readKey("start", cfg.keys.start);
        cfg.keys.select     = readKey("select", cfg.keys.select);
        cfg.keys.str_skip   = readKey("str_skip", cfg.keys.str_skip);
        break;
    }
    if (!primaryLoaded) {
        // File doesn't exist or failed to parse -> save the merged defaults.
        cfg.Save(path);
    }
    return cfg;
}
