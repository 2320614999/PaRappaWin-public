#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

constexpr const wchar_t* kDefaultImage = L"PaRappaWin.exe";
constexpr uint32_t kDrawPacketRva = 0x00192980u;
constexpr uint32_t kDrawTriangleRva = 0x001939F0u;
constexpr uint32_t kFlushSpritesRva = 0x00015860u;
constexpr size_t kPacketSize = 188u;
constexpr size_t kTriangleSize = 100u;
constexpr int kSceneMapModelSet = 1;

struct Options {
    DWORD pid = 0;
    std::wstring image = kDefaultImage;
    std::wstring outPath = L"artifacts\\stage1_mem_probe\\stage1_triangle_trace.jsonl";
    uint32_t drawPacketRva = kDrawPacketRva;
    uint32_t drawTriangleRva = kDrawTriangleRva;
    uint32_t flushSpritesRva = kFlushSpritesRva;
    uint64_t maxEvents = 20000;
    bool sceneMapOnly = false;
    bool includeFlush = false;
};

struct Breakpoint {
    uint64_t address = 0;
    const char* name = "";
};

struct ModuleInfo {
    uint64_t base = 0;
    uint32_t size = 0;
};

struct PacketRecord {
    bool known = false;
    bool valid = false;
    uint8_t handler = 0;
    uint8_t primitiveCode = 0;
    uint8_t primitiveFlag = 0;
    bool rawPacketKnown = false;
    uint32_t rawPrimitiveIndex = 0;
    uint32_t rawPacketOffset = 0;
    bool modelRefKnown = false;
    uint32_t descAddr = 0;
    uint8_t modelSet = 0;
    uint16_t modelIndex = 0;
    uint8_t wordCount = 0;
    std::array<uint32_t, 13> words{};
    uint16_t otz = 0;
    bool projectedScreenKnown = false;
    std::array<float, 4> sx{};
    std::array<float, 4> sy{};
    std::array<float, 4> sz{};
    int32_t otBucket = 0;
    uint8_t workListFlushOrder = 0;
    uint32_t psxCallOrder = 0;
    bool semiTransparent = false;
    uint8_t abr = 0;
};

struct TriangleRecord {
    std::array<float, 3> x{};
    std::array<float, 3> y{};
    std::array<uint8_t, 3> u{};
    std::array<uint8_t, 3> v{};
    bool textured = false;
    uint16_t tpage = 0;
    uint16_t clut = 0;
    std::array<float, 3> r{};
    std::array<float, 3> g{};
    std::array<float, 3> b{};
    std::array<float, 3> a{};
    std::array<float, 3> perspectiveW{};
    uint8_t blend = 0;
};

struct Viewport {
    float x = 0.0f;
    float y = 0.0f;
    float scale = 1.0f;
};

struct PacketGeometry {
    bool known = false;
    bool quad = false;
    uint8_t vertexCount = 0;
    std::array<float, 4> x{};
    std::array<float, 4> y{};
};

struct PacketThreadState {
    PacketRecord packet{};
    uint64_t packetOrdinalInFrame = 0;
    uint8_t nextTriangleIndex = 0;
};

template <typename T>
T ReadLe(const std::vector<uint8_t>& data, size_t offset) {
    T out{};
    if (offset + sizeof(T) <= data.size()) {
        std::memcpy(&out, data.data() + offset, sizeof(T));
    }
    return out;
}

bool ReadProcessBytes(HANDLE process, uint64_t address, void* out, size_t size) {
    SIZE_T read = 0;
    return ReadProcessMemory(process,
                             reinterpret_cast<LPCVOID>(address),
                             out,
                             size,
                             &read) &&
           read == size;
}

template <typename T>
bool ReadProcessValue(HANDLE process, uint64_t address, T& out) {
    return ReadProcessBytes(process, address, &out, sizeof(T));
}

std::vector<uint8_t> ReadBlob(HANDLE process, uint64_t address, size_t size) {
    std::vector<uint8_t> out(size);
    if (!ReadProcessBytes(process, address, out.data(), out.size())) {
        out.clear();
    }
    return out;
}

