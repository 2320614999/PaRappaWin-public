#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2CallbackInitDirect {
using PrStage2LifecycleDirect::Services;

// SCUS callback startup and interrupt control flow. The platform still owns
// setjmp/HookEntryInt/ReturnFromException semantics and actual device events.
// In particular, never longjmp into a C++ initializer that has already returned.
int32_t ResetCallback80035744(Services& s);
int32_t VSyncCallback800357D4(Services& s, uint32_t callback);
int32_t InitCallbacks800358DC(Services& s);
int32_t DispatchInterrupts800359B8(Services& s);
int32_t ClearCallbackState80035E28(Services& s, uint32_t destination, uint32_t count);
int32_t InitVBlankCallbacks80035E54(Services& s);
int32_t DispatchVBlank80035EAC(Services& s);
int32_t SetVBlankCallback80035F24(Services& s, uint32_t channel, uint32_t callback);
int32_t ClearVBlankWords80035F50(Services& s, uint32_t destination, uint32_t count);
bool TryDispatch(Services& s, uint32_t function,
                 std::initializer_list<uint32_t> arguments, int32_t& result);
}
