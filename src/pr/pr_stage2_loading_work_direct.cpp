#include "pr_stage2_loading_work_direct.h"

namespace PrStage2LoadingWorkDirect {
void RestorePacketBanks8001E34C(PrStage2LifecycleDirect::Services& memory) {
    PrStage2LifecycleDirect::BindDrawBuffers8001E33C(memory, 0x80080CF8u, 0x80083FC0u);
}
void Initialize8001E6D0(PrStage2LifecycleDirect::Services& memory) {
    for (uint32_t buffer = 0u; buffer < 2u; ++buffer) {
        const uint32_t table = 0x80087288u + 20u * buffer;
        // Preserve the original write order and leave +12/+16 and the OT
        // entries untouched. ClearOrderingTable owns those later writes.
        memory.Write32(table + 4u, 0x800872B0u + 64u * buffer);
        memory.Write32(table, 4u);
        memory.Write32(table + 8u, 0u);
    }
    RestorePacketBanks8001E34C(memory);
}
}
