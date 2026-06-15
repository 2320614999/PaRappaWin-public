#include "pr_cd.h"

#include <algorithm>
#include <cctype>
#include <fstream>

namespace {

static std::string NormalizePsxPath(const char* name) {
    if (!name) return {};

    std::string s(name);

    while (!s.empty() && (s[0] == '\\' || s[0] == '/')) {
        s.erase(s.begin());
    }

    for (char& c : s) {
        if (c == '\\') c = '/';
    }

    const size_t semi = s.find(';');
    if (semi != std::string::npos) {
        s.resize(semi);
    }

    while (!s.empty() && s.back() == ' ') {
        s.pop_back();
    }

    return s;
}

static std::string ToLowerAscii(const std::string& s) {
    std::string out = s;
    for (char& c : out) {
        if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
    }
    return out;
}

static std::string NormalizeBasename(const std::string& s) {
    std::string t = s;
    const size_t semi = t.find(';');
    if (semi != std::string::npos) {
        t.resize(semi);
    }
    while (!t.empty() && (t.back() == ' ')) {
        t.pop_back();
    }
    return ToLowerAscii(t);
}

} // namespace

void PrCD::Init() {
}

bool PrCD::SearchFile(const std::filesystem::path& root, const char* name, PrDiscFile& out) {
    out = {};

    const std::string norm = NormalizePsxPath(name);
    if (norm.empty()) {
        return false;
    }

    std::error_code ec;

    const std::filesystem::path direct = root / std::filesystem::path(norm);
    if (std::filesystem::exists(direct, ec) && std::filesystem::is_regular_file(direct, ec)) {
        out.path = direct;
        out.size = (size_t)std::filesystem::file_size(direct, ec);
        return true;
    }

    const std::filesystem::path base = std::filesystem::path(norm).filename();
    const std::string want = NormalizeBasename(base.u8string());

    if (!std::filesystem::exists(root, ec)) {
        return false;
    }

    for (auto it = std::filesystem::recursive_directory_iterator(root, ec);
         it != std::filesystem::recursive_directory_iterator();
         it.increment(ec)) {
        if (ec) {
            break;
        }
        if (!it->is_regular_file(ec)) {
            continue;
        }

        const std::string got = NormalizeBasename(it->path().filename().u8string());
        if (got == want) {
            out.path = it->path();
            out.size = (size_t)it->file_size(ec);
            return true;
        }
    }

    return false;
}

bool PrCD::ReadFile(const PrDiscFile& file, void* dst, size_t size) {
    if (!dst || size == 0) {
        return false;
    }

    std::ifstream f(file.path, std::ios::binary);
    if (!f.is_open()) {
        return false;
    }

    f.read(reinterpret_cast<char*>(dst), (std::streamsize)size);
    return f.good() && (size_t)f.gcount() == size;
}
