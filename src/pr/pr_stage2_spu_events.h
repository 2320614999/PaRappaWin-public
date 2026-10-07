#pragma once
#include <array>
#include <cstdint>
#include <initializer_list>
#include <thread>

namespace PrStage2SpuEvents {
// Native software-event ownership for the SPU's mode-2000 completion event.
// This is not a BIOS instruction interpreter, kernel RAM image, or thread API.
// The source treats descriptors as opaque; no host pointer is encoded in one.
class Controller final {
public:
    explicit Controller(bool interruptsInitiallyEnabled);
    bool InterruptsEnabled() const;
    int32_t EnterCritical();
    void ExitCritical(); // Original syscall preserves scalar registers.
    uint32_t Open(uint32_t eventClass,uint32_t spec,uint32_t mode,uint32_t callback);
    int32_t Close(uint32_t descriptor);
    int32_t Enable(uint32_t descriptor);
    int32_t Disable(uint32_t descriptor);
    int32_t Test(uint32_t descriptor);
    // Used by the retained S2 owner to distinguish a BIOS event descriptor
    // from a memory-card file descriptor, which shares the PSX close syscall.
    bool Owns(uint32_t descriptor) const;
    void Deliver(uint32_t eventClass,uint32_t spec);
    uint64_t Opened() const noexcept{return opened_;}
    uint64_t Deliveries() const noexcept{return deliveries_;}
    uint64_t Consumed() const noexcept{return consumed_;}
    bool TryCall(uint32_t function,std::initializer_list<uint32_t>,int32_t& result);
    bool TryCallVoid(uint32_t function,std::initializer_list<uint32_t>);
private:
    enum class State {Unused,Disabled,Busy,Ready};
    struct Event {State state=State::Unused;uint32_t eventClass=0,spec=0;};
    std::array<Event,16> events_{};
    std::thread::id owner_;
    bool interrupts_;
    uint64_t opened_=0,deliveries_=0,consumed_=0;
    void Owner()const;
    Event* Find(uint32_t);
};
}
