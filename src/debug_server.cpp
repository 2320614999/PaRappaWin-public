// Winsock must be included before windows.h
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include "debug_server.h"
#include "logger.h"
#include "pr/pr_stage_scene_submit_debug.h"
#include "pr/pr_tmd_renderer.h"

#include <unordered_map>
#include <unordered_set>
#include <cctype>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <queue>
#include <mutex>
#include <sstream>
#include <variant>

// 内部状态
static SOCKET s_listenSocket = INVALID_SOCKET;
static SOCKET s_clientSocket = INVALID_SOCKET;
static bool s_initialized = false;
static std::unordered_map<std::string, DebugServer::CommandHandler> s_handlers;
static std::unordered_set<std::string> s_redactedArgLogCommands;
static std::string s_recvBuffer;

// 游戏状态快照
static struct {
    int frame = 0;
    int scene = 0;
    int dispatcherEvent = 0;
    int dispatcherState = 0;
    int gameState = 0;        // 0=BootLogo, 1=Menu, 2=Playing, 3=Exit
    int dispMenuIndex = 0;    // dispatcher 菜单选中索引
} s_gameState;
static bool s_legacyPrEventStatusKeysEnabled = true;
static bool s_legacyGenericInputEnabled = true;
static bool s_legacyDebugSceneSwitchEnabled = true;

// 待注入的输入队列
static std::queue<char> s_pendingKeys;
static std::queue<int> s_pendingEvents;
static bool s_pendingSwitch = false;
struct ShotRequest {
    int id;
    std::string stem;
};
static std::queue<ShotRequest> s_pendingShots;
static int s_nextShotId = 1;
static std::queue<int> s_pendingInputOnly;
static std::queue<uint16_t> s_pendingPadInput;
static uint16_t s_heldPadInput = 0;
static std::mutex s_inputMutex;

static bool SendAll(SOCKET socket, const char* data, int size) {
    int sent = 0;
    while (sent < size) {
        const int n = send(socket, data + sent, size - sent, 0);
        if (n > 0) {
            sent += n;
            continue;
        }
        if (n == 0) {
            return false;
        }
        const int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) {
            fd_set writeSet;
            FD_ZERO(&writeSet);
            FD_SET(socket, &writeSet);
            timeval timeout{};
            timeout.tv_usec = 10000;
            const int ready =
                select(0, nullptr, &writeSet, nullptr, &timeout);
            if (ready == SOCKET_ERROR) {
                return false;
            }
            continue;
        }
        return false;
    }
    return true;
}

static std::string TruncateDebugResponseForLog(const std::string& response) {
    constexpr size_t kMaxLoggedResponse = 512;
    if (response.size() <= kMaxLoggedResponse) {
        return response;
    }
    return response.substr(0, kMaxLoggedResponse) +
           "...<truncated len=" + std::to_string(response.size()) + ">";
}

// 通用变量注册表
enum class VarType { Int, Bool, U16, I16 };
struct VarEntry {
    VarType type;
    void* ptr;
    bool readOnly = false;
};
static std::unordered_map<std::string, VarEntry> s_vars;

// 内置命令实现
static bool TryGetVarI32(const std::string& name, int& out);

static std::string CmdStatus(const std::string& args) {
    std::ostringstream ss;
    ss << "frame=" << s_gameState.frame
       << " scene=" << s_gameState.scene;
    for (const char* name : {
             "stageFrame",
             "stageTick96",
             "stageRunning",
             "xaPlaying",
             "xaPlayedMs",
             "transActive",
         }) {
        int value = 0;
        if (TryGetVarI32(name, value)) {
            ss << " " << name << "=" << value;
        }
    }
    if (s_legacyPrEventStatusKeysEnabled) {
        ss << " dispEv=" << s_gameState.dispatcherEvent
           << " dispState=" << s_gameState.dispatcherState;
    }
    return ss.str();
}

static bool ParseU16(const std::string& s, uint16_t& out) {
    if (s.empty()) return false;
    try {
        if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            out = (uint16_t)std::stoul(s, nullptr, 16);
        } else {
            out = (uint16_t)std::stoul(s, nullptr, 10);
        }
        return true;
    } catch (...) {
        return false;
    }
}

static bool ParseI32(const std::string& s, int& out) {
    if (s.empty()) return false;
    try {
        out = std::stoi(s, nullptr, 0);
        return true;
    } catch (...) {
        return false;
    }
}

static bool TryGetVarI32(const std::string& name, int& out) {
    auto it = s_vars.find(name);
    if (it == s_vars.end()) return false;
    const VarEntry& v = it->second;
    switch (v.type) {
        case VarType::Int:  out = *static_cast<int*>(v.ptr); return true;
        case VarType::Bool: out = *static_cast<bool*>(v.ptr) ? 1 : 0; return true;
        case VarType::U16:  out = (int)*static_cast<uint16_t*>(v.ptr); return true;
        case VarType::I16:  out = (int)*static_cast<int16_t*>(v.ptr); return true;
    }
    return false;
}

static bool TrySetVarI32(const std::string& name, int val) {
    auto it = s_vars.find(name);
    if (it == s_vars.end()) return false;
    const VarEntry& v = it->second;
    if (v.readOnly) return false;
    switch (v.type) {
        case VarType::Int:  *static_cast<int*>(v.ptr) = val; return true;
        case VarType::Bool: *static_cast<bool*>(v.ptr) = (val != 0); return true;
        case VarType::U16:  *static_cast<uint16_t*>(v.ptr) = (uint16_t)val; return true;
        case VarType::I16:  *static_cast<int16_t*>(v.ptr) = (int16_t)val; return true;
    }
    return false;
}