PacketRecord DecodePacket(const std::vector<uint8_t>& data) {
    PacketRecord p{};
    if (data.size() < kPacketSize) {
        return p;
    }
    p.known = true;
    p.valid = ReadLe<uint8_t>(data, 0) != 0;
    p.handler = ReadLe<uint8_t>(data, 1);
    p.primitiveCode = ReadLe<uint8_t>(data, 4);
    p.primitiveFlag = ReadLe<uint8_t>(data, 5);
    p.rawPacketKnown = ReadLe<uint8_t>(data, 7) != 0;
    p.rawPrimitiveIndex = ReadLe<uint32_t>(data, 8);
    p.rawPacketOffset = ReadLe<uint32_t>(data, 12);
    p.wordCount = ReadLe<uint8_t>(data, 46);
    for (size_t i = 0; i < p.words.size(); ++i) {
        p.words[i] = ReadLe<uint32_t>(data, 48 + i * 4);
    }
    p.otz = ReadLe<uint16_t>(data, 100);
    p.projectedScreenKnown = ReadLe<uint8_t>(data, 120) != 0;
    for (size_t i = 0; i < 4; ++i) {
        p.sx[i] = ReadLe<float>(data, 124 + i * 4);
        p.sy[i] = ReadLe<float>(data, 140 + i * 4);
        p.sz[i] = ReadLe<float>(data, 156 + i * 4);
    }
    p.otBucket = ReadLe<int32_t>(data, 172);
    p.workListFlushOrder = ReadLe<uint8_t>(data, 176);
    p.psxCallOrder = ReadLe<uint32_t>(data, 180);
    p.semiTransparent = ReadLe<uint8_t>(data, 184) != 0;
    p.abr = ReadLe<uint8_t>(data, 185);
    return p;
}

TriangleRecord DecodeTriangle(const std::vector<uint8_t>& data) {
    TriangleRecord t{};
    if (data.size() < kTriangleSize) {
        return t;
    }
    for (size_t i = 0; i < 3; ++i) {
        t.x[i] = ReadLe<float>(data, 0 + i * 4);
        t.y[i] = ReadLe<float>(data, 12 + i * 4);
        t.u[i] = ReadLe<uint8_t>(data, 24 + i);
        t.v[i] = ReadLe<uint8_t>(data, 27 + i);
        t.r[i] = ReadLe<float>(data, 36 + i * 4);
        t.g[i] = ReadLe<float>(data, 48 + i * 4);
        t.b[i] = ReadLe<float>(data, 60 + i * 4);
        t.a[i] = ReadLe<float>(data, 72 + i * 4);
        t.perspectiveW[i] = ReadLe<float>(data, 84 + i * 4);
    }
    t.textured = ReadLe<uint8_t>(data, 30) != 0;
    t.tpage = ReadLe<uint16_t>(data, 32);
    t.clut = ReadLe<uint16_t>(data, 34);
    t.blend = ReadLe<uint8_t>(data, 96);
    return t;
}

Viewport DecodeViewport(const std::vector<uint8_t>& data) {
    Viewport v{};
    if (data.size() >= 12) {
        v.x = ReadLe<float>(data, 0);
        v.y = ReadLe<float>(data, 4);
        v.scale = ReadLe<float>(data, 8);
    }
    return v;
}

float SxyWordX(uint32_t word) {
    return static_cast<float>(static_cast<int16_t>(word & 0xFFFFu));
}

float SxyWordY(uint32_t word) {
    return static_cast<float>(static_cast<int16_t>((word >> 16) & 0xFFFFu));
}

PacketGeometry DecodePacketGeometry(const PacketRecord& p) {
    PacketGeometry g{};
    if (p.wordCount <= 1u || p.words.empty()) {
        return g;
    }

    constexpr uint8_t kGp0BaseWord = 1u;
    const uint8_t op =
        static_cast<uint8_t>((p.words[kGp0BaseWord] >> 24) & 0xFFu);
    if ((op & 0xE0u) != 0x20u) {
        return g;
    }

    const bool textured = (op & 0x04u) != 0u;
    const bool quad = (op & 0x08u) != 0u;
    const bool gouraud = (op & 0x10u) != 0u;
    const uint8_t vertexCount = quad ? 4u : 3u;

    uint8_t wordIndex = kGp0BaseWord + 1u;
    for (uint8_t vi = 0; vi < vertexCount; ++vi) {
        if (wordIndex >= p.wordCount || wordIndex >= p.words.size()) {
            return PacketGeometry{};
        }
        g.x[vi] = SxyWordX(p.words[wordIndex]);
        g.y[vi] = SxyWordY(p.words[wordIndex]);
        ++wordIndex;
        if (textured) {
            ++wordIndex;
        }
        if (gouraud && vi + 1u < vertexCount) {
            ++wordIndex;
        }
    }

    g.known = true;
    g.quad = quad;
    g.vertexCount = vertexCount;
    return g;
}

