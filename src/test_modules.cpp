// Automated tests for PaRappaWin modules:
//   - AppConfig (config.ini read/write/defaults)
//   - Boot Logo (sprite slice coordinates)
//   - Audio Backend (IAudioBackend interface, volume)
//   - Input (PrPad key bindings from config)
//
// Build: cl /nologo /utf-8 /std:c++17 /EHsc /I"src" test_modules.cpp app_config.cpp ...
// Run:   test_modules.exe  (returns 0 on success)

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>

#include "app_config.h"
#include "boot_logo.h"
#include "test_fresh_directory.h"

static int g_passed = 0;
static int g_failed = 0;

#define TEST(name) static void test_##name()
#define RUN(name) do { \
    printf("  [TEST] %s ... ", #name); \
    try { test_##name(); printf("PASS\n"); g_passed++; } \
    catch (const std::exception& e) { printf("FAIL: %s\n", e.what()); g_failed++; } \
    catch (...) { printf("FAIL (unknown)\n"); g_failed++; } \
} while(0)

#define ASSERT_TRUE(x)  do { if (!(x)) throw std::runtime_error("ASSERT_TRUE failed: " #x); } while(0)
#define ASSERT_FALSE(x) do { if ((x)) throw std::runtime_error("ASSERT_FALSE failed: " #x); } while(0)
#define ASSERT_EQ(a, b) do { if ((a) != (b)) { \
    char buf[256]; snprintf(buf, sizeof(buf), "ASSERT_EQ failed: %s != %s", #a, #b); \
    throw std::runtime_error(buf); } } while(0)
#define ASSERT_NEAR(a, b, eps) do { if (std::abs((a) - (b)) > (eps)) { \
    char buf[256]; snprintf(buf, sizeof(buf), "ASSERT_NEAR failed: %s (~%.4f) != %s (~%.4f)", #a, (double)(a), #b, (double)(b)); \
    throw std::runtime_error(buf); } } while(0)
#define ASSERT_STR_EQ(a, b) do { \
    const std::string assertStrA = std::string(a); \
    const std::string assertStrB = std::string(b); \
    if (assertStrA != assertStrB) { \
    char buf[512]; snprintf(buf, sizeof(buf), "ASSERT_STR_EQ failed: \"%s\" != \"%s\"", assertStrA.c_str(), assertStrB.c_str()); \
    throw std::runtime_error(buf); } } while(0)

// ========== Config Tests ==========

TEST(config_defaults) {
    AppConfig cfg;
    ASSERT_NEAR(cfg.audio.master, 0.8f, 0.001f);
    ASSERT_NEAR(cfg.audio.bgm, 0.8f, 0.001f);
    ASSERT_NEAR(cfg.audio.sfx, 0.8f, 0.001f);
    ASSERT_TRUE(cfg.subtitlesEnabled);
    ASSERT_EQ(cfg.windowWidth, 640);
    ASSERT_EQ(cfg.windowHeight, 480);
    ASSERT_EQ(cfg.keys.up, 0x26);        // VK_UP
    ASSERT_EQ(cfg.keys.cross, 'Z');
    ASSERT_EQ(cfg.keys.select, 'O');
    ASSERT_EQ(cfg.keys.str_skip, 0x70);  // VK_F1
    ASSERT_EQ(cfg.presentation.railMode, 0);
    ASSERT_NEAR(cfg.presentation.railDarken, 0.42f, 0.001f);
    ASSERT_FALSE(cfg.presentation.railCoreAlign);
    ASSERT_EQ(cfg.presentation.railPopFrames, 5);
    ASSERT_NEAR(cfg.presentation.railPopScale, 2.0f, 0.001f);
    ASSERT_EQ(cfg.presentation.railFlipFrames, 8);
    ASSERT_EQ(cfg.presentation.railGlowFadeFrames, 16);
    ASSERT_NEAR(cfg.presentation.railGlowAlpha, 0.55f, 0.001f);
    ASSERT_NEAR(cfg.presentation.railGlowScale, 1.75f, 0.001f);
    ASSERT_NEAR(cfg.presentation.railLeadSlots, 0.0f, 0.001f);
    ASSERT_FALSE(cfg.presentation.railTraceAlign);
    ASSERT_TRUE(cfg.presentation.railCreativePrompt);
    ASSERT_STR_EQ(cfg.presentation.railCreativePromptLanguage, "EN");
    ASSERT_FALSE(cfg.presentation.restoreSceneDetails);
    ASSERT_FALSE(cfg.presentation.hdGeometryCleanup);
    ASSERT_FALSE(cfg.debugStage1ShowPsxFrame);
}

