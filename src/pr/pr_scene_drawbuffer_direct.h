#pragma once

#include <cstdint>

namespace PrSceneDrawBufferDirect {

static constexpr uint32_t kDrawBufferBase80080CF8 = 0x80080CF8u;
static constexpr uint32_t kDrawBufferBase80083FC0 = 0x80083FC0u;
static constexpr uint16_t kGpOffsetDrawBufferA8001E33C = 0x310u;
static constexpr uint16_t kGpOffsetDrawBufferB8001E33C = 0x314u;

struct DrawBufferGlobals8001E33C {
    bool known = false;
    uint32_t sourceFunction = 0;
    uint32_t calleeFunction = 0;
    bool arg0Known = false;
    uint32_t arg0 = 0;
    bool arg1Known = false;
    uint32_t arg1 = 0;
    bool psxWouldWriteGpPlus310 = false;
    bool psxWouldWriteGpPlus314 = false;
    bool hostSideEffects = false;
    uint16_t gpPlus310Offset = kGpOffsetDrawBufferA8001E33C;
    uint16_t gpPlus314Offset = kGpOffsetDrawBufferB8001E33C;
    uint32_t gpPlus310 = 0;
    uint32_t gpPlus314 = 0;
};

DrawBufferGlobals8001E33C PsxCall8001E33C_SetDrawBufferGlobals(
    uint32_t a1,
    uint32_t a2);
DrawBufferGlobals8001E33C PsxCall8001E34C_SetMainDrawBuffers();

} // namespace PrSceneDrawBufferDirect
