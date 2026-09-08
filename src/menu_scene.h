#pragma once
#include "d3d11_renderer.h"
#include "resource_manager.h"

class MenuScene {
public:
    // S0_OLD_DELETE_AFTER_SS0: early Win texture-browser/menu shell.
    // Keep as observe-only until confirmed unused and removable.
    MenuScene();
    ~MenuScene();
    
    bool Initialize(D3D11Renderer* renderer, ResourceManager* resources);
    void Shutdown();
    
    // Returns true when menu selection is made
    bool Update(bool upPressed, bool downPressed, bool confirmPressed,
                bool prevTexPressed, bool nextTexPressed);
    void Render();
    
    int GetSelection() const { return m_selection; }
    bool IsComplete() const { return m_complete; }

    const std::string& GetBrowserTextureName() const;
    int GetBrowserTextureIndex() const { return m_textureIndex; }
    int GetBrowserTextureCount() const { return (int)m_textureNames.size(); }
    int GetBrowserTextureWidth() const { return m_textureWidth; }
    int GetBrowserTextureHeight() const { return m_textureHeight; }
    
private:
    void UpdateCurrentTextureInfo();

    D3D11Renderer* m_renderer = nullptr;
    ResourceManager* m_resources = nullptr;
    
    int m_selection = 0;
    int m_maxSelection = 3;  // Game Start, Options, Exit
    bool m_complete = false;

    std::vector<std::string> m_textureNames;
    int m_textureIndex = 0;
    int m_textureWidth = 0;
    int m_textureHeight = 0;
    
    // Simple cursor animation
    int m_cursorFrame = 0;
    float m_scale = 1.0f;
    float m_offsetX = 0;
    float m_offsetY = 0;
};
