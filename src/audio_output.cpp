#include "audio_output.h"
#include "logger.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>

#include <mmdeviceapi.h>
#include <audioclient.h>
#include <mmreg.h>
#include <ksmedia.h>
#include <combaseapi.h>
#include <avrt.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "avrt.lib")

static bool IsWaveFormatFloat(const WAVEFORMATEX* wfx) {
    if (!wfx) return false;
    if (wfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) return true;
    if (wfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE && wfx->cbSize >= (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))) {
        const WAVEFORMATEXTENSIBLE* ex = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(wfx);
        return ex->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    }
    return false;
}

static int16_t ClampS16(int v) {
    if (v > 32767) return 32767;
    if (v < -32768) return -32768;
    return (int16_t)v;
}

static float ClampFloat(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

AudioOutput::AudioOutput() {}

AudioOutput::~AudioOutput() {
    Shutdown();
}

bool AudioOutput::Initialize(uint32_t sampleRate, bool stereo) {
    Shutdown();

    m_sampleRate = sampleRate;
    m_channels = stereo ? 2 : 1;
    m_streamSampleRate = 0;
    m_streamChannels = 0;
    m_streamFloat = false;
    m_resampleCursor = 0.0;
    m_resampleStep = 1.0;
    m_queueReadPos = 0;
    m_samplesPlayed = 0;
    m_comInited = false;

    if (!m_event) {
        m_event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        if (!m_event) {
            Log::Printf("AudioOutput: CreateEvent failed");
            return false;
        }
    }

    m_running = true;
    m_threadStarted = false;
    m_playing = false;
    m_thread = std::thread([this]() { RenderThreadMain(); });

    const auto t0 = std::chrono::high_resolution_clock::now();
    while (!m_threadStarted.load()) {
        const auto now = std::chrono::high_resolution_clock::now();
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - t0).count();
        if (ms > 2000) break;
        Sleep(1);
    }

    const bool ok = m_playing.load();
    Log::Printf("AudioOutput: Initialized(WASAPI) req=%uHz %s ok=%d stream=%uHz ch=%d float=%d",
                (unsigned)m_sampleRate,
                stereo ? "stereo" : "mono",
                ok ? 1 : 0,
                (unsigned)m_streamSampleRate,
                (int)m_streamChannels,
                m_streamFloat ? 1 : 0);
    return ok;
}

void AudioOutput::Shutdown() {
    m_playing = false;
    m_running = false;
    if (m_event) {
        SetEvent(m_event);
    }
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (m_event) {
        CloseHandle(m_event);
        m_event = nullptr;
    }
    ClearQueue();
}

void AudioOutput::QueueSamples(const int16_t* samples, size_t count) {
    if (!samples || count == 0) return;
    if (!m_playing.load()) return;

    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_queue.insert(m_queue.end(), samples, samples + count);
    }

    if (m_event) {
        SetEvent(m_event);
    }
}

void AudioOutput::ClearQueue() {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_queue.clear();
    m_queueReadPos = 0;
    m_resampleCursor = 0.0;
}

double AudioOutput::GetPlayedSeconds() const {
    return (double)m_samplesPlayed.load() / (double)m_sampleRate;
}

size_t AudioOutput::GetQueuedSampleCount() const {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    if (m_queueReadPos >= m_queue.size()) return 0;
    return m_queue.size() - m_queueReadPos;
}

void AudioOutput::SetVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    m_volume.store(volume);
}

float AudioOutput::GetVolume() const {
    return m_volume.load();
}