std::array<uint8_t, 3> PacketTriangleSlots(const PacketGeometry& g,
                                           uint8_t triangleIndex) {
    if (g.quad && triangleIndex == 1u) {
        return {1u, 3u, 2u};
    }
    return {0u, 1u, 2u};
}

bool IsSceneMapPacket(const PacketRecord& p) {
    return p.modelRefKnown && p.modelSet == kSceneMapModelSet;
}

const char* DescName(uint32_t desc) {
    switch (desc) {
    case 0x801D7EF0u: return "WAKU_L";
    case 0x801D7F00u: return "WAKU_R";
    case 0x801D7FD8u: return "MADO_L";
    case 0x801D7FE8u: return "MADO_R";
    case 0x801D7F10u: return "DONDE_L";
    case 0x801D7F20u: return "DONDE_R";
    case 0x801D7FB8u: return "KAMON_L";
    case 0x801D7FC8u: return "KAMON_R";
    case 0x801D7F88u: return "KABE_R";
    case 0x801D7F98u: return "KABE_L";
    case 0x801D7FA8u: return "KABE_B";
    case 0x801D7F78u: return "TENNJOU";
    default: return "";
    }
}

std::array<float, 3> ApplyCoverageBiasX(const Viewport& viewport,
                                        std::array<float, 3> x,
                                        std::array<float, 3>& y) {
    if (viewport.scale <= 1.0f) {
        return x;
    }
    float bias = 0.5f * (1.0f - 1.0f / viewport.scale);
    if (bias > 0.5f) {
        bias = 0.5f;
    }
    if (bias <= 0.0f) {
        return x;
    }
    const float cx = (x[0] + x[1] + x[2]) / 3.0f;
    const float cy = (y[0] + y[1] + y[2]) / 3.0f;
    for (size_t i = 0; i < 3; ++i) {
        const float dx = x[i] - cx;
        const float dy = y[i] - cy;
        const float len = std::sqrt(dx * dx + dy * dy);
        if (len <= 0.0001f) {
            continue;
        }
        const float s = (len + bias) / len;
        x[i] = cx + dx * s;
        y[i] = cy + dy * s;
    }
    return x;
}

std::string Hex32(uint32_t v) {
    std::ostringstream os;
    os << "\"0x" << std::uppercase << std::hex << std::setw(8)
       << std::setfill('0') << v << "\"";
    return os.str();
}

std::string Hex64(uint64_t v) {
    std::ostringstream os;
    os << "\"0x" << std::uppercase << std::hex << v << "\"";
    return os.str();
}

void EmitFloatArray(std::ostream& os, const std::array<float, 3>& values) {
    os << '[' << values[0] << ',' << values[1] << ',' << values[2] << ']';
}

void EmitFloat4Array(std::ostream& os, const std::array<float, 4>& values) {
    os << '[' << values[0] << ',' << values[1] << ',' << values[2] << ','
       << values[3] << ']';
}

void EmitByteArray(std::ostream& os, const std::array<uint8_t, 3>& values) {
    os << '[' << static_cast<int>(values[0]) << ','
       << static_cast<int>(values[1]) << ',' << static_cast<int>(values[2])
       << ']';
}

