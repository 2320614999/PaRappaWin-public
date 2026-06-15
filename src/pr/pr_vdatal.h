#pragma once

#include <filesystem>

struct PrGameContext;

struct PrVdatal {
    static void Init(PrGameContext& ctx);
    static void Shutdown(PrGameContext& ctx);

    static bool Load(const std::filesystem::path& root, PrGameContext& ctx);
    static void Unload(PrGameContext& ctx);
};
