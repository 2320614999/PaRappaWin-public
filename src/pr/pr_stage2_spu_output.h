#pragma once
#include "pr_stage2_spu_device.h"
#include "../audio_sink.h"
#include <exception>
#include <mutex>

namespace PrStage2SpuOutput {
// Owns only this stream, never the global Stage1 AudioEngine. The supplied sink
// may be the real WasapiSink or a deterministic callback driver for unit tests.
class Stream final {
public:
    Stream(PrStage2SpuDevice::Device& device, IAudioSink& sink);
    ~Stream();
    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;
    void Start();
    void Stop();
    bool Running() const;
    void RethrowFailure() const;
    double SinkSeconds() const;
private:
    PrStage2SpuDevice::Device& device_;
    IAudioSink& sink_;
    bool started_ = false;
    mutable std::mutex mutex_;
    std::exception_ptr failure_;
    static void Render(void*,int16_t*,uint32_t,uint32_t) noexcept;
};
}
