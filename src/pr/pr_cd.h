#pragma once

#include <filesystem>
#include <cstddef>

struct PrDiscFile {
    std::filesystem::path path;
    size_t size = 0;
};

struct PrCD {
    static void Init();
    static bool SearchFile(const std::filesystem::path& root, const char* name, PrDiscFile& out);
    static bool ReadFile(const PrDiscFile& file, void* dst, size_t size);
};
