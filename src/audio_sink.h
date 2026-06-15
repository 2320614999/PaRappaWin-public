#pragma once

#include <cstdint>
#include <cstddef>

using AudioRenderCallback = void(*)(void* user, int16_t* outSamples, uint32_t frames, uint32_t channels);

struct AudioSinkConfig {
    uint32_t sampleRate = 37800;
    uint32_t channels = 2;
    uint32_t bufferMs = 10;
};

class IAudioSink {
public:
    virtual ~IAudioSink() = default;

    virtual bool InitializeSink(const AudioSinkConfig& cfg, AudioRenderCallback cb, void* user) = 0;
    virtual void ShutdownSink() = 0;

    virtual bool IsRunning() const = 0;
    virtual double GetPlayedSeconds() const = 0;

    virtual uint32_t GetSampleRate() const = 0;
    virtual uint32_t GetChannels() const = 0;

    virtual void SetVolume(float volume) = 0;
    virtual float GetVolume() const = 0;
};