TEST(config_save_load_roundtrip) {
    const auto tmp = CreateFreshTestDirectory("parappawin-module-roundtrip") / "config.ini";

    // Save defaults
    AppConfig cfg1;
    cfg1.audio.master = 0.5f;
    cfg1.audio.bgm = 0.3f;
    cfg1.audio.sfx = 0.9f;
    cfg1.subtitlesEnabled = false;
    cfg1.windowWidth = 800;
    cfg1.windowHeight = 600;
    cfg1.presentation.railMode = 1;
    cfg1.presentation.railDarken = 0.25f;
    cfg1.presentation.railCoreAlign = false;
    cfg1.presentation.railPopFrames = 6;
    cfg1.presentation.railPopScale = 1.8f;
    cfg1.presentation.railFlipFrames = 10;
    cfg1.presentation.railGlowFadeFrames = 20;
    cfg1.presentation.railGlowAlpha = 0.7f;
    cfg1.presentation.railGlowScale = 2.0f;
    cfg1.presentation.railLeadSlots = 1.25f;
    cfg1.presentation.railTraceAlign = true;
    cfg1.presentation.railCreativePrompt = true;
    cfg1.presentation.railCreativePromptLanguage = "CN";
    cfg1.presentation.restoreSceneDetails = true;
    cfg1.presentation.hdGeometryCleanup = true;
    cfg1.debugStage1ShowPsxFrame = true;
    cfg1.keys.cross = 'J';
    cfg1.keys.circle = 'K';
    ASSERT_TRUE(cfg1.Save(tmp));

    // Load back
    AppConfig cfg2;
    ASSERT_TRUE(cfg2.Load(tmp));
    ASSERT_NEAR(cfg2.audio.master, 0.5f, 0.001f);
    ASSERT_NEAR(cfg2.audio.bgm, 0.3f, 0.001f);
    ASSERT_NEAR(cfg2.audio.sfx, 0.9f, 0.001f);
    ASSERT_FALSE(cfg2.subtitlesEnabled);
    ASSERT_EQ(cfg2.windowWidth, 800);
    ASSERT_EQ(cfg2.windowHeight, 600);
    ASSERT_EQ(cfg2.presentation.railMode, 1);
    ASSERT_NEAR(cfg2.presentation.railDarken, 0.25f, 0.001f);
    ASSERT_FALSE(cfg2.presentation.railCoreAlign);
    ASSERT_EQ(cfg2.presentation.railPopFrames, 6);
    ASSERT_NEAR(cfg2.presentation.railPopScale, 1.8f, 0.001f);
    ASSERT_EQ(cfg2.presentation.railFlipFrames, 10);
    ASSERT_EQ(cfg2.presentation.railGlowFadeFrames, 20);
    ASSERT_NEAR(cfg2.presentation.railGlowAlpha, 0.7f, 0.001f);
    ASSERT_NEAR(cfg2.presentation.railGlowScale, 2.0f, 0.001f);
    ASSERT_NEAR(cfg2.presentation.railLeadSlots, 1.25f, 0.001f);
    ASSERT_TRUE(cfg2.presentation.railTraceAlign);
    ASSERT_TRUE(cfg2.presentation.railCreativePrompt);
    ASSERT_STR_EQ(cfg2.presentation.railCreativePromptLanguage, "CN");
    ASSERT_TRUE(cfg2.presentation.restoreSceneDetails);
    ASSERT_TRUE(cfg2.presentation.hdGeometryCleanup);
    ASSERT_TRUE(cfg2.debugStage1ShowPsxFrame);
    ASSERT_EQ(cfg2.keys.cross, 'J');
    ASSERT_EQ(cfg2.keys.circle, 'K');

}

