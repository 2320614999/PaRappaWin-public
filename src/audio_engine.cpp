#include "audio_engine.h"
#include "logger.h"
#include <algorithm>
#include <cmath>
#include <cstring>

AudioEngine& AudioEngine::Get() {
    static AudioEngine instance;
    return instance;
}

AudioEngine::~AudioEngine() {
    Shutdown();
}

bool AudioEngine::Initialize(uint32_t sampleRate, uint32_t channels) {
    if (m_initialized) return true;

    m_sampleRate = sampleRate;
    m_channels = channels;

    {
        std::lock_guard<std::mutex> lk(m_mutex);
        for (auto& v : m_voices) {
            v = AudioVoice{};
        }
    }

    AudioSinkConfig cfg;
    cfg.sampleRate = sampleRate;
    cfg.channels = channels;
    cfg.bufferMs = 10;

    if (!m_sink.InitializeSink(cfg, RenderCallback, this)) {
        Log::Printf("AudioEngine: WasapiSink init failed");
        return false;
    }

    m_initialized = true;
    Log::Printf("AudioEngine: Init OK rate=%u ch=%u", sampleRate, channels);
    return true;
}

void AudioEngine::Shutdown() {
    if (!m_initialized) return;
    m_sink.ShutdownSink();
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        for (auto& v : m_voices) v = AudioVoice{};
    }
    m_initialized = false;
    Log::Printf("AudioEngine: Shutdown");
}

bool AudioEngine::IsRunning() const {
    return m_initialized && m_sink.IsRunning();
}

int AudioEngine::AllocVoice(int channels, uint32_t sampleRate, float volume, bool oneShot) {
    std::lock_guard<std::mutex> lk(m_mutex);
    for (int i = 0; i < kMaxVoices; i++) {
        if (!m_voices[i].active) {
            auto& v = m_voices[i];
            v = AudioVoice{};
            v.active = true;
            v.channels = channels;
            v.sampleRate = sampleRate > 0 ? sampleRate : m_sampleRate;
            v.volume = std::clamp(volume, 0.0f, 1.0f);
            v.oneShot = oneShot;
            v.samplePos = 0.0;
            v.playedFrames = 0.0;
            v.buffer.clear();
            v.buffer.reserve(channels * m_sampleRate);  // ~1s pre-alloc
            return i;
        }
    }
    Log::Printf("AudioEngine: No free voice");
    return -1;
}

void AudioEngine::FreeVoice(int id) {
    if (id < 0 || id >= kMaxVoices) return;
    std::lock_guard<std::mutex> lk(m_mutex);
    m_voices[id] = AudioVoice{};
}

void AudioEngine::ResetAllVoices() {
    std::lock_guard<std::mutex> lk(m_mutex);
    for (auto& v : m_voices) {
        v = AudioVoice{};
    }
}

void AudioEngine::QueueSamples(int id, const int16_t* samples, size_t count) {
    if (id < 0 || id >= kMaxVoices || !samples || count == 0) return;
    std::lock_guard<std::mutex> lk(m_mutex);
    auto& v = m_voices[id];
    if (!v.active) return;
    v.buffer.insert(v.buffer.end(), samples, samples + count);
}

void AudioEngine::ClearVoice(int id) {
    if (id < 0 || id >= kMaxVoices) return;
    std::lock_guard<std::mutex> lk(m_mutex);
    auto& v = m_voices[id];
    v.buffer.clear();
    v.samplePos = 0.0;
    v.playedFrames = 0.0;
}

void AudioEngine::SetVoiceVolume(int id, float volume) {
    if (id < 0 || id >= kMaxVoices) return;
    std::lock_guard<std::mutex> lk(m_mutex);
    m_voices[id].volume = std::clamp(volume, 0.0f, 1.0f);
}

void AudioEngine::SetVoicePlaybackRateScale(int id, double scale) {
    if (id < 0 || id >= kMaxVoices) return;
    if (!std::isfinite(scale)) {
        scale = 1.0;
    }
    std::lock_guard<std::mutex> lk(m_mutex);
    auto& v = m_voices[id];
    if (!v.active) return;
    v.playbackRateScale = std::clamp(scale, 0.5, 2.0);
}

double AudioEngine::GetVoicePlaybackRateScale(int id) const {
    if (id < 0 || id >= kMaxVoices) return 1.0;
    std::lock_guard<std::mutex> lk(m_mutex);
    const auto& v = m_voices[id];
    if (!v.active) return 1.0;
    return v.playbackRateScale;
}

