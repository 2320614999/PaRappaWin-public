#include "pr_main.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "logger.h"
#include "pr_event.h"
#include "pr_game_context.h"
#include "pr_overlay_loader.h"
#include "pr_scene_entry_card_feedback_direct.h"
#include "pr_scene_entry_direct.h"
#include "pr_scene_entry_executor_direct.h"
#include "pr_scene1_entry_original_disc_direct.h"
#include "pr_pad.h"
#include "pr_psx_pad_direct.h"
#include "pr_psx_vsync_direct.h"
#include "pr_ss0_scene0_runtime_direct.h"
#include "pr_ss0_title_tmd_backend.h"
#include "pr_stage1_loader_memory_direct.h"
#include "pr_stage1_save_card_hal_direct.h"
#include "pr_stage1_save_ui_direct.h"
#include "pr_transition.h"
#include "pr_scn1.h"
#include "pr_scn2.h"
#include "pr_scn3.h"
#include "pr_scn5.h"
#include "pr_scn6.h"
#include "pr_scn7.h"
#include "pr_scn8.h"
#include "pr_scn9.h"

static int s_stage1Fn2ResultThisFrameDebug = -1;
static int s_stage1PendingSceneThisFrameDebug = -1;

struct SceneJson {
    int index = -1;
    std::vector<std::string> loaderPaths;
};

static void SkipWs(const char*& p, const char* end) {
    while (p < end) {
        const char c = *p;
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            p++;
            continue;
        }
        break;
    }
}

static bool ConsumeChar(const char*& p, const char* end, char c) {
    SkipWs(p, end);
    if (p >= end || *p != c) {
        return false;
    }
    p++;
    return true;
}

static bool ConsumeLiteral(const char*& p, const char* end, const char* lit) {
    SkipWs(p, end);
    const char* s = lit;
    const char* it = p;
    while (*s) {
        if (it >= end || *it != *s) {
            return false;
        }
        it++;
        s++;
    }
    p = it;
    return true;
}

static int HexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

static bool ParseString(const char*& p, const char* end, std::string& out) {
    SkipWs(p, end);
    if (p >= end || *p != '"') {
        return false;
    }
    p++;
    out.clear();
    while (p < end) {
        const char c = *p++;
        if (c == '"') {
            return true;
        }
        if (c == '\\') {
            if (p >= end) {
                return false;
            }
            const char esc = *p++;
            switch (esc) {
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                case '/': out.push_back('/'); break;
                case 'b': out.push_back('\b'); break;
                case 'f': out.push_back('\f'); break;
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                case 'u': {
                    if (end - p < 4) {
                        return false;
                    }
                    int v0 = HexValue(p[0]);
                    int v1 = HexValue(p[1]);
                    int v2 = HexValue(p[2]);
                    int v3 = HexValue(p[3]);
                    if (v0 < 0 || v1 < 0 || v2 < 0 || v3 < 0) {
                        return false;
                    }
                    const int code = (v0 << 12) | (v1 << 8) | (v2 << 4) | v3;
                    if (code >= 0 && code <= 0x7F) {
                        out.push_back((char)code);
                    } else {
                        out.push_back('?');
                    }
                    p += 4;
                    break;
                }
                default:
                    return false;
            }
        } else {
            out.push_back(c);
        }
    }
    return false;
}

static bool ParseInt(const char*& p, const char* end, int& out) {
    SkipWs(p, end);
    if (p >= end) {
        return false;
    }
    bool neg = false;
    if (*p == '-') {
        neg = true;
        p++;
    }
    if (p >= end || *p < '0' || *p > '9') {
        return false;
    }
    int value = 0;
    while (p < end && *p >= '0' && *p <= '9') {
        value = value * 10 + (*p - '0');
        p++;
    }
    out = neg ? -value : value;
    return true;
}

static bool SkipValue(const char*& p, const char* end);

static bool SkipArray(const char*& p, const char* end) {
    if (!ConsumeChar(p, end, '[')) {
        return false;
    }
    SkipWs(p, end);
    if (ConsumeChar(p, end, ']')) {
        return true;
    }
    while (p < end) {
        if (!SkipValue(p, end)) {
            return false;
        }
        SkipWs(p, end);
        if (ConsumeChar(p, end, ',')) {
            continue;
        }
        if (ConsumeChar(p, end, ']')) {
            return true;
        }
        return false;
    }
    return false;
}

static bool SkipObject(const char*& p, const char* end) {
    if (!ConsumeChar(p, end, '{')) {
        return false;
    }
    SkipWs(p, end);
    if (ConsumeChar(p, end, '}')) {
        return true;
    }
    while (p < end) {
        std::string key;
        if (!ParseString(p, end, key)) {
            return false;
        }
        if (!ConsumeChar(p, end, ':')) {
            return false;
        }

        if (!SkipValue(p, end)) {
            return false;
        }

        SkipWs(p, end);
        if (ConsumeChar(p, end, ',')) {
            continue;
        }
        if (ConsumeChar(p, end, '}')) {
            return true;
        }
        return false;
    }
    return false;
}

static bool SkipValue(const char*& p, const char* end) {
    SkipWs(p, end);
    if (p >= end) {
        return false;
    }
    const char c = *p;
    if (c == '"') {
        std::string tmp;
        return ParseString(p, end, tmp);
    }
    if (c == '{') {
        return SkipObject(p, end);
    }
    if (c == '[') {
        return SkipArray(p, end);
    }
    if (c == 'n') {
        return ConsumeLiteral(p, end, "null");
    }
    if (c == 't') {
        return ConsumeLiteral(p, end, "true");
    }
    if (c == 'f') {
        return ConsumeLiteral(p, end, "false");
    }
    if (c == '-' || (c >= '0' && c <= '9')) {
        int tmp = 0;
        return ParseInt(p, end, tmp);
    }
    return false;
}

static bool ParseStringOrNullArray(const char*& p, const char* end, std::vector<std::string>& out) {
    out.clear();
    if (!ConsumeChar(p, end, '[')) {
        return false;
    }
    SkipWs(p, end);
    if (ConsumeChar(p, end, ']')) {
        return true;
    }
    while (p < end) {
        SkipWs(p, end);
        std::string val;
        if (p < end && *p == '"') {
            if (!ParseString(p, end, val)) {
                return false;
            }
            out.push_back(std::move(val));
        } else if (ConsumeLiteral(p, end, "null")) {
            out.push_back(std::string{});
        } else {
            return false;
        }

        SkipWs(p, end);
        if (ConsumeChar(p, end, ',')) {
            continue;
        }
        if (ConsumeChar(p, end, ']')) {
            return true;
        }
        return false;
    }
    return false;
}

static bool ParseSceneObject(const char*& p, const char* end, SceneJson& outScene) {
    if (!ConsumeChar(p, end, '{')) {
        return false;
    }
    SkipWs(p, end);
    if (ConsumeChar(p, end, '}')) {
        return true;
    }

    while (p < end) {
        std::string key;
        if (!ParseString(p, end, key)) {
            return false;
        }
        if (!ConsumeChar(p, end, ':')) {
            return false;
        }

        if (key == "scene_index") {
            int idx = -1;
            if (!ParseInt(p, end, idx)) {
                return false;
            }
            outScene.index = idx;
        } else if (key == "loader_paths") {
            if (!ParseStringOrNullArray(p, end, outScene.loaderPaths)) {
                return false;
            }
        } else {
            if (!SkipValue(p, end)) {
                return false;
            }
        }

        SkipWs(p, end);
        if (ConsumeChar(p, end, ',')) {
            continue;
        }
        if (ConsumeChar(p, end, '}')) {
            return true;
        }
        return false;
    }
    return false;
}

static bool ParseScenesArray(const char*& p, const char* end, std::vector<SceneJson>& out) {
    out.clear();
    if (!ConsumeChar(p, end, '[')) {
        return false;
    }
    SkipWs(p, end);
    if (ConsumeChar(p, end, ']')) {
        return true;
    }

    while (p < end) {
        SceneJson scn;
        if (!ParseSceneObject(p, end, scn)) {
            return false;
        }
        out.push_back(std::move(scn));

        SkipWs(p, end);
        if (ConsumeChar(p, end, ',')) {
            continue;
        }
        if (ConsumeChar(p, end, ']')) {
            return true;
        }
        return false;
    }
    return false;
}

static bool ParseSceneTablesJson(const std::string& json, std::vector<SceneJson>& outScenes) {
    const char* p = json.data();
    const char* end = p + json.size();

    if (!ConsumeChar(p, end, '{')) {
        return false;
    }

    SkipWs(p, end);
    if (ConsumeChar(p, end, '}')) {
        return true;
    }

    while (p < end) {
        std::string key;
        if (!ParseString(p, end, key)) {
            return false;
        }
        if (!ConsumeChar(p, end, ':')) {
            return false;
        }

        if (key == "scenes") {
            if (!ParseScenesArray(p, end, outScenes)) {
                return false;
            }
        } else {
            if (!SkipValue(p, end)) {
                return false;
            }
        }

        SkipWs(p, end);
        if (ConsumeChar(p, end, ',')) {
            continue;
        }
        if (ConsumeChar(p, end, '}')) {
            return true;
        }
        return false;
    }
    return false;
}

static PrSceneMainFn MainFnForScene(PrSceneId id) {
    switch (id) {
        case PrSceneId::Scene0:
            return &PrSS0Scene0RuntimeDirect::Main;
        case PrSceneId::Scene1: return &PrScn1::Main;
        case PrSceneId::Scene2: return &PrScn2::Main;
        case PrSceneId::Scene3: return &PrScn3::Main;
        case PrSceneId::Scene5: return &PrScn5::Main;
        case PrSceneId::Scene6: return &PrScn6::Main;
        case PrSceneId::Scene7: return &PrScn7::Main;
        case PrSceneId::Scene8: return &PrScn8::Main;
        case PrSceneId::Scene9: return &PrScn9::Main;
    }
    return nullptr;
}

static PrSceneFn0 Fn0ForScene(PrSceneId id) {
    switch (id) {
        case PrSceneId::Scene0:
            return &PrSS0Scene0RuntimeDirect::Fn0;
        case PrSceneId::Scene1: return &PrScn1::Fn0;
        case PrSceneId::Scene2: return &PrScn2::Fn0;
        case PrSceneId::Scene3: return &PrScn3::Fn0;
        case PrSceneId::Scene5: return &PrScn5::Fn0;
        case PrSceneId::Scene6: return &PrScn6::Fn0;
        case PrSceneId::Scene7: return &PrScn7::Fn0;
        case PrSceneId::Scene8: return &PrScn8::Fn0;
        case PrSceneId::Scene9: return &PrScn9::Fn0;
    }
    return nullptr;
}

static PrSceneFn1 Fn1ForScene(PrSceneId id) {
    switch (id) {
        case PrSceneId::Scene0:
            return &PrSS0Scene0RuntimeDirect::Fn1;
        case PrSceneId::Scene1: return &PrScn1::Fn1;
        case PrSceneId::Scene2: return &PrScn2::Fn1;
        case PrSceneId::Scene3: return &PrScn3::Fn1;
        case PrSceneId::Scene5: return &PrScn5::Fn1;
        case PrSceneId::Scene6: return &PrScn6::Fn1;
        case PrSceneId::Scene7: return &PrScn7::Fn1;
        case PrSceneId::Scene8: return &PrScn8::Fn1;
        case PrSceneId::Scene9: return &PrScn9::Fn1;
    }
    return nullptr;
}

