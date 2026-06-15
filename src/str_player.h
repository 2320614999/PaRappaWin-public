#pragma once

#include "str_parser.h"
#include "mdec_decoder.h"
#include "xa_decoder.h"
#include "d3d11_renderer.h"
#include <filesystem>
#include <chrono>
#include <vector>

// STR Player states
enum class StrPlayerState {
    Idle,       // Not playing
    Loading,    // Loading file
    Playing,    // Playing video
    Paused,     // Paused (for future use)
    Finished,   // Playback complete
    Skipped,    // User skipped
    Error       // Error occurred
};

// STR Player result (returned from Update)
enum class StrPlayerResult {
    Playing,    // Still playing
    Finished,   // Playback finished normally
    Skipped,    // User skipped playback
    Error       // Error occurred
};

class StrPlayer {
public:
    StrPlayer();
    ~StrPlayer();

    // Initialize the player with a renderer
    bool Initialize(D3D11Renderer* renderer);
    void Shutdown();

    // Start playing an STR file
    bool Play(const std::filesystem::path& strPath);

    // Stop playback
    void Stop();

    void Pause();
    void Resume(bool resetClock = false);

    // Update playback (call once per frame)
    // skipRequested: true if user wants to skip (e.g., F1 or Select button)
    StrPlayerResult Update(bool skipRequested);

    // Render current frame (call after Update, during render phase)
    void Render();

    // Render current frame to a specific rectangle (for viewport-aligned rendering)
    void RenderToRect(float x, float y, float w, float h);

    void RenderToRectUV(float x, float y, float w, float h,
                        float u0, float v0, float u1, float v1);

    // State queries
    StrPlayerState GetState() const { return m_state; }
    bool IsPlaying() const { return m_state == StrPlayerState::Playing; }
    bool IsPaused() const { return m_state == StrPlayerState::Paused; }
    bool IsFinished() const { return m_state == StrPlayerState::Finished ||
                                     m_state == StrPlayerState::Skipped; }
    bool IsVideoFinished() const { return m_videoFinished; }  // PSX: 视频帧播完但音频可能还在
    bool HasAudio() const { return m_audioVoice >= 0; }

    // Playback info
    uint32_t GetCurrentFrame() const { return m_currentFrame; }
    uint32_t GetCurrentStreamFrameNo() const;
    uint32_t GetTotalFrames() const { return (uint32_t)m_parser.GetFrameCount(); }
    uint16_t GetWidth() const { return m_parser.GetWidth(); }
    uint16_t GetHeight() const { return m_parser.GetHeight(); }
    const std::string& GetError() const { return m_error; }

    double GetPlayedSecondsPrecise() const;
    double GetAudioPlayedSecondsPrecise() const;
    double GetDurationSeconds() const { return m_parser.GetDurationSeconds(); }

    // Frame rate (default 30 FPS for STR movies)
    void SetFrameRate(float fps) { m_frameRate = fps; }
    float GetFrameRate() const { return m_frameRate; }

private:
    void AdvanceFrame();
    void UpdateTexture();
    void UpdateAudio();

    D3D11Renderer* m_renderer = nullptr;
    ID3D11ShaderResourceView* m_texture = nullptr;

    StrParser m_parser;
    MdecDecoder m_decoder;
    XaDecoder m_xaDecoder;
    int m_audioVoice = -1;
    uint32_t m_audioSampleRate = 0;
    uint32_t m_audioChannels = 0;

    StrPlayerState m_state = StrPlayerState::Idle;
    uint32_t m_currentFrame = 0;
    uint32_t m_currentAudioSector = 0;
    std::vector<size_t> m_audioSectorIndices;
    uint8_t m_audioSelFile = 0;
    uint8_t m_audioSelChannel = 0;
    uint8_t m_audioSelCoding = 0;
    bool m_audioSelValid = false;
    bool m_videoFinished = false;  // PSX: 视频帧播完但音频还在继续
    float m_frameRate = 30.0f;

    // Frame timing
    std::chrono::high_resolution_clock::time_point m_playStartTime;
    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    double m_frameAccumulator = 0.0;

    std::string m_error;
};