static std::string CmdKey(const std::string& args) {
    if (args.empty()) return "ERR: usage: key <char|ENTER|UP|DOWN|LEFT|RIGHT|SPACE>";

    const bool genericKey =
        args.size() == 1 &&
        (args[0] == 'G' || args[0] == 'g' ||
         (args[0] >= '0' && args[0] <= '9'));
    if (!s_legacyGenericInputEnabled && genericKey) {
        return "ERR: legacy generic input disabled in direct SS0";
    }
    const bool debugSceneSwitchKey =
        args.size() == 1 && (args[0] == 'N' || args[0] == 'n');
    if (!s_legacyDebugSceneSwitchEnabled && debugSceneSwitchKey) {
        return "ERR: legacy debug scene switch disabled in direct SS0";
    }
    
    char k = 0;
    if (args == "ENTER") k = '\r';
    else if (args == "SPACE") k = ' ';
    else if (args == "UP") k = 'W';
    else if (args == "DOWN") k = 'S';
    else if (args == "LEFT") k = 'A';
    else if (args == "RIGHT") k = 'D';
    else if (args.size() == 1) k = args[0];
    else return "ERR: unknown key: " + args;
    
    {
        std::lock_guard<std::mutex> lk(s_inputMutex);
        s_pendingKeys.push(k);
    }
    return "OK: queued key=" + args;
}

static std::string CmdEvent(const std::string& args) {
    if (args.empty()) return "ERR: usage: event <0-17>";
    if (!s_legacyGenericInputEnabled) {
        return "ERR: legacy generic input disabled in direct SS0";
    }
    int ev = std::atoi(args.c_str());
    {
        std::lock_guard<std::mutex> lk(s_inputMutex);
        s_pendingEvents.push(ev);
        s_pendingSwitch = true;
    }
    return "OK: queued event=" + std::to_string(ev) + " + switch";
}

static std::string CmdSwitch(const std::string& args) {
    if (!s_legacyGenericInputEnabled) {
        return "ERR: legacy generic input disabled in direct SS0";
    }
    {
        std::lock_guard<std::mutex> lk(s_inputMutex);
        s_pendingSwitch = true;
    }
    return "OK: queued genericSwitch";
}

static std::string CmdScreenshot(const std::string& args) {
    std::string stem = args;
    while (!stem.empty() && (stem.front() == ' ' || stem.front() == '\t')) stem.erase(stem.begin());
    while (!stem.empty() && (stem.back() == ' ' || stem.back() == '\t')) stem.pop_back();

    std::string safe;
    safe.reserve(stem.size());
    for (unsigned char c : stem) {
        const bool ok = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                        (c == '_') || (c == '-') || (c == '+') || (c == '.');
        safe.push_back(ok ? (char)c : '_');
        if (safe.size() >= 120) break;
    }
    while (!safe.empty() && safe.front() == '_') safe.erase(safe.begin());
    while (!safe.empty() && safe.back() == '_') safe.pop_back();

    int id = 0;
    {
        std::lock_guard<std::mutex> lk(s_inputMutex);
        id = s_nextShotId++;
        s_pendingShots.push(ShotRequest{id, safe});
    }

    char fileBuf[256];
    if (!safe.empty()) {
        snprintf(fileBuf, sizeof(fileBuf), "%s__%04d.png", safe.c_str(), id);
    } else {
        snprintf(fileBuf, sizeof(fileBuf), "shot__%04d.png", id);
    }
    char respBuf[320];
    snprintf(respBuf, sizeof(respBuf), "OK: shot id=%d file=%s", id, fileBuf);
    return respBuf;
}

static std::string CmdInput(const std::string& args) {
    if (args.empty()) return "ERR: usage: input <0-8> (set debugGenericEvent without switch)";
    if (!s_legacyGenericInputEnabled) {
        return "ERR: legacy generic input disabled in direct SS0";
    }
    int inp = std::atoi(args.c_str());
    {
        std::lock_guard<std::mutex> lk(s_inputMutex);
        s_pendingInputOnly.push(inp);
    }
    return "OK: input=" + std::to_string(inp) + " (no switch)";
}

// 通用变量读取
static std::string CmdGet(const std::string& args) {
    if (args.empty()) return "ERR: usage: get <varname>";
    auto it = s_vars.find(args);
    if (it == s_vars.end()) return "ERR: unknown var: " + args;
    
    const VarEntry& v = it->second;
    int val = 0;
    switch (v.type) {
        case VarType::Int:  val = *static_cast<int*>(v.ptr); break;
        case VarType::Bool: val = *static_cast<bool*>(v.ptr) ? 1 : 0; break;
        case VarType::U16:  val = *static_cast<uint16_t*>(v.ptr); break;
        case VarType::I16:  val = *static_cast<int16_t*>(v.ptr); break;
    }
    return args + "=" + std::to_string(val);
}

// 通用变量写入
static std::string CmdSet(const std::string& args) {
    size_t sp = args.find(' ');
    if (sp == std::string::npos) return "ERR: usage: set <varname> <value>";
    std::string name = args.substr(0, sp);
    std::string valStr = args.substr(sp + 1);
    
    auto it = s_vars.find(name);
    if (it == s_vars.end()) return "ERR: unknown var: " + name;
    
    int val = std::atoi(valStr.c_str());
    const VarEntry& v = it->second;
    if (v.readOnly) {
        return "ERR: read-only var: " + name;
    }
    switch (v.type) {
        case VarType::Int:  *static_cast<int*>(v.ptr) = val; break;
        case VarType::Bool: *static_cast<bool*>(v.ptr) = (val != 0); break;
        case VarType::U16:  *static_cast<uint16_t*>(v.ptr) = (uint16_t)val; break;
        case VarType::I16:  *static_cast<int16_t*>(v.ptr) = (int16_t)val; break;
    }
    return "OK: " + name + "=" + std::to_string(val);
}

