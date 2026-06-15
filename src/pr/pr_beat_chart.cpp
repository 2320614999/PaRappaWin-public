#include "pr_beat_chart.h"
#include "logger.h"

#include <cstring>
#include <algorithm>

// 全局单例
static BeatChart s_beatChart;

BeatChart& GetBeatChart() {
    return s_beatChart;
}

BeatChart::BeatChart() {
}

BeatChart::~BeatChart() {
    Clear();
}

void BeatChart::Clear() {
    m_notes.clear();
    m_hitFlags.clear();
}

bool BeatChart::LoadPreset(int sceneId) {
    Clear();

    if (sceneId == 1) {
        // Stage1: "Kick! Punch! It's all in the mind!"
        // BPM=110, 96 ticks/beat
        // 基于歌曲结构创建节拍序列
        // 每小节 4 拍，每拍 96 ticks，一小节 = 384 ticks

        // 前奏（约4小节，从 tick=96 开始）
        // Verse 1: "Kick! Punch! It's all in the mind..."
        // 简化谱面：每拍一个音符，交替按键

        const int kStartTick = 96;  // 从第一拍开始
        const int kBeatTicks = 96;

        // Verse 1: 8小节 (32拍)
        // "Kick!" - X, "Punch!" - O, "It's" - □, "all" - △, "in" - X, "the" - O, "mind" - □
        BeatButton verse1Pattern[] = {
            BeatButton::Cross, BeatButton::Circle, BeatButton::Square, BeatButton::Triangle,
            BeatButton::Cross, BeatButton::Circle, BeatButton::Square, BeatButton::Any
        };

        for (int bar = 0; bar < 8; bar++) {
            for (int beat = 0; beat < 4; beat++) {
                int noteIdx = (bar * 4 + beat) % 8;
                BeatNote note;
                note.tick96 = kStartTick + (bar * 4 + beat) * kBeatTicks;
                note.button = verse1Pattern[noteIdx];
                note.textId = (beat == 0 && bar < 4) ? (uint8_t)(bar + 1) : 0;  // 每小节第一拍触发字幕
                m_notes.push_back(note);
            }
        }

        // Chorus: 4小节 (16拍) - 更密集的节奏
        int chorusStart = kStartTick + 32 * kBeatTicks;
        for (int i = 0; i < 16; i++) {
            BeatNote note;
            note.tick96 = chorusStart + i * kBeatTicks;
            note.button = (i % 2 == 0) ? BeatButton::Cross : BeatButton::Circle;
            note.textId = 0;
            m_notes.push_back(note);
        }

        // Verse 2: 8小节 (32拍) - 同 Verse 1 模式
        int verse2Start = chorusStart + 16 * kBeatTicks;
        for (int bar = 0; bar < 8; bar++) {
            for (int beat = 0; beat < 4; beat++) {
                int noteIdx = (bar * 4 + beat) % 8;
                BeatNote note;
                note.tick96 = verse2Start + (bar * 4 + beat) * kBeatTicks;
                note.button = verse1Pattern[noteIdx];
                note.textId = 0;
                m_notes.push_back(note);
            }
        }

        // Bridge: 4小节 - 半拍节奏 (每拍两个音符)
        int bridgeStart = verse2Start + 32 * kBeatTicks;
        for (int i = 0; i < 32; i++) {
            BeatNote note;
            note.tick96 = bridgeStart + i * (kBeatTicks / 2);
            const BeatButton kCycle[4] = { BeatButton::Cross, BeatButton::Circle, BeatButton::Square, BeatButton::Triangle };
            note.button = kCycle[i % 4];
            note.textId = 0;
            m_notes.push_back(note);
        }

        // Final Chorus: 8小节
        int finalStart = bridgeStart + 16 * kBeatTicks;
        for (int bar = 0; bar < 8; bar++) {
            for (int beat = 0; beat < 4; beat++) {
                BeatNote note;
                note.tick96 = finalStart + (bar * 4 + beat) * kBeatTicks;
                note.button = verse1Pattern[(bar * 4 + beat) % 8];
                note.textId = 0;
                m_notes.push_back(note);
            }
        }

        // 排序确保按时间顺序
        std::sort(m_notes.begin(), m_notes.end(),
            [](const BeatNote& a, const BeatNote& b) { return a.tick96 < b.tick96; });

        m_hitFlags.resize(m_notes.size(), false);

        Log::Printf("BeatChart::LoadPreset: Stage1 loaded %d notes", (int)m_notes.size());
        return true;
    }

    // 其他关卡暂时使用简单的每拍一个音符
    if (sceneId >= 2 && sceneId <= 9 && sceneId != 4) {
        const int kStartTick = 96;
        const int kBeatTicks = 96;
        const int kTotalBeats = 120;  // 约 60 秒
        const BeatButton kCycle[4] = { BeatButton::Cross, BeatButton::Circle, BeatButton::Square, BeatButton::Triangle };

        for (int i = 0; i < kTotalBeats; i++) {
            BeatNote note;
            note.tick96 = kStartTick + i * kBeatTicks;
            note.button = kCycle[i % 4];
            note.textId = 0;
            m_notes.push_back(note);
        }

        m_hitFlags.resize(m_notes.size(), false);
        Log::Printf("BeatChart::LoadPreset: Stage%d loaded %d notes (default)", sceneId, (int)m_notes.size());
        return true;
    }

    return false;
}

bool BeatChart::LoadFromData(const uint8_t* data, size_t size) {
    Clear();

    if (data == nullptr || size < 8) {
        return false;
    }

    // 简单格式: 4字节音符数 + 每个音符8字节 (4字节tick + 1字节button + 1字节textId + 2字节padding)
    uint32_t noteCount = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);

    if (noteCount == 0 || noteCount > 10000 || size < 4 + noteCount * 8) {
        return false;
    }

    for (uint32_t i = 0; i < noteCount; i++) {
        const uint8_t* p = data + 4 + i * 8;
        BeatNote note;
        note.tick96 = (int)(p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
        note.button = (BeatButton)p[4];
        note.textId = p[5];
        m_notes.push_back(note);
    }

    m_hitFlags.resize(m_notes.size(), false);
    Log::Printf("BeatChart::LoadFromData: loaded %d notes", (int)m_notes.size());
    return true;
}

const BeatNote* BeatChart::GetNote(int index) const {
    if (index < 0 || index >= (int)m_notes.size()) {
        return nullptr;
    }
    return &m_notes[index];
}

int BeatChart::FindNearestNote(int currentTick96, int toleranceTicks) const {
    int bestIndex = -1;
    int bestDist = toleranceTicks + 1;

    for (int i = 0; i < (int)m_notes.size(); i++) {
        if (m_hitFlags[i]) continue;  // 已击中的跳过

        int dist = m_notes[i].tick96 - currentTick96;
        // 允许前后各 toleranceTicks 的窗口
        if (dist >= -toleranceTicks && dist <= toleranceTicks) {
            int absDist = (dist < 0) ? -dist : dist;
            if (absDist < bestDist) {
                bestDist = absDist;
                bestIndex = i;
            }
        }
    }

    return bestIndex;
}

void BeatChart::MarkNoteHit(int index) {
    if (index >= 0 && index < (int)m_hitFlags.size()) {
        m_hitFlags[index] = true;
    }
}

bool BeatChart::IsNoteHit(int index) const {
    if (index >= 0 && index < (int)m_hitFlags.size()) {
        return m_hitFlags[index];
    }
    return false;
}

void BeatChart::ResetHitState() {
    std::fill(m_hitFlags.begin(), m_hitFlags.end(), false);
}
