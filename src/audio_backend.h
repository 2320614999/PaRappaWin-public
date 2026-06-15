#pragma once

#include <cstdint>
#include <cstddef>

// Abstract audio output interface.
// Current implementation: WASAPI (AudioOutput).
// Future: CriWare sink (only rendering/output, not decoding).
// PCM data is always produced by existing decoders (STR/XA/etc.) and fed here.

class IAudioBackend {
public:
    virtual ~IAudioBackend() = default;

    virtual bool Initialize(uint32_t sampleRate, bool stereo) = 0;
    virtual void Shutdown() = 0;

    virtual void QueueSamples(const int16_t* samples, size_t count) = 0;
    virtual void ClearQueue() = 0;

    virtual bool IsPlaying() const = 0;
    virtual double GetPlayedSeconds() const = 0;
    virtual size_t GetQueuedSampleCount() const = 0;

    // Volume control (0.0 .. 1.0)
    virtual void SetVolume(float volume) = 0;
    virtual float GetVolume() const = 0;
};