void EmitPacketJson(std::ostream& os,
                    uint64_t seq,
                    uint64_t frameIndex,
                    uint64_t eventOrdinalInFrame,
                    uint64_t packetOrdinalInFrame,
                    DWORD tid,
                    const PacketRecord& p,
                    uint64_t packetPtr) {
    const PacketGeometry g = DecodePacketGeometry(p);
    os << "{\"seq\":" << seq << ",\"kind\":\"packet\",\"tid\":" << tid
       << ",\"frame\":" << frameIndex
       << ",\"eventInFrame\":" << eventOrdinalInFrame
       << ",\"packetOrdinalInFrame\":" << packetOrdinalInFrame
       << ",\"packetPtr\":" << Hex64(packetPtr)
       << ",\"valid\":" << (p.valid ? "true" : "false")
       << ",\"handler\":" << static_cast<int>(p.handler)
       << ",\"primitiveCode\":" << static_cast<int>(p.primitiveCode)
       << ",\"primitiveFlag\":" << static_cast<int>(p.primitiveFlag)
       << ",\"rawPrimitiveIndex\":" << p.rawPrimitiveIndex
       << ",\"rawPacketOffset\":" << p.rawPacketOffset
       << ",\"descAddr\":" << Hex32(p.descAddr)
       << ",\"descName\":\"" << DescName(p.descAddr) << "\""
       << ",\"modelSet\":" << static_cast<int>(p.modelSet)
       << ",\"modelIndex\":" << p.modelIndex
       << ",\"wordCount\":" << static_cast<int>(p.wordCount)
       << ",\"otz\":" << p.otz
       << ",\"otBucket\":" << p.otBucket
       << ",\"workListFlushOrder\":" << static_cast<int>(p.workListFlushOrder)
       << ",\"psxCallOrder\":" << p.psxCallOrder
       << ",\"semiTransparent\":" << (p.semiTransparent ? "true" : "false")
       << ",\"abr\":" << static_cast<int>(p.abr)
       << ",\"words\":[";
    const size_t wc = (std::min<size_t>)(p.wordCount, p.words.size());
    for (size_t i = 0; i < wc; ++i) {
        if (i) os << ',';
        os << Hex32(p.words[i]);
    }
    os << "],\"packetGeometryKnown\":" << (g.known ? "true" : "false")
       << ",\"packetVertexCount\":" << static_cast<int>(g.vertexCount)
       << ",\"packetX\":";
    EmitFloat4Array(os, g.x);
    os << ",\"packetY\":";
    EmitFloat4Array(os, g.y);
    os << ",\"projectedScreenKnown\":"
       << (p.projectedScreenKnown ? "true" : "false") << ",\"projectedX\":";
    EmitFloat4Array(os, p.sx);
    os << ",\"projectedY\":";
    EmitFloat4Array(os, p.sy);
    os << ",\"projectedZ\":";
    EmitFloat4Array(os, p.sz);
    os << "}\n";
}

void EmitTriangleJson(std::ostream& os,
                      uint64_t seq,
                      uint64_t frameIndex,
                      uint64_t eventOrdinalInFrame,
                      uint64_t packetOrdinalInFrame,
                      uint64_t triangleOrdinalInFrame,
                      uint8_t triangleIndexInPacket,
                      DWORD tid,
                      const PacketRecord& p,
                      const TriangleRecord& t,
                      const Viewport& v,
                      int layer,
                      uint64_t order,
                      bool coverageBiasAllowed,
                      uint64_t trianglePtr) {
    const PacketGeometry g = DecodePacketGeometry(p);
    const std::array<uint8_t, 3> slots =
        PacketTriangleSlots(g, triangleIndexInPacket);
    std::array<float, 3> packetX{};
    std::array<float, 3> packetY{};
    std::array<float, 3> projectedX{};
    std::array<float, 3> projectedY{};
    std::array<float, 3> projectedZ{};
    for (size_t i = 0; i < 3; ++i) {
        const uint8_t slot = slots[i];
        if (g.known && slot < g.vertexCount) {
            packetX[i] = g.x[slot];
            packetY[i] = g.y[slot];
        }
        if (slot < p.sx.size()) {
            projectedX[i] = p.sx[slot];
            projectedY[i] = p.sy[slot];
            projectedZ[i] = p.sz[slot];
        }
    }

    std::array<float, 3> adjustedY = t.y;
    std::array<float, 3> adjustedX = t.x;
    if (coverageBiasAllowed) {
        adjustedX = ApplyCoverageBiasX(v, adjustedX, adjustedY);
    }
    std::array<float, 3> screenX{};
    std::array<float, 3> screenY{};
    for (size_t i = 0; i < 3; ++i) {
        screenX[i] = v.x + adjustedX[i] * v.scale;
        screenY[i] = v.y + adjustedY[i] * v.scale;
    }

    os << "{\"seq\":" << seq << ",\"kind\":\"triangle\",\"tid\":" << tid
       << ",\"frame\":" << frameIndex
       << ",\"eventInFrame\":" << eventOrdinalInFrame
       << ",\"packetOrdinalInFrame\":" << packetOrdinalInFrame
       << ",\"triangleOrdinalInFrame\":" << triangleOrdinalInFrame
       << ",\"triangleIndexInPacket\":"
       << static_cast<int>(triangleIndexInPacket)
       << ",\"trianglePtr\":" << Hex64(trianglePtr)
       << ",\"descAddr\":" << Hex32(p.descAddr)
       << ",\"descName\":\"" << DescName(p.descAddr) << "\""
       << ",\"modelSet\":" << static_cast<int>(p.modelSet)
       << ",\"modelIndex\":" << p.modelIndex
       << ",\"handler\":" << static_cast<int>(p.handler)
       << ",\"rawPrimitiveIndex\":" << p.rawPrimitiveIndex
       << ",\"psxCallOrder\":" << p.psxCallOrder
       << ",\"semiTransparent\":" << (p.semiTransparent ? "true" : "false")
       << ",\"layer\":" << layer
       << ",\"order\":" << order
       << ",\"coverageBiasAllowed\":"
       << (coverageBiasAllowed ? "true" : "false")
       << ",\"packetGeometryKnown\":" << (g.known ? "true" : "false")
       << ",\"packetVertexCount\":" << static_cast<int>(g.vertexCount)
       << ",\"slots\":";
    EmitByteArray(os, slots);
    os << ",\"packetX\":";
    EmitFloatArray(os, packetX);
    os << ",\"packetY\":";
    EmitFloatArray(os, packetY);
    os << ",\"projectedScreenKnown\":"
       << (p.projectedScreenKnown ? "true" : "false")
       << ",\"projectedX\":";
    EmitFloatArray(os, projectedX);
    os << ",\"projectedY\":";
    EmitFloatArray(os, projectedY);
    os << ",\"projectedZ\":";
    EmitFloatArray(os, projectedZ);
    os
       << ",\"viewport\":{\"x\":" << v.x << ",\"y\":" << v.y
       << ",\"scale\":" << v.scale << "},\"x\":";
    EmitFloatArray(os, t.x);
    os << ",\"y\":";
    EmitFloatArray(os, t.y);
    os << ",\"screenX\":";
    EmitFloatArray(os, screenX);
    os << ",\"screenY\":";
    EmitFloatArray(os, screenY);
    os << ",\"u\":";
    EmitByteArray(os, t.u);
    os << ",\"v\":";
    EmitByteArray(os, t.v);
    os << ",\"textured\":" << (t.textured ? "true" : "false")
       << ",\"tpage\":" << t.tpage << ",\"clut\":" << t.clut
       << ",\"a\":";
    EmitFloatArray(os, t.a);
    os << ",\"perspectiveW\":";
    EmitFloatArray(os, t.perspectiveW);
    os << ",\"blend\":" << static_cast<int>(t.blend) << "}\n";
}

