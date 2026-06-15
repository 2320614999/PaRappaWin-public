#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdint>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>

#include "audio_backend.h"

struct IAudioClient;
struct IAudioRenderClient;
struct IMMDevice;

class AudioOutput : public IAudioBackend {
public:
    AudioOutput();
    ~AudioOutput() override;

    bool Initialize(uint32_t sampleRate, bool stereo) override;
    void Shutdown() override;

    void QueueSamples(const int16_t* samples, size_t count) override;
    void ClearQueue() override;

    bool IsPlaying() const override { return m_playing.load(); }
    double GetPlayedSeconds() const override;
    size_t GetQueuedSampleCount() const override;

    void SetVolume(float volume) override;
    float GetVolume() const override;

private:
    void RenderThreadMain();
    void Pump();

    IMMDevice* m_device = nullptr;
    IAudioClient* m_audioClient = nullptr;
    IAudioRenderClient* m_renderClient = nullptr;
    HANDLE m_event = nullptr;
    std::thread m_thread;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_threadStarted{false};
    uint32_t m_bufferFrames = 0;

    uint32_t m_sampleRate = 37800;
    int m_channels = 2;
    uint32_t m_streamSampleRate = 0;
    int m_streamChannels = 0;
    bool m_streamFloat = false;
    double m_resampleCursor = 0.0;
    double m_resampleStep = 1.0;
    bool m_comInited = false;
    std::atomic<bool> m_playing{false};

    std::vector<int16_t> m_queue;
    size_t m_queueReadPos = 0;
    mutable std::mutex m_queueMutex;

    std::atomic<uint64_t> m_samplesPlayed{0};
    std::atomic<float> m_volume{1.0f};
};
