#pragma once
#include "scene_event_parser.h"
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

// Callback type: called when a TIM needs to be loaded.
// Parameters: tim filename (lowercase, no extension), e.g. "f_pamel2"
using TimLoadCallback = std::function<void(const std::string& timName)>;

enum class FaceEventMode {
    Full,
    Slot2Only,
    MenuIdle,
};

// Runtime state for one HUD overlay slot
struct OverlaySlotState {
    bool active = false;
    int  tableSlot = -1;        // which slot descriptor (0-5)
    int  currentIndex = 0;      // current entry index
    uint32_t baseFrame = 0;     // frame when this slot was activated
    bool pendingRestore = false;
    uint32_t exhaustedFrame = 0;
    std::vector<std::string> restoreTims;
};

class FaceEventProcessor {
public:
    // Initialize with parsed scene data and the index-to-filename mapping.
    // memFileNames: ordered list of Mem block filenames (index 0 = first file).
    // dword_80091858 mapping: index i → memFileNames[i-1]
    void Init(const SceneEventData& data, const std::vector<std::string>& memFileNames);

    // Reset state for a new playthrough (or loop)
    void Reset();

    // Process one frame. Call this every frame with the current global frame count.
    // The callback will be invoked for each TIM that needs to be loaded into VRAM.
    void Update(uint32_t currentFrameAbs, FaceEventMode mode, const TimLoadCallback& loadTim);

    void RestoreBase(const TimLoadCallback& loadTim);

    void SetSmileEyeHoldFrames(uint32_t holdFrames);

    // MenuIdle: set cursor-driven pointing direction.
    // dir: 0=left (START), 1=right (MENU), -1=unknown/disable finger overlay.
    void SetMenuIdlePointDir(int dir) { m_menuIdlePointDir = dir; }

    // Get the dance start frame (Event 0 frame)
    uint32_t GetDanceStartFrame() const;

    // Get the max frame in parsed face-event timeline
    uint32_t GetMaxFrame() const;

    bool IsValid() const { return m_valid; }

private:
    static std::string TimFamilyKey(const std::string& timName);
    void RememberBaseTim(const std::string& timName);
    void ApplyTim(const std::string& timName, const TimLoadCallback& loadTim, bool rememberBase);
    void BuildRestoreListForSlot(int slotId, OverlaySlotState& slot);

    // Convert a dword_80091858 index to a TIM filename
    std::string IndexToTimName(int index) const;

    // Process an event using PSX flag-based dispatch
    void ProcessEvent(const SceneEvent& evt, uint32_t currentFrameAbs, FaceEventMode mode, const TimLoadCallback& loadTim);

    // Activate a HUD overlay slot
    void ActivateOverlaySlot(int slotId, uint32_t baseFrame);

    // Process active overlay slots
    void ProcessOverlaySlots(uint32_t currentFrame, const TimLoadCallback& loadTim);

    // Load TIM pair from lookup table (for b10/b13/b14)
    void LoadTimPair(int lookupIndex, uint32_t currentFrameAbs, FaceEventMode mode, const TimLoadCallback& loadTim, const char* source, bool applySecond);

    // Convert a face-TIM-only index (1-based, used by timPairTable) to mem index
    // face TIM index 1 = first face TIM in Mem block (e.g. F_PAKU_0)
    std::string FaceTimIndexToName(int faceTimIndex) const;

    SceneEventData m_data;
    std::vector<std::string> m_memFileNames;
    int m_eventIndex = 0;                   // next event to process
    int m_faceTIMOffset = -1;               // 0-based offset of first face TIM within memFileNames
    OverlaySlotState m_slots[6];            // runtime state for overlay slots
    std::unordered_map<std::string, std::string> m_baseFamilyToTim;
    std::unordered_map<std::string, std::string> m_currentFamilyToTim;
    uint32_t m_smileEyeHoldFrames = 0;
    bool m_smileEyeActive = false;
    uint32_t m_smileEyeExpireFrameAbs = 0;
    std::string m_smilePrevEyeL;
    std::string m_smilePrevEyeR;
    uint32_t m_lastFrameInCycle = 0;
    bool m_cycleInit = false;
    bool m_valid = false;

    uint32_t m_menuIdleOverlayFrameAbs = 0;
    uint32_t m_menuIdleNextBlinkFrameAbs = 0;
    uint32_t m_menuIdleRng = 0;

    FaceEventMode m_lastMode = FaceEventMode::Full;
    int m_menuIdlePointDir = -1;
    int m_menuIdleSlotForFL = -1;
    int m_menuIdleSlotForFR = -1;
    std::unordered_map<std::string, int> m_patMaxRankByFamily;
};