void EmitFlushJson(std::ostream& os,
                   uint64_t seq,
                   uint64_t frameIndex,
                   uint64_t eventOrdinalInFrame,
                   DWORD tid,
                   HANDLE process,
                   uint64_t rendererPtr) {
    uint64_t first = 0;
    uint64_t last = 0;
    uint64_t end = 0;
    ReadProcessValue(process, rendererPtr + 208, first);
    ReadProcessValue(process, rendererPtr + 216, last);
    ReadProcessValue(process, rendererPtr + 224, end);
    const uint64_t count =
        (last >= first) ? ((last - first) / 560u) : 0u;
    os << "{\"seq\":" << seq << ",\"kind\":\"flush\",\"tid\":" << tid
       << ",\"frame\":" << frameIndex
       << ",\"eventInFrame\":" << eventOrdinalInFrame
       << ",\"rendererPtr\":" << Hex64(rendererPtr)
       << ",\"queueFirst\":" << Hex64(first)
       << ",\"queueLast\":" << Hex64(last)
       << ",\"queueEnd\":" << Hex64(end)
       << ",\"queueCount\":" << count << "}\n";
}

std::wstring GetArgValue(int& i, int argc, wchar_t** argv) {
    if (i + 1 >= argc) {
        return L"";
    }
    ++i;
    return argv[i];
}

uint64_t ParseU64(const std::wstring& s) {
    return std::wcstoull(s.c_str(), nullptr, 0);
}