static PrSceneFn2 Fn2ForScene(PrSceneId id) {
    switch (id) {
        case PrSceneId::Scene0:
            return &PrSS0Scene0RuntimeDirect::Fn2;
        case PrSceneId::Scene1: return &PrScn1::Fn2;
        case PrSceneId::Scene2: return &PrScn2::Fn2;
        case PrSceneId::Scene3: return &PrScn3::Fn2;
        case PrSceneId::Scene5: return &PrScn5::Fn2;
        case PrSceneId::Scene6: return &PrScn6::Fn2;
        case PrSceneId::Scene7: return &PrScn7::Fn2;
        case PrSceneId::Scene8: return &PrScn8::Fn2;
        case PrSceneId::Scene9: return &PrScn9::Fn2;
    }
    return nullptr;
}

static PrSceneRenderFn RenderFnForScene(PrSceneId id) {
    switch (id) {
        case PrSceneId::Scene0:
            return &PrSS0Scene0RuntimeDirect::Render;
        case PrSceneId::Scene1: return &PrScn1::Render;
        case PrSceneId::Scene2: return &PrScn2::Render;
        case PrSceneId::Scene3: return &PrScn3::Render;
        case PrSceneId::Scene5: return &PrScn5::Render;
        case PrSceneId::Scene6: return &PrScn6::Render;
        case PrSceneId::Scene7: return &PrScn7::Render;
        case PrSceneId::Scene8: return &PrScn8::Render;
        case PrSceneId::Scene9: return &PrScn9::Render;
    }
    return nullptr;
}

static void AssignSceneCallbackPsxFunctions80048D28(PrSceneDef& def,
                                                    PrSceneId id) {
    const PrSceneEntryDirect::SceneCallbackTriplet80048D28 callbacks =
        PrSceneEntryDirect::GetSceneCallbackTriplet80048D28(
            static_cast<uint32_t>(id));
    if (!callbacks.known) {
        def.psxFn0 = 0;
        def.psxFn1 = 0;
        def.psxFn2 = 0;
        return;
    }
    def.psxFn0 = callbacks.fn0;
    def.psxFn1 = callbacks.fn1;
    def.psxFn2 = callbacks.fn2;
}

static char ToUpperAscii(char c) {
    if (c >= 'a' && c <= 'z') {
        return (char)(c - ('a' - 'A'));
    }
    return c;
}

static bool EndsWithAscii(const std::string& s, const char* suffix) {
    size_t sl = s.size();
    size_t tl = std::strlen(suffix);
    if (sl < tl) {
        return false;
    }
    for (size_t i = 0; i < tl; i++) {
        if (s[sl - tl + i] != suffix[i]) {
            return false;
        }
    }
    return true;
}

static std::filesystem::path NormalizeLoaderPath(const std::string& s) {
    std::string p = s;
    while (!p.empty() && (p.front() == '\\' || p.front() == '/')) {
        p.erase(p.begin());
    }
    const size_t semi = p.rfind(';');
    if (semi != std::string::npos) {
        p.erase(semi);
    }
    for (char& c : p) {
        if (c == '\\') {
            c = '/';
        }
    }
    return std::filesystem::path(p);
}

static void AssignLoaderPath(PrSceneDef& def, const std::filesystem::path& relPath) {
    std::string u = relPath.u8string();
    for (char& c : u) {
        c = ToUpperAscii(c);
    }

    if (EndsWithAscii(u, ".BIN") && u.find("COMOD") != std::string::npos) {
        def.comod.path = relPath;
        return;
    }

    if (EndsWithAscii(u, ".INT") && u.find("ZCOMPO") != std::string::npos) {
        def.zcompo.path = relPath;
        return;
    }

    if (EndsWithAscii(u, ".INT") && u.find("COMPO") != std::string::npos) {
        def.compo.path = relPath;
        return;
    }

    if (EndsWithAscii(u, ".STR") && u.find("YMOVIE") != std::string::npos) {
        def.resultMovieB.path = relPath;
        return;
    }

    if (EndsWithAscii(u, ".STR") && u.find("XMOVIE") != std::string::npos) {
        def.resultMovieA.path = relPath;
        return;
    }

    if (EndsWithAscii(u, ".STR") && u.find("MOVIE") != std::string::npos) {
        def.movie.path = relPath;
        return;
    }

    if (EndsWithAscii(u, ".XA1") || EndsWithAscii(u, ".XA")) {
        def.xa.path = relPath;
        return;
    }
}

static void AssignLoaderPathBySlot(
    PrSceneDef& def,
    size_t slotIndex,
    const std::filesystem::path& relPath) {
    switch (slotIndex) {
    case 0: def.comod.path = relPath; return;
    case 1: def.compo.path = relPath; return;
    case 2: def.movie.path = relPath; return;
    case 3: def.xa.path = relPath; return;
    case 4: def.resultMovieA.path = relPath; return;
    case 5: def.resultMovieB.path = relPath; return;
    case 6: def.zcompo.path = relPath; return;
    default:
        AssignLoaderPath(def, relPath);
        return;
    }
}

static PrSceneDef MakeSceneDef(
    const std::filesystem::path& comod,
    const std::filesystem::path& compo,
    const std::filesystem::path& zcompo,
    const std::filesystem::path& movie,
    const std::filesystem::path& xa,
    const std::filesystem::path& resultMovieA,
    const std::filesystem::path& resultMovieB,
    PrSceneId id) {
    PrSceneDef def;
    def.comod.path = comod;
    def.compo.path = compo;
    def.zcompo.path = zcompo;
    def.movie.path = movie;
    def.xa.path = xa;
    def.resultMovieA.path = resultMovieA;
    def.resultMovieB.path = resultMovieB;
    def.fn0 = Fn0ForScene(id);
    def.fn1 = Fn1ForScene(id);
    def.fn2 = Fn2ForScene(id);
    AssignSceneCallbackPsxFunctions80048D28(def, id);
    def.main = MainFnForScene(id);
    def.render = RenderFnForScene(id);
    return def;
}

static PrSceneDef MakeScene0DirectDef8005474C() {
    if (!PrSS0Scene0RuntimeDirect::RuntimeEnabled()) {
        return MakeSceneDef("", "", "", "", "", "", "",
                            PrSceneId::Scene0);
    }
    return MakeSceneDef("S0/COMOD0.BIN",
                        "S0/COMPO00.INT",
                        "S0/ZCOMPO.INT",
                        "SS/MOVIE0.STR",
                        "SS/MOVIE0T.STR",
                        "",
                        "",
                        PrSceneId::Scene0);
}

static void InitSceneTableFallback(PrSceneTable& table) {
    table.Get(PrSceneId::Scene0) = MakeScene0DirectDef8005474C();
    table.Get(PrSceneId::Scene1) = MakeSceneDef("S1/COMOD1.BIN", "S1/COMPO01.INT", "S1/ZCOMPO.INT", "SS/MOVIE1.STR", "S1/STAGE1.XA1", "S1/XMOVIE1.STR", "S1/XMOVIE1.STR", PrSceneId::Scene1);
    table.Get(PrSceneId::Scene2) = MakeSceneDef("S2/COMOD2.BIN", "S2/COMPO02.INT", "S2/ZCOMPO.INT", "SS/MOVIE2.STR", "S2/STAGE2.XA1", "S2/XMOVIE2.STR", "S2/YMOVIE2.STR", PrSceneId::Scene2);
    table.Get(PrSceneId::Scene3) = MakeSceneDef("S3/COMOD3.BIN", "S3/COMPO03.INT", "S3/ZCOMPO.INT", "SS/MOVIE3.STR", "S3/STAGE3.XA1", "S3/XMOVIE3.STR", "S3/YMOVIE3.STR", PrSceneId::Scene3);
    table.Get(PrSceneId::Scene5) = MakeSceneDef("S5/COMOD5.BIN", "S5/COMPO05.INT", "S5/ZCOMPO.INT", "SS/MOVIE5.STR", "S5/STAGE5.XA1", "S5/XMOVIE5.STR", "S5/YMOVIE5.STR", PrSceneId::Scene5);
    table.Get(PrSceneId::Scene6) = MakeSceneDef("S6/COMOD6.BIN", "S6/COMPO06.INT", "S6/ZCOMPO.INT", "SS/MOVIE6.STR", "S6/STAGE6.XA1", "S6/XMOVIE6.STR", "S6/YMOVIE6.STR", PrSceneId::Scene6);
    table.Get(PrSceneId::Scene7) = MakeSceneDef("S7/COMOD7.BIN", "S7/COMPO07.INT", "S7/ZCOMPO.INT", "", "S7/STAGE7.XA1", "", "", PrSceneId::Scene7);
    table.Get(PrSceneId::Scene8) = MakeSceneDef("S8/COMOD8.BIN", "", "S8/ZCOMPO.INT", "", "", "S8/XMOVIE8.STR", "S8/XMOVIE8.STR", PrSceneId::Scene8);
    table.Get(PrSceneId::Scene9) = MakeSceneDef("S9/COMOD9.BIN", "S9/COMPO09.INT", "S9/ZCOMPO.INT", "", "S9/STAGE9.XA1", "", "", PrSceneId::Scene9);
}

bool PrMain::InitSceneTable(PrSceneTable& table, const std::filesystem::path& dataRoot) {
    InitSceneTableFallback(table);

    const std::filesystem::path jsonPath = dataRoot / "out" / "scene_tables.json";
    std::ifstream file(jsonPath, std::ios::binary);
    if (!file.is_open()) {
        Log::Printf("Scene tables json open failed: %s", jsonPath.u8string().c_str());
        return false;
    }

    std::string json;
    file.seekg(0, std::ios::end);
    std::streamoff size = file.tellg();
    if (size <= 0) {
        Log::Printf("Scene tables json empty: %s", jsonPath.u8string().c_str());
        return false;
    }
    json.resize((size_t)size);
    file.seekg(0, std::ios::beg);
    file.read(json.data(), size);

    std::vector<SceneJson> scenes;
    if (!ParseSceneTablesJson(json, scenes)) {
        Log::Printf("Scene tables json parse failed: %s", jsonPath.u8string().c_str());
        return false;
    }

    for (const SceneJson& scn : scenes) {
        if (scn.index < 0 || scn.index >= (int)kPrSceneCount) {
            continue;
        }

        const PrSceneId id = (PrSceneId)(uint8_t)scn.index;
        if (id == PrSceneId::Scene0) {
            table.Get(id) = MakeScene0DirectDef8005474C();
            continue;
        }

        PrSceneDef def;
        def.fn0 = Fn0ForScene(id);
        def.fn1 = Fn1ForScene(id);
        def.fn2 = Fn2ForScene(id);
        AssignSceneCallbackPsxFunctions80048D28(def, id);
        def.main = MainFnForScene(id);
        def.render = RenderFnForScene(id);

        for (size_t i = 0; i < scn.loaderPaths.size(); ++i) {
            const std::string& lp = scn.loaderPaths[i];
            if (lp.empty()) {
                continue;
            }
            const std::filesystem::path rel = NormalizeLoaderPath(lp);
            AssignLoaderPathBySlot(def, i, rel);
        }

        table.Get(id) = def;
    }

    Log::Printf("Scene tables loaded from %s scenes=%llu",
                jsonPath.u8string().c_str(),
                (unsigned long long)scenes.size());
    return true;
}

static PrSceneId NextScene(PrSceneId id) {
    uint8_t next = (uint8_t)id;
    next = (uint8_t)((next + 1) % kPrSceneCount);
    return (PrSceneId)next;
}