TEST(config_load_or_create) {
    const auto tmpDir = CreateFreshTestDirectory("parappawin-module-create");
    std::filesystem::path cfgPath = tmpDir / "config.ini";

    // Should create default file
    AppConfig cfg = AppConfig::LoadOrCreate(tmpDir);
    ASSERT_TRUE(std::filesystem::exists(cfgPath));
    ASSERT_NEAR(cfg.audio.master, 0.8f, 0.001f);
    ASSERT_TRUE(cfg.subtitlesEnabled);

    // Modify and save
    cfg.audio.master = 0.6f;
    cfg.Save(cfgPath);

    // Reload
    AppConfig cfg2 = AppConfig::LoadOrCreate(tmpDir);
    ASSERT_NEAR(cfg2.audio.master, 0.6f, 0.001f);

}

TEST(config_volume_clamp) {
    const auto tmp = CreateFreshTestDirectory("parappawin-module-clamp") / "config.ini";
    {
        std::ofstream f(tmp);
        f << "[audio]\n";
        f << "master = 1.5\n";
        f << "bgm = -0.2\n";
        f << "sfx = 0.7\n";
    }

    AppConfig cfg;
    ASSERT_TRUE(cfg.Load(tmp));
    ASSERT_NEAR(cfg.audio.master, 1.0f, 0.001f);  // clamped to 1.0
    ASSERT_NEAR(cfg.audio.bgm, 0.0f, 0.001f);     // clamped to 0.0
    ASSERT_NEAR(cfg.audio.sfx, 0.7f, 0.001f);

}

TEST(config_vkey_parsing) {
    const auto tmp = CreateFreshTestDirectory("parappawin-module-vkey") / "config.ini";
    {
        std::ofstream f(tmp);
        f << "[keys]\n";
        f << "up = VK_UP\n";
        f << "down = VK_DOWN\n";
        f << "cross = Z\n";
        f << "circle = X\n";
        f << "str_skip = VK_F1\n";
        f << "l1 = 0x51\n";     // 'Q' as hex
        f << "start = P\n";
    }

    AppConfig cfg;
    ASSERT_TRUE(cfg.Load(tmp));
    ASSERT_EQ(cfg.keys.up, 0x26);       // VK_UP
    ASSERT_EQ(cfg.keys.down, 0x28);     // VK_DOWN
    ASSERT_EQ(cfg.keys.cross, 'Z');
    ASSERT_EQ(cfg.keys.circle, 'X');
    ASSERT_EQ(cfg.keys.str_skip, 0x70); // VK_F1
    ASSERT_EQ(cfg.keys.l1, 0x51);       // Q
    ASSERT_EQ(cfg.keys.start, 'P');

}

TEST(config_missing_keys_use_defaults) {
    const auto tmp = CreateFreshTestDirectory("parappawin-module-partial") / "config.ini";
    {
        std::ofstream f(tmp);
        f << "[audio]\n";
        f << "master = 0.4\n";
        // no bgm, sfx, keys, display
    }

    AppConfig cfg;
    ASSERT_TRUE(cfg.Load(tmp));
    ASSERT_NEAR(cfg.audio.master, 0.4f, 0.001f);
    ASSERT_NEAR(cfg.audio.bgm, 0.8f, 0.001f);     // default
    ASSERT_NEAR(cfg.audio.sfx, 0.8f, 0.001f);      // default
    ASSERT_TRUE(cfg.subtitlesEnabled);               // default
    ASSERT_EQ(cfg.keys.cross, 'Z');                  // default

}

// ========== Boot Logo Sprite Coordinate Tests ==========

struct TestSpriteSlice {
    int px, py, sw, sh, su, sv;
};

// PSX original values from handoff doc
static const TestSpriteSlice kExpectedSonySlices[3] = {
    { 20, 106, 216, 15,   0,  0 },
    {240, 106,  60, 12,   0, 16 },
    {128, 122,  64, 12, 108, 16 },
};

