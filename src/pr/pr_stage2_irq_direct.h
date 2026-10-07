#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2IrqDirect {
using PrStage2LifecycleDirect::Services;

// Source-level SCUS IRQ/DMA ownership. Services supplies actual RAM and MMIO;
// this layer does not manufacture pending interrupts or DMA completion.
int32_t SetInterruptMask800358C0(Services& s, uint32_t mask);
int32_t ClearCallbackWords800361F8(Services& s, uint32_t destination, uint32_t count);
int32_t SetInterruptCallback80035BA0(Services& s, uint32_t channel, uint32_t callback);
int32_t InterruptCallback80035774(Services& s, uint32_t channel, uint32_t callback);
int32_t SetDmaCallback80036150(Services& s, uint32_t channel, uint32_t callback);
int32_t DmaCallback800357A4(Services& s, uint32_t channel, uint32_t callback);
int32_t InitDmaCallbacks80035F7C(Services& s);
int32_t DispatchDmaInterrupt80035FCC(Services& s);

// Thin native Services routing: false leaves unknown functions to the caller.
// Known functions execute through completion, never return a success receipt.
bool TryDispatch(Services& s, uint32_t function,
                 std::initializer_list<uint32_t> arguments, int32_t& result);
}