// 列出所有已注册变量
static std::string CmdVars(const std::string& args) {
    std::ostringstream ss;
    ss << "vars[" << s_vars.size() << "]:";
    for (const auto& kv : s_vars) {
        const VarEntry& v = kv.second;
        int val = 0;
        switch (v.type) {
            case VarType::Int:  val = *static_cast<int*>(v.ptr); break;
            case VarType::Bool: val = *static_cast<bool*>(v.ptr) ? 1 : 0; break;
            case VarType::U16:  val = *static_cast<uint16_t*>(v.ptr); break;
            case VarType::I16:  val = *static_cast<int16_t*>(v.ptr); break;
        }
        ss << " " << kv.first << "=" << val;
    }
    return ss.str();
}

// 直接注入 PSX pad mask (16-bit)
static std::string CmdPad(const std::string& args) {
    if (args.empty()) return "ERR: usage: pad <hex|dec> (e.g. pad 0x0040 for Cross)";
    uint16_t pad = 0;
    if (args.size() > 2 && args[0] == '0' && (args[1] == 'x' || args[1] == 'X')) {
        pad = (uint16_t)std::stoul(args, nullptr, 16);
    } else {
        pad = (uint16_t)std::atoi(args.c_str());
    }
    {
        std::lock_guard<std::mutex> lk(s_inputMutex);
        s_pendingPadInput.push(pad);
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "OK: pad=0x%04X", pad);
    return buf;
}

static std::string CmdPadHold(const std::string& args) {
    if (args.empty()) {
        return "ERR: usage: padhold <hex|dec> (0 releases held pad)";
    }
    uint16_t pad = 0;
    if (!ParseU16(args, pad)) {
        return std::string("ERR: bad pad mask: ") + args;
    }
    {
        std::lock_guard<std::mutex> lk(s_inputMutex);
        s_heldPadInput = pad;
    }
    char buf[40];
    snprintf(buf, sizeof(buf), "OK: padhold=0x%04X", pad);
    return buf;
}

static std::string CmdPadSeq(const std::string& args) {
    // usage: padseq <mask|wN|wait:N|hN:mask|hold:N:mask> ...
    // Notes: queue one pad per frame; 'wN' inserts N frames of 0.
    // By default each non-zero mask is treated as a 1-frame pulse (mask then 0) to ensure rising edge.
    // hN:mask keeps mask held for N frames, then inserts one 0 release frame.
    std::istringstream iss(args);
    std::string tok;
    int queued = 0;
    {
        std::lock_guard<std::mutex> lk(s_inputMutex);
        while (iss >> tok) {
            if (tok.empty()) continue;
            int waitN = 0;
            int holdN = 1;
            bool isHoldToken = false;
            std::string maskToken = tok;

            const size_t hSep = tok.find(':');
            if (tok.size() >= 3 && (tok[0] == 'h' || tok[0] == 'H') && hSep != std::string::npos) {
                if (!ParseI32(tok.substr(1, hSep - 1), holdN) || holdN <= 0) {
                    return std::string("ERR: bad hold token: ") + tok;
                }
                maskToken = tok.substr(hSep + 1);
                isHoldToken = true;
            } else if (tok.rfind("hold:", 0) == 0 || tok.rfind("HOLD:", 0) == 0) {
                const size_t maskSep = tok.find(':', 5);
                if (maskSep == std::string::npos ||
                    !ParseI32(tok.substr(5, maskSep - 5), holdN) ||
                    holdN <= 0) {
                    return std::string("ERR: bad hold token: ") + tok;
                }
                maskToken = tok.substr(maskSep + 1);
                isHoldToken = true;
            }

            if (!isHoldToken && tok.size() >= 2 && (tok[0] == 'w' || tok[0] == 'W')) {
                if (!ParseI32(tok.substr(1), waitN) || waitN <= 0) {
                    return std::string("ERR: bad wait token: ") + tok;
                }
            } else if (!isHoldToken && (tok.rfind("wait:", 0) == 0 || tok.rfind("WAIT:", 0) == 0)) {
                if (!ParseI32(tok.substr(5), waitN) || waitN <= 0) {
                    return std::string("ERR: bad wait token: ") + tok;
                }
            }

            if (waitN > 0) {
                const int n = std::min(waitN, 10000);
                for (int i = 0; i < n; i++) {
                    s_pendingPadInput.push((uint16_t)0);
                    queued++;
                }
                continue;
            }

            uint16_t mask = 0;
            if (!ParseU16(maskToken, mask)) {
                return std::string("ERR: bad mask token: ") + tok;
            }
            holdN = std::min(holdN, 120);
            for (int i = 0; i < holdN; i++) {
                s_pendingPadInput.push(mask);
                queued++;
            }
            if (mask != 0) {
                s_pendingPadInput.push((uint16_t)0);
                queued++;
            }
        }
    }

    if (queued <= 0) {
        return "ERR: usage: padseq <mask|wN|wait:N|hN:mask|hold:N:mask> ...";
    }
    return std::string("OK: queued padseq=") + std::to_string(queued);
}

