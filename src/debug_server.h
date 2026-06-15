#pragma once
#include <cstdint>
#include <string>
#include <functional>

// 调试命令服务器 - 通过 TCP 端口暴露游戏状态和命令接口
// 默认端口: 19790 (取自 PaRappa 1997 + 90年代)

class DebugServer {
public:
    // 命令回调类型: 返回响应字符串
    using CommandHandler = std::function<std::string(const std::string& args)>;

    static bool Init(uint16_t port = 19790);
    static void Shutdown();

    // 每帧调用，处理连接和命令（非阻塞）
    static void Update();

    // 注册命令处理器
    static void RegisterCommand(const std::string& cmd, CommandHandler handler);
    static void SetCommandArgsLogRedacted(const std::string& cmd, bool redacted);

    // 通用变量注册（支持 get/set 命令访问）
    static void RegisterVar(const std::string& name, int* ptr);
    static void RegisterVar(const std::string& name, bool* ptr);
    static void RegisterVar(const std::string& name, uint16_t* ptr);
    static void RegisterVar(const std::string& name, int16_t* ptr);
    static void RegisterReadOnlyVar(const std::string& name, int* ptr);
    static void RegisterReadOnlyVar(const std::string& name, bool* ptr);
    static void RegisterReadOnlyVar(const std::string& name, uint16_t* ptr);
    static void RegisterReadOnlyVar(const std::string& name, int16_t* ptr);

    // 内置命令的状态更新（由主循环调用）
    static void SetGameState(int frame, int scene, int dispatcherEvent, int dispatcherState);
    static void SetGameStateEx(int frame, int scene, int dispMenuIndex, int dispState, int gameState);
    static void SetLegacyPrEventStatusKeysEnabled(bool enabled);
    static void SetLegacyGenericInputEnabled(bool enabled);
    static void SetLegacyDebugSceneSwitchEnabled(bool enabled);

    // 获取待注入的输入（由主循环读取并消费）
    static bool ConsumeKeyPress(char& outKey);
    static bool ConsumeGenericEvent(int& outEvent);
    static bool ConsumeGenericSwitch();
    static bool ConsumeInputOnly(int& outInput);
    static bool ConsumeScreenshotRequest();
    static bool ConsumeScreenshotRequestInfo(int& outId, std::string& outFileStem);
    static bool ConsumeScreenshotRequestTag(std::string& outTag);
    static bool ConsumePadInput(uint16_t& outPad);  // 直接 PSX pad mask
    static bool ClearPendingPadInput(int* clearedCount = nullptr, int* clearedNonZeroCount = nullptr);
    static bool ClearHeldPadInput();

private:
    DebugServer() = delete;
};
