#include "str_player.h"
#include "audio_engine.h"
#include "logger.h"
#include <algorithm>

StrPlayer::StrPlayer() = default;
StrPlayer::~StrPlayer() { Shutdown(); }

uint32_t StrPlayer::GetCurrentStreamFrameNo() const {
    const StrVideoFrame* f = m_parser.GetFrame((size_t)m_currentFrame);
    if (!f) return 0u;
    return f->frameNo;
}

bool StrPlayer::Initialize(D3D11Renderer* renderer) {
    if (!renderer) {
        m_error = "Null renderer";
        Log::Printf("StrPlayer: %s", m_error.c_str());
        return false;
    }

    m_renderer = renderer;
    m_state = StrPlayerState::Idle;
    Log::Printf("StrPlayer: Initialized");
    return true;
}

void StrPlayer::Shutdown() {
    Stop();
    m_renderer = nullptr;
}

bool StrPlayer::Play(const std::filesystem::path& strPath) {
    Stop();

    Log::Printf("StrPlayer: Play %s", strPath.u8string().c_str());

    // Load and parse STR file
    m_state = StrPlayerState::Loading;
    if (!m_parser.Load(strPath)) {
        m_error = m_parser.GetError();
        m_state = StrPlayerState::Error;
        return false;
    }

    // Initialize decoder
    if (!m_decoder.Initialize(m_parser.GetWidth(), m_parser.GetHeight())) {
        m_error = "Failed to initialize decoder";
        m_state = StrPlayerState::Error;
        m_parser.Unload();
        return false;
    }

    // Create texture for video output
    if (m_texture) {
        m_renderer->DestroyTexture(m_texture);
        m_texture = nullptr;
    }

    // Create initial texture with first frame
    m_currentFrame = 0;
    m_videoFinished = false;
    const StrVideoFrame* frame = m_parser.GetFrame(0);
    if (frame) {
        m_decoder.DecodeFrame(frame->bitstream.data(), frame->bitstream.size(),
                              frame->frameNo, frame->quantScale, frame->version);
        m_texture = m_renderer->CreateTexture(
            m_decoder.GetPixels().data(),
            m_decoder.GetWidth(),
            m_decoder.GetHeight()
        );
    }

    if (!m_texture) {
        m_error = "Failed to create video texture";
        m_state = StrPlayerState::Error;
        m_decoder.Shutdown();
        m_parser.Unload();
        return false;
    }

    {
        const double durationSec = m_parser.GetDurationSeconds();
        const uint32_t frames = GetTotalFrames();
        if (durationSec > 0.001 && frames > 0) {
            const double derivedFps = (double)frames / durationSec;
            if (derivedFps > 1.0 && derivedFps < 120.0) {
                m_frameRate = (float)derivedFps;
            }
            Log::Printf("StrPlayer: STR timing sectors(total=%u realtime=%u) duration=%.3fs derivedFps=%.3f usingFps=%.3f",
                        (unsigned)m_parser.GetTotalSectorCount(),
                        (unsigned)m_parser.GetRealtimeSectorCount(),
                        durationSec,
                        derivedFps,
                        (double)m_frameRate);
        } else {
            Log::Printf("StrPlayer: STR timing unavailable, using fps=%.3f", (double)m_frameRate);
        }
    }

    // Initialize audio voice
    m_currentAudioSector = 0;
    m_audioSampleRate = 0;
    m_audioChannels = 0;
    m_xaDecoder.Reset();
    if (m_parser.GetAudioSectorCount() > 0) {
        const StrAudioSector* firstAudio = m_parser.GetAudioSector(0);
        if (firstAudio) {
            bool stereo = (firstAudio->coding & 0x01) != 0;
            bool halfRate = (firstAudio->coding & 0x04) != 0;
            uint32_t sampleRate = halfRate ? 18900 : 37800;
            const uint32_t ch = stereo ? 2u : 1u;
            m_audioVoice = AudioEngine::Get().AllocVoice((int)ch, sampleRate, 1.0f);
            if (m_audioVoice >= 0) {
                m_audioSampleRate = sampleRate;
                m_audioChannels = ch;
                Log::Printf("StrPlayer: Audio voice=%d %s %uHz, %zu sectors",
                            m_audioVoice, stereo ? "stereo" : "mono", sampleRate,
                            m_parser.GetAudioSectorCount());
            }
        }
    }

    // Start playback
    m_playStartTime = std::chrono::high_resolution_clock::now();
    m_lastFrameTime = m_playStartTime;
    m_frameAccumulator = 0.0;
    m_state = StrPlayerState::Playing;

    Log::Printf("StrPlayer: Started playback, %u frames @ %.1f fps, %ux%u",
                GetTotalFrames(), m_frameRate, GetWidth(), GetHeight());
    return true;
}

