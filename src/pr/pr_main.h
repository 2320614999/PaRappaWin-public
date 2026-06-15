#pragma once

#include <filesystem>

#include "pr_scene_table.h"

struct PrGameContext;

struct PrMain {
    static bool InitSceneTable(PrSceneTable& table, const std::filesystem::path& dataRoot);
    static void Run(PrGameContext& ctx, PrSceneTable& table);
    static int GetStage1Fn2ResultThisFrameDebug();
    static int GetStage1PendingSceneThisFrameDebug();
};