bool ParseArgs(int argc, wchar_t** argv, Options& opt) {
    for (int i = 1; i < argc; ++i) {
        std::wstring a = argv[i];
        if (a == L"--pid") {
            opt.pid = static_cast<DWORD>(ParseU64(GetArgValue(i, argc, argv)));
        } else if (a == L"--image") {
            opt.image = GetArgValue(i, argc, argv);
        } else if (a == L"--out") {
            opt.outPath = GetArgValue(i, argc, argv);
        } else if (a == L"--max-events") {
            opt.maxEvents = ParseU64(GetArgValue(i, argc, argv));
        } else if (a == L"--draw-packet-rva") {
            opt.drawPacketRva =
                static_cast<uint32_t>(ParseU64(GetArgValue(i, argc, argv)));
        } else if (a == L"--draw-triangle-rva") {
            opt.drawTriangleRva =
                static_cast<uint32_t>(ParseU64(GetArgValue(i, argc, argv)));
        } else if (a == L"--flush-rva") {
            opt.flushSpritesRva =
                static_cast<uint32_t>(ParseU64(GetArgValue(i, argc, argv)));
        } else if (a == L"--all-models") {
            opt.sceneMapOnly = false;
        } else if (a == L"--include-flush") {
            opt.includeFlush = true;
        } else if (a == L"--help" || a == L"-h") {
            return false;
        } else {
            std::wcerr << L"unknown arg: " << a << L"\n";
            return false;
        }
    }
    return true;
}

void PrintUsage(const wchar_t* exe) {
    std::wcerr
        << L"usage: " << exe
        << L" --pid <pid> [--out trace.jsonl] [--max-events N]\n"
        << L"       [--include-flush]\n"
        << L"       [--draw-packet-rva 0x...] [--draw-triangle-rva 0x...]\n";
}

std::vector<DWORD> FindPidsByImage(const std::wstring& image) {
    std::vector<DWORD> out;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        return out;
    }
    PROCESSENTRY32W pe{};
    pe.dwSize = sizeof(pe);
    for (BOOL ok = Process32FirstW(snap, &pe); ok; ok = Process32NextW(snap, &pe)) {
        if (_wcsicmp(pe.szExeFile, image.c_str()) == 0) {
            out.push_back(pe.th32ProcessID);
        }
    }
    CloseHandle(snap);
    return out;
}

ModuleInfo FindModule(DWORD pid, const std::wstring& image) {
    ModuleInfo out{};
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE) {
        return out;
    }
    MODULEENTRY32W me{};
    me.dwSize = sizeof(me);
    for (BOOL ok = Module32FirstW(snap, &me); ok; ok = Module32NextW(snap, &me)) {
        if (_wcsicmp(me.szModule, image.c_str()) == 0) {
            out.base = reinterpret_cast<uint64_t>(me.modBaseAddr);
            out.size = me.modBaseSize;
            break;
        }
    }
    CloseHandle(snap);
    return out;
}

std::vector<DWORD> EnumerateThreads(DWORD pid) {
    std::vector<DWORD> out;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        return out;
    }
    THREADENTRY32 te{};
    te.dwSize = sizeof(te);
    for (BOOL ok = Thread32First(snap, &te); ok; ok = Thread32Next(snap, &te)) {
        if (te.th32OwnerProcessID == pid) {
            out.push_back(te.th32ThreadID);
        }
    }
    CloseHandle(snap);
    return out;
}

bool SetBreakpointsForThread(DWORD tid, const std::vector<Breakpoint>& bps) {
    HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_SET_CONTEXT |
                                   THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION,
                               FALSE,
                               tid);
    if (!thread) {
        return false;
    }
    SuspendThread(thread);
    CONTEXT ctx{};
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    const bool got = GetThreadContext(thread, &ctx) != FALSE;
    if (got) {
        ctx.Dr0 = bps.size() > 0 ? bps[0].address : 0;
        ctx.Dr1 = bps.size() > 1 ? bps[1].address : 0;
        ctx.Dr2 = bps.size() > 2 ? bps[2].address : 0;
        ctx.Dr3 = bps.size() > 3 ? bps[3].address : 0;
        ctx.Dr7 &= ~0xFFFF00FFull;
        for (size_t i = 0; i < bps.size() && i < 4; ++i) {
            ctx.Dr7 |= (1ull << (i * 2));
        }
        ctx.Dr6 = 0;
    }
    const bool set = got && SetThreadContext(thread, &ctx) != FALSE;
    ResumeThread(thread);
    CloseHandle(thread);
    return set;
}

bool GetThreadCtx(DWORD tid, CONTEXT& ctx) {
    HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, tid);
    if (!thread) {
        return false;
    }
    ctx = {};
    ctx.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
    const bool ok = GetThreadContext(thread, &ctx) != FALSE;
    CloseHandle(thread);
    return ok;
}

