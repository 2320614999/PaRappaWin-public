#pragma once

#include <array>
#include <cstdint>
#include <cstddef>

struct PrGameContext;
struct PrStage1ResolvedTextEvent;

// 字幕信息
struct SubtitleInfo {
    const char* text;
    uint32_t startFrame;
    uint32_t durationFrames;
    uint32_t eventId;
};

struct ParserTextDebugInfo {
    bool valid = false;
    uint8_t streamId = 0;
    uint32_t eventPsxAddr = 0;
    uint32_t eventFrame = 0;
    uint32_t dueFrame = 0;
    uint8_t textId = 0;
    uint8_t slot0 = 0;
    std::array<uint8_t, 4> slot1{};
    uint8_t slot2 = 0;
};

namespace PrSqevs1 {
    void Init(PrGameContext& ctx);
    void Shutdown(PrGameContext& ctx);
    void Update(PrGameContext& ctx);

    void SetEventSourceBytes(const uint8_t* data, size_t size);

    void Emit(uint32_t id, uint32_t a0, uint32_t a1, uint32_t a2);
    
    // 字幕系统
    const SubtitleInfo* GetActiveSubtitle();
    bool HasActiveSubtitle();

    // Active parser event = last text-bearing event emitted by PumpEvents.
    // This is useful for text-layer debugging, but is not the HUD comparison authority.
    bool ResolveActiveParserEvent(const PrGameContext& ctx, PrStage1ResolvedTextEvent& out);

    // Latest parser snapshot = latest parser event resolved against the current
    // parser query frame. This is the single authority for Stage1 HUD comparison/debug.
    bool GetLatestResolvedSnapshot(PrStage1ResolvedTextEvent& out, uint32_t& outQueryFrame);
    bool ResolveLatestParserEvent(const PrGameContext& ctx, PrStage1ResolvedTextEvent& out);

    // 事件表加载（从 COMPO01.INT MEM 段）
    bool LoadEventTable(const uint8_t* data, size_t size);
    int GetEventCount();
    uint32_t GetLastEventFrame();
    uint32_t GetLastSubtitleFrame();

    void PumpEvents(uint32_t stageFrame, PrGameContext& ctx);
}
