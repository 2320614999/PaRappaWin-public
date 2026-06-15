#pragma once

#include <cstdint>
#include <vector>

// 按键类型（对应 PSX pad）
enum class BeatButton : uint8_t {
    None = 0,
    Cross    = 0x01,  // X
    Circle   = 0x02,  // O
    Square   = 0x04,  // □
    Triangle = 0x08,  // △
    Any      = 0x0F   // 任意按键
};

// 单个节拍音符
struct BeatNote {
    int tick96;          // 绝对 tick 位置（96 ticks/beat）
    BeatButton button;   // 期望按键
    uint8_t textId;      // 关联字幕 ID（0=无）
};

// 节拍谱面
class BeatChart {
public:
    BeatChart();
    ~BeatChart();

    // 加载预设谱面（sceneId: 1-9）
    bool LoadPreset(int sceneId);

    // 从数据加载
    bool LoadFromData(const uint8_t* data, size_t size);

    // 清空
    void Clear();

    // 查询
    int GetNoteCount() const { return (int)m_notes.size(); }
    const BeatNote* GetNote(int index) const;

    // 查找当前 tick 附近的音符（用于判定）
    // 返回最近的未击中音符索引，-1 表示没有
    int FindNearestNote(int currentTick96, int toleranceTicks) const;

    // 标记音符已击中
    void MarkNoteHit(int index);
    bool IsNoteHit(int index) const;

    // 重置击中状态
    void ResetHitState();

private:
    std::vector<BeatNote> m_notes;
    std::vector<bool> m_hitFlags;
};

// 全局谱面单例
BeatChart& GetBeatChart();