void AudioOutput::RenderThreadMain() {
    const HRESULT hrCo = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hrCo)) {
        Log::Printf("AudioOutput: CoInitializeEx failed hr=0x%08X", (unsigned)hrCo);
        m_threadStarted = true;
        return;
    }
    m_comInited = true;

    DWORD mmcssTaskIndex = 0;
    HANDLE mmcss = AvSetMmThreadCharacteristicsW(L"Pro Audio", &mmcssTaskIndex);

    IMMDeviceEnumerator* enumerator = nullptr;
    WAVEFORMATEX* mix = nullptr;

    auto cleanup = [&]() {
        if (m_audioClient) {
            m_audioClient->Stop();
        }
        if (m_renderClient) {
            m_renderClient->Release();
            m_renderClient = nullptr;
        }
        if (m_audioClient) {
            m_audioClient->Release();
            m_audioClient = nullptr;
        }
        if (m_device) {
            m_device->Release();
            m_device = nullptr;
        }
        if (enumerator) {
            enumerator->Release();
            enumerator = nullptr;
        }
        if (mix) {
            CoTaskMemFree(mix);
            mix = nullptr;
        }
        if (mmcss) {
            AvRevertMmThreadCharacteristics(mmcss);
            mmcss = nullptr;
        }
        if (m_comInited) {
            CoUninitialize();
            m_comInited = false;
        }
    };

    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                  __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
    if (FAILED(hr)) {
        Log::Printf("AudioOutput: CoCreateInstance(MMDeviceEnumerator) failed hr=0x%08X", (unsigned)hr);
        m_threadStarted = true;
        cleanup();
        return;
    }

    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_device);
    if (FAILED(hr)) {
        Log::Printf("AudioOutput: GetDefaultAudioEndpoint failed hr=0x%08X", (unsigned)hr);
        m_threadStarted = true;
        cleanup();
        return;
    }

    hr = m_device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&m_audioClient);
    if (FAILED(hr)) {
        Log::Printf("AudioOutput: IMMDevice Activate(IAudioClient) failed hr=0x%08X", (unsigned)hr);
        m_threadStarted = true;
        cleanup();
        return;
    }

    hr = m_audioClient->GetMixFormat(&mix);
    if (FAILED(hr) || !mix) {
        Log::Printf("AudioOutput: GetMixFormat failed hr=0x%08X", (unsigned)hr);
        m_threadStarted = true;
        cleanup();
        return;
    }

    const REFERENCE_TIME bufferDur = 50 * 1000 * 10;
    DWORD flags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;

    WAVEFORMATEX req = {};
    req.wFormatTag = WAVE_FORMAT_PCM;
    req.nChannels = (WORD)m_channels;
    req.nSamplesPerSec = m_sampleRate;
    req.wBitsPerSample = 16;
    req.nBlockAlign = (WORD)(req.nChannels * (req.wBitsPerSample / 8));
    req.nAvgBytesPerSec = req.nSamplesPerSec * req.nBlockAlign;

    hr = m_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, flags, bufferDur, 0, &req, nullptr);
    if (FAILED(hr)) {
        flags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;
        hr = m_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, flags, bufferDur, 0, mix, nullptr);
        if (FAILED(hr)) {
            Log::Printf("AudioOutput: Initialize failed hr=0x%08X", (unsigned)hr);
            m_threadStarted = true;
            cleanup();
            return;
        }

        if (!IsWaveFormatFloat(mix) && mix->wBitsPerSample != 16) {
            Log::Printf("AudioOutput: unsupported mix PCM bits=%u", (unsigned)mix->wBitsPerSample);
            m_threadStarted = true;
            cleanup();
            return;
        }

        m_streamSampleRate = mix->nSamplesPerSec;
        m_streamChannels = (int)mix->nChannels;
        m_streamFloat = IsWaveFormatFloat(mix);
    } else {
        m_streamSampleRate = m_sampleRate;
        m_streamChannels = m_channels;
        m_streamFloat = false;
    }

    if (m_streamSampleRate == 0 || m_streamChannels <= 0) {
        Log::Printf("AudioOutput: invalid stream format sr=%u ch=%d", (unsigned)m_streamSampleRate, (int)m_streamChannels);
        m_threadStarted = true;
        cleanup();
        return;
    }

    m_resampleCursor = 0.0;
    m_resampleStep = (double)m_sampleRate / (double)m_streamSampleRate;

    hr = m_audioClient->SetEventHandle(m_event);
    if (FAILED(hr)) {
        Log::Printf("AudioOutput: SetEventHandle failed hr=0x%08X", (unsigned)hr);
        m_threadStarted = true;
        cleanup();
        return;
    }

    UINT32 bufferFrames = 0;
    hr = m_audioClient->GetBufferSize(&bufferFrames);
    if (FAILED(hr) || bufferFrames == 0) {
        Log::Printf("AudioOutput: GetBufferSize failed hr=0x%08X", (unsigned)hr);
        m_threadStarted = true;
        cleanup();
        return;
    }
    m_bufferFrames = (uint32_t)bufferFrames;

    hr = m_audioClient->GetService(__uuidof(IAudioRenderClient), (void**)&m_renderClient);
    if (FAILED(hr) || !m_renderClient) {
        Log::Printf("AudioOutput: GetService(IAudioRenderClient) failed hr=0x%08X", (unsigned)hr);
        m_threadStarted = true;
        cleanup();
        return;
    }

    hr = m_audioClient->Start();
    if (FAILED(hr)) {
        Log::Printf("AudioOutput: Start failed hr=0x%08X", (unsigned)hr);
        m_threadStarted = true;
        cleanup();
        return;
    }

    m_playing = true;
    m_threadStarted = true;

    while (m_running.load()) {
        DWORD w = WaitForSingleObject(m_event, 50);
        if (w == WAIT_OBJECT_0) {
            Pump();
        }
    }

    cleanup();
}

