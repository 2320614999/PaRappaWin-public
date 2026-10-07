#pragma once
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>

namespace PrStage2FrameTask {
enum class State { Ready, Running, Waiting, Completed, Cancelled, Failed };
enum class WaitKind { VBlankCounter, GpuField, PlatformDependency, NativePresentation };
struct Request {
    uint64_t ticket = 0;
    WaitKind kind = WaitKind::VBlankCounter;
    uint32_t function = 0;
    uint32_t target = 0;
};

// A single-threaded Windows stackful task for synchronous S2 source calls.
// Suspension retains local INT descriptors and all C++ object lifetimes.
// No new OS thread, PSX interpreter, fake return value or implicit frame tick.
// Destroy/cancel on the creating thread, before destroying captured Services.
class Task final {
public:
    using Entry = std::function<int32_t()>;
    using Event = std::function<void()>;
    explicit Task(Entry entry);
    ~Task() noexcept;
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    Task(Task&&) = delete;
    Task& operator=(Task&&) = delete;
    State Start();
    // Execute an observed host event ON the retained task stack, then recheck
    // the wait condition. No event means merely a recheck, never completion.
    State Resume(uint64_t ticket, Event event = {});
    State Cancel();
    void Suspend(WaitKind kind, uint32_t function, uint32_t target);
    State GetState() const;
    Request Pending() const;
    int32_t Result() const;
    void RethrowFailure() const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