static std::string CmdSetMany(const std::string& args) {
    // usage: setmany a=1 b=2 ...
    std::istringstream iss(args);
    std::string tok;
    int n = 0;
    while (iss >> tok) {
        const size_t eq = tok.find('=');
        if (eq == std::string::npos || eq == 0 || eq + 1 >= tok.size()) {
            return std::string("ERR: bad token (need name=value): ") + tok;
        }
        const std::string name = tok.substr(0, eq);
        const std::string valStr = tok.substr(eq + 1);
        int val = 0;
        if (!ParseI32(valStr, val)) {
            return std::string("ERR: bad value for ") + name + ": " + valStr;
        }
        auto it = s_vars.find(name);
        if (it == s_vars.end()) {
            return std::string("ERR: unknown var: ") + name;
        }
        if (it->second.readOnly) {
            return std::string("ERR: read-only var: ") + name;
        }
        if (!TrySetVarI32(name, val)) {
            return std::string("ERR: unknown var: ") + name;
        }
        n++;
    }
    if (n <= 0) return "ERR: usage: setmany a=1 b=2 ...";
    return std::string("OK: setmany n=") + std::to_string(n);
}

static std::string CmdAssert(const std::string& args) {
    // usage: assert <var> <op> <value>
    // returns OK or FAIL with details
    std::istringstream iss(args);
    std::string name, op, valStr;
    iss >> name >> op >> valStr;
    if (name.empty() || op.empty() || valStr.empty()) {
        return "ERR: usage: assert <var> <op> <value>";
    }

    int got = 0;
    if (!TryGetVarI32(name, got)) {
        return std::string("ERR: unknown var: ") + name;
    }

    int want = 0;
    if (!ParseI32(valStr, want)) {
        return std::string("ERR: bad value: ") + valStr;
    }

    bool ok = false;
    if (op == "==") ok = (got == want);
    else if (op == "!=") ok = (got != want);
    else if (op == ">") ok = (got > want);
    else if (op == "<") ok = (got < want);
    else if (op == ">=") ok = (got >= want);
    else if (op == "<=") ok = (got <= want);
    else return std::string("ERR: bad op: ") + op;

    if (ok) {
        return std::string("OK: ") + name + "=" + std::to_string(got);
    }
    return std::string("FAIL: ") + name + "=" + std::to_string(got) + " expected " + op + " " + std::to_string(want);
}

static std::string CmdEv3Goto(const std::string& args) {
    // usage: ev3goto <0-4>
    int want = 0;
    if (!ParseI32(args, want) || want < 0 || want >= 5) {
        return "ERR: usage: ev3goto <0-4>";
    }

    int dispState = 0;
    int dispEventId = 0;
    int cur = -1;
    if (!TryGetVarI32("dispState", dispState)) {
        dispState = s_gameState.dispatcherState;
    }
    if (!TryGetVarI32("dispEventId", dispEventId)) {
        dispEventId = 0;
    }
    if (!TryGetVarI32("dispMenuIndex", cur)) {
        cur = s_gameState.dispMenuIndex;
    }

    if (dispState == 0 || dispEventId != 3 || cur < 0) {
        return std::string("ERR: ev3goto requires dispState>0 and dispEventId==3 (got dispState=")
            + std::to_string(dispState) + " dispEventId=" + std::to_string(dispEventId) + ")";
    }

    const int menuCount = 5;
    int diffDown = (want - cur) % menuCount;
    if (diffDown < 0) diffDown += menuCount;
    int diffUp = (cur - want) % menuCount;
    if (diffUp < 0) diffUp += menuCount;

    constexpr uint16_t PAD_UP = 0x1000;
    constexpr uint16_t PAD_DOWN = 0x4000;

    int steps = 0;
    {
        std::lock_guard<std::mutex> lk(s_inputMutex);
        if (diffDown <= diffUp) {
            steps = diffDown;
            for (int i = 0; i < steps; i++) {
                s_pendingPadInput.push(PAD_DOWN);
                s_pendingPadInput.push((uint16_t)0);
            }
        } else {
            steps = diffUp;
            for (int i = 0; i < steps; i++) {
                s_pendingPadInput.push(PAD_UP);
                s_pendingPadInput.push((uint16_t)0);
            }
        }
    }

    return std::string("OK: ev3goto cur=") + std::to_string(cur) + " want=" + std::to_string(want) + " steps=" + std::to_string(steps);
}

static void QueuePadPulseLocked(uint16_t mask) {
    s_pendingPadInput.push(mask);
    if (mask != 0) {
        s_pendingPadInput.push((uint16_t)0);
    }
}

static void QueueWaitFramesLocked(int frames) {
    const int n = std::max(0, std::min(frames, 60000));
    for (int i = 0; i < n; i++) {
        s_pendingPadInput.push((uint16_t)0);
    }
}