void StrPlayer::Stop() {
    if (m_audioVoice >= 0) {
        AudioEngine::Get().FreeVoice(m_audioVoice);
        m_audioVoice = -1;
    }

    m_audioSampleRate = 0;
    m_audioChannels = 0;

    if (m_texture) {
        if (m_renderer) {
            m_renderer->DestroyTexture(m_texture);
        }
        m_texture = nullptr;
    }

    m_decoder.Shutdown();
    m_xaDecoder.Reset();
    m_parser.Unload();
    m_currentFrame = 0;
    m_videoFinished = false;
    m_currentAudioSector = 0;
    m_frameAccumulator = 0.0;
    m_state = StrPlayerState::Idle;
    m_error.clear();
}

void StrPlayer::Pause() {
    if (m_state != StrPlayerState::Playing && m_state != StrPlayerState::Skipped) {
        return;
    }
    m_state = StrPlayerState::Paused;
    m_lastFrameTime = std::chrono::high_resolution_clock::now();
}

void StrPlayer::Resume(bool resetClock) {
    if (m_state != StrPlayerState::Paused) {
        return;
    }

    const auto now = std::chrono::high_resolution_clock::now();
    if (resetClock) {
        const double fps = (m_frameRate > 0.1f) ? (double)m_frameRate : 30.0;
        const double playedSec = (fps > 0.0) ? ((double)m_currentFrame / fps) : 0.0;
        m_playStartTime = now - std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(
                                    std::chrono::duration<double>(playedSec));
    }

    m_lastFrameTime = now;
    m_frameAccumulator = 0.0;
    m_state = StrPlayerState::Playing;
}

StrPlayerResult StrPlayer::Update(bool skipRequested) {
    if (m_state != StrPlayerState::Playing) {
        if (m_state == StrPlayerState::Paused) return StrPlayerResult::Playing;
        if (m_state == StrPlayerState::Finished) return StrPlayerResult::Finished;
        if (m_state == StrPlayerState::Skipped) return StrPlayerResult::Skipped;
        if (m_state == StrPlayerState::Error) return StrPlayerResult::Error;
        return StrPlayerResult::Playing;
    }

    // Check for skip request (PSX: sub_80035510() == 256, i.e., Select button)
    if (skipRequested) {
        Log::Printf("StrPlayer: Skip requested at frame %u", m_currentFrame);
        m_state = StrPlayerState::Skipped;
        return StrPlayerResult::Skipped;
    }

    // Frame timing - only advance video frames if video not finished yet
    if (!m_videoFinished) {
        auto now = std::chrono::high_resolution_clock::now();
        double elapsedMs = std::chrono::duration<double, std::milli>(now - m_lastFrameTime).count();
        m_lastFrameTime = now;

        // Accumulate time and advance frames as needed
        double msPerFrame = 1000.0 / m_frameRate;
        m_frameAccumulator += elapsedMs;

        while (m_frameAccumulator >= msPerFrame) {
            m_frameAccumulator -= msPerFrame;
            AdvanceFrame();

            if (m_state != StrPlayerState::Playing || m_videoFinished) {
                break;
            }
        }
    }

    // Update audio (continues even after video frames are done)
    UpdateAudio();

    // PSX: video finished but audio still playing - check if audio is also done
    if (m_videoFinished && m_audioVoice >= 0) {
        bool audioSectorsDone = (m_currentAudioSector >= m_parser.GetAudioSectorCount());
        size_t queued = AudioEngine::Get().GetVoiceQueuedSamples(m_audioVoice);
        if (audioSectorsDone && queued == 0) {
            Log::Printf("StrPlayer: Audio also finished, fully done");
            m_state = StrPlayerState::Finished;
            return StrPlayerResult::Finished;
        }
        return StrPlayerResult::Playing;  // audio still playing
    } else if (m_videoFinished) {
        // No audio track - just finish
        m_state = StrPlayerState::Finished;
        return StrPlayerResult::Finished;
    }

    // Return current state
    if (m_state == StrPlayerState::Finished) return StrPlayerResult::Finished;
    if (m_state == StrPlayerState::Skipped) return StrPlayerResult::Skipped;
    return StrPlayerResult::Playing;
}

