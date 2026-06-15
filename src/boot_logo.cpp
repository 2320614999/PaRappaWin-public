#include "boot_logo.h"
#include "tim_data.h"
#include "logger.h"
#include <algorithm>

#undef min
#undef max

// PSX original sprite layout (320x240 coordinate space)
// Sony TIM (216x28): 3 sub-sprites
struct SpriteSlice {
    int px, py;     // position in PS1 coords
    int sw, sh;     // size in pixels
    int su, sv;     // UV origin in TIM pixels
};

static const SpriteSlice kSonySlices[3] = {
    { 20, 106, 216, 15,   0,  0 },  // main text line
    {240, 106,  60, 12,   0, 16 },  // right portion
    {128, 122,  64, 12, 108, 16 },  // "Inc." below
};

// Masaya TIM (142x126): 2 sub-sprites
static const SpriteSlice kMasayaSlices[2] = {
    { 21, 103, 142, 66,  0,  0 },  // "MASAYA MATSUURA"
    {163, 103, 140, 60,  0, 66 },  // "PRESENTS"
};

BootLogo::BootLogo()
    : m_renderer(nullptr)
    , m_state(State::SonyFadeIn)
    , m_frameCount(0)
    , m_totalFrameCount(0)
    , m_complete(false)
    , m_skipEnabled(false)
    , m_sonySRV(nullptr)
    , m_masayaSRV(nullptr)
    , m_scale(1.0f)
    , m_offsetX(0)
    , m_offsetY(0)
{
}

BootLogo::~BootLogo() {
    Shutdown();
}

bool BootLogo::Initialize(D3D11Renderer* renderer) {
    m_renderer = renderer;

    // Calculate scale to fit PS1 resolution in window
    float scaleX = (float)renderer->GetWidth() / PS1_WIDTH;
    float scaleY = (float)renderer->GetHeight() / PS1_HEIGHT;
    m_scale = std::min(scaleX, scaleY);

    // Center the image
    m_offsetX = (renderer->GetWidth() - PS1_WIDTH * m_scale) / 2;
    m_offsetY = (renderer->GetHeight() - PS1_HEIGHT * m_scale) / 2;

    LoadTimData();

    // Create initial textures with black (fade step 0)
    UpdateFadeTexture(m_sonyTim, m_sonySRV, 0);
    UpdateFadeTexture(m_masayaTim, m_masayaSRV, 0);

    return true;
}

void BootLogo::Shutdown() {
    if (m_sonySRV) {
        m_renderer->DestroyTexture(m_sonySRV);
        m_sonySRV = nullptr;
    }
    if (m_masayaSRV) {
        m_renderer->DestroyTexture(m_masayaSRV);
        m_masayaSRV = nullptr;
    }
}

void BootLogo::LoadTimData() {
    // Decode Sony TIM
    const bool sonyOk = TimDecoder::Decode(tim_sony, sizeof(tim_sony), m_sonyTim);
    Log::Printf("BootLogo: Decode sony ok=%d bpp=%u w=%u h=%u pal=%llu pix=%llu",
                sonyOk ? 1 : 0,
                (unsigned)m_sonyTim.bpp,
                (unsigned)m_sonyTim.width,
                (unsigned)m_sonyTim.height,
                (unsigned long long)m_sonyTim.palette.size(),
                (unsigned long long)m_sonyTim.pixels.size());

    // Decode Masaya TIM
    const bool masayaOk = TimDecoder::Decode(tim_masaya, sizeof(tim_masaya), m_masayaTim);
    Log::Printf("BootLogo: Decode masaya ok=%d bpp=%u w=%u h=%u pal=%llu pix=%llu",
                masayaOk ? 1 : 0,
                (unsigned)m_masayaTim.bpp,
                (unsigned)m_masayaTim.width,
                (unsigned)m_masayaTim.height,
                (unsigned long long)m_masayaTim.palette.size(),
                (unsigned long long)m_masayaTim.pixels.size());
}

void BootLogo::UpdateFadeTexture(TimImage& img, ID3D11ShaderResourceView*& srv, int fadeStep) {
    std::vector<uint32_t> fadedRGBA;

    // Generate faded palette
    TimDecoder::GenerateFadePalette(img.palette, fadeStep, FADE_IN_FRAMES, fadedRGBA);

    // Apply faded palette to pixels
    Log::Printf("BootLogo: Fade step=%d bpp=%u w=%u h=%u pal=%llu pix=%llu",
                fadeStep,
                (unsigned)img.bpp,
                (unsigned)img.width,
                (unsigned)img.height,
                (unsigned long long)img.palette.size(),
                (unsigned long long)img.pixels.size());
    std::vector<uint32_t> pixels(img.width * img.height);

    if (img.bpp == 4) {
        size_t idx = 0;
        for (size_t i = 0; i < img.pixels.size() && idx < pixels.size(); i++) {
            uint8_t byte = img.pixels[i];
            uint8_t lo = byte & 0x0F;
            uint8_t hi = (byte >> 4) & 0x0F;

            if (idx < pixels.size() && lo < fadedRGBA.size())
                pixels[idx++] = fadedRGBA[lo];
            if (idx < pixels.size() && hi < fadedRGBA.size())
                pixels[idx++] = fadedRGBA[hi];
        }
    } else if (img.bpp == 8) {
        for (size_t i = 0; i < img.pixels.size() && i < pixels.size(); i++) {
            uint8_t palIdx = img.pixels[i];
            if (palIdx < fadedRGBA.size())
                pixels[i] = fadedRGBA[palIdx];
        }
    }

    // Create or update texture
    if (srv == nullptr) {
        srv = m_renderer->CreateTexture(pixels.data(), img.width, img.height);
    } else {
        m_renderer->UpdateTexture(srv, pixels.data(), img.width, img.height);
    }
}

