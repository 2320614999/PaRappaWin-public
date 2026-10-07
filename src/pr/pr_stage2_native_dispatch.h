#pragma once
#include "pr_stage2_image_source_direct.h"

namespace PrStage2NativeDispatch {
using BaseServices = PrStage2LifecycleDirect::Services;

// Closed source-level routes for translated SCUS graph/GPU/TIM/IRQ functions.
// Unknown identities leave result untouched. Known ABI mismatches throw before
// any device effect. This is native C++ routing, not PSX instruction execution.
bool TryCall(BaseServices& services, uint32_t function,
             std::initializer_list<uint32_t> arguments, int32_t& result);
// A caller may discard a scalar result, but may not invent one for a void entry.
bool TryCallVoid(BaseServices& services, uint32_t function,
                 std::initializer_list<uint32_t> arguments);

// Shared native call ownership for a device adapter. Memory, BIOS, clocks,
// uploads and scheduling are still explicit adapter responsibilities; no test
// fixture or legacy StageRunner is used as a fallback by this implementation.
class Services : public PrStage2GpuDirect::ImageSourceServices {
protected:
    virtual int32_t CallExternal(uint32_t function,
                                std::initializer_list<uint32_t> arguments) = 0;
    virtual void CallExternalVoid(uint32_t function,
                                  std::initializer_list<uint32_t> arguments) = 0;
public:
    int32_t Call(uint32_t function, std::initializer_list<uint32_t> arguments) final;
    void CallVoid(uint32_t function, std::initializer_list<uint32_t> arguments) final;
};
}