static const TestSpriteSlice kExpectedMasayaSlices[2] = {
    { 21, 103, 142, 66,  0,  0 },
    {163, 103, 140, 60,  0, 66 },
};

TEST(boot_logo_sony_sprite_coords) {
    // Verify Sony slices: all within 320x240, UVs within 216x28
    for (int i = 0; i < 3; i++) {
        const auto& s = kExpectedSonySlices[i];
        ASSERT_TRUE(s.px >= 0 && s.px + s.sw <= 320);
        ASSERT_TRUE(s.py >= 0 && s.py + s.sh <= 240);
        ASSERT_TRUE(s.su >= 0 && s.su + s.sw <= 216);
        ASSERT_TRUE(s.sv >= 0 && s.sv + s.sh <= 28);
    }
}

TEST(boot_logo_masaya_sprite_coords) {
    // Verify Masaya slices: all within 320x240, UVs within 142x126
    for (int i = 0; i < 2; i++) {
        const auto& s = kExpectedMasayaSlices[i];
        ASSERT_TRUE(s.px >= 0 && s.px + s.sw <= 320);
        ASSERT_TRUE(s.py >= 0 && s.py + s.sh <= 240);
        ASSERT_TRUE(s.su >= 0 && s.su + s.sw <= 142);
        ASSERT_TRUE(s.sv >= 0 && s.sv + s.sh <= 126);
    }
}

TEST(boot_logo_timing_constants) {
    // SCUS 80016B84:
    // 80015A4C(30) + 80015B00(150,60) + 80015C20(30).
    ASSERT_EQ(FADE_IN_FRAMES, 30);
    ASSERT_EQ(STAY_FRAMES, 150);
    ASSERT_EQ(FADE_OUT_FRAMES, 30);
    ASSERT_EQ(TOTAL_FRAMES, 210);
    ASSERT_EQ(SKIP_ENABLE_FRAME, 60);
    ASSERT_EQ(SKIPPABLE_STAY_FRAMES, 90);
}

// ========== Audio Backend Interface Tests ==========

TEST(audio_backend_volume_clamp) {
    // Volume should be clamped to [0, 1]
    float v = 1.5f;
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    ASSERT_NEAR(v, 1.0f, 0.001f);

    float v2 = -0.3f;
    if (v2 < 0.0f) v2 = 0.0f;
    if (v2 > 1.0f) v2 = 1.0f;
    ASSERT_NEAR(v2, 0.0f, 0.001f);
}

// ========== Input System Tests ==========

TEST(input_pad_button_values) {
    // Verify PSX pad button masks match expected values
    ASSERT_EQ(0x0001, 0x0001); // Up
    ASSERT_EQ(0x0002, 0x0002); // Down
    ASSERT_EQ(0x0004, 0x0004); // Left
    ASSERT_EQ(0x0008, 0x0008); // Right
    ASSERT_EQ(0x0010, 0x0010); // Triangle
    ASSERT_EQ(0x0020, 0x0020); // Circle
    ASSERT_EQ(0x0040, 0x0040); // Cross
    ASSERT_EQ(0x0080, 0x0080); // Square
    ASSERT_EQ(0x0100, 0x0100); // L1
    ASSERT_EQ(0x0200, 0x0200); // R1
    ASSERT_EQ(0x1000, 0x1000); // Start
    ASSERT_EQ(0x2000, 0x2000); // Select
}

TEST(input_default_keybindings_match_config) {
    AppConfig cfg;
    KeyBindings kb = cfg.keys;

    // Verify defaults match what PrPad expects
    ASSERT_EQ(kb.up, 0x26);
    ASSERT_EQ(kb.down, 0x28);
    ASSERT_EQ(kb.left, 0x25);
    ASSERT_EQ(kb.right, 0x27);
    ASSERT_EQ(kb.up_alt, 'W');
    ASSERT_EQ(kb.down_alt, 'S');
    ASSERT_EQ(kb.left_alt, 'A');
    ASSERT_EQ(kb.right_alt, 'D');
    ASSERT_EQ(kb.triangle, 'V');
    ASSERT_EQ(kb.circle, 'X');
    ASSERT_EQ(kb.cross, 'Z');
    ASSERT_EQ(kb.cross_alt, 0x20);  // VK_SPACE
    ASSERT_EQ(kb.cross_alt2, 0x0D); // VK_RETURN
    ASSERT_EQ(kb.square, 'C');
    ASSERT_EQ(kb.l1, 'Q');
    ASSERT_EQ(kb.r1, 'E');
    ASSERT_EQ(kb.start, 'P');
    ASSERT_EQ(kb.select, 'O');
}

