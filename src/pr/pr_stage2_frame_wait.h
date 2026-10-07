#pragma once
#include "pr_stage2_lifecycle_direct.h"
#include "pr_stage2_frame_task.h"

namespace PrStage2FrameWait {
// Host scheduling boundary, not an implementation of a PSX CPU timer.
// Must return only to recheck after an observed event or cancellation/error.
// The event owner performs native IRQ/callback work and publishes actual MMIO.
struct Progress {
    virtual ~Progress() = default;
    virtual void Await(PrStage2FrameTask::WaitKind kind, uint32_t target) = 0;
};
class TaskProgress final : public Progress {
public:
    explicit TaskProgress(PrStage2FrameTask::Task& task) : task_(task) {}
    void Await(PrStage2FrameTask::WaitKind kind, uint32_t target) override {
        task_.Suspend(kind, 0x80035560u, target);
    }
private:
    PrStage2FrameTask::Task& task_;
};
// Windows event-wait adaptation of 80035560's successful/query paths.
// Preserves ordered RAM/MMIO accesses, two signed targets, timer result and
// interlace-field gate. Does NOT reproduce 800356A8's CPU-spin timeout: a host
// failure/cancel unwinds rather than pretending a VBlank or returning success.
// This function never advances a counter or supplies device status itself.
int32_t VSync80035560(PrStage2LifecycleDirect::Services& memory,
                     Progress& progress, int32_t argument);
}