static PrSceneEntryDirect::MainSceneState80015D18 s_mainScene80015D18;
static PrStage1LoaderMemoryDirectState s_mainSceneLoaderMemory80015D18;
static PrScene1EntryOriginalDiscDirect::Transaction80015D18
    s_scene1EntryOriginalDiscTransaction80015D18;
static bool s_scene1EntryOriginalDiscOverlayLoadKnown80015D18 = false;
static bool s_scene1EntryOriginalDiscOverlayLoadAccepted80015D18 = false;
static bool s_startupFinalReset80025A34Committed = false;
static PrStage1LoaderMemoryDirectResetResult80025A34
    s_startupFinalReset80025A34{};
static bool s_startupPadWait80016AB4Committed = false;
static PrPsxPadDirect::StartupPadWaitResult80016AB4
    s_startupPadWait80016AB4{};

struct Stage1RuntimePsxMemoryProviderUserData {
    const PrGameContext* ctx = nullptr;
    PrStage1LoaderMemoryDirectState* loaderMemory = nullptr;
    PrStage1XaCdDirectState* xaCd = nullptr;
    PrStage1XaCdDirectRuntimePsxMemoryProvider* runtimeProvider = nullptr;
};

static Stage1RuntimePsxMemoryProviderUserData
    s_stage1RuntimePsxMemoryProviderUserData;
static PrStage1XaCdDirectCdMmioSnapshotRuntimeSource
    s_stage1CdMmioSnapshotRuntimeSource;
static PrMainWord800916F0ProviderIngressDebug
    s_word800916F0ProviderIngressDebug;

static void WriteU16LE(uint8_t* outBytes, uint16_t value) {
    outBytes[0] = static_cast<uint8_t>(value & 0xFFu);
    outBytes[1] = static_cast<uint8_t>((value >> 8) & 0xFFu);
}

static bool ReadMainGlobalRuntimePsxMemory(const PrGameContext* ctx,
                                           uint32_t psxAddress,
                                           uint32_t byteSize,
                                           uint8_t* outBytes,
                                           size_t outSize) {
    if (ctx == nullptr || outBytes == nullptr || outSize < byteSize ||
        byteSize != 2u) {
        return false;
    }
    switch (psxAddress) {
    case 0x800916D0u:
        WriteU16LE(outBytes, static_cast<uint16_t>(ctx->transitionState));
        return true;
    case 0x800916DAu:
        WriteU16LE(outBytes, static_cast<uint16_t>(ctx->transitionStateDA));
        return true;
    case 0x800916D8u:
        WriteU16LE(outBytes, static_cast<uint16_t>(ctx->languageIndex));
        return true;
    case 0x800916DCu:
        WriteU16LE(outBytes, static_cast<uint16_t>(ctx->subtitleFlag));
        return true;
    case 0x800916E0u:
        WriteU16LE(outBytes, static_cast<uint16_t>(ctx->sceneExitReason));
        return true;
    case 0x800916F0u:
        if (ctx->word800916F0Known) {
            WriteU16LE(outBytes, ctx->word800916F0);
            return true;
        }
        return false;
    default:
        return false;
    }
}

static bool ReadStage1RuntimePsxMemory(void* userData,
                                       uint32_t psxAddress,
                                       uint32_t byteSize,
                                       uint8_t* outBytes,
                                       size_t outSize) {
    if (userData == nullptr) {
        return false;
    }
    auto* provider =
        static_cast<Stage1RuntimePsxMemoryProviderUserData*>(userData);
    if (ReadMainGlobalRuntimePsxMemory(provider->ctx,
                                       psxAddress,
                                       byteSize,
                                       outBytes,
                                       outSize)) {
        return true;
    }
    if (provider->loaderMemory != nullptr &&
        PrStage1LoaderMemoryDirectReadRuntimePsxMemory(
            provider->loaderMemory,
            psxAddress,
            byteSize,
            outBytes,
            outSize)) {
        return true;
    }
    if (provider->xaCd != nullptr &&
        PrStage1XaCdDirectReadKnownStateRuntimePsxMemory(
            provider->xaCd,
            psxAddress,
            byteSize,
            outBytes,
            outSize)) {
        return true;
    }
    if (provider->runtimeProvider != nullptr &&
        provider->runtimeProvider->cdMmioSourceInstalled &&
        provider->runtimeProvider->cdMmioRead != nullptr &&
        provider->runtimeProvider->cdMmioRead(
            provider->runtimeProvider->cdMmioUserData,
            psxAddress,
            byteSize,
            outBytes,
            outSize)) {
        return true;
    }
    return provider->runtimeProvider != nullptr &&
           provider->runtimeProvider->exactCdSourceInstalled &&
           provider->runtimeProvider->exactCdRead != nullptr &&
           provider->runtimeProvider->exactCdRead(
               provider->runtimeProvider->exactCdUserData,
               psxAddress,
               byteSize,
               outBytes,
               outSize);
}

static bool ReadStage1ExactCdRuntimePsxMemory(void* userData,
                                              uint32_t psxAddress,
                                              uint32_t byteSize,
                                              uint8_t* outBytes,
                                              size_t outSize) {
    if (userData == nullptr || outBytes == nullptr || outSize < byteSize) {
        return false;
    }
    const auto* state = static_cast<const PrStage1XaCdDirectState*>(userData);
    if (psxAddress == 0x1F801803u && byteSize == 1u) {
        if (!state->rawEvent80036AF8InitialInterruptKnown ||
            state->rawEvent80036AF8InitialInterruptObservationAcceptedCount ==
                0u) {
            return false;
        }
        outBytes[0] = state->rawEvent80036AF8InitialInterrupt;
        return true;
    }
    if (psxAddress == 0x1F801800u && byteSize == 1u) {
        if (!state->rawEvent80036AF8CdReg0StatusKnown) {
            return false;
        }
        outBytes[0] = state->rawEvent80036AF8CdReg0Status;
        return true;
    }
    return false;
}

static void InstallStage1LoaderHeapRuntimePsxMemoryProvider(
    PrGameContext& ctx) {
    s_stage1RuntimePsxMemoryProviderUserData.ctx = &ctx;
    s_stage1RuntimePsxMemoryProviderUserData.loaderMemory =
        &s_mainSceneLoaderMemory80015D18;
    s_stage1RuntimePsxMemoryProviderUserData.xaCd = &ctx.stage1XaCdDirect;
    s_stage1RuntimePsxMemoryProviderUserData.runtimeProvider =
        &ctx.stage1RuntimePsxMemoryProvider;
    ctx.stage1RuntimePsxMemoryProvider.installed = true;
    ctx.stage1RuntimePsxMemoryProvider.read = ReadStage1RuntimePsxMemory;
    ctx.stage1RuntimePsxMemoryProvider.userData =
        &s_stage1RuntimePsxMemoryProviderUserData;
    ctx.stage1RuntimePsxMemoryProvider.loaderHeapSourceInstalled = true;
    ctx.stage1RuntimePsxMemoryProvider.xaCdKnownStateRelayInstalled = true;
    s_stage1CdMmioSnapshotRuntimeSource = {};
    s_stage1CdMmioSnapshotRuntimeSource.producerIngressInstalled = true;
    ctx.stage1RuntimePsxMemoryProvider.cdMmioSourceInstalled = true;
    ctx.stage1RuntimePsxMemoryProvider.cdMmioRead =
        PrStage1XaCdDirectReadCdMmioSnapshotRuntimePsxMemory;
    ctx.stage1RuntimePsxMemoryProvider.cdMmioUserData =
        &s_stage1CdMmioSnapshotRuntimeSource;
    ctx.stage1RuntimePsxMemoryProvider.exactCdSourceInstalled = true;
    ctx.stage1RuntimePsxMemoryProvider.exactCdRead =
        ReadStage1ExactCdRuntimePsxMemory;
    ctx.stage1RuntimePsxMemoryProvider.exactCdUserData =
        &ctx.stage1XaCdDirect;
    ctx.stage1RuntimePsxMemoryProvider.frameKnown = true;
    ctx.stage1RuntimePsxMemoryProvider.frame = ctx.frame;
    ctx.stage1RuntimePsxMemoryProvider.pcKnown = false;
    ctx.stage1RuntimePsxMemoryProvider.pc = 0u;
}

static bool TryPublishWord800916F0FromStage1RuntimePsxMemoryProvider(
    PrGameContext& ctx) {
    PrSS0Scene0RuntimeDirect::Word800916F0RuntimePsxMemoryProviderRead read{};
    read.attempted = ctx.stage1RuntimePsxMemoryProvider.installed &&
                     ctx.stage1RuntimePsxMemoryProvider.read != nullptr;
    read.psxAddress = 0x800916F0u;
    read.byteSize = 2u;
    read.frameKnown = ctx.stage1RuntimePsxMemoryProvider.frameKnown;
    read.frame = ctx.stage1RuntimePsxMemoryProvider.frame;
    read.pcKnown = ctx.stage1RuntimePsxMemoryProvider.pcKnown;
    read.pc = ctx.stage1RuntimePsxMemoryProvider.pc;
    if (read.attempted) {
        read.readable = ctx.stage1RuntimePsxMemoryProvider.read(
            ctx.stage1RuntimePsxMemoryProvider.userData,
            read.psxAddress,
            read.byteSize,
            read.bytes,
            sizeof(read.bytes));
    }
    const auto sourceAudit =
        PrMain::AuditStage1RuntimePsxMemorySources(ctx, 0x800916F0u, 2u);
    s_word800916F0ProviderIngressDebug.attempted = read.attempted;
    s_word800916F0ProviderIngressDebug.readable = read.readable;
    s_word800916F0ProviderIngressDebug.publishAttempted = read.readable;
    s_word800916F0ProviderIngressDebug.frame = ctx.frame;
    s_word800916F0ProviderIngressDebug.sourceMainGlobalAttempted =
        sourceAudit.mainGlobalReadAttempted;
    s_word800916F0ProviderIngressDebug.sourceMainGlobalReadable =
        sourceAudit.mainGlobalReadable;
    s_word800916F0ProviderIngressDebug.sourceMainGlobalSelfBootstrapBlocked =
        sourceAudit.mainGlobalSelfBootstrapBlocked;
    s_word800916F0ProviderIngressDebug.sourceLoaderHeapAttempted =
        sourceAudit.loaderHeapReadAttempted;
    s_word800916F0ProviderIngressDebug.sourceLoaderHeapReadable =
        sourceAudit.loaderHeapReadable;
    s_word800916F0ProviderIngressDebug.sourceKnownStateRelayAttempted =
        sourceAudit.knownStateRelayReadAttempted;
    s_word800916F0ProviderIngressDebug.sourceKnownStateRelayReadable =
        sourceAudit.knownStateRelayReadable;
    s_word800916F0ProviderIngressDebug.sourceCdMmioSnapshotAttempted =
        sourceAudit.cdMmioSnapshotReadAttempted;
    s_word800916F0ProviderIngressDebug.sourceCdMmioSnapshotReadable =
        sourceAudit.cdMmioSnapshotReadable;
    s_word800916F0ProviderIngressDebug.sourceExactCdAttempted =
        sourceAudit.exactCdReadAttempted;
    s_word800916F0ProviderIngressDebug.sourceExactCdReadable =
        sourceAudit.exactCdReadable;
    if (read.attempted) {
        ++s_word800916F0ProviderIngressDebug.attemptCount;
    }
    if (read.readable) {
        ++s_word800916F0ProviderIngressDebug.readableCount;
    } else if (read.attempted) {
        ++s_word800916F0ProviderIngressDebug.sourceMissingCount;
    }
    const bool published = PrSS0Scene0RuntimeDirect::
        PublishWord800916F0FromRuntimePsxMemoryProviderRead(read);
    s_word800916F0ProviderIngressDebug.published = published;
    s_word800916F0ProviderIngressDebug.knownAfter =
        PrSS0Scene0RuntimeDirect::IsWord800916F0Known();
    if (published) {
        ctx.word800916F0Known = true;
        ctx.word800916F0 =
            static_cast<uint16_t>(read.bytes[0]) |
            static_cast<uint16_t>(
                static_cast<uint16_t>(read.bytes[1]) << 8);
        ++s_word800916F0ProviderIngressDebug.publishedCount;
    }
    return published;
}