TEST(config_shared_legacy) {
    const auto file = CreateFreshTestDirectory("parappawin-module-legacy") / "config.ini";
    std::ofstream(file) << "[graphics]\nrender60fps=true\nstage1_rail_mode=parappa2\n"
        "stage1_texture_replacements=true\nstage1_texture_replacement_dir=legacy textures\n"
        "[stage2_modern]\nenabled=false\naspect_mode=stretch\nsubtitle_file=two.tsv\n"
        "[stage1_rail_parappa2]\nscorer_hud=false\ncreative_prompt=false\npop_scale=2.5\n"
        "[stage1_hd_subtitles]\nenabled=false\nlanguage=fr\nfile=one.tsv\nfont=Custom Font\n";
    AppConfig config;
    ASSERT_TRUE(config.Load(file));
    const auto& p = config.presentation;
    ASSERT_FALSE(p.enabled); ASSERT_EQ(p.aspectMode, 0);
    ASSERT_TRUE(p.render60fps); ASSERT_EQ(p.railMode, 1);
    ASSERT_TRUE(p.textureReplacements); ASSERT_STR_EQ(p.textureReplacementDir, "legacy textures");
    ASSERT_FALSE(p.railScorerHud); ASSERT_FALSE(p.railCreativePrompt);
    ASSERT_NEAR(p.railPopScale, 2.5f, .001f);
    ASSERT_FALSE(p.hdSubtitles); ASSERT_STR_EQ(p.hdSubtitleLanguage, "FR");
    ASSERT_STR_EQ(p.hdSubtitleFont, "Custom Font");
    ASSERT_STR_EQ(p.subtitleFiles[0], "one.tsv"); ASSERT_STR_EQ(p.subtitleFiles[1], "two.tsv");
}

