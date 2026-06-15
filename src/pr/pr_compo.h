#pragma once

#include <filesystem>

struct PrGameContext;

struct PrCompo {
    static bool LoadCommon(const std::filesystem::path& root, PrGameContext& ctx);
    static bool LoadForScene(const std::filesystem::path& root, int sceneNumber, PrGameContext& ctx);
    static void Unload(PrGameContext& ctx);
};