bool PrMain::TryPublishWord800916F0FromStage1RuntimePsxMemoryProvider(
    PrGameContext& ctx) {
    return ::TryPublishWord800916F0FromStage1RuntimePsxMemoryProvider(ctx);
}

PrMainStage1RuntimePsxMemorySourceReadAudit
PrMain::AuditStage1RuntimePsxMemorySources(const PrGameContext&,
                                           uint32_t psxAddress,
                                           uint32_t byteSize) {
    PrMainStage1RuntimePsxMemorySourceReadAudit audit{};
    std::array<uint8_t, 0x40> bytes{};
    if (byteSize == 0u || byteSize > bytes.size()) {
        return audit;
    }

    audit.mainGlobalReadAttempted = true;
    const PrGameContext* mainGlobalCtx =
        s_stage1RuntimePsxMemoryProviderUserData.ctx;
    audit.mainGlobalSelfBootstrapBlocked =
        psxAddress == 0x800916F0u &&
        byteSize == 2u &&
        (mainGlobalCtx == nullptr || !mainGlobalCtx->word800916F0Known);
    audit.mainGlobalReadable =
        ReadMainGlobalRuntimePsxMemory(
            mainGlobalCtx,
            psxAddress,
            byteSize,
            bytes.data(),
            bytes.size());
    if (s_stage1RuntimePsxMemoryProviderUserData.loaderMemory != nullptr) {
        audit.loaderHeapReadAttempted = true;
        audit.loaderHeapReadable =
            PrStage1LoaderMemoryDirectReadRuntimePsxMemory(
                s_stage1RuntimePsxMemoryProviderUserData.loaderMemory,
                psxAddress,
                byteSize,
                bytes.data(),
                bytes.size());
    }
    if (s_stage1RuntimePsxMemoryProviderUserData.xaCd != nullptr) {
        audit.knownStateRelayReadAttempted = true;
        audit.knownStateRelayReadable =
            PrStage1XaCdDirectReadKnownStateRuntimePsxMemory(
                s_stage1RuntimePsxMemoryProviderUserData.xaCd,
                psxAddress,
                byteSize,
                bytes.data(),
                bytes.size());
    }

    const auto* runtimeProvider =
        s_stage1RuntimePsxMemoryProviderUserData.runtimeProvider;
    if (runtimeProvider != nullptr &&
        runtimeProvider->cdMmioSourceInstalled &&
        runtimeProvider->cdMmioRead != nullptr &&
        runtimeProvider->cdMmioUserData != nullptr) {
        audit.cdMmioSnapshotReadAttempted = true;
        audit.cdMmioSnapshotReadable =
            runtimeProvider->cdMmioRead(runtimeProvider->cdMmioUserData,
                                        psxAddress,
                                        byteSize,
                                        bytes.data(),
                                        bytes.size());
    }
    if (runtimeProvider != nullptr &&
        runtimeProvider->exactCdSourceInstalled &&
        runtimeProvider->exactCdRead != nullptr &&
        runtimeProvider->exactCdUserData != nullptr) {
        audit.exactCdReadAttempted = true;
        audit.exactCdReadable =
            runtimeProvider->exactCdRead(runtimeProvider->exactCdUserData,
                                         psxAddress,
                                         byteSize,
                                         bytes.data(),
                                         bytes.size());
    }
    return audit;
}

PrMainWord800916F0ProviderIngressDebug
PrMain::GetWord800916F0ProviderIngressDebug() {
    return s_word800916F0ProviderIngressDebug;
}

static const char* Case17CardReadTypedCarrierSourceName(
    PrStage1SaveCardHalDirect::CardReadTypedCarrierSource800179B4 source) {
    switch (source) {
    case PrStage1SaveCardHalDirect::CardReadTypedCarrierSource800179B4::
        Unknown:
        return "unknown";
    case PrStage1SaveCardHalDirect::CardReadTypedCarrierSource800179B4::
        RuntimeLowerCardProducer:
        return "runtime-lower-card-producer";
    case PrStage1SaveCardHalDirect::CardReadTypedCarrierSource800179B4::
        DebugSyntheticFixture:
        return "debug-synthetic-fixture";
    }
    return "invalid";
}

static bool BuildCase17CompletedCall19414InputFromLiveFactsAndPayloadBytes(
    PrSceneEntryExecutorDirect::CompletedCall80019414Input80015788& out) {
    out = {};

    const PrStage1SaveStatusPrefix80092F10 statusPrefix =
        PrStage1SaveUiDirect::GetSaveStatusPrefix80092F10();
    if (!statusPrefix.known || statusPrefix.helperGap ||
        !statusPrefix.statusBankKnown80092F1D) {
        Log::Printf(
            "case17 80019414 bridge gate: statusPrefixKnown=%d helperGap=%d "
            "statusBankKnown=%d psxAddress=%08X byteCount=%u "
            "bridgeCompleted=0 outCompleted=0",
            statusPrefix.known ? 1 : 0,
            statusPrefix.helperGap ? 1 : 0,
            statusPrefix.statusBankKnown80092F1D ? 1 : 0,
            statusPrefix.psxAddress,
            statusPrefix.byteCount);
        return false;
    }

    PrStage1SaveCardHalDirect::Case17CardReadTypedCarrier800179B4 carrier{};
    if (!PrStage1SaveCardHalDirect::GetCase17CardReadTypedCarrier800179B4(
            &carrier)) {
        Log::Printf(
            "case17 80019414 bridge gate: typed case17 carrier absent "
            "bridgeCompleted=0 outCompleted=0");
        return false;
    }

    const bool sourceAllowed =
        carrier.source ==
        PrStage1SaveCardHalDirect::CardReadTypedCarrierSource800179B4::
            RuntimeLowerCardProducer;
    if (!sourceAllowed ||
        !carrier.producerWired800173A8_80016EB8_800179B4 ||
        !carrier.case17LoopCompletionKnown80019D7C ||
        carrier.incomplete) {
        Log::Printf(
            "case17 80019414 bridge gate: typed lower-card read producer incomplete "
            "source=%s sourceAllowed=%d producer=%d completionKnown=%d laneKnown=%d readSuccess=%d payloadBytes=%d incomplete=%d "
            "bridgeCompleted=0 outCompleted=0",
            Case17CardReadTypedCarrierSourceName(carrier.source),
            sourceAllowed ? 1 : 0,
            carrier.producerWired800173A8_80016EB8_800179B4 ? 1 : 0,
            carrier.case17LoopCompletionKnown80019D7C ? 1 : 0,
            carrier.case17HiScorePayloadLaneKnown ? 1 : 0,
            carrier.typedReadSuccessKnown800179B4 ? 1 : 0,
            carrier.payloadBytesKnown8007ADE8 ? 1 : 0,
            carrier.incomplete ? 1 : 0);
        PrStage1SaveCardHalDirect::ClearCase17CardReadTypedCarrier800179B4();
        return false;
    }

    PrSceneEntryCardFeedbackDirect::Case17CardReadHalFeedback80019D7C
        case17Hal{};
    PrSceneEntryCardFeedbackDirect::
        BuildCase17CardReadHalFeedbackFromSaveCardHal800179B4(
            carrier.feedback,
            carrier.hal,
            &case17Hal);

    PrSceneEntryCardFeedbackDirect::Case17To19414FeedbackBuildResult80015788
        bridge{};
    PrSceneEntryCardFeedbackDirect::BuildFeedback80019414FromCase17CardReadFacts(
        statusPrefix,
        true,
        17,
        case17Hal,
        &bridge);
    if (!bridge.completed || bridge.gap) {
        Log::Printf(
            "case17 80019414 bridge gate: typed case17 feedback gap "
            "completed=%d gap=%d missingHal=%d bridgeCompleted=0 outCompleted=0",
            bridge.completed ? 1 : 0,
            bridge.gap ? 1 : 0,
            bridge.missingHalFacts800179B4 ? 1 : 0);
        PrStage1SaveCardHalDirect::ClearCase17CardReadTypedCarrier800179B4();
        return false;
    }

    out = PrSceneEntryExecutorDirect::
        BuildCompletedCall80019414InputFromFeedbackAdapterResult80019414(
            bridge.adapter);
    PrStage1SaveCardHalDirect::ClearCase17CardReadTypedCarrier800179B4();
    Log::Printf(
        "case17 80019414 bridge gate: typed case17 carrier consumed "
        "bridgeCompleted=%d outCompleted=%d tablePsx=%08X tableBytes=%llu",
        bridge.completed ? 1 : 0,
        out.completed ? 1 : 0,
        out.tablePsxAddress,
        static_cast<unsigned long long>(out.tableByteCount));
    return out.completed;
}

static PrSceneId PrevSceneFromMainSceneState80015D18(
    const PrSceneEntryDirect::MainSceneState80015D18& state,
    PrSceneId fallback) {
    if (!state.previousSceneV1Known ||
        state.previousSceneV1 < 0 ||
        state.previousSceneV1 >= static_cast<int32_t>(kPrSceneCount)) {
        return fallback;
    }
    return static_cast<PrSceneId>(
        static_cast<uint8_t>(state.previousSceneV1));
}

static void SyncMainSceneState80015D18Globals(PrGameContext& ctx,
                                              PrSceneId scene) {
    s_mainScene80015D18.currentSceneV0Known = true;
    s_mainScene80015D18.currentSceneV0 = static_cast<int32_t>(scene);
    s_mainScene80015D18.word800916D0Known = true;
    s_mainScene80015D18.word800916D0 =
        static_cast<uint16_t>(ctx.transitionState);
}

static void SyncMainSceneState80015D18D0(PrGameContext& ctx) {
    s_mainScene80015D18.word800916D0Known = true;
    s_mainScene80015D18.word800916D0 =
        static_cast<uint16_t>(ctx.transitionState);
}

static PrSceneEntryDirect::MainSceneStepResult80015D18
PrepareSceneCallbacksStep80015D18() {
    const PrSceneEntryDirect::MainSceneStepResult80015D18 callbacks =
        PrSceneEntryDirect::PrepareMainSceneCallbacks80015D18(
            s_mainScene80015D18);
    s_mainScene80015D18 = callbacks.state;
    return callbacks;
}

static bool SS0DirectOwnsSceneInit80015D18(PrSceneId scene) {
    return PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
           (scene == PrSceneId::Scene0 || scene == PrSceneId::Scene1);
}

static void ResetDirectSceneLoaderBeforeOverlay80015DC0(PrSceneId scene) {
    if (!SS0DirectOwnsSceneInit80015D18(scene)) {
        return;
    }
    PrStage1LoaderMemoryDirectReset(s_mainSceneLoaderMemory80015D18);
    Log::Printf(
        "PrMain direct SS0: scene=%u pre-overlay 80015DC0 loader reset",
        static_cast<unsigned>(scene));
}