void AudioOutput::Pump() {
    if (!m_audioClient || !m_renderClient) return;
    if (!m_playing.load()) return;
    if (m_bufferFrames == 0) return;

    UINT32 padding = 0;
    HRESULT hr = m_audioClient->GetCurrentPadding(&padding);
    if (FAILED(hr)) return;

    const UINT32 framesAvail = (padding < m_bufferFrames) ? (m_bufferFrames - padding) : 0;
    if (framesAvail == 0) return;

    BYTE* out = nullptr;
    hr = m_renderClient->GetBuffer(framesAvail, &out);
    if (FAILED(hr) || !out) return;

    const int inCh = (m_channels > 0) ? m_channels : 1;
    const int outCh = (m_streamChannels > 0) ? m_streamChannels : 1;

    const double step = (m_resampleStep > 0.0) ? m_resampleStep : 1.0;
    size_t needInFrames = (size_t)std::ceil((double)framesAvail * step);
    if (needInFrames < 1) needInFrames = 1;
    std::vector<int16_t> local;
    double pos = 0.0;

    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        if (m_queueReadPos > m_queue.size()) m_queueReadPos = m_queue.size();
        const size_t availSamples = m_queue.size() - m_queueReadPos;
        const size_t maxNeedSamples = needInFrames * (size_t)inCh;
        const size_t take = std::min(availSamples, maxNeedSamples);
        local.assign(m_queue.begin() + m_queueReadPos, m_queue.begin() + m_queueReadPos + take);
        pos = m_resampleCursor;
    }

    const size_t localFrames = (inCh > 0) ? (local.size() / (size_t)inCh) : 0;

    auto readLR = [&](size_t frameIdx, float& L, float& R) {
        if (frameIdx >= localFrames) {
            L = 0.0f;
            R = 0.0f;
            return;
        }
        const size_t base = frameIdx * (size_t)inCh;
        if (inCh == 1) {
            const float s = (float)local[base] / 32768.0f;
            L = s;
            R = s;
            return;
        }
        L = (float)local[base + 0] / 32768.0f;
        R = (float)local[base + 1] / 32768.0f;
    };

    const float vol = m_volume.load();

    UINT32 produced = 0;
    if (m_streamFloat) {
        float* dst = reinterpret_cast<float*>(out);
        for (UINT32 f = 0; f < framesAvail; f++) {
            size_t i0 = (size_t)pos;
            if (localFrames > 0 && i0 >= localFrames) {
                i0 = localFrames - 1;
            }
            size_t i1 = (i0 + 1 < localFrames) ? (i0 + 1) : i0;
            const float t = (float)(pos - (double)i0);
            float L0 = 0.0f, R0 = 0.0f, L1 = 0.0f, R1 = 0.0f;
            readLR(i0, L0, R0);
            readLR(i1, L1, R1);
            const float L = (L0 + (L1 - L0) * t) * vol;
            const float R = (R0 + (R1 - R0) * t) * vol;

            if (outCh == 1) {
                dst[f * (size_t)outCh] = ClampFloat((L + R) * 0.5f, -1.0f, 1.0f);
            } else {
                dst[f * (size_t)outCh + 0] = ClampFloat(L, -1.0f, 1.0f);
                if (outCh >= 2) dst[f * (size_t)outCh + 1] = ClampFloat(R, -1.0f, 1.0f);
                for (int c = 2; c < outCh; c++) dst[f * (size_t)outCh + (size_t)c] = 0.0f;
            }

            pos += step;
            produced++;
        }

        for (UINT32 f = produced; f < framesAvail; f++) {
            for (int c = 0; c < outCh; c++) dst[f * (size_t)outCh + (size_t)c] = 0.0f;
        }
    } else {
        int16_t* dst = reinterpret_cast<int16_t*>(out);
        for (UINT32 f = 0; f < framesAvail; f++) {
            size_t i0 = (size_t)pos;
            if (localFrames > 0 && i0 >= localFrames) {
                i0 = localFrames - 1;
            }
            size_t i1 = (i0 + 1 < localFrames) ? (i0 + 1) : i0;
            const float t = (float)(pos - (double)i0);
            float L0 = 0.0f, R0 = 0.0f, L1 = 0.0f, R1 = 0.0f;
            readLR(i0, L0, R0);
            readLR(i1, L1, R1);
            const float L = (L0 + (L1 - L0) * t) * vol;
            const float R = (R0 + (R1 - R0) * t) * vol;

            if (outCh == 1) {
                const float m = (L + R) * 0.5f;
                dst[f * (size_t)outCh] = ClampS16((int)std::lround(ClampFloat(m, -1.0f, 1.0f) * 32767.0f));
            } else {
                dst[f * (size_t)outCh + 0] = ClampS16((int)std::lround(ClampFloat(L, -1.0f, 1.0f) * 32767.0f));
                if (outCh >= 2) dst[f * (size_t)outCh + 1] = ClampS16((int)std::lround(ClampFloat(R, -1.0f, 1.0f) * 32767.0f));
                for (int c = 2; c < outCh; c++) dst[f * (size_t)outCh + (size_t)c] = 0;
            }

            pos += step;
            produced++;
        }

        for (UINT32 f = produced; f < framesAvail; f++) {
            for (int c = 0; c < outCh; c++) dst[f * (size_t)outCh + (size_t)c] = 0;
        }
    }

    m_renderClient->ReleaseBuffer(framesAvail, 0);

    const size_t advFrames = (size_t)pos;
    pos -= (double)advFrames;

    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_queueReadPos += advFrames * (size_t)inCh;
        if (m_queueReadPos > m_queue.size()) m_queueReadPos = m_queue.size();
        m_resampleCursor = pos;

        if (m_queueReadPos > 0 && (m_queueReadPos > 65536 || m_queueReadPos > (m_queue.size() / 2))) {
            m_queue.erase(m_queue.begin(), m_queue.begin() + m_queueReadPos);
            m_queueReadPos = 0;
        }
    }

    m_samplesPlayed += advFrames;
}
