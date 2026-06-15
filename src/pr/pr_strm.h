#pragma once

#include <filesystem>

struct PrGameContext;

struct PrStrm {
    static void Init(PrGameContext& ctx);
    static void Shutdown(PrGameContext& ctx);

    static bool Open(const std::filesystem::path& path, PrGameContext& ctx);
    static void Close(PrGameContext& ctx);
    static void Update(PrGameContext& ctx);
};