static std::string CmdEv17Set(const std::string& args) {
    // usage: ev17set <lang0-4> <subtitleFlag(0=off,1=on)> [confirm0|1]
    // Notes: does not start dispatcher; requires dispEventId==17 running.
    std::istringstream iss(args);
    int lang = 0;
    int subFlag = 0;
    int confirm = 1;
    iss >> lang >> subFlag;
    if (iss.fail()) {
        return "ERR: usage: ev17set <lang0-4> <subtitleFlag(0=off,1=on)> [confirm0|1]";
    }
    iss >> confirm;
    if (lang < 0 || lang > 4) {
        return "ERR: ev17set lang must be 0..4";
    }
    if (subFlag != 0 && subFlag != 1) {
        return "ERR: ev17set subtitleFlag must be 0 or 1";
    }

    int dispState = s_gameState.dispatcherState;
    int dispEventId = 0;
    int curMenu = s_gameState.dispMenuIndex;
    TryGetVarI32("dispEventId", dispEventId);
    TryGetVarI32("dispMenuIndex", curMenu);
    if (dispState == 0 || dispEventId != 17 || curMenu < 0) {
        return std::string("ERR: ev17set requires dispState>0 and dispEventId==17 (got dispState=")
            + std::to_string(dispState) + " dispEventId=" + std::to_string(dispEventId) + ")";
    }

    constexpr uint16_t PAD_UP = 0x1000;
    constexpr uint16_t PAD_DOWN = 0x4000;
    constexpr uint16_t PAD_LEFT = 0x8000;
    constexpr uint16_t PAD_RIGHT = 0x2000;
    constexpr uint16_t PAD_CROSS = 0x0040;
    constexpr uint16_t PAD_CIRCLE = 0x0020;

    auto queueGotoMenuIdx = [&](int wantIdx) {
        // menuCount=3, cyclic up/down
        const int menuCount = 3;
        int cur = curMenu;
        int diffDown = (wantIdx - cur) % menuCount;
        if (diffDown < 0) diffDown += menuCount;
        int diffUp = (cur - wantIdx) % menuCount;
        if (diffUp < 0) diffUp += menuCount;
        if (diffDown <= diffUp) {
            for (int i = 0; i < diffDown; i++) QueuePadPulseLocked(PAD_DOWN);
        } else {
            for (int i = 0; i < diffUp; i++) QueuePadPulseLocked(PAD_UP);
        }
        curMenu = wantIdx;
    };

    int queued = 0;
    {
        std::lock_guard<std::mutex> lk(s_inputMutex);

        // Set language: go to idx=1 then rotate to 0 and advance to target
        queueGotoMenuIdx(1);
        for (int i = 0; i < 5; i++) {
            QueuePadPulseLocked(PAD_LEFT);
            QueueWaitFramesLocked(16);
        }
        for (int i = 0; i < lang; i++) {
            QueuePadPulseLocked(PAD_RIGHT);
            QueueWaitFramesLocked(16);
        }

        // cooldown blocks all inputs, so wait out after last left/right
        QueueWaitFramesLocked(16);

        // Set subtitle flag: go to idx=0 then press desired button
        queueGotoMenuIdx(0);
        QueuePadPulseLocked((subFlag == 0) ? PAD_CIRCLE : PAD_CROSS);

        if (confirm != 0) {
            queueGotoMenuIdx(2);
            QueuePadPulseLocked(PAD_CROSS);
        }

        queued = (int)s_pendingPadInput.size();
        (void)queued;
    }

    return std::string("OK: ev17set queued");
}

static int McNextIndex(int idx, uint16_t moveMask) {
    const int kExitIdx = 15;
    const int kGridCols = 3;
    const int kGridSlots = 15;

    if (moveMask == 0x1000 /*UP*/) {
        if (idx == kExitIdx) {
            return kGridSlots - 1;
        }
        if (idx >= kGridCols) {
            return idx - kGridCols;
        }
        return idx;
    }
    if (moveMask == 0x4000 /*DOWN*/) {
        if (idx >= kGridSlots - kGridCols && idx < kGridSlots) {
            return kExitIdx;
        }
        if (idx + kGridCols < kGridSlots) {
            return idx + kGridCols;
        }
        return idx;
    }
    if (moveMask == 0x8000 /*LEFT*/) {
        if (idx > 0 && idx < kGridSlots) {
            return idx - 1;
        }
        return idx;
    }
    if (moveMask == 0x2000 /*RIGHT*/) {
        if (idx + 1 < kGridSlots) {
            return idx + 1;
        }
        return idx;
    }
    return idx;
}

static std::string CmdMcGoto(const std::string& args) {
    // usage: mcgoto <0-14|15|exit>
    int dispState = s_gameState.dispatcherState;
    int dispEventId = 0;
    int curMenu = s_gameState.dispMenuIndex;
    TryGetVarI32("dispEventId", dispEventId);
    TryGetVarI32("dispMenuIndex", curMenu);

    if (dispState == 0 || (dispEventId != 7 && dispEventId != 8 && dispEventId != 9) || curMenu < 0) {
        return std::string("ERR: mcgoto requires dispState>0 and dispEventId in {7,8,9} (got dispState=")
            + std::to_string(dispState) + " dispEventId=" + std::to_string(dispEventId) + ")";
    }

    std::string a = args;
    while (!a.empty() && std::isspace((unsigned char)a.front())) a.erase(a.begin());
    while (!a.empty() && std::isspace((unsigned char)a.back())) a.pop_back();
    if (a.empty()) return "ERR: usage: mcgoto <0-14|15|exit>";
    for (auto& c : a) c = (char)std::tolower((unsigned char)c);

    int target = -1;
    if (a == "exit") {
        target = 15;
    } else {
        if (!ParseI32(a, target)) {
            return std::string("ERR: bad target: ") + a;
        }
    }
    if (target < 0 || target > 15) {
        return "ERR: mcgoto target must be 0..15 or 'exit'";
    }
    if (curMenu == target) {
        return std::string("OK: mcgoto already at ") + std::to_string(target);
    }

    constexpr uint16_t PAD_UP = 0x1000;
    constexpr uint16_t PAD_DOWN = 0x4000;
    constexpr uint16_t PAD_LEFT = 0x8000;
    constexpr uint16_t PAD_RIGHT = 0x2000;
    const uint16_t moves[4] = { PAD_UP, PAD_DOWN, PAD_LEFT, PAD_RIGHT };

    // BFS over 0..15
    std::vector<int> prev(16, -1);
    std::vector<uint16_t> prevMove(16, 0);
    std::queue<int> q;
    prev[curMenu] = curMenu;
    q.push(curMenu);
    while (!q.empty()) {
        int v = q.front();
        q.pop();
        if (v == target) break;
        for (int i = 0; i < 4; i++) {
            const uint16_t m = moves[i];
            const int nxt = McNextIndex(v, m);
            if (nxt == v) continue;
            if (prev[nxt] != -1) continue;
            prev[nxt] = v;
            prevMove[nxt] = m;
            q.push(nxt);
        }
    }
    if (prev[target] == -1) {
        return "ERR: mcgoto path not found";
    }

    std::vector<uint16_t> path;
    for (int v = target; v != curMenu; v = prev[v]) {
        path.push_back(prevMove[v]);
    }
    std::reverse(path.begin(), path.end());

    {
        std::lock_guard<std::mutex> lk(s_inputMutex);
        for (uint16_t m : path) {
            QueuePadPulseLocked(m);
        }
    }

    return std::string("OK: mcgoto cur=") + std::to_string(curMenu) + " target=" + std::to_string(target) + " steps=" + std::to_string((int)path.size());
}