bool SetThreadCtx(DWORD tid, const CONTEXT& ctx) {
    HANDLE thread = OpenThread(THREAD_SET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, tid);
    if (!thread) {
        return false;
    }
    CONTEXT writable = ctx;
    writable.ContextFlags = CONTEXT_CONTROL;
    const bool ok = SetThreadContext(thread, &writable) != FALSE;
    CloseHandle(thread);
    return ok;
}

const Breakpoint* FindBreakpoint(uint64_t rip, const std::vector<Breakpoint>& bps) {
    for (const auto& bp : bps) {
        if (bp.address == rip) {
            return &bp;
        }
    }
    return nullptr;
}

bool EnsureParentDir(const std::wstring& path) {
    const size_t slash = path.find_last_of(L"\\/");
    if (slash == std::wstring::npos) {
        return true;
    }
    const std::wstring dir = path.substr(0, slash);
    if (dir.empty()) {
        return true;
    }
    std::wstring cur;
    for (wchar_t ch : dir) {
        cur.push_back(ch);
        if (ch == L'\\' || ch == L'/') {
            if (cur.size() > 2) {
                CreateDirectoryW(cur.c_str(), nullptr);
            }
        }
    }
    CreateDirectoryW(dir.c_str(), nullptr);
    return true;
}

}  // namespace

