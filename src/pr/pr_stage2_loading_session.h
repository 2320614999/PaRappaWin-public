#pragma once
#include "pr_stage2_scene_entry.h"
#include "pr_stage2_loading_present_direct.h"

namespace PrStage2LoadingSession {
// Native Loading bindings for the existing, stack-preserving S2 entry owner.
// The base scene-entry contract remains available for isolated dependency
// tests. This session cannot replace these seven source calls with host
// success receipts; a host must implement their actual remaining callees.
class Services : public PrStage2SceneEntry::Services {
public:
    using PrStage2SceneEntry::Services::Services;
protected:
    virtual int32_t CallLoadingDevice(uint32_t function,
        std::initializer_list<uint32_t> args) = 0;
    virtual void CallLoadingDeviceVoid(uint32_t function,
        std::initializer_list<uint32_t> args) = 0;
private:
    int32_t CallPlatform(uint32_t function,
        std::initializer_list<uint32_t> args) final {
        int32_t result;
        if (PrStage2LoadingPresentDirect::TryCall(*this, function, args, result))
            return result;
        return CallLoadingDevice(function, args);
    }
    void CallPlatformVoid(uint32_t function,
        std::initializer_list<uint32_t> args) final {
        int32_t ignored;
        if (!PrStage2LoadingPresentDirect::TryCall(*this, function, args, ignored))
            CallLoadingDeviceVoid(function, args);
    }
};
}