// JSON 格式状态输出
static std::string CmdJson(const std::string& args) {
    std::ostringstream ss;
    ss << "{\"frame\":" << s_gameState.frame
       << ",\"scene\":" << s_gameState.scene
       << ",\"gameState\":" << s_gameState.gameState;
    if (s_legacyPrEventStatusKeysEnabled) {
        ss << ",\"dispEv\":" << s_gameState.dispatcherEvent
           << ",\"dispState\":" << s_gameState.dispatcherState
           << ",\"dispMenuIndex\":" << s_gameState.dispMenuIndex;
    }
    std::unordered_set<std::string> usedKeys;
    usedKeys.insert("frame");
    usedKeys.insert("scene");
    usedKeys.insert("gameState");
    if (s_legacyPrEventStatusKeysEnabled) {
        usedKeys.insert("dispEv");
        usedKeys.insert("dispState");
        usedKeys.insert("dispMenuIndex");
    }
    // 追加所有注册变量
    for (const auto& kv : s_vars) {
        if (usedKeys.find(kv.first) != usedKeys.end()) {
            continue;
        }
        const VarEntry& v = kv.second;
        int val = 0;
        switch (v.type) {
            case VarType::Int:  val = *static_cast<int*>(v.ptr); break;
            case VarType::Bool: val = *static_cast<bool*>(v.ptr) ? 1 : 0; break;
            case VarType::U16:  val = *static_cast<uint16_t*>(v.ptr); break;
            case VarType::I16:  val = *static_cast<int16_t*>(v.ptr); break;
        }
        ss << ",\"" << kv.first << "\":" << val;
    }
    ss << "}";
    return ss.str();
}

static std::string CmdTmdDump(const std::string& args) {
    std::string out = PrTmdRenderer::DumpStats();
    out += PrStageSceneSubmitDebug::DumpStage1Stats();
    return out;
}

static std::string CmdHelp(const std::string& args) {
    return "Commands: status, json, vars, get <var>, set <var> <val>, "
           "pad <hex>, padhold <hex>, padseq <...>, ev3goto <0-4>, ev17set <lang> <subFlag> [confirm], mcgoto <slot|exit>, "
           "key <k>, event <n>, input <n>, switch, setmany a=1 b=2, assert <var> <op> <val>, "
           "screenshot [tag], shot [tag], texdump, tmddump, help";
}