void AudioEngine::SetVoiceLooping(int id, bool looping) {
    if (id < 0 || id >= kMaxVoices) return;
    std::lock_guard<std::mutex> lk(m_mutex);
    m_voices[id].looping = looping;
    if (looping) m_voices[id].oneShot = false;
}

void AudioEngine::SetVoiceLoopRegion(int id, size_t loopStartFrame, size_t loopEndFrame) {
    if (id < 0 || id >= kMaxVoices) return;
    std::lock_guard<std::mutex> lk(m_mutex);
    auto& v = m_voices[id];
    v.loopStartFrame = loopStartFrame;
    v.loopEndFrame = loopEndFrame;
}

float AudioEngine::GetVoiceVolume(int id) const {
    if (id < 0 || id >= kMaxVoices) return 0.0f;
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_voices[id].volume;
}

size_t AudioEngine::GetVoiceQueuedSamples(int id) const {
    if (id < 0 || id >= kMaxVoices) return 0;
    std::lock_guard<std::mutex> lk(m_mutex);
    const auto& v = m_voices[id];
    if (!v.active) return 0;
    size_t currentFrame = (size_t)v.samplePos;
    size_t totalFrames = v.buffer.size() / (v.channels > 0 ? v.channels : 1);
    if (currentFrame >= totalFrames) return 0;
    return (totalFrames - currentFrame) * v.channels;
}

bool AudioEngine::IsVoiceActive(int id) const {
    if (id < 0 || id >= kMaxVoices) return false;
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_voices[id].active;
}

double AudioEngine::GetVoicePlayedSeconds(int id) const {
    if (id < 0 || id >= kMaxVoices) return 0.0;
    std::lock_guard<std::mutex> lk(m_mutex);
    const auto& v = m_voices[id];
    if (!v.active || v.sampleRate == 0) return 0.0;
    return v.playedFrames / (double)v.sampleRate;
}

uint32_t AudioEngine::GetVoiceSampleRate(int id) const {
    if (id < 0 || id >= kMaxVoices) return 0;
    std::lock_guard<std::mutex> lk(m_mutex);
    const auto& v = m_voices[id];
    if (!v.active) return 0;
    return v.sampleRate;
}

int AudioEngine::GetVoiceChannels(int id) const {
    if (id < 0 || id >= kMaxVoices) return 0;
    std::lock_guard<std::mutex> lk(m_mutex);
    const auto& v = m_voices[id];
    if (!v.active) return 0;
    return v.channels;
}

void AudioEngine::SetMasterVolume(float vol) {
    m_masterVolume.store(std::clamp(vol, 0.0f, 1.0f));
}

float AudioEngine::GetMasterVolume() const {
    return m_masterVolume.load();
}

double AudioEngine::GetPlayedSeconds() const {
    return m_sink.GetPlayedSeconds();
}

// ---- Pull callback (called by WasapiSink on audio thread) ----

void AudioEngine::RenderCallback(void* user, int16_t* outSamples,
                                  uint32_t frames, uint32_t channels) {
    auto* engine = static_cast<AudioEngine*>(user);
    engine->MixVoices(outSamples, frames, channels);
}

