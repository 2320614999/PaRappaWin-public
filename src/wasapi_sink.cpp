#include "wasapi_sink.h"
#include "logger.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

#include <mmdeviceapi.h>
#include <audioclient.h>
#include <mmreg.h>
#include <ksmedia.h>
#include <combaseapi.h>
#include <avrt.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "avrt.lib")

static bool IsFloat(const WAVEFORMATEX* wfx) {
    if (!wfx) return false;
    if (wfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) return true;
    if (wfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE && wfx->cbSize >= (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))) {
        return reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(wfx)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    }
    return false;
}

WasapiSink::WasapiSink() {}
WasapiSink::~WasapiSink() { ShutdownSink(); }

bool WasapiSink::InitializeSink(const AudioSinkConfig& cfg, AudioRenderCallback cb, void* user) {
    ShutdownSink();

    m_callback = cb;
    m_callbackUser = user;
    m_requestedRate = cfg.sampleRate;
    m_requestedChannels = cfg.channels;
    m_framesPlayed = 0;

    m_event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!m_event) {
        Log::Printf("WasapiSink: CreateEvent failed");
        return false;
    }

    m_running = true;
    m_threadReady = false;
    m_playing = false;
    m_thread = std::thread([this]() { RenderThread(); });

    // Wait for thread init (up to 3s)
    auto t0 = std::chrono::high_resolution_clock::now();
    while (!m_threadReady.load()) {
        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - t0).count() > 3000) break;
        Sleep(1);
    }

    bool ok = m_playing.load();
    Log::Printf("WasapiSink: Init req=%uHz ch=%u buf=%ums ok=%d stream=%uHz sCh=%d float=%d",
                cfg.sampleRate, cfg.channels, cfg.bufferMs, ok ? 1 : 0,
                m_streamRate, m_streamChannels, m_streamFloat ? 1 : 0);
    return ok;
}

void WasapiSink::ShutdownSink() {
    m_playing.store(false);
    m_running.store(false);
    if (m_event) SetEvent(m_event);
    if (m_thread.joinable()) m_thread.join();
    if (m_event) { CloseHandle(m_event); m_event = nullptr; }
    m_callback = nullptr;
    m_callbackUser = nullptr;
}

double WasapiSink::GetPlayedSeconds() const {
    if (m_requestedRate == 0) return 0.0;
    return (double)m_framesPlayed.load(std::memory_order_relaxed) / (double)m_requestedRate;
}

void WasapiSink::SetVolume(float v) {
    m_volume.store(std::clamp(v, 0.0f, 1.0f));
}

float WasapiSink::GetVolume() const {
    return m_volume.load();
}

