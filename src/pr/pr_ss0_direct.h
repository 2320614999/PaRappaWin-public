#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace PrSS0Direct {

enum class Unit : uint8_t {
    Scene0Lifecycle,
    TitleMenuState,
    TitleHudEvents,
    TitleRender,
    StrCdMdec,
    ResourceAudioPreload,
    SfxCueBridge,
    Transition,
    DirectoryDispatcher,
    EventFrameLoop,
    EventText,
    DirectoryPagesRender,
    PromptCardRender,
    PracticeLifecycle,
    StageProgressBank,
    CardMemcard,
    Stage1SavePage,
};

enum class EndpointCategory : uint8_t {
    Lifecycle,
    Resource,
    Audio,
    StrCd,
    Render,
    Transition,
    EventFrame,
    Dispatcher,
    Handoff,
    ProgressBank,
    Card,
};

enum class GapPriority : uint8_t {
    P0,
    P1,
};

struct StateSpec {
    const char* name = nullptr;
    uint32_t ownerFunction = 0;
    const char* enter = nullptr;
    const char* update = nullptr;
    const char* exit = nullptr;
    const char* next = nullptr;
};

struct EndpointSpec {
    uint32_t primaryFunction = 0;
    EndpointCategory category = EndpointCategory::Lifecycle;
    Unit owner = Unit::Scene0Lifecycle;
    const char* name = nullptr;
    const char* callers = nullptr;
    const char* params = nullptr;
    const char* ordering = nullptr;
};

struct RawTableSpec {
    uint32_t address = 0;
    const char* name = nullptr;
    const char* shape = nullptr;
    const char* consumers = nullptr;
    const char* ss0Use = nullptr;
};

struct GapSpec {
    const char* id = nullptr;
    GapPriority priority = GapPriority::P0;
    Unit owner = Unit::Scene0Lifecycle;
    const char* missing = nullptr;
    const char* impact = nullptr;
};

constexpr uint32_t kComod0MappedBase = 0x801C3870u;
constexpr uint32_t kScene0Fn0InitGlobals = 0x801C5B14u;
constexpr uint32_t kScene0Fn1Init = 0x801C4260u;
constexpr uint32_t kScene0Fn2RunMovie0AndMenu = 0x801C4DC4u;
constexpr uint32_t kTitleMenuLoop = 0x801C4894u;
constexpr uint32_t kTitleSelectorInput = 0x801C47ECu;
constexpr uint32_t kTitleRenderInit = 0x801C609Cu;
constexpr uint32_t kTitleRenderDriver = 0x801C6410u;
constexpr uint32_t kTitlePresentMovie0T = 0x801C689Cu;

const std::array<StateSpec, 11>& StateSpecs();
const std::array<EndpointSpec, 111>& EndpointSpecs();
const std::array<RawTableSpec, 69>& RawTableSpecs();
const std::array<GapSpec, 25>& GapSpecs();

const char* UnitName(Unit unit);
const char* EndpointCategoryName(EndpointCategory category);
const char* GapPriorityName(GapPriority priority);
bool RuntimeCutoverAllowed();
std::size_t P0GapCount();

} // namespace PrSS0Direct
