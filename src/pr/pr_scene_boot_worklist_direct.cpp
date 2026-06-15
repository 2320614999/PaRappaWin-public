#include "pr_scene_boot_worklist_direct.h"

namespace PrSceneBootWorkListDirect {

WorkListDrawBufferInit8001E6D0
PsxCall8001E6D0_InitWorkListsAndDrawBuffers() {
    WorkListDrawBufferInit8001E6D0 out{};
    out.known = true;
    out.sourceFunction = 0x8001E6D0u;
    out.workListsKnown = true;

    uint32_t headAddr = kWorkListHeadBase8001E6D0;
    for (size_t i = 0; i < out.workLists.size(); ++i) {
        WorkListSeed8001E6D0& seed = out.workLists[i];
        seed.known = true;
        seed.descAddr =
            kWorkListBase8001E6D0 +
            static_cast<uint32_t>(i) * kWorkListStride8001E6D0;
        seed.order_00 = 4u;
        seed.headAddr_04 = headAddr;
        seed.lastAddr_08 = 0u;
        headAddr += kWorkListHeadStride8001E6D0;
    }

    out.drawBuffers =
        PrSceneDrawBufferDirect::PsxCall8001E33C_SetDrawBufferGlobals(
            PrSceneDrawBufferDirect::kDrawBufferBase80080CF8,
            PrSceneDrawBufferDirect::kDrawBufferBase80083FC0);
    out.drawBuffers.sourceFunction = 0x8001E6D0u;
    out.drawBuffers.calleeFunction = 0x8001E33Cu;
    return out;
}

} // namespace PrSceneBootWorkListDirect