bool DebugServer::Init(uint16_t port) {
    if (s_initialized) return true;
    
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        Log::Printf("DebugServer: WSAStartup failed: %d", result);
        return false;
    }
    
    s_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s_listenSocket == INVALID_SOCKET) {
        Log::Printf("DebugServer: socket failed: %d", WSAGetLastError());
        WSACleanup();
        return false;
    }
    
    // 设置非阻塞
    u_long mode = 1;
    ioctlsocket(s_listenSocket, FIONBIO, &mode);
    
    // 允许端口重用
    int optval = 1;
    setsockopt(s_listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
    
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // 只监听 127.0.0.1
    addr.sin_port = htons(port);
    
    if (bind(s_listenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        Log::Printf("DebugServer: bind failed: %d", WSAGetLastError());
        closesocket(s_listenSocket);
        s_listenSocket = INVALID_SOCKET;
        WSACleanup();
        return false;
    }
    
    if (listen(s_listenSocket, 1) == SOCKET_ERROR) {
        Log::Printf("DebugServer: listen failed: %d", WSAGetLastError());
        closesocket(s_listenSocket);
        s_listenSocket = INVALID_SOCKET;
        WSACleanup();
        return false;
    }
    
    // 注册内置命令
    s_handlers["status"] = CmdStatus;
    s_handlers["json"] = CmdJson;
    s_handlers["vars"] = CmdVars;
    s_handlers["get"] = CmdGet;
    s_handlers["set"] = CmdSet;
    s_handlers["pad"] = CmdPad;
    s_handlers["padhold"] = CmdPadHold;
    s_handlers["padseq"] = CmdPadSeq;
    s_handlers["key"] = CmdKey;
    s_handlers["event"] = CmdEvent;
    s_handlers["switch"] = CmdSwitch;
    s_handlers["setmany"] = CmdSetMany;
    s_handlers["assert"] = CmdAssert;
    s_handlers["ev3goto"] = CmdEv3Goto;
    s_handlers["ev17set"] = CmdEv17Set;
    s_handlers["mcgoto"] = CmdMcGoto;
    s_handlers["help"] = CmdHelp;
    s_handlers["screenshot"] = CmdScreenshot;
    s_handlers["shot"] = CmdScreenshot;
    s_handlers["input"] = CmdInput;
    s_handlers["in"] = CmdInput;
    s_handlers["tmddump"] = CmdTmdDump;
    
    s_initialized = true;
    Log::Printf("DebugServer: listening on 127.0.0.1:%d", (int)port);
    return true;
}

void DebugServer::Shutdown() {
    if (!s_initialized) return;
    
    if (s_clientSocket != INVALID_SOCKET) {
        closesocket(s_clientSocket);
        s_clientSocket = INVALID_SOCKET;
    }
    if (s_listenSocket != INVALID_SOCKET) {
        closesocket(s_listenSocket);
        s_listenSocket = INVALID_SOCKET;
    }
    WSACleanup();
    s_handlers.clear();
    s_redactedArgLogCommands.clear();
    s_initialized = false;
    Log::Printf("DebugServer: shutdown");
}

static void ProcessCommand(const std::string& line) {
    if (line.empty()) return;

    std::string input = line;
    if (input.size() >= 3 &&
        (unsigned char)input[0] == 0xEF &&
        (unsigned char)input[1] == 0xBB &&
        (unsigned char)input[2] == 0xBF) {
        input.erase(0, 3);
    }
    while (!input.empty() && (input.front() == ' ' || input.front() == '\t')) {
        input.erase(input.begin());
    }
    if (input.empty()) return;
    
    // 解析命令和参数
    std::string cmd, args;
    size_t sp = input.find(' ');
    if (sp != std::string::npos) {
        cmd = input.substr(0, sp);
        args = input.substr(sp + 1);
    } else {
        cmd = input;
    }
    
    // 转小写
    for (auto& c : cmd) c = (char)tolower((unsigned char)c);
    
    std::string response;
    auto it = s_handlers.find(cmd);
    if (it != s_handlers.end()) {
        response = it->second(args);
    } else {
        response = "ERR: unknown command: " + cmd + " (try 'help')";
    }
    
    response += "\n";
    if (s_clientSocket != INVALID_SOCKET) {
        if (!SendAll(s_clientSocket, response.c_str(), (int)response.size())) {
            closesocket(s_clientSocket);
            s_clientSocket = INVALID_SOCKET;
        }
    }
    
    const char* loggedArgs =
        s_redactedArgLogCommands.find(cmd) != s_redactedArgLogCommands.end()
            ? "<redacted>"
            : args.c_str();
    const std::string loggedResponse = TruncateDebugResponseForLog(response);
    Log::Printf("DebugServer: cmd='%s' args='%s' -> '%s'", 
                cmd.c_str(), loggedArgs, loggedResponse.c_str());
}

void DebugServer::Update() {
    if (!s_initialized) return;
    
    // 尝试接受新连接
    if (s_clientSocket == INVALID_SOCKET) {
        sockaddr_in clientAddr;
        int addrLen = sizeof(clientAddr);
        SOCKET newClient = accept(s_listenSocket, (sockaddr*)&clientAddr, &addrLen);
        if (newClient != INVALID_SOCKET) {
            s_clientSocket = newClient;
            u_long mode = 1;
            ioctlsocket(s_clientSocket, FIONBIO, &mode);
            s_recvBuffer.clear();
            
            const char* welcome = "PaRappa Debug Console (type 'help')\n> ";
            SendAll(s_clientSocket, welcome, (int)strlen(welcome));
            Log::Printf("DebugServer: client connected");
        }
    }
    
    // 处理已连接客户端的数据
    if (s_clientSocket != INVALID_SOCKET) {
        char buf[256];
        int bytes = recv(s_clientSocket, buf, sizeof(buf) - 1, 0);
        
        if (bytes > 0) {
            buf[bytes] = '\0';
            s_recvBuffer += buf;
            
            // 处理完整的行
            size_t pos;
            while ((pos = s_recvBuffer.find('\n')) != std::string::npos) {
                std::string line = s_recvBuffer.substr(0, pos);
                s_recvBuffer.erase(0, pos + 1);
                
                // 去掉 \r
                if (!line.empty() && line.back() == '\r') line.pop_back();
                
                ProcessCommand(line);
                
                // 发送提示符
                const char* prompt = "> ";
                if (s_clientSocket != INVALID_SOCKET) {
                    SendAll(s_clientSocket, prompt, 2);
                }
            }
        } else if (bytes == 0 || (bytes < 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
            // 连接关闭或错误
            closesocket(s_clientSocket);
            s_clientSocket = INVALID_SOCKET;
            s_recvBuffer.clear();
            Log::Printf("DebugServer: client disconnected");
        }
    }
}

void DebugServer::RegisterCommand(const std::string& cmd, CommandHandler handler) {
    s_handlers[cmd] = handler;
}

void DebugServer::SetCommandArgsLogRedacted(const std::string& cmd, bool redacted) {
    std::string key = cmd;
    for (auto& c : key) c = (char)tolower((unsigned char)c);
    if (redacted) {
        s_redactedArgLogCommands.insert(key);
    } else {
        s_redactedArgLogCommands.erase(key);
    }
}

void DebugServer::SetGameState(int frame, int scene, int dispatcherEvent, int dispatcherState) {
    s_gameState.frame = frame;
    s_gameState.scene = scene;
    s_gameState.dispatcherEvent = dispatcherEvent;
    s_gameState.dispatcherState = dispatcherState;
}

void DebugServer::SetGameStateEx(int frame, int scene, int dispMenuIndex, int dispState, int gameState) {
    s_gameState.frame = frame;
    s_gameState.scene = scene;
    s_gameState.dispMenuIndex = dispMenuIndex;
    s_gameState.dispatcherState = dispState;
    s_gameState.gameState = gameState;
}

void DebugServer::SetLegacyPrEventStatusKeysEnabled(bool enabled) {
    s_legacyPrEventStatusKeysEnabled = enabled;
}

void DebugServer::SetLegacyGenericInputEnabled(bool enabled) {
    s_legacyGenericInputEnabled = enabled;
}

void DebugServer::SetLegacyDebugSceneSwitchEnabled(bool enabled) {
    s_legacyDebugSceneSwitchEnabled = enabled;
}

bool DebugServer::ConsumeKeyPress(char& outKey) {
    std::lock_guard<std::mutex> lk(s_inputMutex);
    if (s_pendingKeys.empty()) return false;
    outKey = s_pendingKeys.front();
    s_pendingKeys.pop();
    return true;
}

bool DebugServer::ConsumeGenericEvent(int& outEvent) {
    std::lock_guard<std::mutex> lk(s_inputMutex);
    if (s_pendingEvents.empty()) return false;
    outEvent = s_pendingEvents.front();
    s_pendingEvents.pop();
    return true;
}

bool DebugServer::ConsumeGenericSwitch() {
    std::lock_guard<std::mutex> lk(s_inputMutex);
    if (!s_pendingSwitch) return false;
    s_pendingSwitch = false;
    return true;
}

bool DebugServer::ConsumeScreenshotRequest() {
    std::lock_guard<std::mutex> lk(s_inputMutex);
    if (s_pendingShots.empty()) return false;
    s_pendingShots.pop();
    return true;
}

bool DebugServer::ConsumeScreenshotRequestInfo(int& outId, std::string& outFileStem) {
    std::lock_guard<std::mutex> lk(s_inputMutex);
    if (s_pendingShots.empty()) return false;
    ShotRequest r = s_pendingShots.front();
    s_pendingShots.pop();
    outId = r.id;
    outFileStem = r.stem;
    return true;
}

bool DebugServer::ConsumeScreenshotRequestTag(std::string& outTag) {
    int id = 0;
    std::string stem;
    if (!ConsumeScreenshotRequestInfo(id, stem)) return false;
    outTag = stem;
    return true;
}

bool DebugServer::ConsumeInputOnly(int& outInput) {
    std::lock_guard<std::mutex> lk(s_inputMutex);
    if (s_pendingInputOnly.empty()) return false;
    outInput = s_pendingInputOnly.front();
    s_pendingInputOnly.pop();
    return true;
}

bool DebugServer::ConsumePadInput(uint16_t& outPad) {
    std::lock_guard<std::mutex> lk(s_inputMutex);
    if (!s_pendingPadInput.empty()) {
        outPad = s_pendingPadInput.front();
        s_pendingPadInput.pop();
        return true;
    }
    if (s_heldPadInput != 0u) {
        outPad = s_heldPadInput;
        return true;
    }
    return false;
}

bool DebugServer::ClearPendingPadInput(int* clearedCount, int* clearedNonZeroCount) {
    std::lock_guard<std::mutex> lk(s_inputMutex);
    if (s_pendingPadInput.empty()) {
        if (clearedCount) *clearedCount = 0;
        if (clearedNonZeroCount) *clearedNonZeroCount = 0;
        return false;
    }
    int count = 0;
    int nonZeroCount = 0;
    std::queue<uint16_t> snapshot = s_pendingPadInput;
    while (!snapshot.empty()) {
        if (snapshot.front() != 0u) {
            ++nonZeroCount;
        }
        ++count;
        snapshot.pop();
    }
    std::queue<uint16_t> empty;
    s_pendingPadInput.swap(empty);
    if (clearedCount) *clearedCount = count;
    if (clearedNonZeroCount) *clearedNonZeroCount = nonZeroCount;
    return true;
}

bool DebugServer::ClearHeldPadInput() {
    std::lock_guard<std::mutex> lk(s_inputMutex);
    if (s_heldPadInput == 0u) {
        return false;
    }
    s_heldPadInput = 0u;
    return true;
}

void DebugServer::RegisterVar(const std::string& name, int* ptr) {
    s_vars[name] = { VarType::Int, ptr, false };
}

void DebugServer::RegisterVar(const std::string& name, bool* ptr) {
    s_vars[name] = { VarType::Bool, ptr, false };
}

void DebugServer::RegisterVar(const std::string& name, uint16_t* ptr) {
    s_vars[name] = { VarType::U16, ptr, false };
}

void DebugServer::RegisterVar(const std::string& name, int16_t* ptr) {
    s_vars[name] = { VarType::I16, ptr, false };
}

void DebugServer::RegisterReadOnlyVar(const std::string& name, int* ptr) {
    s_vars[name] = { VarType::Int, ptr, true };
}

void DebugServer::RegisterReadOnlyVar(const std::string& name, bool* ptr) {
    s_vars[name] = { VarType::Bool, ptr, true };
}

void DebugServer::RegisterReadOnlyVar(const std::string& name, uint16_t* ptr) {
    s_vars[name] = { VarType::U16, ptr, true };
}

void DebugServer::RegisterReadOnlyVar(const std::string& name, int16_t* ptr) {
    s_vars[name] = { VarType::I16, ptr, true };
}
