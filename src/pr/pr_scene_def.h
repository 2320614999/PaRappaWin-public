#pragma once

#include <cstdint>
#include <filesystem>

#include "pr_scene.h"

struct PrGameContext;

struct PrSceneFile {
    std::filesystem::path path;
};

// PSX scene function signatures
// fn0: initialize global pointers, return scene index
using PrSceneFn0 = int (*)(PrGameContext&);
// fn1: load resources (COMPO etc.) and init subsystems
using PrSceneFn1 = void (*)(PrGameContext&);
// fn2: main loop, return next scene index (-1 means stay/return to caller)
using PrSceneFn2 = int (*)(PrGameContext&);
// Legacy per-frame update
using PrSceneMainFn = void (*)(PrGameContext&);
// Render callback (render only, no logic update)
using PrSceneRenderFn = void (*)(PrGameContext&);

struct PrSceneDef {
    PrSceneFile comod;
    PrSceneFile compo;
    PrSceneFile zcompo;
    PrSceneFile movie;
    PrSceneFile xa;
    PrSceneFile resultMovieA;
    PrSceneFile resultMovieB;

    // PSX blueprint functions
    PrSceneFn0 fn0 = nullptr;
    PrSceneFn1 fn1 = nullptr;
    PrSceneFn2 fn2 = nullptr;
    uint32_t psxFn0 = 0;
    uint32_t psxFn1 = 0;
    uint32_t psxFn2 = 0;

    // Legacy per-frame callback
    PrSceneMainFn main = nullptr;

    // Render callback
    PrSceneRenderFn render = nullptr;
};
