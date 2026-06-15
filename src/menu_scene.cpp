#include "menu_scene.h"
#include <algorithm>
 #include "logger.h"

#undef min
#undef max

// PS1 resolution
static const int PS1_WIDTH = 320;
static const int PS1_HEIGHT = 240;

// Menu item positions
static const int MENU_X = 100;
static const int MENU_Y = 100;
static const int MENU_SPACING = 30;

MenuScene::MenuScene() {}

MenuScene::~MenuScene() {
    Shutdown();
}

bool MenuScene::Initialize(D3D11Renderer* renderer, ResourceManager* resources) {
    m_renderer = renderer;
    m_resources = resources;

    // Calculate scale to fit PS1 resolution in window
    float scaleX = (float)renderer->GetWidth() / PS1_WIDTH;
    float scaleY = (float)renderer->GetHeight() / PS1_HEIGHT;
    m_scale = std::min(scaleX, scaleY);

    // Center the image
    m_offsetX = (renderer->GetWidth() - PS1_WIDTH * m_scale) / 2;
    m_offsetY = (renderer->GetHeight() - PS1_HEIGHT * m_scale) / 2;

    if (m_resources) {
        m_textureNames = m_resources->GetTextureNames();
        if (!m_textureNames.empty()) {
            m_textureIndex = 0;
            UpdateCurrentTextureInfo();
            Log::Printf("Menu texture browser: loaded %d textures", (int)m_textureNames.size());
            Log::Printf("Menu texture browser: start at %d/%d '%s' (%dx%d)",
                        m_textureIndex + 1,
                        (int)m_textureNames.size(),
                        GetBrowserTextureName().c_str(),
                        m_textureWidth, m_textureHeight);
        } else {
            Log::Printf("Menu texture browser: no textures loaded");
        }
    }

    return true;
}

void MenuScene::Shutdown() {
    // Resources are managed by ResourceManager
}

bool MenuScene::Update(bool upPressed, bool downPressed, bool confirmPressed,
                       bool prevTexPressed, bool nextTexPressed) {
    if (m_complete) return true;

    // Handle input
    if (upPressed) {
        m_selection = (m_selection - 1 + m_maxSelection) % m_maxSelection;
    }
    if (downPressed) {
        m_selection = (m_selection + 1) % m_maxSelection;
    }
    if (confirmPressed && m_selection != 1) {
        m_complete = true;
        return true;
    }

    if (!m_textureNames.empty()) {
        bool changed = false;
        if (prevTexPressed) {
            m_textureIndex = (m_textureIndex - 1 + (int)m_textureNames.size()) % (int)m_textureNames.size();
            changed = true;
        }
        if (nextTexPressed) {
            m_textureIndex = (m_textureIndex + 1) % (int)m_textureNames.size();
            changed = true;
        }

        if (changed) {
            UpdateCurrentTextureInfo();
            Log::Printf("Menu texture browser: %d/%d '%s' (%dx%d)",
                        m_textureIndex + 1,
                        (int)m_textureNames.size(),
                        GetBrowserTextureName().c_str(),
                        m_textureWidth, m_textureHeight);
        }
    }

    // Cursor animation
    m_cursorFrame++;

    return false;
}

void MenuScene::Render() {
    if (!m_renderer || !m_resources) return;
    if (m_textureNames.empty()) return;

    const std::string& name = GetBrowserTextureName();
    if (name.empty()) return;

    ID3D11ShaderResourceView* srv = m_resources->GetTextureView(name);
    if (!srv) return;

    const float texW = (float)m_textureWidth;
    const float texH = (float)m_textureHeight;
    if (texW <= 0 || texH <= 0) return;

    const float fitX = (float)PS1_WIDTH / texW;
    const float fitY = (float)PS1_HEIGHT / texH;
    const float fitScale = std::min(1.0f, std::min(fitX, fitY));

    const float w = texW * fitScale * m_scale;
    const float h = texH * fitScale * m_scale;
    const float x = m_offsetX + ((float)PS1_WIDTH * m_scale - w) * 0.5f;
    const float y = m_offsetY + ((float)PS1_HEIGHT * m_scale - h) * 0.5f;

    m_renderer->DrawSprite(srv, x, y, w, h);
}

const std::string& MenuScene::GetBrowserTextureName() const {
    static const std::string empty;
    if (m_textureNames.empty()) return empty;
    if (m_textureIndex < 0 || m_textureIndex >= (int)m_textureNames.size()) return empty;
    return m_textureNames[(size_t)m_textureIndex];
}

void MenuScene::UpdateCurrentTextureInfo() {
    m_textureWidth = 0;
    m_textureHeight = 0;

    if (!m_resources) return;
    const std::string& name = GetBrowserTextureName();
    if (name.empty()) return;

    TextureResource* res = m_resources->GetTexture(name);
    if (!res) return;

    m_textureWidth = res->tim.width;
    m_textureHeight = res->tim.height;
}
