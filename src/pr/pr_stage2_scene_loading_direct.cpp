#include "pr_stage2_scene_loading_direct.h"

namespace PrStage2SceneLoadingDirect {
namespace {
int32_t Signed(uint32_t value) {
    return value <= 0x7FFFFFFFu ? static_cast<int32_t>(value)
        : static_cast<int32_t>(static_cast<int64_t>(value) - 0x100000000LL);
}
}
int32_t LoadingFrame8001537C(Services& s) {
    if (s.Read32(0x8006ECD4u) == 0u) {
        s.Call(0x8001EA74u, {1u, 0u});
        s.Call(0x80026ECCu, {});
        // Both callees may change the shared flag. Preserve the original LW
        // after the calls and ADDIU wrap, rather than replacing this with 1.
        const uint32_t next = s.Read32(0x8006ECD4u) + 1u;
        s.Write32(0x8006ECD4u, next);
        return Signed(next);
    }
    const int32_t result = s.Call(0x8001EBF4u, {0u});
    s.Write32(0x8006ECD4u, 0u);
    return result;
}
int32_t BeginLoading80015408(Services& s, uint32_t mode) {
    s.Write16(0x800916E0u, static_cast<uint16_t>(mode));
    s.Write32(0x8006ECD4u, 0u);
    return s.Call(0x800357D4u, {0x8001537Cu});
}
int32_t EndLoading8001545C(Services& s) {
    s.Call(0x800357D4u, {0u});
    // LW occurs AFTER unregistering the callback. IDA's folded return 0
    // omits both the mutable value and the conditional final drawing call.
    const uint32_t drawn = s.Read32(0x8006ECD4u);
    if (drawn == 1u) return s.Call(0x8001EBF4u, {0u});
    return Signed(drawn);
}
int32_t LoadSaveResources80015590(Services& s,uint32_t scene) {
    BeginLoading80015408(s,3u);
    const uint32_t record=0x80054758u+scene*364u+288u;
    s.Call(0x8001AC18u,{record,1u});
    return EndLoading8001545C(s);
}
int32_t LoadSceneResources80015660(Services& s, uint32_t scene,
                                uint32_t mode, uint32_t loadingSound) {
    s.Write16(0x800916E0u, static_cast<uint16_t>(mode));
    BeginLoading80015408(s, mode);
    // Original unsigned shifts/ADDU/SUBU compute 364*scene, wrapping at 32
    // bits. For scene 2 this is the real COMPO02 descriptor at 80054A60.
    const uint32_t record = 0x80054758u + scene * 364u + 48u;
    s.Call(0x8001AC18u, {record, loadingSound});
    // The original ignores the loader scalar result, but NOT its effects or
    // failures. Do not replace the call above with a success receipt.
    return EndLoading8001545C(s);
}
bool TryCall(Services& s, uint32_t function,
             std::initializer_list<uint32_t> args, int32_t& result) {
    size_t arity;
    switch (function) {
    case 0x80015408u: arity = 1u; break;
    case 0x80015590u: arity = 1u; break;
    case 0x8001537Cu: case 0x8001545Cu: arity = 0u; break;
    case 0x80015660u: arity = 3u; break;
    default: return false;
    }
    if (args.size() != arity)
        throw std::invalid_argument("S2 loading call argument count mismatch");
    const auto a = args.begin();
    switch (function) {
    case 0x8001537Cu: result = LoadingFrame8001537C(s); break;
    case 0x80015408u: result = BeginLoading80015408(s, a[0]); break;
    case 0x8001545Cu: result = EndLoading8001545C(s); break;
    case 0x80015590u: result = LoadSaveResources80015590(s,a[0]); break;
    case 0x80015660u: result = LoadSceneResources80015660(s, a[0], a[1], a[2]); break;
    }
    return true;
}
}