int wmain(int argc, wchar_t** argv) {
    Options opt{};
    if (!ParseArgs(argc, argv, opt)) {
        PrintUsage(argv[0]);
        return 2;
    }
    if (opt.pid == 0) {
        auto pids = FindPidsByImage(opt.image);
        if (pids.empty()) {
            std::wcerr << L"process not found: " << opt.image << L"\n";
            return 2;
        }
        opt.pid = pids.front();
    }

    const ModuleInfo module = FindModule(opt.pid, opt.image);
    if (!module.base) {
        std::wcerr << L"module not found in pid " << opt.pid << L": "
                   << opt.image << L"\n";
        return 2;
    }

    std::vector<Breakpoint> bps = {
        {module.base + opt.drawPacketRva, "packet"},
        {module.base + opt.drawTriangleRva, "triangle"},
    };
    if (opt.includeFlush) {
        bps.push_back({module.base + opt.flushSpritesRva, "flush"});
    }
    if (bps.size() > 4) {
        std::wcerr << L"too many hardware breakpoints\n";
        return 2;
    }

    HANDLE process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
                                 FALSE,
                                 opt.pid);
    if (!process) {
        std::wcerr << L"OpenProcess failed: " << GetLastError() << L"\n";
        return 1;
    }

    EnsureParentDir(opt.outPath);
    std::ofstream out(opt.outPath, std::ios::binary | std::ios::trunc);
    if (!out) {
        std::wcerr << L"cannot open output: " << opt.outPath << L"\n";
        CloseHandle(process);
        return 1;
    }

    if (!DebugActiveProcess(opt.pid)) {
        std::wcerr << L"DebugActiveProcess failed: " << GetLastError() << L"\n";
        CloseHandle(process);
        return 1;
    }
    DebugSetProcessKillOnExit(FALSE);

    for (DWORD tid : EnumerateThreads(opt.pid)) {
        SetBreakpointsForThread(tid, bps);
    }

    std::unordered_map<DWORD, PacketThreadState> lastPacketByThread;
    uint64_t seq = 0;
    uint64_t frameIndex = 0;
    uint64_t eventOrdinalInFrame = 0;
    uint64_t packetOrdinalInFrame = 0;
    uint64_t triangleOrdinalInFrame = 0;
    bool running = true;
    std::wcerr << L"attached pid=" << opt.pid << L" base=0x" << std::hex
               << module.base << std::dec << L" out=" << opt.outPath << L"\n";

    while (running && seq < opt.maxEvents) {
        DEBUG_EVENT ev{};
        if (!WaitForDebugEvent(&ev, 1000)) {
            continue;
        }
        DWORD continueStatus = DBG_CONTINUE;
        if (ev.dwDebugEventCode == CREATE_THREAD_DEBUG_EVENT) {
            SetBreakpointsForThread(ev.dwThreadId, bps);
            if (ev.u.CreateThread.hThread) {
                CloseHandle(ev.u.CreateThread.hThread);
            }
        } else if (ev.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
            SetBreakpointsForThread(ev.dwThreadId, bps);
            if (ev.u.CreateProcessInfo.hFile) {
                CloseHandle(ev.u.CreateProcessInfo.hFile);
            }
            if (ev.u.CreateProcessInfo.hProcess) {
                CloseHandle(ev.u.CreateProcessInfo.hProcess);
            }
            if (ev.u.CreateProcessInfo.hThread) {
                CloseHandle(ev.u.CreateProcessInfo.hThread);
            }
        } else if (ev.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) {
            running = false;
        } else if (ev.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
            const DWORD code = ev.u.Exception.ExceptionRecord.ExceptionCode;
            if (code == EXCEPTION_SINGLE_STEP) {
                CONTEXT ctx{};
                if (GetThreadCtx(ev.dwThreadId, ctx)) {
                    const Breakpoint* bp = FindBreakpoint(ctx.Rip, bps);
                    if (bp != nullptr && std::strcmp(bp->name, "packet") == 0) {
                        ctx.EFlags |= 0x10000u; // RF: resume past this execution breakpoint.
                        SetThreadCtx(ev.dwThreadId, ctx);
                        PacketRecord p =
                            DecodePacket(ReadBlob(process, ctx.Rdx, kPacketSize));
                        PacketThreadState state{};
                        state.packet = p;
                        state.packetOrdinalInFrame = packetOrdinalInFrame;
                        state.nextTriangleIndex = 0;
                        lastPacketByThread[ev.dwThreadId] = state;
                        if (!opt.sceneMapOnly || IsSceneMapPacket(p)) {
                            EmitPacketJson(out,
                                           seq++,
                                           frameIndex,
                                           eventOrdinalInFrame++,
                                           packetOrdinalInFrame,
                                           ev.dwThreadId,
                                           p,
                                           ctx.Rdx);
                        }
                        ++packetOrdinalInFrame;
                    } else if (bp != nullptr &&
                               std::strcmp(bp->name, "triangle") == 0) {
                        ctx.EFlags |= 0x10000u; // RF: resume past this execution breakpoint.
                        SetThreadCtx(ev.dwThreadId, ctx);
                        PacketRecord p{};
                        uint64_t packetOrdinal = packetOrdinalInFrame;
                        uint8_t triangleIndexInPacket = 0;
                        auto it = lastPacketByThread.find(ev.dwThreadId);
                        if (it != lastPacketByThread.end()) {
                            p = it->second.packet;
                            packetOrdinal = it->second.packetOrdinalInFrame;
                            triangleIndexInPacket =
                                it->second.nextTriangleIndex++;
                        }
                        if (!opt.sceneMapOnly || IsSceneMapPacket(p)) {
                            TriangleRecord t = DecodeTriangle(
                                ReadBlob(process, ctx.Rdx, kTriangleSize));
                            Viewport v = DecodeViewport(ReadBlob(process, ctx.R8, 12));
                            uint64_t order = 0;
                            uint64_t coverageRaw = 0;
                            ReadProcessValue(process, ctx.Rsp + 0x28, order);
                            ReadProcessValue(process, ctx.Rsp + 0x30, coverageRaw);
                            EmitTriangleJson(out,
                                             seq++,
                                             frameIndex,
                                             eventOrdinalInFrame++,
                                             packetOrdinal,
                                             triangleOrdinalInFrame,
                                             triangleIndexInPacket,
                                             ev.dwThreadId,
                                             p,
                                             t,
                                             v,
                                             static_cast<int>(ctx.R9),
                                             order,
                                             (coverageRaw & 0xFFu) != 0,
                                             ctx.Rdx);
                        }
                        ++triangleOrdinalInFrame;
                    } else if (bp != nullptr &&
                               std::strcmp(bp->name, "flush") == 0) {
                        ctx.EFlags |= 0x10000u; // RF: resume past this execution breakpoint.
                        SetThreadCtx(ev.dwThreadId, ctx);
                        EmitFlushJson(out,
                                      seq++,
                                      frameIndex,
                                      eventOrdinalInFrame++,
                                      ev.dwThreadId,
                                      process,
                                      ctx.Rcx);
                        ++frameIndex;
                        eventOrdinalInFrame = 0;
                        packetOrdinalInFrame = 0;
                        triangleOrdinalInFrame = 0;
                        lastPacketByThread.clear();
                    }
                }
            } else {
                continueStatus = DBG_EXCEPTION_NOT_HANDLED;
            }
        }
        ContinueDebugEvent(ev.dwProcessId, ev.dwThreadId, continueStatus);
    }

    DebugActiveProcessStop(opt.pid);
    CloseHandle(process);
    std::wcerr << L"detached events=" << seq << L"\n";
    return 0;
}
