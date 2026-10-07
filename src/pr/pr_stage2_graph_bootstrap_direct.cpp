#include "pr_stage2_graph_bootstrap_direct.h"

namespace PrStage2GraphBootstrapDirect {

int32_t InitPrimitiveDispatch8001C1E8(Services& s) {
    // Original instruction order. These are PSX callee identities in shared
    // RAM, never host function pointers or values captured from a replay.
    s.Write32(0x8008EDE0u, 0x8003B9C8u);
    s.Write32(0x8008EDF0u, 0x8003B88Cu);
    s.Write32(0x8008EE00u, 0x8003C4B4u);
    s.Write32(0x8008EE10u, 0x8003C36Cu);
    s.Write32(0x8008EE20u, 0x8003D148u);
    s.Write32(0x8008EE30u, 0x8003CFDCu);
    s.Write32(0x8008EE40u, 0x8003DDA4u);
    s.Write32(0x8008EE50u, 0x8003DC2Cu);
    s.Write32(0x8008EDD8u, 0u);
    s.Write32(0x8008EDDCu, 0u);
    s.Write32(0x8008EDE4u, 0u);
    s.Write32(0x8008EDE8u, 0u);
    s.Write32(0x8008EDECu, 0u);
    s.Write32(0x8008EDF4u, 0u);
    s.Write32(0x8008EDF8u, 0u);
    s.Write32(0x8008EDFCu, 0u);
    s.Write32(0x8008EE04u, 0u);
    s.Write32(0x8008EE08u, 0u);
    s.Write32(0x8008EE0Cu, 0u);
    s.Write32(0x8008EE14u, 0u);
    s.Write32(0x8008EE18u, 0u);
    s.Write32(0x8008EE1Cu, 0u);
    s.Write32(0x8008EE24u, 0u);
    s.Write32(0x8008EE28u, 0u);
    s.Write32(0x8008EE2Cu, 0u);
    s.Write32(0x8008EE34u, 0u);
    s.Write32(0x8008EE38u, 0u);
    s.Write32(0x8008EE3Cu, 0u);
    s.Write32(0x8008EE44u, 0u);
    s.Write32(0x8008EE48u, 0u);
    s.Write32(0x8008EE4Cu, 0u);
    s.Write32(0x8008EE54u, 0u);
    s.Write32(0x8008EE58u, 0u);
    s.Write32(0x8008EE5Cu, 0u);
    s.Write32(0x8008EE60u, 0x8003BF04u);
    s.Write32(0x8008EE70u, 0x8003BD9Cu);
    s.Write32(0x8008EE80u, 0x8003CA9Cu);
    s.Write32(0x8008EE90u, 0x8003C91Cu);
    s.Write32(0x8008EEA0u, 0x8003D72Cu);
    s.Write32(0x8008EEB0u, 0x8003D58Cu);
    s.Write32(0x8008EEC0u, 0x8003E428u);
    s.Write32(0x8008EE64u, 0u);
    s.Write32(0x8008EE68u, 0u);
    s.Write32(0x8008EE6Cu, 0u);
    s.Write32(0x8008EE74u, 0u);
    s.Write32(0x8008EE78u, 0u);
    s.Write32(0x8008EE7Cu, 0u);
    s.Write32(0x8008EE84u, 0u);
    s.Write32(0x8008EE88u, 0u);
    s.Write32(0x8008EE8Cu, 0u);
    s.Write32(0x8008EE94u, 0u);
    s.Write32(0x8008EE98u, 0u);
    s.Write32(0x8008EE9Cu, 0u);
    s.Write32(0x8008EEA4u, 0u);
    s.Write32(0x8008EEA8u, 0u);
    s.Write32(0x8008EEACu, 0u);
    s.Write32(0x8008EEB4u, 0u);
    s.Write32(0x8008EEB8u, 0u);
    s.Write32(0x8008EEBCu, 0u);
    s.Write32(0x8008EEC4u, 0u);
    s.Write32(0x8008EEC8u, 0u);
    s.Write32(0x8008EECCu, 0u);
    s.Write32(0x8008EED0u, 0x8003E26Cu);
    s.Write32(0x8008EED4u, 0u);
    return static_cast<int32_t>(int64_t{0x8003E26C} - 0x100000000LL);
}

int32_t InitPad800354C0(Services& s, uint32_t mode) {
    s.Write32(0x80091830u, mode);
    s.Write32(0x800882F0u, 0xFFFFFFFFu);
    s.Call(0x80035744u, {});
    s.Call(0x800489F0u, {0x20000001u, 0x800882F0u});
    return s.Call(0x80048AE0u, {0u});
}

int32_t InitGraphics8001C470(Services& s) {
    InitPrimitiveDispatch8001C1E8(s);
    PrStage2GraphInitDirect::ResetGraph800446A0(s, 0u);
    InitPad800354C0(s, 0u);
    // InitGraph includes a second ResetGraph. Both resets and the intervening
    // PAD initialization belong to the original control flow; do not coalesce.
    PrStage2GraphInitDirect::InitGraph8003FB9C(s, 320u, 240u, 4u, 0u, 0u);
    PrStage2GraphInitDirect::SetDoubleBufferOffsets80040AE4(s, 0u, 0u, 0u, 240u);
    PrStage2GraphInitDirect::SetScreenCenter80040B84(s);
    PrStage2GpuDirect::SetProjection80040C74(s, 440u);
    return s.Call(0x8001B1B0u, {0u, 0u, 0u});
}
}