#pragma once
#include "pr_native_movie_frame.h"
#include "pr_stage2_lifecycle_direct.h"
#include <array>
#include <memory>

namespace PrStage2MovieCodecBridge {
using Services = PrStage2LifecycleDirect::Services;
// Source wrappers above the replaced SDK device boundary. Both scalar results
// are discarded at 800273A4. Never return a made-up MMIO address or success code.
void Input80047558(Services&, uint32_t packet, uint32_t mode);
void Output800475D4(Services&, uint32_t destination, uint32_t words);

// A host codec job, not an MDEC/DMA device. It consumes the actual RLE, writes
// the actual source image buffer, then invokes the currently registered output
// callback on an allowed outer host event pass. It owns no hardware registers,
// interrupt flags, emulated clock, scene-global state or input playback policy.
class Output final {
public:
    enum class Phase { Empty, Decoded, Queued, Published, Dispatching, Delivered, Faulted };
    void Input(Services&, uint32_t packet, uint32_t words,
               const std::array<uint8_t,128>& quant,
               const std::array<uint8_t,128>& scale);
    void Request(Services&, uint32_t destination, uint32_t words);
    // A media deadline may hold the image as well as its notification.
    // Interrupt masking alone still permits RAM output (the default).
    bool Service(Services&, bool callbacksAllowed, bool outputAllowed = true);
    void Cancel() noexcept;
    Phase State() const noexcept { return phase_; }
    bool Pending() const noexcept { return phase_ == Phase::Queued || phase_ == Phase::Published; }
    uint64_t DecodedFrames() const noexcept { return decoded_; }
    uint64_t PublishedFrames() const noexcept { return published_; }
    uint64_t CallbackCalls() const noexcept { return callbacks_; }
    uint32_t Destination() const noexcept { return destination_; }
    std::shared_ptr<const PrNativeMovieFrame::Frame> LastCompleted() const noexcept { return completed_; }
private:
    Phase phase_ = Phase::Empty;
    std::shared_ptr<const PrNativeMovieFrame::Frame> pending_, completed_;
    uint32_t control_=0, destination_=0;
    uint64_t decoded_=0, published_=0, callbacks_=0;
    static void RamRange(uint32_t address, uint64_t bytes);
    static void Tables(const std::array<uint8_t,128>& quant,
                       const std::array<uint8_t,128>& scale);
};
}
