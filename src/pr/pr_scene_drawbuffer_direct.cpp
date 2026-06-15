#include "pr_scene_drawbuffer_direct.h"

namespace PrSceneDrawBufferDirect {

DrawBufferGlobals8001E33C PsxCall8001E33C_SetDrawBufferGlobals(
    uint32_t a1,
    uint32_t a2) {
    DrawBufferGlobals8001E33C out{};
    out.known = true;
    out.sourceFunction = 0x8001E33Cu;
    out.arg0Known = true;
    out.arg0 = a1;
    out.arg1Known = true;
    out.arg1 = a2;
    out.psxWouldWriteGpPlus310 = true;
    out.psxWouldWriteGpPlus314 = true;
    out.hostSideEffects = false;
    out.gpPlus310 = a1;
    out.gpPlus314 = a2;
    return out;
}

DrawBufferGlobals8001E33C PsxCall8001E34C_SetMainDrawBuffers() {
    DrawBufferGlobals8001E33C out =
        PsxCall8001E33C_SetDrawBufferGlobals(kDrawBufferBase80080CF8,
                                             kDrawBufferBase80083FC0);
    out.sourceFunction = 0x8001E34Cu;
    out.calleeFunction = 0x8001E33Cu;
    return out;
}

} // namespace PrSceneDrawBufferDirect
