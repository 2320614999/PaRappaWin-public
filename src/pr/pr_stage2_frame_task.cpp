#include "pr_stage2_frame_task.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <limits>
#include <stdexcept>
#include <system_error>
#include <utility>

namespace PrStage2FrameTask {
namespace {
struct CancelSignal {};
[[noreturn]] void WinError(const char* operation) {
    throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), operation);
}
}
struct Task::Impl {
    const DWORD thread = GetCurrentThreadId();
    Entry entry;
    Event event;
    std::exception_ptr failure;
    void* fiber = nullptr;
    void* caller = nullptr;
    State state = State::Ready;
    Request request{};
    uint64_t serial = 0;
    int32_t result = 0;
    bool cancelled = false;

    explicit Impl(Entry body) : entry(std::move(body)) {
        if (!entry) throw std::invalid_argument("S2 task requires an entry");
        fiber = CreateFiberEx(64u * 1024u, 8u * 1024u * 1024u,
                              FIBER_FLAG_FLOAT_SWITCH, &Run, this);
        if (!fiber) WinError("CreateFiberEx S2");
    }
    void Owner() const {
        if (GetCurrentThreadId() != thread)
            throw std::logic_error("S2 task accessed from another thread");
    }
    static VOID WINAPI Run(void* data) noexcept {
        auto& self = *static_cast<Impl*>(data);
        try {
            if (self.cancelled) throw CancelSignal{};
            self.result = self.entry();
            self.state = self.cancelled ? State::Cancelled : State::Completed;
        } catch (const CancelSignal&) {
            self.state = State::Cancelled;
        } catch (...) {
            self.failure = std::current_exception();
            self.state = State::Failed;
        }
        // All automatic objects in the entry and pending event have unwound.
        // A FiberProc must never return: Windows would terminate its thread.
        self.request = {};
        SwitchToFiber(self.caller);
        std::terminate(); // A completed task is never scheduled a second time.
    }
    State Switch() {
        Owner();
        const bool converted = !IsThreadAFiber();
        void* root = converted ? ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH)
                               : GetCurrentFiber();
        if (!root) WinError("ConvertThreadToFiberEx S2");
        if (root == fiber) throw std::logic_error("Recursive S2 task scheduling");
        caller = root;
        state = State::Running;
        SwitchToFiber(fiber);
        caller = nullptr;
        // Do not revert a conversion made by the caller or another subsystem.
        if (converted && !ConvertFiberToThread()) WinError("ConvertFiberToThread S2");
        return state;
    }
    State Cancel() {
        Owner();
        if (state == State::Running) throw std::logic_error("Cannot cancel a running S2 task");
        if (state == State::Ready) { cancelled = true; state = State::Cancelled; return state; }
        if (state != State::Waiting) return state;
        // Resume at the suspension and throw there. Deleting a suspended fiber
        // directly would discard stack objects without their C++ destructors.
        cancelled = true;
        event = {};
        return Switch();
    }
};

Task::Task(Entry entry) : impl_(std::make_unique<Impl>(std::move(entry))) {}
Task::~Task() noexcept {
    try {
        impl_->Cancel();
        DeleteFiber(impl_->fiber);
    } catch (...) {
        // Wrong-thread destruction is a lifetime bug; do not free a live stack.
        std::terminate();
    }
}
State Task::Start() {
    impl_->Owner();
    if (impl_->state != State::Ready) throw std::logic_error("S2 task already started");
    return impl_->Switch();
}
State Task::Resume(uint64_t ticket, Event event) {
    impl_->Owner();
    if (impl_->state != State::Waiting || ticket != impl_->request.ticket)
        throw std::logic_error("S2 task stale ticket or not waiting");
    impl_->event = std::move(event);
    return impl_->Switch();
}
State Task::Cancel() { return impl_->Cancel(); }
void Task::Suspend(WaitKind kind, uint32_t function, uint32_t target) {
    impl_->Owner();
    if (impl_->state != State::Running || !IsThreadAFiber() || GetCurrentFiber() != impl_->fiber)
        throw std::logic_error("S2 suspension outside its running stack");
    if (impl_->cancelled) throw CancelSignal{};
    if (impl_->serial == std::numeric_limits<uint64_t>::max())
        throw std::overflow_error("S2 wait ticket overflow");
    impl_->request = {++impl_->serial, kind, function, target};
    impl_->state = State::Waiting;
    SwitchToFiber(impl_->caller);
    if (impl_->cancelled) throw CancelSignal{};
    auto event = std::move(impl_->event);
    impl_->event = {};
    // This callback may itself suspend (e.g. a Loading render request).
    // Its stack is retained as part of the same native initialization.
    if (event) event();
    if (impl_->cancelled) throw CancelSignal{};
}
State Task::GetState() const { impl_->Owner(); return impl_->state; }
Request Task::Pending() const {
    impl_->Owner();
    if (impl_->state != State::Waiting) throw std::logic_error("S2 task has no pending wait");
    return impl_->request;
}
int32_t Task::Result() const {
    impl_->Owner();
    if (impl_->state != State::Completed) throw std::logic_error("S2 task has no completed result");
    return impl_->result;
}
void Task::RethrowFailure() const {
    impl_->Owner();
    if (impl_->failure) std::rethrow_exception(impl_->failure);
}
}
