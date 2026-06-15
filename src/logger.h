#pragma once

#include <windows.h>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <cstdarg>
#include <vector>
#include <chrono>

namespace Log {

inline std::ofstream g_stream;
inline std::recursive_mutex g_mutex;
inline std::filesystem::path g_currentLogPath;

inline std::wstring GetExecutableDir() {
    wchar_t path[MAX_PATH];
    DWORD len = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) {
        return std::filesystem::current_path().wstring();
    }
    return std::filesystem::path(path).parent_path().wstring();
}

inline std::wstring MakeTimestampForFile() {
    SYSTEMTIME st;
    GetLocalTime(&st);

    wchar_t buf[64];
    swprintf(buf, 64, L"%04d%02d%02d_%02d%02d%02d",
             (int)st.wYear, (int)st.wMonth, (int)st.wDay,
             (int)st.wHour, (int)st.wMinute, (int)st.wSecond);
    return std::wstring(buf);
}

inline std::string MakeTimestampForLine() {
    SYSTEMTIME st;
    GetLocalTime(&st);

    char buf[64];
    sprintf_s(buf, "[%02d:%02d:%02d.%03d] ",
              (int)st.wHour, (int)st.wMinute, (int)st.wSecond,
              (int)st.wMilliseconds);
    return std::string(buf);
}

inline std::string WideToUtf8(const std::wstring& w) {
    if (w.empty()) return {};

    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    if (sizeNeeded <= 0) return {};

    std::string result;
    result.resize((size_t)sizeNeeded);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), result.data(), sizeNeeded, nullptr, nullptr);
    return result;
}

inline std::ofstream& Stream() {
    return g_stream;
}

inline std::recursive_mutex& Mutex() {
    return g_mutex;
}

inline std::filesystem::path& CurrentLogPath() {
    return g_currentLogPath;
}

inline void Init() {
    std::lock_guard<std::recursive_mutex> lock(Mutex());

    if (Stream().is_open()) return;

    std::filesystem::path dir = std::filesystem::path(GetExecutableDir()) / L"logs";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);

    std::filesystem::path& keepPath = CurrentLogPath();
    const bool havePath = !keepPath.empty();
    std::filesystem::path path = havePath ? keepPath : (dir / (MakeTimestampForFile() + L".log"));
    keepPath = path;

    const bool existed = std::filesystem::exists(path, ec);
    Stream().open(path, std::ios::binary | (existed ? std::ios::app : std::ios::out));
    if (Stream().is_open() && !existed) {
        std::string bom = "\xEF\xBB\xBF";
        Stream().write(bom.data(), (std::streamsize)bom.size());
        Stream().flush();
    }
}

inline void Shutdown() {
    std::lock_guard<std::recursive_mutex> lock(Mutex());

    if (Stream().is_open()) {
        Stream().flush();
        Stream().close();
    }
}

inline void WriteLineUtf8(const std::string& line) {
    std::lock_guard<std::recursive_mutex> lock(Mutex());

    if (!Stream().is_open()) {
        Init();
    }

    if (!Stream().is_open()) {
        OutputDebugStringA(line.c_str());
        OutputDebugStringA("\n");
        return;
    }

    std::string prefix = MakeTimestampForLine();
    Stream().write(prefix.data(), (std::streamsize)prefix.size());
    Stream().write(line.data(), (std::streamsize)line.size());
    Stream().write("\n", 1);

    {
        static uint32_t s_linesSinceFlush = 0;
        static auto s_lastFlush = std::chrono::steady_clock::now();
        s_linesSinceFlush++;
        auto now = std::chrono::steady_clock::now();
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - s_lastFlush).count();
        if (s_linesSinceFlush >= 64 || ms >= 250) {
            Stream().flush();
            s_linesSinceFlush = 0;
            s_lastFlush = now;
        }
    }

    if (IsDebuggerPresent()) {
        OutputDebugStringA(prefix.c_str());
        OutputDebugStringA(line.c_str());
        OutputDebugStringA("\n");
    }
}

inline void WriteLineW(const std::wstring& line) {
    WriteLineUtf8(WideToUtf8(line));
}

inline void Printf(const char* fmt, ...) {
    if (!fmt) return;

    va_list args;
    va_start(args, fmt);

    char buf[2048];
    vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, args);

    va_end(args);

    WriteLineUtf8(std::string(buf));
}

} // namespace Log
