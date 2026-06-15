#pragma once

#include "wasapi_sink.h"
#include <cstdint>
#include <array>
#include <vector>
#include <mutex>
#include <atomic>

// A single audio voice (mixing channel)
struct AudioVoice {
    std::vector<int16_t> buffer;
    size_t readPos = 0;
    size_t writePos = 0;
    float volume = 1.0f;
    int channels = 2;       // source channel count
    uint32_t sampleRate = 44100; // source sample rate
    double samplePos = 0.0; // fractional read position
    double playedFrames = 0.0;
    double playbackRateScale = 1.0;
    bool active = false;
    bool looping = false;
    bool oneShot = false;    // auto-free when drained
    size_t loopStartFrame = 0;
    size_t loopEndFrame = 0;
};

class AudioEngine {
public:
    static constexpr int kMaxVoices = 24;

    static AudioEngine& Get();

    // Initialize the engine (call once at startup)
    bool Initialize(uint32_t sampleRate = 44100, uint32_t channels = 2);
    void Shutdown();
    bool IsRunning() const;

    // Voice management
    int AllocVoice(int channels, uint32_t sampleRate, float volume = 1.0f, bool oneShot = false);
    void FreeVoice(int id);
    void ResetAllVoices();
    void QueueSamples(int id, const int16_t* samples, size_t count);
    void ClearVoice(int id);
    void SetVoiceVolume(int id, float volume);
    void SetVoicePlaybackRateScale(int id, double scale);
    double GetVoicePlaybackRateScale(int id) const;
    void SetVoiceLooping(int id, bool looping);
    void SetVoiceLoopRegion(int id, size_t loopStartFrame, size_t loopEndFrame);
    float GetVoiceVolume(int id) const;
    size_t GetVoiceQueuedSamples(int id) const;
    bool IsVoiceActive(int id) const;
    double GetVoicePlayedSeconds(int id) const;
    uint32_t GetVoiceSampleRate(int id) const;
    int GetVoiceChannels(int id) const;

    // Master
    void SetMasterVolume(float vol);
    float GetMasterVolume() const;
    double GetPlayedSeconds() const;

    uint32_t GetSampleRate() const { return m_sampleRate; }
    uint32_t GetChannels() const { return m_channels; }

private:
    AudioEngine() = default;
    ~AudioEngine();
    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    static void RenderCallback(void* user, int16_t* outSamples,
                                uint32_t frames, uint32_t channels);
    void MixVoices(int16_t* out, uint32_t frames, uint32_t outChannels);

    WasapiSink m_sink;
    std::array<AudioVoice, kMaxVoices> m_voices{};
    mutable std::mutex m_mutex;  // protects m_voices

    uint32_t m_sampleRate = 44100;
    uint32_t m_channels = 2;
    std::atomic<float> m_masterVolume{1.0f};
    bool m_initialized = false;
};
