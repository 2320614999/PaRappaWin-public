#include "pr_stage2_spu_device.h"
#include "pr_stage2_spu_gauss.h"
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>

namespace PrStage2SpuDevice {
namespace {
int32_t Shift(int64_t value, uint32_t bits) {
    const int64_t divisor = int64_t(1) << bits;
    return static_cast<int32_t>(value >= 0 ? value / divisor : -((-value + divisor - 1) / divisor));
}
int16_t Clamp(int32_t value) { return static_cast<int16_t>((std::max)(-32768, (std::min)(32767, value))); }
[[noreturn]] void Unsupported(const char* reason) {
    throw std::runtime_error(std::string("S2 SPU: ") + reason);
}
}

void StepEnvelope(Envelope& e, uint16_t a, uint16_t b) {
    if (e.phase == Phase::Off) return;
    uint32_t shift = 0, value = 0, all = 0x7Fu;
    bool decreasing = false, exponential = false;
    switch (e.phase) {
    case Phase::Attack: shift = (a >> 10u) & 31u; value = (a >> 8u) & 3u; exponential = (a & 0x8000u) != 0; break;
    case Phase::Decay: shift = (a >> 4u) & 15u; decreasing = exponential = true; break;
    case Phase::Sustain: shift = (b >> 8u) & 31u; value = (b >> 6u) & 3u;
        decreasing = (b & 0x4000u) != 0; exponential = (b & 0x8000u) != 0; break;
    case Phase::Release: shift = b & 31u; decreasing = true; exponential = (b & 32u) != 0; all = 0x7Cu; break;
    default: return;
    }
    int32_t step = decreasing ? -8 + static_cast<int32_t>(value) : 7 - static_cast<int32_t>(value);
    step *= int32_t(1u << (shift < 11u ? 11u - shift : 0u));
    uint32_t increment = 0x8000u >> (shift > 11u ? shift - 11u : 0u);
    if (exponential && !decreasing && e.level > 0x6000) {
        if (shift < 10u) step = Shift(step, 2);
        else if (shift >= 11u) increment >>= 2u;
        else { step = Shift(step, 1); increment >>= 1u; }
    } else if (exponential && decreasing) step = Shift(int64_t(step) * e.level, 15);
    if ((value | (shift << 2u)) != all) increment = (std::max)(increment, 1u);
    e.counter += increment;
    if ((e.counter & 0x8000u) == 0) return;
    e.counter &= 0x7FFFu;
    e.level = decreasing ? (std::max)(0, e.level + step) : int32_t(Clamp(e.level + step));
    if (e.phase == Phase::Attack && e.level == 32767) { e.phase = Phase::Decay; e.counter = 0; }
    else if (e.phase == Phase::Decay && e.level <= int32_t((uint32_t(a & 15u) + 1u) * 0x800u)) {
        e.phase = Phase::Sustain; e.counter = 0;
    } else if (e.phase == Phase::Release && e.level == 0) { e.phase = Phase::Off; e.counter = 0; }
}

std::array<int16_t,28> DecodeBlock(const std::array<uint8_t,16>& b, int32_t& previous, int32_t& older) {
    const uint32_t filter = b[0] >> 4u, shift = b[0] & 15u;
    // Reserved encodings are outside this port's certified voice boundary.
    if (filter > 4u || shift > 12u || (b[1] & ~7u)) Unsupported("unsupported ADPCM block encoding");
    constexpr int32_t positive[5] = {0,60,115,98,122};
    constexpr int32_t negative[5] = {0,0,-52,-55,-60};
    std::array<int16_t,28> result{};
    int32_t p = previous, q = older;
    for (uint32_t i = 0; i < 28u; ++i) {
        const int32_t nibble = (b[2u+i/2u] >> (4u*(i&1u))) & 15u;
        const int32_t residual = (nibble < 8 ? nibble : nibble - 16) * int32_t(1u << (12u-shift));
        const int32_t predicted = Shift(int64_t(p)*positive[filter] + int64_t(q)*negative[filter] + 32, 6);
        result[i] = Clamp(residual + predicted); q = p; p = result[i];
    }
    previous = p; older = q;
    return result;
}
int16_t Interpolate(const std::array<int16_t,4>& h, uint32_t pitch) {
    const uint32_t i = (pitch >> 4u) & 255u;
    const int32_t value = Shift(int64_t(Gaussian[255u-i])*h[0],15)
        + Shift(int64_t(Gaussian[511u-i])*h[1],15)
        + Shift(int64_t(Gaussian[256u+i])*h[2],15)
        + Shift(int64_t(Gaussian[i])*h[3],15);
    return Clamp(value);
}

Device::Device() : ram_(RamBytes), known_(RamBytes) {}
bool Device::Address(uint32_t address, uint32_t& offset) {
    const uint32_t segment = address & 0xE0000000u;
    if (segment != 0u && segment != 0x80000000u && segment != 0xA0000000u) return false;
    const uint32_t physical = address & 0x1FFFFFFFu;
    if (physical < 0x1F801C00u || physical >= 0x1F801E00u) return false;
    offset = physical - 0x1F801C00u;
    return true;
}
void Device::Width(uint32_t address, uint32_t width) {
    if (width != 2u || (address & 1u)) throw std::invalid_argument("S2 SPU requires aligned 16-bit register transactions");
}
int32_t Device::FixedVolume(uint16_t value) {
    if (value & 0x8000u) Unsupported("volume sweep is not bound");
    const int32_t signed15 = (value & 0x4000u) ? int32_t(value & 0x7FFFu)-32768 : int32_t(value);
    return signed15 * 2;
}
bool Device::TryRead(uint32_t address, uint32_t width, uint32_t& result) const {
    uint32_t offset;
    if (!Address(address, offset)) return false;
    Width(address, width);
    std::lock_guard<std::mutex> lock(mutex_);
    if (offset < 0x180u) {
        const auto& voice = voices_[offset/16u];
        result = (offset & 15u) == 12u ? uint16_t(voice.envelope.level) : voice.registers[(offset & 15u)/2u];
    } else if (offset == 0x19Cu || offset == 0x19Eu) result = (ended_ >> (offset==0x19Cu?0u:16u)) & 65535u;
    else if (offset == 0x1AEu) result = global_[(0x1AAu-0x180u)/2u] & 0x3Fu;
    else if (offset == 0x1B8u || offset == 0x1BAu) result = uint16_t(FixedVolume(global_[(offset-0x1B8u)/2u]));
    else if (offset <= 0x1B6u && offset != 0x1A0u && offset != 0x1A8u) result = global_[(offset-0x180u)/2u];
    else Unsupported("unbound register read");
    return true;
}
void Device::ValidateBlock(uint32_t address) const {
    if ((address & 15u) || address > RamBytes-16u) Unsupported("unsupported ADPCM block alignment or RAM wrap");
    for (uint32_t i=0; i<16u; ++i) if (!known_[address+i]) Unsupported("sample data has not been uploaded");
    if ((ram_[address] >> 4u) > 4u || (ram_[address] & 15u) > 12u || (ram_[address+1u] & ~7u))
        Unsupported("unsupported ADPCM block encoding");
}
void Device::Start(uint32_t index) {
    auto& v = voices_[index];
    v.envelope = {Phase::Attack,0,0}; v.playing = true;
    v.blockAddress = uint32_t(v.registers[3])*8u;
    v.sample = 28u; v.flags = 0u; v.fraction = 0u;
    v.previous = v.older = 0; v.history.fill(0);
    ended_ &= ~(1u << index);
}
bool Device::TryWrite(uint32_t address, uint32_t width, uint32_t value) {
    uint32_t offset;
    if (!Address(address, offset)) return false;
    Width(address, width);
    if (value > 65535u) throw std::invalid_argument("S2 SPU halfword exceeds transport width");
    std::lock_guard<std::mutex> lock(mutex_);
    const uint16_t half = static_cast<uint16_t>(value);
    if (offset < 0x180u) {
        auto& v = voices_[offset/16u]; const uint32_t field = (offset & 15u)/2u;
        if (field < 2u) (void)FixedVolume(half);
        if (field == 6u) { v.envelope.level = int32_t(half & 0x7FFFu); }
        v.registers[field] = half;
    } else {
        const uint32_t i = (offset-0x180u)/2u;
        if (offset == 0x180u || offset == 0x182u) (void)FixedVolume(half);
        else if (offset >= 0x188u && offset <= 0x18Eu) {
            const uint32_t mask = (value & ((offset&2u)?255u:65535u)) << ((offset&2u)?16u:0u);
            if (offset < 0x18Cu) {
                // Reject missing data before publishing any selected key-on.
                for (uint32_t n=0;n<24u;++n) if (mask & (1u<<n)) ValidateBlock(uint32_t(voices_[n].registers[3])*8u);
                for (uint32_t n=0;n<24u;++n) if (mask & (1u<<n)) Start(n);
            } else {
                for (uint32_t n=0;n<24u;++n) if ((mask & (1u<<n)) && voices_[n].playing) {
                    voices_[n].envelope.phase = Phase::Release; voices_[n].envelope.counter = 0;
                }
            }
        } else if (offset >= 0x190u && offset <= 0x196u) {
            if (value & ((offset&2u)?255u:65535u)) Unsupported("pitch modulation or noise is not bound");
        } else if (offset == 0x198u || offset == 0x19Au) {
            // The stored send mask is real state. DSP enable is separately
            // rejected until the reverb buffer/filter implementation exists.
        } else if (offset == 0x184u || offset == 0x186u || offset == 0x1A2u || offset == 0x1A4u ||
                   offset == 0x1A6u || (offset >= 0x1B0u && offset <= 0x1B6u)) {
            // Volume/address configuration alone does not initiate a transfer.
        } else if (offset == 0x1AAu) {
            if (half & 0x00FEu) Unsupported("SPU reverb/IRQ/transfer/external-input mode is not bound");
            if (!(half & 0x8000u)) for (auto& v:voices_) {v.playing=false;v.envelope={};}
        } else if (offset == 0x1ACu) {
            if (half != 4u) Unsupported("nonstandard transfer FIFO control");
        } else Unsupported("unbound register write");
        global_[i] = half;
    }
    ++writes_;
    return true;
}
void Device::Upload(uint32_t destination, const std::vector<uint8_t>& data) {
    if (destination > RamBytes || data.size() > RamBytes-destination) throw std::out_of_range("S2 SPU upload outside RAM");
    std::lock_guard<std::mutex> lock(mutex_);
    std::copy(data.begin(), data.end(), ram_.begin()+destination);
    std::fill(known_.begin()+destination, known_.begin()+destination+data.size(), 1u);
    uploads_ += data.size();
}
void Device::ApplyTransferControl(uint16_t value) {
    // Reverb DSP, SPU address IRQ, and external/reverb mixing remain unbound.
    // This is a device capability boundary, not a masked register receipt.
    if (value & 0x00CEu) Unsupported("unbound SPU DSP/IRQ/external mix mode");
    std::lock_guard<std::mutex> lock(mutex_);
    if (!(value & 0x8000u)) for (auto& voice:voices_) {voice.playing=false;voice.envelope={};}
    global_[(0x1AAu-0x180u)/2u]=value;++writes_;
}
void Device::WriteSdkEndStatusZero() {
    // SDK writes zero to ENDX while resetting. The sample owner, not software,
    // owns subsequent end flags; this does not clear or invent voice endings.
    std::lock_guard<std::mutex> lock(mutex_);++writes_;
}
std::vector<uint8_t> Device::ReadRam(uint32_t address, uint32_t count) const {
    if (address > RamBytes || count > RamBytes-address) throw std::out_of_range("S2 SPU read outside RAM");
    std::lock_guard<std::mutex> lock(mutex_);
    for (uint32_t i=0;i<count;++i) if (!known_[address+i]) Unsupported("unknown sound RAM read");
    return {ram_.begin()+address,ram_.begin()+address+count};
}
int16_t Device::NextSample(Voice& v, uint32_t index) {
    if (v.sample == 28u) {
        if (v.flags & 1u) {
            ended_ |= 1u << index;
            if (!(v.flags & 2u)) { v.envelope={};v.playing=false;return 0; }
            v.blockAddress = uint32_t(v.registers[7])*8u;
        }
        ValidateBlock(v.blockAddress);
        std::array<uint8_t,16> block;
        std::copy_n(ram_.begin()+v.blockAddress,16u,block.begin());
        if (block[1]&4u) v.registers[7] = static_cast<uint16_t>(v.blockAddress/8u);
        v.decoded = DecodeBlock(block,v.previous,v.older);
        v.flags = block[1]; v.blockAddress += 16u; v.sample = 0u;
    }
    return v.decoded[v.sample++];
}
void Device::Render(int16_t* stereo, uint32_t frames) {
    if (!stereo && frames) throw std::invalid_argument("S2 SPU null output buffer");
    std::lock_guard<std::mutex> lock(mutex_);
    for (uint32_t frame=0; frame<frames; ++frame) {
        int32_t left=0,right=0;
        const uint16_t control=global_[(0x1AAu-0x180u)/2u];
        if (control & 0x8000u) for (uint32_t n=0;n<24u;++n) {
            auto& v=voices_[n]; if (!v.playing) continue;
            const int32_t sample=Shift(int64_t(Interpolate(v.history,v.fraction))*v.envelope.level,15);
            left += Shift(int64_t(sample)*FixedVolume(v.registers[0]),15);
            right += Shift(int64_t(sample)*FixedVolume(v.registers[1]),15);
            StepEnvelope(v.envelope,v.registers[4],v.registers[5]);
            if (v.envelope.phase == Phase::Off) {v.playing=false;continue;}
            const uint32_t step=(std::min)(uint32_t(v.registers[2]),0x4000u);
            v.fraction += step;
            while (v.fraction >= 0x1000u) {
                v.fraction -= 0x1000u;
                v.history[0]=v.history[1];v.history[1]=v.history[2];v.history[2]=v.history[3];
                v.history[3]=NextSample(v,n);
                if (!v.playing) break;
            }
        }
        // SPU enable/mute gates voices, not the separate CD audio input.
        if ((control & 0xC000u) != 0xC000u) left=right=0;
        int16_t cdLeft=0,cdRight=0;
        if(cdCount_){
            cdLeft=cdQueue_[2u*cdRead_];cdRight=cdQueue_[2u*cdRead_+1u];
            cdRead_=(cdRead_+1u)%(cdQueue_.size()/2u);--cdCount_;++cdConsumed_;
            cdNonzero_+=(cdLeft!=0)+(cdRight!=0);
        }else if(cdAttached_)++cdUnderflow_;
        if(cdAttached_){
            const uint32_t at=uint32_t(frames_&511u)*2u;
            for(uint32_t channel=0;channel<2;++channel){
                const uint16_t sample=uint16_t(channel?cdRight:cdLeft);
                const uint32_t p=at+channel*1024u;
                ram_[p]=uint8_t(sample);ram_[p+1u]=uint8_t(sample>>8u);known_[p]=known_[p+1u]=1u;
            }
        }
        if(control&1u){
            const uint16_t l=global_[(0x1B0u-0x180u)/2u],r=global_[(0x1B2u-0x180u)/2u];
            const int32_t lv=l<32768u?int32_t(l):int32_t(l)-65536;
            const int32_t rv=r<32768u?int32_t(r):int32_t(r)-65536;
            left+=Shift(int64_t(cdLeft)*lv,15);right+=Shift(int64_t(cdRight)*rv,15);
        }
        left=Shift(int64_t(Clamp(left))*FixedVolume(global_[0]),15);
        right=Shift(int64_t(Clamp(right))*FixedVolume(global_[1]),15);
        stereo[frame*2u]=Clamp(left);stereo[frame*2u+1u]=Clamp(right);
        nonzero_ += (stereo[frame*2u]!=0)+(stereo[frame*2u+1u]!=0);
        ++frames_;
    }
}
uint64_t Device::RenderedFrames() const {std::lock_guard<std::mutex> l(mutex_);return frames_;}
bool Device::CanQueueCdFrames(size_t frames) const {std::lock_guard<std::mutex> l(mutex_);return frames<=cdQueue_.size()/2u-cdCount_;}
bool Device::QueueCdAudio(const std::vector<int16_t>& pcm){
    if(pcm.size()&1u)throw std::invalid_argument("CD audio must contain whole stereo frames");
    std::lock_guard<std::mutex> l(mutex_);const size_t count=pcm.size()/2u,capacity=cdQueue_.size()/2u;
    if(count>capacity-cdCount_)return false;
    for(size_t i=0;i<count;++i){cdQueue_[2u*cdWrite_]=pcm[2u*i];cdQueue_[2u*cdWrite_+1u]=pcm[2u*i+1u];cdWrite_=(cdWrite_+1u)%capacity;}
    cdCount_+=count;cdQueued_+=count;if(count)cdAttached_=true;return true;
}
uint64_t Device::CdFramesQueued() const {std::lock_guard<std::mutex> l(mutex_);return cdQueued_;}
uint64_t Device::CdFramesConsumed() const {std::lock_guard<std::mutex> l(mutex_);return cdConsumed_;}
uint64_t Device::CdNonzeroSamples() const {std::lock_guard<std::mutex> l(mutex_);return cdNonzero_;}
uint64_t Device::CdUnderflowFrames() const {std::lock_guard<std::mutex> l(mutex_);return cdUnderflow_;}
uint64_t Device::NonzeroSamples() const {std::lock_guard<std::mutex> l(mutex_);return nonzero_;}
uint64_t Device::RegisterWrites() const {std::lock_guard<std::mutex> l(mutex_);return writes_;}
uint64_t Device::UploadedBytes() const {std::lock_guard<std::mutex> l(mutex_);return uploads_;}
uint32_t Device::ActiveMask() const {
    std::lock_guard<std::mutex> l(mutex_);uint32_t mask=0;
    for(uint32_t n=0;n<24u;++n)if(voices_[n].playing)mask|=1u<<n;
    return mask;
}
}
