#pragma once
#include "d3d11_renderer.h"
#include "tim_decoder.h"

// PS1 original resolution
constexpr int PS1_WIDTH = 320;
constexpr int PS1_HEIGHT = 240;

// Exact SCUS 80016B84 boot-logo timing (30 Hz):
//   80015A4C: 30 fade-in frames, indices 0..29
//   80015B00(150, 60): 150 held frames; only the last 90 poll Start
//   80015C20: 30 fade-out frames, indices 29..0
constexpr int FADE_IN_FRAMES = 30;
constexpr int STAY_FRAMES = 150;
constexpr int FADE_OUT_FRAMES = 30;
constexpr int TOTAL_FRAMES = FADE_IN_FRAMES + STAY_FRAMES + FADE_OUT_FRAMES;  // 210
constexpr int SKIP_ENABLE_FRAME = 60;
constexpr int SKIPPABLE_STAY_FRAMES = STAY_FRAMES - SKIP_ENABLE_FRAME;  // 90

// Sony logo sprite data (from prcompo.c)
struct SpriteData {
    int x, y;
    int w, h;
    int u, v;
};

class BootLogo {
public:
    BootLogo();
    ~BootLogo();
    
    bool Initialize(D3D11Renderer* renderer);
    void Shutdown();
    
    // Returns true when sequence complete
    bool Update(bool skipPressed);
    void Render();
    
    bool IsComplete() const { return m_complete; }
    
private:
    enum class State {
        SonyFadeIn,
        SonyStay,
        SonyFadeOut,
        MasayaFadeIn,
        MasayaStay,
        MasayaFadeOut,
        Complete
    };
    
    void LoadTimData();
    void UpdateFadeTexture(TimImage& img,
                           ID3D11ShaderResourceView*& srv,
                           int fadeStep,
                           bool psxAbr1Stp);
    void RenderSonySprites();
    void RenderMasayaSprite();
    
    D3D11Renderer* m_renderer;
    State m_state;
    int m_frameCount;
    int m_totalFrameCount;
    bool m_complete;
    bool m_skipEnabled;
    
    // Sony logo data
    TimImage m_sonyTim;
    ID3D11ShaderResourceView* m_sonySRV;
    std::vector<uint32_t> m_sonyFadeRGBA;
    
    // Masaya logo data
    TimImage m_masayaTim;
    ID3D11ShaderResourceView* m_masayaSRV;
    std::vector<uint32_t> m_masayaFadeRGBA;
    
    // Scale factor for rendering
    float m_scale;
    float m_offsetX, m_offsetY;
};
