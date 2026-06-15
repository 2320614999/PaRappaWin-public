#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "pr_scene_drawbuffer_direct.h"

namespace PrSceneBootWorkListDirect {

static constexpr uint32_t kWorkListBase8001E6D0 = 0x80087288u;
static constexpr uint32_t kWorkListHeadBase8001E6D0 = 0x800872B0u;
static constexpr uint32_t kWorkListStride8001E6D0 = 0x14u;
static constexpr uint32_t kWorkListHeadStride8001E6D0 = 0x40u;
static constexpr size_t kWorkListCount8001E6D0 = 2u;

struct WorkListSeed8001E6D0 {
    bool known = false;
    uint32_t descAddr = 0;
    uint32_t order_00 = 0;
    uint32_t headAddr_04 = 0;
    uint32_t lastAddr_08 = 0;
};

struct WorkListDrawBufferInit8001E6D0 {
    bool known = false;
    uint32_t sourceFunction = 0x8001E6D0u;
    std::array<WorkListSeed8001E6D0, kWorkListCount8001E6D0> workLists{};
    bool workListsKnown = false;
    PrSceneDrawBufferDirect::DrawBufferGlobals8001E33C drawBuffers{};
};

WorkListDrawBufferInit8001E6D0
PsxCall8001E6D0_InitWorkListsAndDrawBuffers();

} // namespace PrSceneBootWorkListDirect
