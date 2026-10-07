#pragma once
#include "pr_stage2_data_services.h"
#include <filesystem>
#include <stdexcept>

namespace PrStage2SceneEntry {
using Memory = PrStage2LifecycleDirect::Services;
using Arguments = std::initializer_list<uint32_t>;
constexpr uint32_t OverlayBase = 0x801C3870u;
constexpr uint32_t OverlayBytes = 79904u;
constexpr uint32_t SceneRecord = 0x80054A24u;

// Only whole, already translated S2 functions are registered here. The two
// runtime-TIM loop fragments are not separate original entry points.
bool TryCall(Memory& memory, uint32_t function, Arguments args, int32_t& result);
bool TryCallVoid(Memory& memory, uint32_t function, Arguments args);

enum class Phase { Loaded, GlobalsReturned, InitializerReturned, Running, Returned, Failed,
                   GlobalsInitializing, SceneInitializing };

// One native S2 image plus the SCUS data owner. No instruction interpreter,
// legacy StageRunner, full-RAM zero fill, fabricated device success or test
// fixture is used here. A separate platform owner must implement its services.
// RunScene is deliberately synchronous like 801C74E4; a Windows frame adapter
// must preserve the call stack across waits before wiring it into PrScn2::Fn2.
class Services : public PrStage2DataServices::Services {
public:
    Services(const std::filesystem::path& scus, const std::filesystem::path& overlay);
    int32_t InitializeGlobals();
    int32_t InitializeScene();
    int32_t RunScene();
    Phase GetPhase() const noexcept { return phase_; }
protected:
    virtual int32_t CallPlatform(uint32_t function, Arguments args) = 0;
    virtual void CallPlatformVoid(uint32_t function, Arguments args) = 0;
    // 80094438 may point at the original S2 nullsub only after the retained
    // device owner installs that callback; isolated entry tests stay fail-closed.
    virtual bool SupportsNullsub801C9728() const { return false; }
private:
    int32_t CallExternal(uint32_t function, Arguments args) final;
    void CallExternalVoid(uint32_t function, Arguments args) final;
    void Require(Phase expected) const;
    Phase phase_ = Phase::Loaded;
};
}