void WasapiSink::RenderThread() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        Log::Printf("WasapiSink: CoInit failed 0x%08X", (unsigned)hr);
        m_threadReady = true;
        return;
    }
    m_comInited = true;

    DWORD mmcssIdx = 0;
    HANDLE mmcss = AvSetMmThreadCharacteristicsW(L"Pro Audio", &mmcssIdx);

    IMMDeviceEnumerator* enumerator = nullptr;
    WAVEFORMATEX* mixFmt = nullptr;

    auto cleanup = [&]() {
        if (m_audioClient) m_audioClient->Stop();
        if (m_renderClient) { m_renderClient->Release(); m_renderClient = nullptr; }
        if (m_audioClient) { m_audioClient->Release(); m_audioClient = nullptr; }
        if (m_device) { m_device->Release(); m_device = nullptr; }
        if (enumerator) { enumerator->Release(); }
        if (mixFmt) CoTaskMemFree(mixFmt);
        if (mmcss) AvRevertMmThreadCharacteristics(mmcss);
        if (m_comInited) { CoUninitialize(); m_comInited = false; }
    };

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
    if (FAILED(hr)) { Log::Printf("WasapiSink: enum failed"); m_threadReady = true; cleanup(); return; }

    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_device);
    if (FAILED(hr)) { Log::Printf("WasapiSink: endpoint failed"); m_threadReady = true; cleanup(); return; }

    hr = m_device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&m_audioClient);
    if (FAILED(hr)) { Log::Printf("WasapiSink: activate failed"); m_threadReady = true; cleanup(); return; }

    hr = m_audioClient->GetMixFormat(&mixFmt);
    if (FAILED(hr) || !mixFmt) { Log::Printf("WasapiSink: GetMixFormat failed"); m_threadReady = true; cleanup(); return; }

    // Try to use our requested format with auto-convert
    const REFERENCE_TIME bufDur = 100000;  // 10ms
    DWORD flags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;

    WAVEFORMATEX reqFmt = {};
    reqFmt.wFormatTag = WAVE_FORMAT_PCM;
    reqFmt.nChannels = (WORD)m_requestedChannels;
    reqFmt.nSamplesPerSec = m_requestedRate;
    reqFmt.wBitsPerSample = 16;
    reqFmt.nBlockAlign = (WORD)(reqFmt.nChannels * (reqFmt.wBitsPerSample / 8));
    reqFmt.nAvgBytesPerSec = reqFmt.nSamplesPerSec * reqFmt.nBlockAlign;

    hr = m_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, flags, bufDur, 0, &reqFmt, nullptr);
    if (FAILED(hr)) {
        // Fallback: use mix format, we'll resample ourselves
        flags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;
        hr = m_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, flags, bufDur, 0, mixFmt, nullptr);
        if (FAILED(hr)) {
            Log::Printf("WasapiSink: Init failed 0x%08X", (unsigned)hr);
            m_threadReady = true; cleanup(); return;
        }
        if (!IsFloat(mixFmt) && mixFmt->wBitsPerSample != 16) {
            Log::Printf("WasapiSink: unsupported bits=%u", (unsigned)mixFmt->wBitsPerSample);
            m_threadReady = true; cleanup(); return;
        }
        m_streamRate = mixFmt->nSamplesPerSec;
        m_streamChannels = (int)mixFmt->nChannels;
        m_streamFloat = IsFloat(mixFmt);
    } else {
        m_streamRate = m_requestedRate;
        m_streamChannels = (int)m_requestedChannels;
        m_streamFloat = false;
    }

    if (m_streamRate == 0 || m_streamChannels <= 0) {
        m_threadReady = true; cleanup(); return;
    }
    m_resampleStep = (double)m_requestedRate / (double)m_streamRate;

    hr = m_audioClient->SetEventHandle(m_event);
    if (FAILED(hr)) { m_threadReady = true; cleanup(); return; }

    UINT32 bufSz = 0;
    hr = m_audioClient->GetBufferSize(&bufSz);
    if (FAILED(hr) || bufSz == 0) { m_threadReady = true; cleanup(); return; }
    m_bufferFrames = bufSz;

    hr = m_audioClient->GetService(__uuidof(IAudioRenderClient), (void**)&m_renderClient);
    if (FAILED(hr)) { m_threadReady = true; cleanup(); return; }

    hr = m_audioClient->Start();
    if (FAILED(hr)) { m_threadReady = true; cleanup(); return; }

    m_playing = true;
    m_threadReady = true;

    // Temp buffer for pull callback
    const uint32_t maxPullFrames = m_bufferFrames + 256;
    std::vector<int16_t> pullBuf(maxPullFrames * m_requestedChannels, 0);

    while (m_running.load()) {
        DWORD w = WaitForSingleObject(m_event, 50);
        if (w != WAIT_OBJECT_0) continue;
        if (!m_playing.load()) continue;

        UINT32 padding = 0;
        hr = m_audioClient->GetCurrentPadding(&padding);
        if (FAILED(hr)) continue;

        UINT32 avail = (padding < m_bufferFrames) ? (m_bufferFrames - padding) : 0;
        if (avail == 0) continue;

        BYTE* outPtr = nullptr;
        hr = m_renderClient->GetBuffer(avail, &outPtr);
        if (FAILED(hr) || !outPtr) continue;

        // How many source frames we need
        const double step = m_resampleStep > 0.0 ? m_resampleStep : 1.0;
        uint32_t srcFrames = (uint32_t)std::ceil((double)avail * step);
        if (srcFrames < 1) srcFrames = 1;
        if (srcFrames > maxPullFrames) srcFrames = maxPullFrames;

        // Pull from mixer
        std::memset(pullBuf.data(), 0, srcFrames * m_requestedChannels * sizeof(int16_t));
        if (m_callback) {
            m_callback(m_callbackUser, pullBuf.data(), srcFrames, m_requestedChannels);
        }

        const float vol = m_volume.load();
        const int inCh = (int)m_requestedChannels;
        const int outCh = m_streamChannels;

        // Resample + write to WASAPI buffer
        if (m_streamFloat) {
            float* dst = reinterpret_cast<float*>(outPtr);
            double pos = 0.0;
            for (UINT32 f = 0; f < avail; f++) {
                size_t i0 = (size_t)pos;
                if (i0 >= srcFrames) i0 = (srcFrames > 0) ? (srcFrames - 1) : 0;
                size_t i1 = (i0 + 1 < srcFrames) ? (i0 + 1) : i0;
                float t = (float)(pos - (double)i0);

                // Read two frames for linear interp
                float L0 = 0, R0 = 0, L1 = 0, R1 = 0;
                if (inCh == 1) {
                    L0 = R0 = (float)pullBuf[i0] / 32768.0f;
                    L1 = R1 = (float)pullBuf[i1] / 32768.0f;
                } else {
                    L0 = (float)pullBuf[i0 * 2] / 32768.0f;
                    R0 = (float)pullBuf[i0 * 2 + 1] / 32768.0f;
                    L1 = (float)pullBuf[i1 * 2] / 32768.0f;
                    R1 = (float)pullBuf[i1 * 2 + 1] / 32768.0f;
                }
                float L = (L0 + (L1 - L0) * t) * vol;
                float R = (R0 + (R1 - R0) * t) * vol;

                if (outCh == 1) {
                    dst[f] = std::clamp((L + R) * 0.5f, -1.0f, 1.0f);
                } else {
                    dst[f * outCh + 0] = std::clamp(L, -1.0f, 1.0f);
                    if (outCh >= 2) dst[f * outCh + 1] = std::clamp(R, -1.0f, 1.0f);
                    for (int c = 2; c < outCh; c++) dst[f * outCh + c] = 0.0f;
                }
                pos += step;
            }
        } else {
            int16_t* dst = reinterpret_cast<int16_t*>(outPtr);
            double pos = 0.0;
            for (UINT32 f = 0; f < avail; f++) {
                size_t i0 = (size_t)pos;
                if (i0 >= srcFrames) i0 = (srcFrames > 0) ? (srcFrames - 1) : 0;
                size_t i1 = (i0 + 1 < srcFrames) ? (i0 + 1) : i0;
                float t = (float)(pos - (double)i0);

                float L0 = 0, R0 = 0, L1 = 0, R1 = 0;
                if (inCh == 1) {
                    L0 = R0 = (float)pullBuf[i0] / 32768.0f;
                    L1 = R1 = (float)pullBuf[i1] / 32768.0f;
                } else {
                    L0 = (float)pullBuf[i0 * 2] / 32768.0f;
                    R0 = (float)pullBuf[i0 * 2 + 1] / 32768.0f;
                    L1 = (float)pullBuf[i1 * 2] / 32768.0f;
                    R1 = (float)pullBuf[i1 * 2 + 1] / 32768.0f;
                }
                float L = (L0 + (L1 - L0) * t) * vol;
                float R = (R0 + (R1 - R0) * t) * vol;

                auto toS16 = [](float v) -> int16_t {
                    int s = (int)std::lround(std::clamp(v, -1.0f, 1.0f) * 32767.0f);
                    return (int16_t)std::clamp(s, -32768, 32767);
                };

                if (outCh == 1) {
                    dst[f] = toS16((L + R) * 0.5f);
                } else {
                    dst[f * outCh + 0] = toS16(L);
                    if (outCh >= 2) dst[f * outCh + 1] = toS16(R);
                    for (int c = 2; c < outCh; c++) dst[f * outCh + c] = 0;
                }
                pos += step;
            }
        }

        m_renderClient->ReleaseBuffer(avail, 0);
        m_framesPlayed.fetch_add(avail, std::memory_order_relaxed);
    }

    cleanup();
}
