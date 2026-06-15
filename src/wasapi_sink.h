#pragma once

#include "audio_sink.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdint>
#include <atomic>
#include <thread>

struct IAudioClient;
struct IAudioRenderClient;
struct IMMDevice;

class WasapiSink : public IAudioSink {
public:
    WasapiSink();
    ~WasapiSink() override;

    bool InitializeSink(const AudioSinkConfig& cfg, AudioRenderCallback cb, void* user) override;
    void ShutdownSink() override;

    bool IsRunning() const override { return m_playing.load(std::memory_order_acquire); }
    double GetPlayedSeconds() const override;

    uint32_t GetSampleRate() const override { return m_requestedRate; }
    uint32_t GetChannels() const override { return m_requestedChannels; }

    void SetVolume(float volume) override;
    float GetVolume() const override;

private:
    void RenderThread();

    AudioRenderCallback m_callback = nullptr;
    void* m_callbackUser = nullptr;

    uint32_t m_requestedRate = 44100;
    uint32_t m_requestedChannels = 2;

    // WASAPI
    IMMDevice* m_device = nullptr;
    IAudioClient* m_audioClient = nullptr;
    IAudioRenderClient* m_renderClient = nullptr;
    HANDLE m_event = nullptr;
    uint32_t m_bufferFrames = 0;

    // Stream format (may differ from requested)
    uint32_t m_streamRate = 0;
    int m_streamChannels = 0;
    bool m_streamFloat = false;

    // Resampling
    double m_resampleStep = 1.0;

    // Thread
    std::thread m_thread;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_threadReady{false};
    std::atomic<bool> m_playing{false};
    bool m_comInited = false;

    std::atomic<uint64_t> m_framesPlayed{0};
    std::atomic<float> m_volume{1.0f};
};
