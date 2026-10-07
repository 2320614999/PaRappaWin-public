#include "pr_stage2_frame_wait.h"

namespace PrStage2FrameWait {
namespace {
int32_t Signed(uint32_t v) {
    return v <= 0x7FFFFFFFu ? static_cast<int32_t>(v)
        : static_cast<int32_t>(static_cast<int64_t>(v) - 0x100000000LL);
}
void Counter(PrStage2LifecycleDirect::Services& s, Progress& progress, uint32_t target) {
    while (Signed(s.Read32(0x80057034u)) < Signed(target))
        progress.Await(PrStage2FrameTask::WaitKind::VBlankCounter, target);
}
}
int32_t VSync80035560(PrStage2LifecycleDirect::Services& s, Progress& progress, int32_t argument) {
    // These reads happen even for query-only arguments in the original.
    const uint32_t initialGpu = s.Read32(0x80055F68u);
    const uint32_t initialTimer = s.Read32(0x80055F6Cu);
    (void)s.Read32(initialGpu);
    const uint32_t timer = s.Read32(initialTimer);
    const uint32_t previousTimer = s.Read32(0x80055F70u);
    const int32_t delta = static_cast<int32_t>((timer - previousTimer) & 0xFFFFu);
    if (argument < 0) return Signed(s.Read32(0x80057034u));
    if (argument == 1) return delta;

    const uint32_t previous = s.Read32(0x80055F74u);
    const uint32_t target = previous + (argument > 0 ? static_cast<uint32_t>(argument) - 1u : 0u);
    Counter(s, progress, target);
    const uint32_t gpu = s.Read32(s.Read32(0x80055F68u));
    const uint32_t next = s.Read32(0x80057034u) + 1u;
    Counter(s, progress, next);
    if (gpu & 0x00080000u) {
        // The field-poll address is cached once here in the original, unlike
        // the earlier pointer reloads. Do not re-resolve it on every frame.
        const uint32_t address = s.Read32(0x80055F68u);
        while (((gpu ^ s.Read32(address)) & 0x80000000u) == 0u)
            progress.Await(PrStage2FrameTask::WaitKind::GpuField, gpu);
    }
    const uint32_t counter = s.Read32(0x80057034u);
    const uint32_t timerAddress = s.Read32(0x80055F6Cu);
    s.Write32(0x80055F74u, counter);
    const uint32_t finalTimer = s.Read32(timerAddress);
    s.Write32(0x80055F70u, finalTimer);
    return delta;
}
}