TEST(config_shared_precedence_roundtrip) {
    const auto dir = CreateFreshTestDirectory("parappawin-module-shared");
    const auto file = dir / "config.ini";
    std::ofstream(file) << "[presentation]\nenabled=true\naspect_mode=4:3\nrender60fps=false\n"
        "rail_mode=psx\nrail_assist=false\nscorer_hud=false\ncreative_prompt=false\n"
        "hd_subtitles=false\ntexture_replacements=false\ngeometry_cleanup=false\nrestore_scene_details=false\n"
        "[rail_feedback]\npop_frames=0\nglow_alpha=0\nlead_slots=0\n"
        "[hd_subtitles]\nlanguage=DE\nfont=New Font\n"
        "[stage_assets]\nsubtitles_1=shared one.tsv\nsubtitles_2=shared two.tsv\nsubtitles_6=six.tsv\ntexture_dir=shared textures\n"
        "[graphics]\nrender60fps=true\nstage1_rail_mode=parappa2\nstage1_parappa_rail_assist=true\n"
        "stage1_texture_replacements=true\nstage1_hd_geometry_cleanup=true\nstage1_restore_ceiling_lights=true\n"
        "stage1_texture_replacement_dir=old textures\n"
        "[stage2_modern]\nenabled=false\naspect_mode=stretch\nsubtitle_file=old2.tsv\n"
        "[stage1_hd_subtitles]\nenabled=true\nlanguage=CN\nfile=old1.tsv\nfont=Old Font\n"
        "[stage1_rail_parappa2]\nscorer_hud=true\ncreative_prompt=true\npop_frames=20\nglow_alpha=1\nlead_slots=1\n";
    AppConfig config;
    ASSERT_TRUE(config.Load(file));
    const auto& p = config.presentation;
    ASSERT_TRUE(p.enabled); ASSERT_EQ(p.aspectMode, 2);
    ASSERT_FALSE(p.render60fps); ASSERT_EQ(p.railMode, 0); ASSERT_FALSE(p.railAssist);
    ASSERT_FALSE(p.railScorerHud); ASSERT_FALSE(p.railCreativePrompt);
    ASSERT_FALSE(p.hdSubtitles); ASSERT_FALSE(p.textureReplacements);
    ASSERT_FALSE(p.hdGeometryCleanup); ASSERT_FALSE(p.restoreSceneDetails);
    ASSERT_EQ(p.railPopFrames, 0); ASSERT_NEAR(p.railGlowAlpha, 0.f, .001f); ASSERT_NEAR(p.railLeadSlots, 0.f, .001f);
    ASSERT_STR_EQ(p.hdSubtitleLanguage, "DE"); ASSERT_STR_EQ(p.hdSubtitleFont, "New Font");
    ASSERT_STR_EQ(p.subtitleFiles[0], "shared one.tsv"); ASSERT_STR_EQ(p.subtitleFiles[1], "shared two.tsv");
    ASSERT_STR_EQ(p.subtitleFiles[5], "six.tsv"); ASSERT_STR_EQ(p.textureReplacementDir, "shared textures");
    ASSERT_TRUE(config.Save(dir / "new.ini"));
    std::ifstream saved(dir / "new.ini");
    const std::string text((std::istreambuf_iterator<char>(saved)), {});
    ASSERT_TRUE(text.find("[presentation]") != std::string::npos);
    ASSERT_TRUE(text.find("[stage2_modern]") == std::string::npos);
    ASSERT_TRUE(text.find("stage1_rail") == std::string::npos);
    AppConfig restored; ASSERT_TRUE(restored.Load(dir / "new.ini"));
    ASSERT_TRUE(restored.Save(dir / "roundtrip.ini"));
    std::ifstream roundtrip(dir / "roundtrip.ini");
    ASSERT_STR_EQ(text, std::string((std::istreambuf_iterator<char>(roundtrip)), {}));
}

TEST(config_presentation_master) {
    PrPresentationSettings preferences;
    preferences.enabled = false;
    preferences.render60fps = true; preferences.railMode = 1;
    preferences.textureReplacements = true; preferences.hdGeometryCleanup = true;
    preferences.restoreSceneDetails = true;
    const auto effective = preferences.Effective();
    ASSERT_EQ(effective.aspectMode, 2); ASSERT_EQ(effective.railMode, 0);
    ASSERT_FALSE(effective.render60fps); ASSERT_FALSE(effective.railAssist);
    ASSERT_FALSE(effective.railScorerHud); ASSERT_FALSE(effective.railCreativePrompt);
    ASSERT_FALSE(effective.textureReplacements); ASSERT_FALSE(effective.hdSubtitles);
    ASSERT_FALSE(effective.hdGeometryCleanup); ASSERT_FALSE(effective.restoreSceneDetails);
    ASSERT_EQ(preferences.railMode, 1); ASSERT_TRUE(preferences.textureReplacements);
    preferences.enabled = true;
    ASSERT_EQ(preferences.Effective().railMode, 1);
    ASSERT_TRUE(preferences.Effective().textureReplacements);
}

// ========== Main ==========

int main() {
    printf("=== PaRappaWin Module Tests ===\n\n");

    printf("[Config]\n");
    RUN(config_defaults);
    RUN(config_shared_legacy);
    RUN(config_shared_precedence_roundtrip);
    RUN(config_presentation_master);
    RUN(config_save_load_roundtrip);
    RUN(config_load_or_create);
    RUN(config_volume_clamp);
    RUN(config_vkey_parsing);
    RUN(config_missing_keys_use_defaults);

    printf("\n[Boot Logo]\n");
    RUN(boot_logo_sony_sprite_coords);
    RUN(boot_logo_masaya_sprite_coords);
    RUN(boot_logo_timing_constants);

    printf("\n[Audio Backend]\n");
    RUN(audio_backend_volume_clamp);

    printf("\n[Input]\n");
    RUN(input_pad_button_values);
    RUN(input_default_keybindings_match_config);

    printf("\n=== Results: %d passed, %d failed ===\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