void StrPlayer::UpdateAudio() {
    if (m_audioVoice < 0) return;

    const size_t audioSectorCount = m_parser.GetAudioSectorCount();
    if (audioSectorCount == 0) return;

    if (m_audioSampleRate == 0 || m_audioChannels == 0) {
        return;
    }

    size_t queuedSamples = AudioEngine::Get().GetVoiceQueuedSamples(m_audioVoice);
    double queuedSec = (double)queuedSamples / ((double)m_audioSampleRate * (double)m_audioChannels);
    const double targetQueuedSec = 0.50;
    int pushed = 0;
    const int maxSectorsPerUpdate = 32;

    while (m_currentAudioSector < audioSectorCount && queuedSec < targetQueuedSec && pushed < maxSectorsPerUpdate) {
        const StrAudioSector* sector = m_parser.GetAudioSector(m_currentAudioSector);
        if (sector && !sector->data.empty()) {
            if (m_xaDecoder.DecodeSector(sector->data.data(), sector->data.size(), sector->coding)) {
                const auto& samples = m_xaDecoder.GetSamples();
                if (!samples.empty()) {
                    if (m_currentAudioSector == 0) {
                        const auto mm = std::minmax_element(samples.begin(), samples.end());
                        Log::Printf("StrPlayer: Audio sector0 decoded samples=%zu rate=%u %s min=%d max=%d coding=0x%02X file=%u ch=%u",
                                    samples.size(),
                                    (unsigned)m_xaDecoder.GetSampleRate(),
                                    m_xaDecoder.IsStereo() ? "stereo" : "mono",
                                    (int)*mm.first, (int)*mm.second,
                                    (unsigned)sector->coding,
                                    (unsigned)sector->file,
                                    (unsigned)sector->channel);
                    }
                    AudioEngine::Get().QueueSamples(m_audioVoice, samples.data(), samples.size());
                    queuedSamples += samples.size();
                    queuedSec = (double)queuedSamples / ((double)m_audioSampleRate * (double)m_audioChannels);
                }
            }
        }
        m_currentAudioSector++;
        pushed++;
    }
}

void StrPlayer::AdvanceFrame() {
    m_currentFrame++;

    if (m_currentFrame >= m_parser.GetFrameCount()) {
        const auto now = std::chrono::high_resolution_clock::now();
        const double wallSec = std::chrono::duration<double>(now - m_playStartTime).count();
        const double strSec = m_parser.GetDurationSeconds();
        Log::Printf("StrPlayer: Video frames finished at frame %u wall=%.3fs str=%.3fs (audio continues)",
                    m_currentFrame, wallSec, strSec);
        // PSX原版: 视频帧播完后不立即结束，音频继续播放
        m_videoFinished = true;
        return;
    }

    // Decode next frame
    const StrVideoFrame* frame = m_parser.GetFrame(m_currentFrame);
    if (!frame) {
        Log::Printf("StrPlayer: Failed to get frame %u", m_currentFrame);
        m_state = StrPlayerState::Error;
        m_error = "Failed to get frame";
        return;
    }

    m_decoder.DecodeFrame(frame->bitstream.data(), frame->bitstream.size(),
                          frame->frameNo, frame->quantScale, frame->version);
    UpdateTexture();

    // Log progress periodically
    if (m_currentFrame % 30 == 0) {
        Log::Printf("StrPlayer: Frame %u/%zu", m_currentFrame, m_parser.GetFrameCount());
    }
}

