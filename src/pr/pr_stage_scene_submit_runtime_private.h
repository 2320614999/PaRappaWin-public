#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

struct PrStage1RuntimeSlotsSnapshot;
struct PrStage1HudPresentationDirectBee4ActionCarrier;
struct PrStage1HudPresentationDirectBee4ConsumeResult;

namespace PrStageSceneSubmitDirect {

struct Stage1SceneSubmitRuntimePrivateAccess;

class Stage1SceneSubmitRuntime {
public:
    Stage1SceneSubmitRuntime();
    ~Stage1SceneSubmitRuntime();
    Stage1SceneSubmitRuntime(Stage1SceneSubmitRuntime&&) noexcept;
    Stage1SceneSubmitRuntime& operator=(Stage1SceneSubmitRuntime&&) noexcept;
    Stage1SceneSubmitRuntime(const Stage1SceneSubmitRuntime&);
    Stage1SceneSubmitRuntime& operator=(const Stage1SceneSubmitRuntime&);

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;

    friend struct Stage1SceneSubmitRuntimePrivateAccess;
};

struct Stage1SceneSubmitRuntimePrivateAccess {
    static const Stage1SceneSubmitRuntime::Impl& ConstImpl(
        const Stage1SceneSubmitRuntime& runtime);
    static Stage1SceneSubmitRuntime::Impl& MutableImpl(
        Stage1SceneSubmitRuntime& runtime);
};

using Stage1SceneResourceProvider801CBFDC = bool (*)(
    uint16_t handle,
    const uint8_t*& outBytes,
    std::size_t& outByteSize,
    void* userData);

bool AdvanceStage1SceneSubmitRuntime801CBFDC190(
    Stage1SceneSubmitRuntime& runtime,
    const PrStage1RuntimeSlotsSnapshot& runtimeSlots,
    const char* scriptBoxTextPtr10C,
    uint8_t renderSubFrame8,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData);

void ResetOwnedStage1SceneSubmitRuntime801CBFDC190();

const Stage1SceneSubmitRuntime& GetOwnedStage1SceneSubmitRuntime801CBFDC190();

bool AdvanceOwnedStage1SceneSubmitRuntime801CBFDC190(
    const PrStage1RuntimeSlotsSnapshot& runtimeSlots,
    const char* scriptBoxTextPtr10C,
    uint8_t renderSubFrame8,
    Stage1SceneResourceProvider801CBFDC resolveResource,
    void* resourceUserData);

bool ApplyStage1SceneSubmitRuntimeHudBee4RawActions801CBFDC190(
    Stage1SceneSubmitRuntime& runtime,
    const PrStage1HudPresentationDirectBee4ActionCarrier& carrier,
    PrStage1HudPresentationDirectBee4ConsumeResult* outResult = nullptr);

bool ApplyOwnedStage1SceneSubmitRuntimeHudBee4RawActions801CBFDC190(
    const PrStage1HudPresentationDirectBee4ActionCarrier& carrier,
    PrStage1HudPresentationDirectBee4ConsumeResult* outResult = nullptr);

} // namespace PrStageSceneSubmitDirect
