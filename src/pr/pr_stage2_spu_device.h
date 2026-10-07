#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

namespace PrStage2SpuDevice {
// Sample-clock implementation for normal ADPCM voices. No global Stage1 state,
// host SFX command receipts, or display-frame driven envelopes. Unsupported
// noise, pitch modulation, reverb DSP, sweep and SPU IRQ/DMA modes fail closed.
// Register timing is serialized at host sample-block boundaries, not CPU cycles.
enum class Phase { Off, Attack, Decay, Sustain, Release };
struct Envelope {
    Phase phase = Phase::Off;
    int32_t level = 0;
    uint32_t counter = 0;
};
void StepEnvelope(Envelope& envelope, uint16_t adsr1, uint16_t adsr2);
std::array<int16_t,28> DecodeBlock(const std::array<uint8_t,16>& block,
                                  int32_t& previous, int32_t& older);
int16_t Interpolate(const std::array<int16_t,4>& history, uint32_t fractionalPitch);

class Device final {
public:
    static constexpr uint32_t RamBytes = 512u*1024u;
    Device();
    bool TryRead(uint32_t address, uint32_t width, uint32_t& result) const;
    bool TryWrite(uint32_t address, uint32_t width, uint32_t value);
    // Only the concrete FIFO/DMA owner may enable its implemented transfer
    // mode. CD mixing may be configured with an explicitly idle CD producer;
    // no CD-play API or synthesized external samples are provided here.
    void ApplyTransferControl(uint16_t value);
    void WriteSdkEndStatusZero();
    // Actual data transport; the caller separately owns DMA descriptors/IRQs.
    void Upload(uint32_t destination, const std::vector<uint8_t>& bytes);
    std::vector<uint8_t> ReadRam(uint32_t address, uint32_t count) const;
    // Exactly `frames` native 44.1 kHz stereo samples. No allocation is needed
    // in the normal render path. Failure is propagated to the output owner.
    void Render(int16_t* stereo, uint32_t frames);
    uint64_t RenderedFrames() const;
    uint64_t NonzeroSamples() const;
    uint64_t RegisterWrites() const;
    uint64_t UploadedBytes() const;
    uint32_t ActiveMask() const;
    // Producer copies real 44.1-kHz stereo samples. Full queues return false
    // without consuming input; the audio sink alone advances the read cursor.
    bool CanQueueCdFrames(size_t frames) const;
    bool QueueCdAudio(const std::vector<int16_t>& stereo);
    uint64_t CdFramesQueued() const;
    uint64_t CdFramesConsumed() const;
    uint64_t CdNonzeroSamples() const;
    uint64_t CdUnderflowFrames() const;
private:
    struct Voice {
        std::array<uint16_t,8> registers{};
        Envelope envelope;
        std::array<int16_t,28> decoded{};
        std::array<int16_t,4> history{};
        uint32_t blockAddress = 0, sample = 28, fraction = 0;
        int32_t previous = 0, older = 0;
        uint8_t flags = 0;
        bool playing = false;
    };
    mutable std::mutex mutex_;
    std::array<Voice,24> voices_{};
    std::array<uint16_t,64> global_{};
    std::vector<uint8_t> ram_, known_;
    uint32_t ended_ = 0;
    uint64_t frames_ = 0, nonzero_ = 0, writes_ = 0, uploads_ = 0;
    std::vector<int16_t> cdQueue_=std::vector<int16_t>(88200u);
    size_t cdRead_=0,cdWrite_=0,cdCount_=0;
    uint64_t cdQueued_=0,cdConsumed_=0,cdNonzero_=0,cdUnderflow_=0;
    bool cdAttached_=false;
    static bool Address(uint32_t address, uint32_t& offset);
    static void Width(uint32_t address, uint32_t width);
    void ValidateBlock(uint32_t address) const;
    int16_t NextSample(Voice& voice, uint32_t index);
    void Start(uint32_t index);
    static int32_t FixedVolume(uint16_t value);
};
}
