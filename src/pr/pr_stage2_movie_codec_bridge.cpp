#include "pr_stage2_movie_codec_bridge.h"
#include <stdexcept>
#include <utility>

namespace PrStage2MovieCodecBridge {
namespace { constexpr uint32_t Control = 0x8006ED78u; }
void Input80047558(Services& s, uint32_t packet, uint32_t mode) {
    const uint32_t first = s.Read32(packet);
    s.Write32(packet, (mode & 1u) ? (first & 0xF7FFFFFFu) : (first | 0x08000000u));
    // The second load is real: source instrumentation/reentry may change RAM.
    const uint32_t second = s.Read32(packet);
    s.Write32(packet, (mode & 2u) ? (second | 0x02000000u) : (second & 0xFDFFFFFFu));
    s.CallVoid(0x80047778u, {packet, s.Read16(packet)});
}
void Output800475D4(Services& s, uint32_t destination, uint32_t words) {
    s.CallVoid(0x8004780Cu, {destination, words});
}
void Output::RamRange(uint32_t address, uint64_t bytes) {
    const uint32_t segment = address & 0xE0000000u;
    const uint32_t physical = address & 0x1FFFFFFFu;
    if ((segment != 0u && segment != 0x80000000u && segment != 0xA0000000u) ||
        (address & 3u) || !bytes || uint64_t(physical) + bytes > 0x200000u)
        throw std::out_of_range("Native movie image is outside aligned main RAM");
}
void Output::Tables(const std::array<uint8_t,128>& quant,
                    const std::array<uint8_t,128>& scale) {
    // Data contract of the EXISTING S0 codec, checked against current SCUS
    // 8005D858 and 8005D8DC. Do not silently ignore a different runtime table.
    static constexpr uint8_t q[64] = {
        2,16,16,19,16,19,22,22,22,22,22,22,26,24,26,27,
        27,27,26,26,26,26,27,27,27,29,29,29,34,34,34,29,
        29,29,27,27,29,29,32,32,34,34,37,38,37,35,35,34,
        35,38,38,40,40,40,48,48,46,46,56,56,58,69,69,83
    };
    static constexpr int16_t matrix[64] = {
        23170,23170,23170,23170,23170,23170,23170,23170,
        32138,27245,18204,6392,-6393,-18205,-27246,-32139,
        30273,12539,-12540,-30274,-30274,-12540,12539,30273,
        27245,-6393,-32139,-18205,18204,32138,6392,-27246,
        23170,-23171,-23171,23170,23170,-23171,-23171,23170,
        18204,-32139,6392,27245,-27246,-6393,32138,-18205,
        12539,-30274,30273,-12540,-12540,30273,-30274,12539,
        6392,-18205,27245,-32139,32138,-27246,18204,-6393
    };
    for (size_t i=0; i<128u; ++i) {
        const uint16_t v = uint16_t(matrix[i/2u]);
        if (quant[i] != q[i%64u] || scale[i] != uint8_t(v >> ((i%2u)*8u)))
            throw std::invalid_argument("Native movie codec does not support the supplied quantization/scale tables");
    }
}
void Output::Input(Services& s, uint32_t packet, uint32_t words,
                   const std::array<uint8_t,128>& quant,
                   const std::array<uint8_t,128>& scale) {
    if (phase_ != Phase::Empty && phase_ != Phase::Delivered)
        throw std::logic_error("Native movie input replaced an unfinished codec job");
    Tables(quant, scale);
    if (!words || words > 65535u) throw std::invalid_argument("Native movie RLE word count");
    const uint64_t bytes = 4u + uint64_t(words)*4u;
    RamRange(packet, bytes);
    const uint32_t control = s.Read32(Control);
    const uint32_t width = s.Read32(control+8u), height = s.Read32(control+12u);
    if (!width || !height || width > 1024u || height > 512u || (width & 15u))
        throw std::invalid_argument("Native source movie requires whole 16-pixel strips");
    if (s.Read32(control) != packet || s.Read16(packet) != words)
        throw std::invalid_argument("Native movie input does not belong to this source control block");
    std::vector<uint8_t> input(size_t(bytes), uint8_t(0));
    for (uint32_t i=0; i<bytes; ++i) input[i] = s.Read8(packet+i);
    auto frame = std::make_shared<const PrNativeMovieFrame::Frame>(
        PrNativeMovieFrame::Decode15bppRle(input.data(), input.size(), uint16_t(width), uint16_t(height)));
    // Publish the pending job only after actual shared-codec decoding succeeds.
    pending_ = std::move(frame); control_=control; destination_=0;
    ++decoded_; phase_=Phase::Decoded;
}
void Output::Request(Services& s, uint32_t destination, uint32_t words) {
    if (phase_ != Phase::Decoded || !pending_)
        throw std::logic_error("Native movie output has no completed decode input");
    const uint64_t bytes = uint64_t(words)*4u;
    if (bytes != pending_->strips15.size())
        throw std::invalid_argument("Native movie output length differs from the actual decoded frame");
    RamRange(destination, bytes);
    if (s.Read32(Control) != control_ || s.Read32(control_+4u) != destination ||
        s.Read32(control_+8u) != pending_->width || s.Read32(control_+12u) != pending_->height)
        throw std::logic_error("Native movie output owner/geometry changed before submission");
    destination_=destination; phase_=Phase::Queued;
    // No RAM write or callback here. 800273A4 still has to publish its +32 flag.
}
bool Output::Service(Services& s, bool callbacksAllowed, bool outputAllowed) {
    if (phase_ == Phase::Faulted) throw std::logic_error("Faulted native movie output cannot be completed");
    bool progressed=false;
    try {
        if (phase_ == Phase::Queued && outputAllowed) {
            if (!pending_ || s.Read32(Control) != control_ ||
                s.Read32(control_+4u) != destination_ || s.Read32(control_+32u) != 1u)
                throw std::logic_error("Native movie caller has not published the owned output request");
            for (uint32_t i=0; i<pending_->strips15.size(); ++i)
                s.Write8(destination_+i, pending_->strips15[i]);
            // A write failure leaves its real prefix but publishes neither an
            // image nor a callback. Old successfully returned frames stay valid.
            completed_=pending_; pending_.reset(); ++published_;
            phase_=Phase::Published; progressed=true;
        }
        if (phase_ == Phase::Published && callbacksAllowed) {
            // Original setter 80047658 installs channel-1's callback here.
            // Re-read at delivery; do not cache it at input/submission time.
            const uint32_t callback=s.Read32(0x80057044u);
            phase_=Phase::Dispatching;
            if (callback) { s.CallVoid(callback, {}); ++callbacks_; }
            phase_=Phase::Delivered; progressed=true;
        }
    } catch (...) { phase_=Phase::Faulted; throw; }
    return progressed;
}
void Output::Cancel() noexcept {
    pending_.reset(); phase_=Phase::Empty; control_=destination_=0;
}
}