void StrPlayer::UpdateTexture() {
    if (!m_texture || !m_renderer) return;

    m_renderer->UpdateTexture(
        m_texture,
        m_decoder.GetPixels().data(),
        m_decoder.GetWidth(),
        m_decoder.GetHeight()
    );
}

void StrPlayer::Render() {
    if (!m_texture || !m_renderer) return;
    if (m_state != StrPlayerState::Playing &&
        m_state != StrPlayerState::Paused &&
        m_state != StrPlayerState::Finished &&
        m_state != StrPlayerState::Skipped) {
        return;
    }

    // Calculate centered, aspect-preserving display rect
    float texW = (float)m_decoder.GetWidth();
    float texH = (float)m_decoder.GetHeight();
    float winW = (float)m_renderer->GetWidth();
    float winH = (float)m_renderer->GetHeight();

    if (texW <= 0 || texH <= 0) return;

    // Fit to window while preserving aspect ratio
    float scaleX = winW / texW;
    float scaleY = winH / texH;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    const float windowScale = 0.65f;
    scale *= windowScale;

    float w = texW * scale;
    float h = texH * scale;
    float x = (winW - w) * 0.5f;
    float y = (winH - h) * 0.5f;

    m_renderer->DrawSprite(m_texture, x, y, w, h);
}

void StrPlayer::RenderToRect(float rx, float ry, float rw, float rh) {
    if (!m_texture || !m_renderer) return;
    if (m_state != StrPlayerState::Playing &&
        m_state != StrPlayerState::Paused &&
        m_state != StrPlayerState::Finished &&
        m_state != StrPlayerState::Skipped) {
        return;
    }
    m_renderer->DrawSprite(m_texture, rx, ry, rw, rh);
}

void StrPlayer::RenderToRectUV(float rx, float ry, float rw, float rh,
                               float u0, float v0, float u1, float v1) {
    if (!m_texture || !m_renderer) return;
    if (m_state != StrPlayerState::Playing &&
        m_state != StrPlayerState::Paused &&
        m_state != StrPlayerState::Finished &&
        m_state != StrPlayerState::Skipped) {
        return;
    }
    m_renderer->DrawSprite(m_texture, rx, ry, rw, rh, u0, v0, u1, v1);
}

double StrPlayer::GetPlayedSecondsPrecise() const {
    if (m_state == StrPlayerState::Idle || m_state == StrPlayerState::Error) {
        return 0.0;
    }

    const auto now = (m_state == StrPlayerState::Paused) ? m_lastFrameTime
                                                         : std::chrono::high_resolution_clock::now();
    double wallSec = std::chrono::duration<double>(now - m_playStartTime).count();
    if (wallSec < 0.0) wallSec = 0.0;

    double msPerFrame = (m_frameRate > 0.1f) ? (1000.0 / (double)m_frameRate) : (1000.0 / 30.0);
    double adjust = 0.0;
    if (msPerFrame > 0.001) {
        adjust = (m_frameAccumulator / msPerFrame) * (1.0 / (double)m_frameRate);
        if (adjust < 0.0) adjust = 0.0;
        if (adjust > 0.5) adjust = 0.5;
    }

    double played = wallSec - adjust;
    if (played < 0.0) played = 0.0;

    const double dur = m_parser.GetDurationSeconds();
    if (dur > 0.001 && played > dur) {
        played = dur;
    }
    return played;
}

double StrPlayer::GetAudioPlayedSecondsPrecise() const {
    if (m_audioVoice < 0) {
        return 0.0;
    }
    double played = AudioEngine::Get().GetVoicePlayedSeconds(m_audioVoice);
    if (played < 0.0) played = 0.0;
    const double dur = m_parser.GetDurationSeconds();
    if (dur > 0.001 && played > dur) {
        played = dur;
    }
    return played;
}
