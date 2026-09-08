#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <vector>

namespace PrStage1LoaderSpuHal {

constexpr uint32_t kFn80027120 = 0x80027120u;
constexpr uint32_t kFn80027078 = 0x80027078u;
constexpr uint32_t kFn800270D4 = 0x800270D4u;
constexpr uint32_t kFn800270FC = 0x800270FCu;
constexpr uint32_t kFn80026FA4 = 0x80026FA4u;
constexpr uint32_t kFn80026EF8 = 0x80026EF8u;
constexpr uint32_t kFn80026ECC = 0x80026ECCu;

constexpr uint32_t kFnSsVabClose8002DF80 = 0x8002DF80u;
constexpr uint32_t kFn8002E3D8 = 0x8002E3D8u;
constexpr uint32_t kFn8002EB80 = 0x8002EB80u;
constexpr uint32_t kFn8002EEFC = 0x8002EEFCu;
constexpr uint32_t kFn800351B8 = 0x800351B8u;
constexpr uint32_t kFn80034240 = 0x80034240u;
constexpr uint32_t kFn8002EFF4 = 0x8002EFF4u;

constexpr uint32_t kWord800943A8 = 0x800943A8u;
constexpr uint32_t kWord800943AA = 0x800943AAu;
constexpr uint32_t kWord800943AC = 0x800943ACu;
constexpr uint32_t kDword800943B4 = 0x800943B4u;
constexpr uint32_t kDword80094410 = 0x80094410u;

constexpr int16_t kClosedVabSentinel80027120 = -1;
constexpr int16_t kClosedVoiceSentinel = -1;
constexpr int32_t kOpenVabAnySlotArg80027078 = -1;
constexpr int32_t kResetArg80026FA4 = 0;
constexpr uint8_t kCuePitchAdd80026EF8 = 24;

struct Cue4 {
    uint8_t program = 0;
    uint8_t note = 0;
    uint8_t pitch = 0;
    uint8_t volume = 0;
};

struct State {
    int16_t word_800943A8 = kClosedVabSentinel80027120;
    int16_t word_800943AA = kClosedVoiceSentinel;
    int16_t word_800943AC = kClosedVoiceSentinel;
    int32_t dword_800943B4 = 0;
    uint32_t dword_80094410 = kDword80094410;
};

// Software part of 8003226C / 8002E474. Allocation and PCM transfer remain
// explicit lower services; a decoded host bank is not a native VAB handle.
struct VabSlots8002E474 {
    bool initialized = false;
    bool transferReady800555F8 = true;
    uint16_t count801C35F0 = 0;
    uint16_t programCapacity800917A8 = 128;
    std::array<uint8_t, 16> status800928F8{};
    std::array<uint32_t, 16> base801C35F8{};
    std::array<uint32_t, 16> bytes801C35B0{};
};
using VabSpuAllocate8002E87C = bool (*)(uint32_t bytes, int32_t& result, void* user);
void InitializeVabSlots8003226C(VabSlots8002E474& state);
bool OpenVab8002E474(VabSlots8002E474& state, std::vector<uint8_t>& vh,
                    int16_t requestedSlot, VabSpuAllocate8002E87C allocate,
                    void* user, int16_t& result);

enum class ActionKind : uint8_t {
    None = 0,
    DirectWrapper80027120,
    HalSsVabClose8002DF80,
    DirectWrapper80027078,
    HalVabOpen8002E3D8,
    DirectWrapper800270D4,
    HalVabTransfer8002EB80,
    DirectWrapper800270FC,
    HalVabEnable8002EEFC,
    DirectWrapper80026FA4,
    HalSfxReset800351B8,
    DirectWrapper80026EF8,
    HalSfxCue80034240,
    DirectWrapper80026ECC,
    HalSfxFlush8002EFF4,
};

struct Action {
    ActionKind kind = ActionKind::None;
    uint32_t wrapperFunction = 0;
    uint32_t lowerFunction = 0;
    uint32_t pointerArg = 0;
    int32_t arg0 = 0;
    int32_t arg1 = 0;
    int32_t arg2 = 0;
    int32_t arg3 = 0;
    int32_t arg4 = 0;
    int32_t arg5 = 0;
    int32_t arg6 = 0;
    int32_t result = 0;
    int16_t vabIdBefore = kClosedVabSentinel80027120;
    int16_t vabIdAfter = kClosedVabSentinel80027120;
    int16_t voiceBefore = kClosedVoiceSentinel;
    int16_t voiceAfter = kClosedVoiceSentinel;
    int32_t flushBusyFlag = 0;
    Cue4 cueBefore{};
    Cue4 cueAfter{};
    bool lowerResultProvided = false;
    bool skippedByGuard = false;
};

struct ActionList {
    Action actions[16]{};
    uint32_t count = 0;
    bool truncated = false;
};

struct LiveActionInput {
    ActionKind actionKind = ActionKind::None;
    uint32_t vhPtr = 0u;
    bool vhPtrKnown = false;
    uint32_t vbPtr = 0u;
    bool vbPtrKnown = false;
    Cue4 cue{};
    bool cueKnown = false;
    uint32_t cuePointer = kDword80094410;
    int32_t lowerResult = 0;
    bool lowerResultKnown = false;
};

struct LiveActionResult {
    bool produced = false;
    bool incomplete = false;
    ActionKind actionKind = ActionKind::None;
    ActionList actionList{};
    int32_t lowerResult = 0;
    bool lowerResultKnown = false;
};

void Clear(ActionList& out);
bool Append(ActionList& out, const Action& action);

void ApplyPadStartComAudioGlobalResetContract(State& state);

int32_t Apply80027120(State& state, ActionList* out = nullptr);
int32_t Apply80027078(State& state,
                     uint32_t vhPtr,
                     int16_t lowerVabId,
                     ActionList* out = nullptr);
int32_t Apply800270D4(State& state,
                     uint32_t vbPtr,
                     int32_t lowerTransferResult,
                     ActionList* out = nullptr);
int32_t Apply800270FC(State& state,
                     int32_t arg0,
                     int32_t lowerEnableResult,
                     ActionList* out = nullptr);
int32_t Apply80026FA4(State& state,
                     int32_t ignoredCallerArg,
                     int32_t lowerResetResult,
                     ActionList* out = nullptr);
int32_t Apply80026EF8(State& state,
                     Cue4& cue,
                     int16_t lowerVoiceId,
                     uint32_t cuePointer = kDword80094410,
                     ActionList* out = nullptr);
int32_t Apply80026ECC(State& state,
                     int32_t lowerFlushResult,
                     ActionList* out = nullptr);
bool ApplyLiveActionResult(State& state,
                           const LiveActionInput& input,
                           LiveActionResult& out);

}  // namespace PrStage1LoaderSpuHal
