#include "pr_stage2_loading_present_direct.h"
#include "pr_stage2_gpu_direct.h"
#include "pr_stage2_loading_pattern_direct.h"

namespace PrStage2LoadingPresentDirect {
namespace {
int32_t Signed(uint32_t value) {
    return value <= 0x7FFFFFFFu ? static_cast<int32_t>(value)
        : static_cast<int32_t>(static_cast<int64_t>(value) - 0x100000000LL);
}
uint32_t ExtendHalf(uint16_t value) {
    return value < 0x8000u ? uint32_t(value) : uint32_t(value) | 0xFFFF0000u;
}
constexpr uint32_t WorkBuffer = 0x8006EDA8u; // Original GP + 368h.
constexpr uint32_t ClearFlag = 0x8006ED58u;  // Original GP + 318h.
constexpr uint32_t WorkTables = 0x80087288u;
}

int32_t Prepare8001EA74(Services& s, uint32_t drawPattern, uint32_t mode) {
    const uint32_t buffer = static_cast<uint32_t>(s.Call(0x8004019Cu, {}));
    // Read the original packet-bank entry BEFORE publishing the work index.
    const uint32_t packet = s.Read32(0x8006ED50u + 4u * buffer);
    s.Write32(WorkBuffer, buffer);
    s.Write32(ClearFlag, 1u);
    PrStage2GpuDirect::SetWorkBase80040F90(s, packet);
    const uint32_t table = WorkTables + 20u * s.Read32(WorkBuffer);
    (void)s.Call(0x80040CC8u, {0u, 0u, table});
    switch (mode) {
    case 1u:
        return s.Call(0x8001F524u, {0u, 8u});
    case 2u:
        s.Write32(ClearFlag, 0u);
        (void)s.Call(0x8001FCBCu, {8u, 4u});
        return s.Call(0x8001FDC0u, {0u});
    case 3u:
        (void)s.Call(0x80022CBCu, {4u, drawPattern});
        (void)s.Call(0x8001D74Cu, {5u, s.Read32(WorkBuffer)});
        (void)s.Call(0x8001FC40u, {4u, 8u});
        return s.Call(0x8001FDC0u, {0u});
    case 4u:
        (void)s.Call(0x80021E60u, {0u});
        (void)s.Call(0x8001D74Cu, {5u, s.Read32(WorkBuffer)});
        (void)s.Call(0x8001FC40u, {4u, 8u});
        return s.Call(0x8001FDC0u, {0u});
    case 5u: {
        const uint32_t value = static_cast<uint32_t>(
            s.Call(0x80020308u, {ExtendHalf(s.Read16(0x800916DCu))}));
        s.Write32(ClearFlag, value);
        return Signed(value);
    }
    case 6u:
        return s.Call(0x80020248u, {ExtendHalf(s.Read16(0x800916DCu))});
    default:
        if (drawPattern != 0u)
            return s.Call(0x8001EF40u, {ExtendHalf(s.Read16(0x800916E0u)), 0u});
        // 8001EAE8's delay-slot SLL overwrites V0 even on the default branch.
        // IDA incorrectly retains the preceding ClearOrderingTable result.
        return Signed((mode - 1u) << 2u);
    }
}

int32_t Present8001EBF4(Services& s, uint32_t /*unused*/) {
    (void)s.Call(0x80040370u, {});
    // Both reads occur AFTER their preceding callees. Do not use the newly
    // current draw buffer: the source submits its saved preparation index.
    if (s.Read32(ClearFlag) != 0u)
        (void)s.Call(0x80040420u, {255u, 255u, 255u});
    return s.Call(0x80040CA4u, {WorkTables + 20u * s.Read32(WorkBuffer)});
}

int32_t ClearCurrentFramebuffer80040420(Services& s, uint32_t red,
                                      uint32_t green, uint32_t blue) {
    const uint32_t offset = ExtendHalf(s.Read16(0x80096590u)) << 1u;
    PrStage2LifecycleDirect::ImageRect rect;
    rect.x = s.Read16(0x8008ECA8u + offset);
    rect.y = s.Read16(0x8008ECACu + offset);
    rect.width = s.Read16(0x800917FCu);
    rect.height = s.Read16(0x8009182Cu);
    // Native typed transport retains the stack RECT lifetime/queue-copy rules.
    return PrStage2GpuDirect::ClearLocalImage80044CD0(
        s, rect, red & 255u, green & 255u, blue & 255u);
}

bool TryCall(Services& s, uint32_t function,
             std::initializer_list<uint32_t> args, int32_t& result) {
    size_t arity;
    switch (function) {
    case 0x8001EA74u: case 0x8001EF40u: arity = 2u; break;
    case 0x8001EBF4u: case 0x80040CA4u: arity = 1u; break;
    case 0x80040420u: case 0x80040CC8u: arity = 3u; break;
    case 0x80026ECCu: arity = 0u; break;
    default: return false;
    }
    if (args.size() != arity)
        throw std::invalid_argument("S2 Loading presentation argument count mismatch");
    const auto a = args.begin();
    switch (function) {
    case 0x8001EA74u: result = Prepare8001EA74(s, a[0], a[1]); break;
    case 0x8001EBF4u: result = Present8001EBF4(s, a[0]); break;
    case 0x80040420u: result = ClearCurrentFramebuffer80040420(s, a[0], a[1], a[2]); break;
    case 0x80040CC8u:
        result = PrStage2LifecycleDirect::ClearOrderingTable80040CC8(s, a[0], a[1], a[2]); break;
    case 0x80040CA4u:
        result = PrStage2LifecycleDirect::SubmitOrderingTable80040CA4(s, a[0]); break;
    case 0x80026ECCu:
        result = PrStage2LifecycleDirect::FlushAudio80026ECC(s); break;
    case 0x8001EF40u:
        result = PrStage2LoadingPatternDirect::DrawPattern8001EF40(s, a[0], a[1]); break;
    }
    return true;
}
}
