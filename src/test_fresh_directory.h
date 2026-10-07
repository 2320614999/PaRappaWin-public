#pragma once

#include <windows.h>
#include <filesystem>
#include <stdexcept>
#include <string>

// Test fixtures are retained for diagnosis. Never adopt or clean up an existing
// directory: a process crash must not put another process's data at risk.
inline std::filesystem::path CreateFreshTestDirectory(const char* prefix)
{
    std::error_code ec;
    const auto base = std::filesystem::temp_directory_path(ec);
    if (ec || !base.is_absolute())
        throw std::runtime_error("test temporary directory unavailable");
    const auto identity = std::string(prefix) + "-" +
        std::to_string(GetCurrentProcessId()) + "-" +
        std::to_string(GetTickCount64()) + "-";
    for (unsigned attempt = 0; attempt < 128; ++attempt) {
        const auto candidate = base / (identity + std::to_string(attempt));
        ec.clear();
        if (std::filesystem::create_directory(candidate, ec))
            return candidate;
        if (ec && ec != std::errc::file_exists)
            throw std::runtime_error("cannot create fresh test directory");
    }
    throw std::runtime_error("cannot allocate unique test directory");
}