bool BootLogo::Update(bool skipPressed) {
    if (m_complete) return true;

    m_totalFrameCount++;

    // Enable skip after SKIP_ENABLE_FRAME frames
    if (m_totalFrameCount >= SKIP_ENABLE_FRAME) {
        m_skipEnabled = true;
    }

    // Check for skip
    if (m_skipEnabled && skipPressed) {
        // Skip current logo sequence
        switch (m_state) {
            case State::SonyFadeIn:
            case State::SonyStay:
            case State::SonyFadeOut:
                m_state = State::MasayaFadeIn;
                m_frameCount = 0;
                m_totalFrameCount = SKIP_ENABLE_FRAME;  // Keep skip enabled
                break;
            case State::MasayaFadeIn:
            case State::MasayaStay:
            case State::MasayaFadeOut:
                m_state = State::Complete;
                m_complete = true;
                return true;
            default:
                break;
        }
    }

    m_frameCount++;

    // State machine
    switch (m_state) {
        case State::SonyFadeIn:
            UpdateFadeTexture(m_sonyTim, m_sonySRV, m_frameCount);
            if (m_frameCount >= FADE_IN_FRAMES) {
                m_state = State::SonyStay;
                m_frameCount = 0;
            }
            break;

        case State::SonyStay:
            if (m_frameCount >= STAY_FRAMES) {
                m_state = State::SonyFadeOut;
                m_frameCount = 0;
            }
            break;

        case State::SonyFadeOut:
            UpdateFadeTexture(m_sonyTim, m_sonySRV, FADE_OUT_FRAMES - m_frameCount);
            if (m_frameCount >= FADE_OUT_FRAMES) {
                m_state = State::MasayaFadeIn;
                m_frameCount = 0;
            }
            break;

        case State::MasayaFadeIn:
            UpdateFadeTexture(m_masayaTim, m_masayaSRV, m_frameCount);
            if (m_frameCount >= FADE_IN_FRAMES) {
                m_state = State::MasayaStay;
                m_frameCount = 0;
            }
            break;

        case State::MasayaStay:
            if (m_frameCount >= STAY_FRAMES) {
                m_state = State::MasayaFadeOut;
                m_frameCount = 0;
            }
            break;

        case State::MasayaFadeOut:
            UpdateFadeTexture(m_masayaTim, m_masayaSRV, FADE_OUT_FRAMES - m_frameCount);
            if (m_frameCount >= FADE_OUT_FRAMES) {
                m_state = State::Complete;
                m_complete = true;
            }
            break;

        case State::Complete:
            m_complete = true;
            break;
    }

    return m_complete;
}

void BootLogo::Render() {
    switch (m_state) {
        case State::SonyFadeIn:
        case State::SonyStay:
        case State::SonyFadeOut:
            RenderSonySprites();
            break;

        case State::MasayaFadeIn:
        case State::MasayaStay:
        case State::MasayaFadeOut:
            RenderMasayaSprite();
            break;

        default:
            break;
    }
}

void BootLogo::RenderSonySprites() {
    if (!m_sonySRV) return;

    const float timW = (float)m_sonyTim.width;   // 216
    const float timH = (float)m_sonyTim.height;  // 28
    if (timW <= 0.0f || timH <= 0.0f) return;

    for (int i = 0; i < 3; i++) {
        const SpriteSlice& s = kSonySlices[i];
        const float x = m_offsetX + (float)s.px * m_scale;
        const float y = m_offsetY + (float)s.py * m_scale;
        const float w = (float)s.sw * m_scale;
        const float h = (float)s.sh * m_scale;
        const float u0 = (float)s.su / timW;
        const float v0 = (float)s.sv / timH;
        const float u1 = (float)(s.su + s.sw) / timW;
        const float v1 = (float)(s.sv + s.sh) / timH;
        m_renderer->DrawSprite(m_sonySRV, x, y, w, h, u0, v0, u1, v1);
    }
}

void BootLogo::RenderMasayaSprite() {
    if (!m_masayaSRV) return;

    const float timW = (float)m_masayaTim.width;   // 142
    const float timH = (float)m_masayaTim.height;  // 126
    if (timW <= 0.0f || timH <= 0.0f) return;

    for (int i = 0; i < 2; i++) {
        const SpriteSlice& s = kMasayaSlices[i];
        const float x = m_offsetX + (float)s.px * m_scale;
        const float y = m_offsetY + (float)s.py * m_scale;
        const float w = (float)s.sw * m_scale;
        const float h = (float)s.sh * m_scale;
        const float u0 = (float)s.su / timW;
        const float v0 = (float)s.sv / timH;
        const float u1 = (float)(s.su + s.sw) / timW;
        const float v1 = (float)(s.sv + s.sh) / timH;
        m_renderer->DrawSprite(m_masayaSRV, x, y, w, h, u0, v0, u1, v1);
    }
}