static bool ReadScene1ProjectedComodComparisonBytes80015D18(
    const std::filesystem::path& path,
    std::vector<uint8_t>& out) {
    out.clear();
    if (path.empty()) {
        return false;
    }
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }
    const std::streamsize size = file.tellg();
    if (size <= 0) {
        return false;
    }
    file.seekg(0);
    out.resize(static_cast<size_t>(size));
    file.read(reinterpret_cast<char*>(out.data()), size);
    if (!file.good()) {
        out.clear();
        return false;
    }
    return true;
}

static const PrScene1EntryOriginalDiscDirect::Transaction80015D18*
PrepareDirectScene1EntryOriginalDiscTransactionBeforeOverlay80015D18(
    PrSceneId scene,
    const PrSceneDef& def,
    const PrGameContext& ctx) {
    s_scene1EntryOriginalDiscTransaction80015D18 = {};
    s_scene1EntryOriginalDiscOverlayLoadKnown80015D18 = false;
    s_scene1EntryOriginalDiscOverlayLoadAccepted80015D18 = false;
    if (!PrSS0Scene0RuntimeDirect::RuntimeEnabled() ||
        scene != PrSceneId::Scene1) {
        return nullptr;
    }

    const std::filesystem::path projectedComodPath =
        def.comod.path.empty()
            ? std::filesystem::path{}
            : ctx.dataRoot / def.comod.path;
    std::vector<uint8_t> projectedComodComparisonBytes;
    const bool projectedComodComparisonKnown =
        ReadScene1ProjectedComodComparisonBytes80015D18(
            projectedComodPath,
            projectedComodComparisonBytes);

    PrScene1EntryOriginalDiscDirect::Source80015D18 source{};
    source.sceneIndexKnown = true;
    source.sceneIndex =
        PrScene1EntryOriginalDiscDirect::kSceneIndex80015D18;
    source.rowIndexKnown = true;
    source.rowIndex = PrScene1EntryOriginalDiscDirect::kRowIndex80015D18;
    source.rowAddressKnown = true;
    source.rowAddress =
        PrScene1EntryOriginalDiscDirect::kRowAddress80015D18;
    source.pathPtrKnown = true;
    source.pathPtr = PrScene1EntryOriginalDiscDirect::kPathPtr80015D18;
    source.psxPathKnown = true;
    source.psxPath = PrScene1EntryOriginalDiscDirect::kPsxPath80015D18;
    source.dataRoot = ctx.dataRoot;
    source.projectedComodViewKnown = projectedComodComparisonKnown;
    source.projectedComodViewData = projectedComodComparisonKnown
                                        ? projectedComodComparisonBytes.data()
                                        : nullptr;
    source.projectedComodViewSize = projectedComodComparisonBytes.size();
    source.projectedComodComparisonOnly = true;
    source.projectedComodSourceAuthority = false;
    source.hostFilesystemAuthority = false;
    source.replayAuthority = false;
    source.oldS0Authority = false;
    source.stage2PlusAuthority = false;

    s_scene1EntryOriginalDiscTransaction80015D18 =
        PrScene1EntryOriginalDiscDirect::BuildTransaction80015D18(source);
    const auto& transaction =
        s_scene1EntryOriginalDiscTransaction80015D18;
    Log::Printf(
        "PrMain direct SS0: Scene1 row0 original-disc transaction "
        "status=%u accepted=%d disc=%d projection=%d seams=%u "
        "projectedPath=%s",
        static_cast<unsigned>(transaction.status),
        transaction.accepted ? 1 : 0,
        transaction.originalDiscSourceAuthority ? 1 : 0,
        transaction.byteForByteProjectionMatch ? 1 : 0,
        transaction.cdSeamCount,
        projectedComodPath.u8string().c_str());
    return &transaction;
}

static void RecordDirectScene1OriginalDiscOverlayLoad80015D18(
    PrSceneId scene,
    const PrScene1EntryOriginalDiscDirect::Transaction80015D18* transaction,
    bool overlayLoaded) {
    const bool directScene1 =
        PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
        scene == PrSceneId::Scene1;
    s_scene1EntryOriginalDiscOverlayLoadKnown80015D18 = directScene1;
    s_scene1EntryOriginalDiscOverlayLoadAccepted80015D18 =
        directScene1 && overlayLoaded && transaction != nullptr &&
        PrScene1EntryOriginalDiscDirect::
            IsExactAcceptedTransaction80015D18(*transaction);
    if (directScene1) {
        Log::Printf(
            "PrMain direct SS0: Scene1 original-disc overlay load "
            "loaded=%d accepted=%d",
            overlayLoaded ? 1 : 0,
            s_scene1EntryOriginalDiscOverlayLoadAccepted80015D18 ? 1 : 0);
    }
}

static bool ResetDirectSceneLoaderAfterOverlay80015E5C(PrSceneId scene) {
    if (!SS0DirectOwnsSceneInit80015D18(scene)) {
        return true;
    }
    if (scene == PrSceneId::Scene0) {
        if (!PrSS0TitleTmdBackend::
                ApplyScene0LoaderResetPreservingTitlePacketArena80025A34(
                    s_mainSceneLoaderMemory80015D18)) {
            Log::Printf(
                "PrMain direct SS0: Scene0 post-overlay 80015E5C loader reset blocked");
            return false;
        }
    } else {
        PrStage1LoaderMemoryDirectReset(s_mainSceneLoaderMemory80015D18);
    }
    Log::Printf(
        "PrMain direct SS0: scene=%u post-overlay 80015E5C loader reset",
        static_cast<unsigned>(scene));
    return true;
}

static PrSceneEntryExecutorDirect::MainSceneCallbackExecution80015D18
ExecutePrMainInitCallbacksOnce80015D18(
    PrSceneEntryExecutorDirect::MainSceneInitGateState80015D18& gate,
    PrSceneId scene,
    const PrSceneEntryDirect::MainSceneStepResult80015D18& step,
    PrGameContext& ctx,
    const PrSceneDef& def) {
    PrSceneEntryExecutorDirect::MainSceneCallbackExecution80015D18 out{};
    if (PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
        scene == PrSceneId::Scene0) {
        if (gate.initializedSceneKnown && gate.initializedScene == scene) {
            return out;
        }
        if (!ResetDirectSceneLoaderAfterOverlay80015E5C(scene)) {
            return out;
        }
        if (def.fn0) {
            (void)def.fn0(ctx);
            out.fn0FunctionMatched = true;
            out.fn0Executed = true;
        }
        if (def.fn1) {
            def.fn1(ctx);
            out.fn1FunctionMatched = true;
            out.fn1Executed = true;
        }
        gate.initializedSceneKnown = true;
        gate.initializedScene = scene;
        Log::Printf(
            "PrMain direct SS0: scene=%u init callbacks owned by runtime",
            static_cast<unsigned>(scene));
        return out;
    }

    if (PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
        scene == PrSceneId::Scene1) {
        if (gate.initializedSceneKnown && gate.initializedScene == scene) {
            return out;
        }
        if (gate.directScene1InitAttempted) {
            return out;
        }
        const auto& transaction =
            s_scene1EntryOriginalDiscTransaction80015D18;
        if (!s_scene1EntryOriginalDiscOverlayLoadKnown80015D18 ||
            !s_scene1EntryOriginalDiscOverlayLoadAccepted80015D18 ||
            !PrScene1EntryOriginalDiscDirect::
                IsExactAcceptedTransaction80015D18(transaction)) {
            return out;
        }

        out = PrSceneEntryExecutorDirect::
            ExecuteDirectScene1InitCallbacksOnce80015D18(
                gate,
                scene,
                step,
                ctx,
                def,
                &transaction.rowFeedback8001A324,
                transaction.cdSeams.data(),
                transaction.cdSeamCount,
                &s_mainSceneLoaderMemory80015D18);
        const bool complete = PrSceneEntryExecutorDirect::
            IsExactDirectScene1InitCompletion80015D18(out);
        if (complete || gate.directScene1InitAttempted) {
            Log::Printf(
                "PrMain direct SS0: Scene1 original-disc row0 init "
                "complete=%d resets=%u rowInit=%u transfer=%u fn0=%d fn1=%d",
                complete ? 1 : 0,
                out.call80025A34Count,
                out.call8001A324Count,
                out.call800154B0Count,
                out.fn0Executed ? 1 : 0,
                out.fn1Executed ? 1 : 0);
        }
        return out;
    }

    return PrSceneEntryExecutorDirect::ExecuteMainSceneInitCallbacksOnce80015D18(
        gate,
        scene,
        step,
        ctx,
        def,
        &s_mainSceneLoaderMemory80015D18);
}

