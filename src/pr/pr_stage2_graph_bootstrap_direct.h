#pragma once
#include "pr_stage2_graph_init_direct.h"

namespace PrStage2GraphBootstrapDirect {
using PrStage2LifecycleDirect::Services;

// SCUS startup control flow, not a second scene owner or a hardware emulator.
// BIOS PAD/callbacks, GTE initialization and 8001B1B0 remain explicit callee
// dependencies. Linking this unit does not switch the native Stage2 entry.
int32_t InitPrimitiveDispatch8001C1E8(Services& s);
int32_t InitPad800354C0(Services& s, uint32_t mode);
int32_t InitGraphics8001C470(Services& s);
}