void AudioEngine::MixVoices(int16_t* out, uint32_t frames, uint32_t outChannels) {
    const size_t totalSamples = (size_t)frames * (size_t)outChannels;
    // Use float accumulator to avoid clipping during mix
    thread_local std::vector<float> mixBuf;
    if (mixBuf.size() < totalSamples) mixBuf.resize(totalSamples, 0.0f);
    std::fill(mixBuf.begin(), mixBuf.begin() + totalSamples, 0.0f);

    const float masterVol = m_masterVolume.load();

    {
        std::lock_guard<std::mutex> lk(m_mutex);

        for (int vi = 0; vi < kMaxVoices; vi++) {
            auto& v = m_voices[vi];
            if (!v.active) continue;

            const size_t vCh = (size_t)v.channels;
            if (vCh == 0) continue;

            // Check if we have data
            const size_t bufSamples = v.buffer.size();
            const size_t totalFrames = bufSamples / vCh;
            size_t loopStart = v.loopStartFrame;
            size_t loopEnd = v.loopEndFrame;
            if (loopEnd == 0 || loopEnd > totalFrames) loopEnd = totalFrames;
            if (loopStart >= loopEnd) loopStart = 0;
            const size_t loopFrames = (loopEnd > loopStart) ? (loopEnd - loopStart) : 0;

            // If we reached end of buffer
            if ((size_t)v.samplePos >= loopEnd) {
                if (v.looping && loopFrames > 0) {
                    v.samplePos = (double)loopStart + std::fmod(v.samplePos - (double)loopStart, (double)loopFrames);
                } else if (v.oneShot) {
                    v = AudioVoice{};  // auto-free
                    continue;
                } else {
                    continue;
                }
            }

            const float vol = v.volume * masterVol;
            const double step =
                ((double)v.sampleRate / (double)m_sampleRate) *
                std::clamp(v.playbackRateScale, 0.5, 2.0);

            const double samplePosBegin = v.samplePos;

            for (size_t f = 0; f < frames; f++) {
                // Linear interpolation
                double pos = v.samplePos;
                size_t i0 = (size_t)pos;
                size_t i1 = i0 + 1;

                if (i0 >= loopEnd) {
                    // End of stream within this mix block
                    break;
                }

                float frac = (float)(pos - i0);

                // Fetch samples (handle boundary for i1)
                float l0 = 0.0f, r0 = 0.0f;
                float l1 = 0.0f, r1 = 0.0f;

                const int16_t* s0 = v.buffer.data() + i0 * vCh;
                l0 = (float)s0[0] / 32768.0f;
                r0 = (vCh > 1) ? ((float)s0[1] / 32768.0f) : l0;

                if (i1 < loopEnd) {
                    const int16_t* s1 = v.buffer.data() + i1 * vCh;
                    l1 = (float)s1[0] / 32768.0f;
                    r1 = (vCh > 1) ? ((float)s1[1] / 32768.0f) : l1;
                } else if (v.looping && loopFrames > 0) {
                    const int16_t* s1 = v.buffer.data() + loopStart * vCh;
                    l1 = (float)s1[0] / 32768.0f;
                    r1 = (vCh > 1) ? ((float)s1[1] / 32768.0f) : l1;
                } else {
                    // Past end, just hold last sample or zero?
                    // Let's hold zero for safety or just duplicate
                    l1 = l0; r1 = r0;
                }

                float L = l0 + (l1 - l0) * frac;
                float R = r0 + (r1 - r0) * frac;

                L *= vol;
                R *= vol;

                if (outChannels == 1) {
                    mixBuf[f] += (L + R) * 0.5f;
                } else {
                    mixBuf[f * outChannels + 0] += L;
                    mixBuf[f * outChannels + 1] += R;
                }

                // Advance
                v.samplePos += step;
                if ((size_t)v.samplePos >= loopEnd) {
                    if (v.looping && loopFrames > 0) {
                        v.samplePos = (double)loopStart + std::fmod(v.samplePos - (double)loopStart, (double)loopFrames);
                    } else {
                        break;
                    }
                }
            }

            {
                double delta = v.samplePos - samplePosBegin;
                if (delta < 0.0) delta = 0.0;
                v.playedFrames += delta;
            }

            // Compact buffer if needed (remove consumed frames)
            // Keep a small margin to avoid erasing every frame
            // Skip compact for looping voices (they need the full buffer for wrap-around)
            if (!v.looping) {
                const size_t consumedFrames = (size_t)v.samplePos;
                if (consumedFrames > 4000) { // arbitrary threshold ~100ms
                     size_t removeSamples = consumedFrames * vCh;
                     if (removeSamples < v.buffer.size()) {
                         v.buffer.erase(v.buffer.begin(), v.buffer.begin() + removeSamples);
                         v.samplePos -= (double)consumedFrames;
                     } else {
                         v.buffer.clear();
                         v.samplePos = 0.0;
                     }
                }
            }

            // Check if drained
             if (v.buffer.empty() || (size_t)v.samplePos >= (v.buffer.size() / vCh)) {
                if (v.oneShot) {
                     v = AudioVoice{};
                }
             }
        }
    }

    // Convert float mix to int16_t with clamp
    for (size_t i = 0; i < totalSamples; i++) {
        float s = mixBuf[i];
        if (s > 1.0f) s = 1.0f;
        if (s < -1.0f) s = -1.0f;
        out[i] = (int16_t)(s * 32767.0f);
    }
}