void PrMain::Run(PrGameContext& ctx, PrSceneTable& table) {
    static bool s_started = false;
    static PrStage1ColdBootStatusSeedContext800154F4
        s_coldBootStatusSeedContext800154F4;
    static PrSceneId s_scene = PrSceneId::Scene0;
    static PrSceneId s_prevScene = PrSceneId::Scene0;
    static PrSceneEntryExecutorDirect::MainSceneInitGateState80015D18
        s_sceneInitGate80015D18;
    static uint32_t s_sceneStartFrame = 0;
    static PrOverlayLoader s_loader;
    static bool s_sceneSwitchRequest = false;
    static bool s_residentDirectoryRequest = false;
    static int s_pendingScene = -1;
    static bool s_prEventInitialized = false;
    static bool s_scene1InitBlockedLogged = false;

    ctx.mainSceneLoaderMemoryDirect = &s_mainSceneLoaderMemory80015D18;
    InstallStage1LoaderHeapRuntimePsxMemoryProvider(ctx);
    (void)TryPublishWord800916F0FromStage1RuntimePsxMemoryProvider(ctx);

    // 80016B84 ends with a second 80025A34 after both logos, before the
    // separate 80016AB4 PAD/VBlank tail.  Apply the exact software loader
    // table/pointer reset once at the first post-logo main-loop entry; the
    // later scene callback still performs its own source-ordered reset.
    if (PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
        !s_startupFinalReset80025A34Committed) {
        if (ctx.mainSceneLoaderMemoryDirect == nullptr) {
            Log::Printf(
                "PrMain direct SS0: post-logo 80025A34 reset blocked loader state missing");
            return;
        }
        s_startupFinalReset80025A34 =
            PrStage1LoaderMemoryDirectReset80025A34(
                *ctx.mainSceneLoaderMemoryDirect);
        if (!s_startupFinalReset80025A34.committed ||
            s_startupFinalReset80025A34.function !=
                kPrStage1LoaderMemoryDirectFn80025A34 ||
            s_startupFinalReset80025A34.tailFunction !=
                kPrStage1LoaderMemoryDirectFn80025A00 ||
            s_startupFinalReset80025A34.zeroDwordCount != 1024u ||
            !s_startupFinalReset80025A34.stackTableCleared ||
            s_startupFinalReset80025A34.heapBase !=
                kPrStage1LoaderMemoryDirectHeapBase800965B0 ||
            s_startupFinalReset80025A34.heapEnd !=
                kPrStage1LoaderMemoryDirectHeapEnd801C35B0 ||
            s_startupFinalReset80025A34.heapCursor !=
                kPrStage1LoaderMemoryDirectHeapBase800965B0 ||
            s_startupFinalReset80025A34.stackDepth != 0u ||
            s_startupFinalReset80025A34.physicalMemoryAuthority) {
            Log::Printf(
                "PrMain direct SS0: post-logo 80025A34 reset blocked committed=%d zeroDwords=%u stackCleared=%d heap=%08X/%08X/%08X depth=%u",
                s_startupFinalReset80025A34.committed ? 1 : 0,
                static_cast<unsigned>(
                    s_startupFinalReset80025A34.zeroDwordCount),
                s_startupFinalReset80025A34.stackTableCleared ? 1 : 0,
                s_startupFinalReset80025A34.heapBase,
                s_startupFinalReset80025A34.heapEnd,
                s_startupFinalReset80025A34.heapCursor,
                s_startupFinalReset80025A34.stackDepth);
            return;
        }
        s_startupFinalReset80025A34Committed = true;
        Log::Printf(
            "PrMain direct SS0: post-logo 80025A34->80025A00 committed zeroRange=%08X..%08X dwords=%u heap=%08X/%08X/%08X stackDepth=%u softwareOnly=1 physicalMemoryAuthority=0",
            s_startupFinalReset80025A34.zeroStartAddress,
            s_startupFinalReset80025A34.zeroEndAddress,
            static_cast<unsigned>(
                s_startupFinalReset80025A34.zeroDwordCount),
            s_startupFinalReset80025A34.heapBase,
            s_startupFinalReset80025A34.heapEnd,
            s_startupFinalReset80025A34.heapCursor,
            s_startupFinalReset80025A34.stackDepth);
    }

    // SCUS 80015D18 calls 80016AB4 after 80016B84 (including both logos) and
    // before entering its scene loop.  Keep this one-shot startup tail out of
    // the regular scene/Loading state machine: it is a bounded PAD_dr poll
    // with a 80035560(0) wait on zero input, not a second Loading callback.
    if (PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
        !s_startupPadWait80016AB4Committed) {
        const PrPadState pad = PrPad::GetState(0);
        const uint16_t localPadMask = static_cast<uint16_t>(
            pad.pressed | pad.held);
        const uint16_t returnedPadMask =
            PrPsxPadDirect::BuildReturnedMask80035510FromLocalAndDebugPad(
                localPadMask,
                static_cast<uint16_t>(ctx.debugPadInput));
        s_startupPadWait80016AB4 =
            PrPsxPadDirect::ExecuteStartupPadWait80016AB4(
                returnedPadMask,
                PrPsxVSyncDirect::ProcessVSyncState80035560());
        if (!s_startupPadWait80016AB4.committed ||
            s_startupPadWait80016AB4.wrapperFunction !=
                PrPsxPadDirect::kFn80016AB4_StartupPadWait ||
            s_startupPadWait80016AB4.padReadFunction !=
                PrPsxPadDirect::kFn80035510_PadRead ||
            s_startupPadWait80016AB4.waitFunction !=
                PrPsxPadDirect::kFn80035560_StartupWait ||
            s_startupPadWait80016AB4.waitArgument != 0 ||
            s_startupPadWait80016AB4.pollCount == 0u ||
            s_startupPadWait80016AB4.pollCount >
                PrPsxPadDirect::kStartupPadWaitPollLimit80016AB4 ||
            s_startupPadWait80016AB4.hardwarePadHalAuthority ||
            s_startupPadWait80016AB4.physicalVblankTimingAuthority) {
            Log::Printf(
                "PrMain direct SS0: startup 80016AB4 pad/wait tail blocked polls=%u waits=%u return=%u",
                static_cast<unsigned>(
                    s_startupPadWait80016AB4.pollCount),
                static_cast<unsigned>(
                    s_startupPadWait80016AB4.waitCallCount),
                static_cast<unsigned>(
                    s_startupPadWait80016AB4.returnValue));
            return;
        }
        s_startupPadWait80016AB4Committed = true;
        Log::Printf(
            "PrMain direct SS0: startup 80016AB4 committed polls=%u waits=%u consumedVblanks=%u pad=%03X resultBeforeSpecial=%u return=%u special287=%u word800916FA=%u softwareOnly=1 hardwarePadHalAuthority=0 physicalVblankTimingAuthority=0",
            static_cast<unsigned>(s_startupPadWait80016AB4.pollCount),
            static_cast<unsigned>(s_startupPadWait80016AB4.waitCallCount),
            static_cast<unsigned>(
                s_startupPadWait80016AB4.consumedVblankCount),
            static_cast<unsigned>(
                s_startupPadWait80016AB4.returnedPadMask),
            static_cast<unsigned>(
                s_startupPadWait80016AB4.resultBeforeSpecialMap),
            static_cast<unsigned>(s_startupPadWait80016AB4.returnValue),
            s_startupPadWait80016AB4.specialMaskMatched ? 1u : 0u,
            s_startupPadWait80016AB4.word800916FAWritten
                ? static_cast<unsigned>(
                      s_startupPadWait80016AB4.word800916FAValue)
                : 0u);
    }

    const auto ensureLegacyPrEventInitialized = [&]() {
        if (!s_prEventInitialized) {
            PrEvent::Init();
            s_prEventInitialized = true;
        }
    };

    if (!s_started) {
        PrSS0Scene0RuntimeDirect::InitializeWord800916F0FromProcessStartupZero80028590();
        if (!PrSS0Scene0RuntimeDirect::
                InitializeWord800916F6FromProcessStartupZero80028590()) {
            Log::Printf(
                "PrMain direct SS0: current-IDA 80028590 word_800916F6 startup-zero initialization rejected");
        }
        if (!PrSS0Scene0RuntimeDirect::
                InitializeWord800916FCFromProcessStartupZero80028590()) {
            Log::Printf(
                "PrMain direct SS0: current-IDA 80028590 word_800916FC startup-zero initialization rejected");
        }
        s_started = true;
        s_scene = PrSceneId::Scene0;
        s_prevScene = s_scene;
        s_sceneStartFrame = ctx.frame;
        Log::Printf("PrMain start");
        if (PrSS0Scene0RuntimeDirect::RuntimeEnabled()) {
            Log::Printf("PrMain direct SS0: legacy PrEvent init deferred");
        } else {
            ensureLegacyPrEventInitialized();
        }
        PrTransition::Init();
        s_mainScene80015D18 =
            PrSceneEntryDirect::InitMainSceneState80015D18();
        const PrStage1SavePayloadProducerResult coldBootStatusPrefix =
            PrStage1SaveUiDirect::SeedColdBootStatusPrefix800154F4(
                s_coldBootStatusSeedContext800154F4);
        const PrStage1SavePayloadBankRuntimeSnapshot coldBootRuntime =
            PrStage1SaveUiDirect::GetSavePayloadBankRuntimeSnapshot();
        Log::Printf(
            "PrMain cold boot 800154F4 status seed: ok=%d payloadKnown=%d "
            "helperGap=%d result=%d lastFault=%08X "
            "sub8001635CReplaySourceKnownAtEntry=%d "
            "sub8001635CReplayShapeKnownAtEntry=%d "
            "sub8001635CReplaySetCountAtEntry=%u "
            "sub8001635CReplayInvalidSetCountAtEntry=%u "
            "sub8001635CReplayHydrateCountAtEntry=%u "
            "sub8001635CPreflightMirrorSourceKnown=%d "
            "sub8001635CPreflightReplayProducerSourceKnown=%d "
            "sub8001635CPreflightStartupZeroKnown=%d "
            "startupZero80028590Accepted=%d "
            "replayMirrorSourceKnown8008EEF8=%d "
            "replayMirrorSourceSetCount=%u "
            "replayMirrorSourceHydrateCount=%u "
            "replayMirrorAuthorityKnown8001635C=%d "
            "wrote8001635C=%d",
            coldBootStatusPrefix.ok ? 1 : 0,
            coldBootStatusPrefix.payloadKnown ? 1 : 0,
            coldBootStatusPrefix.helperGap ? 1 : 0,
            coldBootStatusPrefix.result,
            coldBootStatusPrefix.lastFaultAddress,
            coldBootRuntime.sub8001635CReplayMirrorSourceKnownAtEntry ? 1 : 0,
            coldBootRuntime.sub8001635CReplayMirrorSourceShapeKnownAtEntry
                ? 1
                : 0,
            coldBootRuntime.sub8001635CReplayMirrorSourceSetCountAtEntry,
            coldBootRuntime.sub8001635CReplayMirrorSourceInvalidSetCountAtEntry,
            coldBootRuntime.sub8001635CReplayMirrorSourceHydrateCountAtEntry,
            coldBootRuntime.sub8001635CPreflightMirrorSourceKnown ? 1 : 0,
            coldBootRuntime.sub8001635CPreflightReplayMirrorProducerSourceKnown
                ? 1
                : 0,
            coldBootRuntime.sub8001635CPreflightStartupZeroSourceKnown ? 1 : 0,
            coldBootRuntime.seedColdBootStartupZeroAccepted ? 1 : 0,
            coldBootRuntime.replayMirrorSourceKnown8008EEF8 ? 1 : 0,
            coldBootRuntime.replayMirrorSourceSetCount,
            coldBootRuntime.replayMirrorSourceHydrateCount,
            coldBootRuntime.replayMirrorAuthorityKnown8001635C ? 1 : 0,
            coldBootRuntime.wrote8001635C ? 1 : 0);
        SyncMainSceneState80015D18Globals(ctx, s_scene);
        const PrSceneDef& initialDef = table.Get(s_scene);
        const auto* initialScene1Transaction =
            PrepareDirectScene1EntryOriginalDiscTransactionBeforeOverlay80015D18(
                s_scene,
                initialDef,
                ctx);
        ResetDirectSceneLoaderBeforeOverlay80015DC0(s_scene);
        const bool initialOverlayLoaded = s_loader.Load(
            s_scene,
            initialDef,
            ctx,
            initialScene1Transaction);
        RecordDirectScene1OriginalDiscOverlayLoad80015D18(
            s_scene,
            initialScene1Transaction,
            initialOverlayLoaded);
        PrSceneEntryExecutorDirect::ResetMainSceneInitGate80015D18(
            s_sceneInitGate80015D18);
        s_scene1InitBlockedLogged = false;
    }

    const auto switchSceneNow =
        [&](PrSceneId next, bool initImmediately, bool forceEntry = false) {
            if (next == s_scene && !forceEntry) {
                return;
            }
            s_loader.Unload(ctx);
            s_scene = next;
            s_sceneStartFrame = ctx.frame;
            Log::Printf("PrMain switch scene=%u", (unsigned)s_scene);
            const PrSceneDef& loadDef = table.Get(s_scene);
            const auto* scene1Transaction =
                PrepareDirectScene1EntryOriginalDiscTransactionBeforeOverlay80015D18(
                    s_scene,
                    loadDef,
                    ctx);
            ResetDirectSceneLoaderBeforeOverlay80015DC0(s_scene);
            const bool overlayLoaded = s_loader.Load(
                s_scene,
                loadDef,
                ctx,
                scene1Transaction);
            RecordDirectScene1OriginalDiscOverlayLoad80015D18(
                s_scene,
                scene1Transaction,
                overlayLoaded);
            PrSceneEntryExecutorDirect::ResetMainSceneInitGate80015D18(
                s_sceneInitGate80015D18);
            s_scene1InitBlockedLogged = false;
            SyncMainSceneState80015D18Globals(ctx, s_scene);
            if (initImmediately) {
                const PrSceneDef& nextDef = table.Get(s_scene);
                ctx.currentSceneDef = &nextDef;
                const PrSceneEntryDirect::MainSceneStepResult80015D18 step =
                    PrepareSceneCallbacksStep80015D18();
                (void)ExecutePrMainInitCallbacksOnce80015D18(
                    s_sceneInitGate80015D18,
                    s_scene,
                    step,
                    ctx,
                    nextDef);
            }
        };

    // SCUS keeps v0 at the outgoing stage while 80015788 is blocked. Do not
    // run its Fn2 again or change it to Scene0 merely to borrow the menu UI.
    if (s_residentDirectoryRequest ||
        PrSS0Scene0RuntimeDirect::IsResidentDirectoryActive80015788()) {
        if (s_residentDirectoryRequest) {
            if (s_mainScene80015D18.word800916DE == 1u) {
                s_mainScene80015D18 = PrSceneEntryDirect::
                    BeginMainSceneLoopIteration80015D18(s_mainScene80015D18).state;
            }
            if (!s_mainScene80015D18.previousSceneV1Known ||
                !PrSS0Scene0RuntimeDirect::BeginResidentDirectory80015788(
                    ctx, s_mainScene80015D18.previousSceneV1)) {
                Log::Printf(
                    "PrMain direct SS0: resident 80015788 begin deferred "
                    "v1Known=%d v1=%d scene=%u initialized=%d renderer=%d resources=%d",
                    s_mainScene80015D18.previousSceneV1Known ? 1 : 0,
                    s_mainScene80015D18.previousSceneV1,
                    static_cast<unsigned>(s_scene),
                    PrSS0Scene0RuntimeDirect::RuntimeEnabled() ? 1 : 0,
                    ctx.renderer ? 1 : 0,
                    ctx.resources ? 1 : 0);
                return;
            }
            s_residentDirectoryRequest = false;
        }
        PrSS0Scene0RuntimeDirect::TickResidentDirectory80015788(ctx);
        const int result = PrSS0Scene0RuntimeDirect::ConsumeResidentDirectoryResult80015788();
        if (result < 0) return;
        s_mainScene80015D18 = PrSceneEntryDirect::ApplyMainSceneSwitchResult80015D18(
            s_mainScene80015D18, result);
        s_prevScene = static_cast<PrSceneId>(result);
        s_pendingScene = -1;
        // Reselecting the same stage is still a fresh native callback entry.
        switchSceneNow(s_prevScene, true, true);
    }

    bool completedMainLoopThisFrame = false;
    if (s_sceneSwitchRequest) {
        s_sceneSwitchRequest = false;
        SyncMainSceneState80015D18Globals(ctx, s_scene);

        if (s_scene == PrSceneId::Scene0) {
            s_prevScene = PrSceneId::Scene0;
            s_mainScene80015D18.previousSceneV1Known = true;
            s_mainScene80015D18.previousSceneV1 = 0;
        }

        const PrSceneEntryDirect::MainSceneStepResult80015D18 beginSwitch =
            PrSceneEntryDirect::BeginMainSceneLoopIteration80015D18(
                s_mainScene80015D18);
        s_mainScene80015D18 = beginSwitch.state;
        (void)PrSceneEntryExecutorDirect::ExecuteMainSceneHostTrace80015D18(
            beginSwitch.trace,
            false,
            false);
        const PrSceneId prevForSwitch =
            PrevSceneFromMainSceneState80015D18(s_mainScene80015D18,
                                                s_prevScene);
        PrSceneEntryExecutorDirect::CompletedCall80019414Input80015788
            completedCall19414{};
        const bool completedCall19414Known =
            BuildCase17CompletedCall19414InputFromLiveFactsAndPayloadBytes(
                completedCall19414);
        Log::Printf(
            "case17 80015D18 switch input: completedCall19414Known=%d "
            "completed=%d ptrPassed=%d tablePsx=%08X tableBytes=%llu",
            completedCall19414Known ? 1 : 0,
            completedCall19414.completed ? 1 : 0,
            (completedCall19414Known && completedCall19414.completed) ? 1 : 0,
            completedCall19414.tablePsxAddress,
            static_cast<unsigned long long>(completedCall19414.tableByteCount));
        const PrSceneEntryExecutorDirect::GenericSwitchRunResult80015788
            switchResult =
                PrSceneEntryExecutorDirect::ExecuteMainSceneSwitchTrace80015D18(
                    beginSwitch.trace,
                    ctx,
                    table,
                    prevForSwitch,
                    completedCall19414Known ? &completedCall19414 : nullptr);
        if (!switchResult.decided) {
            s_sceneSwitchRequest = true;
        } else {
            const PrSceneId next = switchResult.scene;
            const PrSceneEntryDirect::MainSceneStepResult80015D18 complete =
                PrSceneEntryDirect::CompleteMainSceneLoopIteration80015D18(
                    s_mainScene80015D18,
                    static_cast<int32_t>(next));
            s_mainScene80015D18 =
                complete.state;
            (void)PrSceneEntryExecutorDirect::ExecuteMainSceneHostTrace80015D18(
                complete.trace,
                false,
                false);
            SyncMainSceneState80015D18D0(ctx);
            s_mainScene80015D18.previousSceneV1Known = true;
            s_mainScene80015D18.previousSceneV1 = static_cast<int32_t>(next);
            completedMainLoopThisFrame = true;
            s_prevScene = next;

            if (next != s_scene) {
                switchSceneNow(next, false);
            }
        }
    } else if (s_pendingScene >= 0) {
        const int pending = s_pendingScene;
        s_pendingScene = -1;

        const bool ss0DirectOwnsPendingSceneCarrier =
            PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
            (s_scene == PrSceneId::Scene0 || s_scene == PrSceneId::Scene1);
        if (ss0DirectOwnsPendingSceneCarrier) {
            Log::Printf(
                "PrMain pending scene ignored for direct scene=%u pending=%d",
                static_cast<unsigned>(s_scene),
                pending);
        } else if (pending >= 0 && pending < (int)kPrSceneCount) {
            const PrSceneId next = (PrSceneId)(uint8_t)pending;
            s_mainScene80015D18 =
                PrSceneEntryDirect::ApplyMainSceneSwitchResult80015D18(
                    s_mainScene80015D18,
                    static_cast<int32_t>(next));
            if (next != s_scene) {
                switchSceneNow(next, false);
            }
        } else {
            Log::Printf("PrMain invalid pending scene=%d", pending);
        }
    }

    ctx.currentScene = s_scene;
    const PrSceneDef& def = table.Get(s_scene);
    ctx.currentSceneDef = &def;
    const PrSceneEntryDirect::MainSceneStepResult80015D18 sceneCallbackStep =
        PrepareSceneCallbacksStep80015D18();

    s_stage1Fn2ResultThisFrameDebug = -1;
    s_stage1PendingSceneThisFrameDebug = -1;
    const bool ss0DirectOwnsMainLoopAtFrameStart =
        PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
        (s_scene == PrSceneId::Scene0 || s_scene == PrSceneId::Scene1);
    bool ss0DirectScene0Fn2ResultThisFrame = false;
    bool ss0DirectDirectoryResultThisFrame = false;
    bool ss0DirectScene1Fn2ResultThisFrame = false;
    // A direct scene result can switch s_scene after the destination's
    // normal Fn0/Fn1 entry pass above.  Remember that destination so its first
    // Fn2 tick is executed before the same host frame is rendered; otherwise
    // Scene1 exposes a one-frame None route (the host clear colour).
    PrSceneId ss0DirectDestinationNeedsFirstFn2 =
        static_cast<PrSceneId>(0xFF);

    const auto initExecution = ExecutePrMainInitCallbacksOnce80015D18(
        s_sceneInitGate80015D18,
        s_scene,
        sceneCallbackStep,
        ctx,
        def);

    const bool directScene1InitReady =
        !PrSS0Scene0RuntimeDirect::RuntimeEnabled() ||
        s_scene != PrSceneId::Scene1 ||
        (s_sceneInitGate80015D18.initializedSceneKnown &&
         s_sceneInitGate80015D18.initializedScene == PrSceneId::Scene1);
    if (!directScene1InitReady) {
        if (!s_scene1InitBlockedLogged) {
            Log::Printf(
                "PrMain direct SS0: Scene1 main/Fn2 blocked before exact init "
                "overlay=%d/%d attempted=%d failed=%d scene=%d trace=%d "
                "row=%d seams=%d loaderGap=%d prefix=%d",
                s_scene1EntryOriginalDiscOverlayLoadKnown80015D18 ? 1 : 0,
                s_scene1EntryOriginalDiscOverlayLoadAccepted80015D18 ? 1 : 0,
                s_sceneInitGate80015D18.directScene1InitAttempted ? 1 : 0,
                s_sceneInitGate80015D18.directScene1InitFailed ? 1 : 0,
                initExecution.directScene1InitSceneMatched ? 1 : 0,
                initExecution.directScene1InitTraceMatched ? 1 : 0,
                initExecution.directScene1InitRow0FeedbackAccepted ? 1 : 0,
                initExecution.directScene1InitCompletedCdSeamsAccepted ? 1 : 0,
                initExecution.call80025A34Gap ? 1 : 0,
                initExecution.directScene1InitRow0PrefixComplete ? 1 : 0);
            s_scene1InitBlockedLogged = true;
        }
        return;
    }
    s_scene1InitBlockedLogged = false;

    if (def.main) {
        def.main(ctx);
    }

    int v2 = (int)s_scene;
    if (ctx.debugStage1DirectBoot &&
        s_scene == PrSceneId::Scene0 &&
        ss0DirectOwnsMainLoopAtFrameStart) {
        Log::Printf(
            "PrMain stage1-direct-boot ignored for direct scene=%u",
            static_cast<unsigned>(s_scene));
        ctx.debugStage1DirectBoot = false;
        v2 = (int)s_scene;
    } else if (ctx.debugStage1DirectBoot && s_scene == PrSceneId::Scene0) {
        Log::Printf("PrMain stage1-direct-boot (debug)");
        v2 = (int)PrSceneId::Scene1;
    } else if (ctx.debugNextScene && ss0DirectOwnsMainLoopAtFrameStart) {
        Log::Printf(
            "PrMain next-scene ignored for direct scene=%u",
            static_cast<unsigned>(s_scene));
        ctx.debugNextScene = false;
        v2 = (int)s_scene;
    } else if (ctx.debugNextScene) {
        Log::Printf("PrMain next-scene (debug)");
        v2 = (int)NextScene(s_scene);
    } else if (ctx.debugGenericSwitch && ss0DirectOwnsMainLoopAtFrameStart) {
        Log::Printf(
            "PrMain generic-switch-request ignored for direct scene=%u",
            static_cast<unsigned>(s_scene));
        ctx.debugGenericSwitch = false;
        v2 = (int)s_scene;
    } else if (ctx.debugGenericSwitch) {
        Log::Printf("PrMain generic-switch-request (debug)");
        v2 = -1;
    } else if (ss0DirectOwnsMainLoopAtFrameStart &&
               s_scene == PrSceneId::Scene1 &&
               def.fn2) {
        ss0DirectScene1Fn2ResultThisFrame = true;
        v2 = def.fn2(ctx);
        s_stage1Fn2ResultThisFrameDebug = v2;
    } else if (def.fn2) {
        const PrSceneEntryExecutorDirect::MainSceneCallbackExecution80015D18
            callbackResult =
                PrSceneEntryExecutorDirect::ExecuteMainSceneCallbacks80015D18(
                    sceneCallbackStep,
                    ctx,
                    def,
                    false,
                    true,
                    nullptr,
                    nullptr,
                    0u,
                    nullptr,
                    0u);
        if (callbackResult.fn2ResultKnown) {
            v2 = callbackResult.fn2Result;
            ss0DirectScene0Fn2ResultThisFrame =
                ss0DirectOwnsMainLoopAtFrameStart &&
                s_scene == PrSceneId::Scene0;
        }
        if (s_scene == PrSceneId::Scene1) {
            s_stage1Fn2ResultThisFrameDebug = v2;
        }
    }

    // Direct Scene0 owns its 80020110 handoff carrier.  Consume the target
    // only after Fn2 has completed the translated final tail; this removes
    // the former dependency on the Windows PrTransition state machine while
    // preserving the existing main-scene completion/switch path below.
    if (ss0DirectOwnsMainLoopAtFrameStart &&
        s_scene == PrSceneId::Scene0) {
        const int directHandoffTarget =
            PrSS0Scene0RuntimeDirect::ConsumeSceneHandoffTarget();
        if (directHandoffTarget >= 0 &&
            directHandoffTarget < (int)kPrSceneCount) {
            v2 = directHandoffTarget;
            ss0DirectDirectoryResultThisFrame = true;
        }
    }

    if (ss0DirectOwnsMainLoopAtFrameStart &&
        s_scene == PrSceneId::Scene0 &&
        v2 > 0 &&
        ctx.transitionState == 1) {
        PrSS0Scene0RuntimeDirect::
            ApplyWord800916EEMainLoopWriteback80015D18(
                v2,
                static_cast<uint16_t>(ctx.transitionState));
    }

    const int transSwitch = PrTransition::Update(ctx);
    const bool transActive = PrTransition::IsActive();
    const bool ss0DirectOwnsTransitionCarrier =
        ss0DirectOwnsMainLoopAtFrameStart &&
        (s_scene == PrSceneId::Scene0 || s_scene == PrSceneId::Scene1);
    static bool s_loggedSuppressed = false;

    if (transSwitch >= 0 && transSwitch < (int)kPrSceneCount) {
        const TransitionSource transitionSource = PrTransition::GetSource();
        if (ss0DirectOwnsTransitionCarrier &&
            transitionSource != TransitionSource::SS0Direct) {
            Log::Printf(
                "PrMain transition switch ignored for direct scene=%u target=%d source=%d",
                static_cast<unsigned>(s_scene),
                transSwitch,
                static_cast<int>(transitionSource));
            PrTransition::Cancel();
            v2 = (int)s_scene;
        } else {
            v2 = transSwitch;
            s_loggedSuppressed = false;
            const PrSceneId next = (PrSceneId)(uint8_t)transSwitch;
            if (next != s_scene) {
                s_pendingScene = -1;
                if (!completedMainLoopThisFrame) {
                    const PrSceneEntryDirect::MainSceneStepResult80015D18
                        complete =
                            PrSceneEntryDirect::
                                CompleteMainSceneLoopIteration80015D18(
                                    s_mainScene80015D18,
                                    static_cast<int32_t>(next));
                    s_mainScene80015D18 = complete.state;
                    (void)PrSceneEntryExecutorDirect::
                        ExecuteMainSceneHostTrace80015D18(
                            complete.trace,
                            false,
                            false);
                    SyncMainSceneState80015D18D0(ctx);
                    s_mainScene80015D18.previousSceneV1Known = true;
                    s_mainScene80015D18.previousSceneV1 =
                        static_cast<int32_t>(next);
                    completedMainLoopThisFrame = true;
                }
                switchSceneNow(next, true);
                v2 = (int)s_scene;
            }
        }
    } else if (transActive &&
               ss0DirectOwnsTransitionCarrier &&
               PrTransition::GetSource() != TransitionSource::SS0Direct) {
        Log::Printf(
            "PrMain transition active ignored for direct scene=%u source=%d",
            static_cast<unsigned>(s_scene),
            static_cast<int>(PrTransition::GetSource()));
        PrTransition::Cancel();
        v2 = (int)s_scene;
        s_loggedSuppressed = false;
    } else if (transActive) {
        if (v2 != (int)s_scene && !s_loggedSuppressed) {
            Log::Printf("PrMain: transition active, suppressing v2=%d", v2);
            s_loggedSuppressed = true;
        }
        v2 = (int)s_scene;
    } else {
        s_loggedSuppressed = false;
    }

    if (completedMainLoopThisFrame) {
        // The 80015788 switch path already ran 80015D18's frame-complete
        // epilogue above, while currentSceneV0 still held the previous scene.
    } else if (ss0DirectDirectoryResultThisFrame) {
        // Embedded Scene0 directory selection is 80015788's return, not the
        // title's ordinary Fn2 result. Only this edge updates retained v1.
        s_mainScene80015D18 = PrSceneEntryDirect::
            BeginMainSceneLoopIteration80015D18(s_mainScene80015D18).state;
        s_mainScene80015D18 = PrSceneEntryDirect::ApplyMainSceneSwitchResult80015D18(
            s_mainScene80015D18, v2);
        s_prevScene = static_cast<PrSceneId>(v2);
        s_pendingScene = -1;
        switchSceneNow(s_prevScene, true, true);
        ss0DirectDestinationNeedsFirstFn2 = s_scene;
    } else if (ss0DirectScene0Fn2ResultThisFrame &&
               v2 >= 0 &&
               v2 < (int)kPrSceneCount) {
        const PrSceneId next = (PrSceneId)(uint8_t)v2;
        SyncMainSceneState80015D18Globals(ctx, s_scene);
        const PrSceneEntryDirect::MainSceneStepResult80015D18 complete =
            PrSceneEntryDirect::CompleteMainSceneLoopIteration80015D18(
                s_mainScene80015D18,
                v2);
        s_mainScene80015D18 = complete.state;
        (void)PrSceneEntryExecutorDirect::ExecuteMainSceneHostTrace80015D18(
            complete.trace,
            false,
            false);
        SyncMainSceneState80015D18D0(ctx);
        if (next != s_scene) {
            Log::Printf(
                "PrMain direct Scene0 result switch target=%d",
                v2);
            s_pendingScene = -1;
            // The direct 80015D18 completion has already consumed the final
            // handoff frame.  Run the destination entry callbacks in this
            // same logic tick so its first presented frame is initialized;
            // deferring them one tick leaves the host clear color exposed as
            // a one-frame black flash between scenes.
            switchSceneNow(next, true);
            ss0DirectDestinationNeedsFirstFn2 = next;
            v2 = (int)s_scene;
        }
    } else if (ss0DirectScene1Fn2ResultThisFrame &&
               v2 >= 0 &&
               v2 < (int)kPrSceneCount) {
        const PrSceneId next = (PrSceneId)(uint8_t)v2;
        SyncMainSceneState80015D18Globals(ctx, s_scene);
        const PrSceneEntryDirect::MainSceneStepResult80015D18 complete =
            PrSceneEntryDirect::CompleteMainSceneLoopIteration80015D18(
                s_mainScene80015D18,
                v2);
        s_mainScene80015D18 = complete.state;
        (void)PrSceneEntryExecutorDirect::ExecuteMainSceneHostTrace80015D18(
            complete.trace,
            false,
            false);
        SyncMainSceneState80015D18D0(ctx);
        if (next != s_scene) {
            Log::Printf(
                "PrMain direct Scene1 result switch target=%d",
                v2);
            s_pendingScene = -1;
            // Scene1's direct entry owns the first frame after the translated
            // 80015D18 result.  Initialize it before presentation rather than
            // exposing an uninitialized renderer frame for one tick.
            switchSceneNow(next, true);
            ss0DirectDestinationNeedsFirstFn2 = next;
            v2 = (int)s_scene;
        }
    } else if (v2 >= 0 && v2 < (int)kPrSceneCount) {
        const PrSceneEntryDirect::MainSceneStepResult80015D18 complete =
            PrSceneEntryDirect::CompleteMainSceneLoopIteration80015D18(
                s_mainScene80015D18,
                v2);
        s_mainScene80015D18 = complete.state;
        (void)PrSceneEntryExecutorDirect::ExecuteMainSceneHostTrace80015D18(
            complete.trace,
            false,
            false);
        SyncMainSceneState80015D18D0(ctx);
        const PrSceneId next = (PrSceneId)(uint8_t)v2;
        if (next != s_scene) {
            s_pendingScene = v2;
        }
    } else if (v2 < 0 && ss0DirectOwnsMainLoopAtFrameStart) {
        Log::Printf(
            "PrMain direct scene=%u native directory request v2=%d",
            static_cast<unsigned>(s_scene),
            v2);
        SyncMainSceneState80015D18Globals(ctx, s_scene);
        const PrSceneEntryDirect::MainSceneStepResult80015D18 complete =
            PrSceneEntryDirect::CompleteMainSceneLoopIteration80015D18(
                s_mainScene80015D18,
                v2);
        s_mainScene80015D18 = complete.state;
        (void)PrSceneEntryExecutorDirect::ExecuteMainSceneHostTrace80015D18(
            complete.trace,
            false,
            false);
        SyncMainSceneState80015D18D0(ctx);
        // Scene0 already owns its embedded directory. Stage1 needs the
        // independent resident owner and its own native ZCOMPO reload.
        if (s_scene == PrSceneId::Scene1) s_residentDirectoryRequest = true;
    } else if (v2 < 0) {
        const PrSceneEntryDirect::MainSceneStepResult80015D18 complete =
            PrSceneEntryDirect::CompleteMainSceneLoopIteration80015D18(
                s_mainScene80015D18,
                v2);
        s_mainScene80015D18 = complete.state;
        (void)PrSceneEntryExecutorDirect::ExecuteMainSceneHostTrace80015D18(
            complete.trace,
            false,
            false);
        SyncMainSceneState80015D18D0(ctx);
        s_sceneSwitchRequest = true;
    } else {
        Log::Printf("PrMain invalid next scene=%d", v2);
    }

    if (s_scene == PrSceneId::Scene1) {
        s_stage1PendingSceneThisFrameDebug = s_pendingScene;
    }

    ctx.currentScene = s_scene;
    if (ss0DirectDestinationNeedsFirstFn2 == s_scene &&
        (s_scene == PrSceneId::Scene0 || s_scene == PrSceneId::Scene1)) {
        const PrSceneDef& destinationDef = table.Get(s_scene);
        ctx.currentSceneDef = &destinationDef;
        if (destinationDef.fn2) {
            const int destinationResult = destinationDef.fn2(ctx);
            if (s_scene == PrSceneId::Scene1) {
                s_stage1Fn2ResultThisFrameDebug = destinationResult;
            }
            if (destinationResult != 0 &&
                destinationResult != static_cast<int>(s_scene)) {
                s_pendingScene = destinationResult;
            }
        }
    }
    const bool ss0DirectOwnsEventPump =
        PrSS0Scene0RuntimeDirect::RuntimeEnabled() &&
        (ctx.currentScene == PrSceneId::Scene0 ||
         ctx.currentScene == PrSceneId::Scene1);
    if (!ss0DirectOwnsEventPump) {
        ensureLegacyPrEventInitialized();
        PrEvent::Update(ctx);
    }
}

int PrMain::GetStage1Fn2ResultThisFrameDebug() {
    return s_stage1Fn2ResultThisFrameDebug;
}

int PrMain::GetStage1PendingSceneThisFrameDebug() {
    return s_stage1PendingSceneThisFrameDebug;
}
