#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <cstdint>
#include <string>
 #include <vector>

using Microsoft::WRL::ComPtr;

struct Vertex {
    float x, y;
    float u, v;
};

struct ColorVertex {
    float x, y;
    float r, g, b, a;
};

struct TexturedVertex {
    float x, y;
    float u, v;
    float r, g, b, a;
    float perspectiveW = 1.0f;
};

class D3D11Renderer {
public:
    enum class BlendMode : uint8_t {
        Alpha = 0,
        Additive = 1,
        Subtractive = 2,
    };

    struct SpriteCmd {
        ID3D11ShaderResourceView* texture = nullptr;
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
        float u0 = 0.0f;
        float v0 = 0.0f;
        float u1 = 1.0f;
        float v1 = 1.0f;
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
        float a = 1.0f;
        BlendMode blend = BlendMode::Alpha;
        int layer = 0;
        uint64_t order = 0;
    };

    struct TexturedTriCmd {
        ID3D11ShaderResourceView* texture = nullptr;
        TexturedVertex vertices[6]{};
        int vertexCount = 0;
        BlendMode blend = BlendMode::Alpha;
        int layer = 0;
        uint64_t order = 0;
    };

    struct ColorTriCmd {
        ColorVertex vertices[6]{};
        int vertexCount = 0;
        BlendMode blend = BlendMode::Alpha;
        int layer = 0;
        uint64_t order = 0;
    };

    struct SolidRectCmd {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
        float a = 1.0f;
        int layer = 0;
        uint64_t order = 0;
    };

    D3D11Renderer();
    ~D3D11Renderer();

    bool Initialize(HWND hwnd, int width, int height);
    void Shutdown();

    void BeginFrame(float r, float g, float b);
    void EndFrame();

    // Texture management
    ID3D11ShaderResourceView* CreateTexture(const uint32_t* rgba, int width, int height);
    void UpdateTexture(ID3D11ShaderResourceView* srv, const uint32_t* rgba, int width, int height);
    void DestroyTexture(ID3D11ShaderResourceView* srv);

    // Sprite rendering
    void DrawSprite(ID3D11ShaderResourceView* texture,
                    float x, float y, float w, float h,
                    float u0 = 0, float v0 = 0, float u1 = 1, float v1 = 1);

    void DrawSpriteTint(ID3D11ShaderResourceView* texture,
                        float x, float y, float w, float h,
                        float u0, float v0, float u1, float v1,
                        float r, float g, float b, float a,
                        BlendMode blend = BlendMode::Alpha);

    // Solid color rectangle (for UI overlay)
    void DrawRect(float x, float y, float w, float h,
                  float r, float g, float b, float a);

    // Solid color triangle (for TMD primitives) - single call, slow
    void DrawTriangle(float x0, float y0, float x1, float y1, float x2, float y2,
                      float r, float g, float b, float a);

    // Batched colored triangles (for TMD) - all triangles in one draw call
    void DrawTriangleBatch(const ColorVertex* vertices,
                           int vertexCount,
                           BlendMode blend = BlendMode::Alpha);

    // Batched textured triangles (for TMD with textures) - per-vertex color * texture
    void DrawTexturedTriangleBatch(ID3D11ShaderResourceView* texture,
                                   const TexturedVertex* vertices,
                                   int vertexCount,
                                   BlendMode blend = BlendMode::Alpha);

    void BeginShadowStencil();
    void EndShadowStencil();

    void SubmitSprite(const SpriteCmd& cmd);
    void SubmitTexturedTriangles(const TexturedTriCmd& cmd);
    void SubmitColorTriangles(const ColorTriCmd& cmd);
    void SubmitSolidRect(const SolidRectCmd& cmd);
    void FlushSprites();
    void ClearSprites();

    // Screenshot
    bool SaveScreenshot(const std::wstring& filename);
    bool SaveRgbaPng(const std::wstring& filename, const uint32_t* rgba, int width, int height);

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    bool CreateShaders();
    bool CreateBuffers();

    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_rtv;

    ComPtr<ID3D11Texture2D> m_depthStencil;
    ComPtr<ID3D11DepthStencilView> m_dsv;
    ComPtr<ID3D11DepthStencilState> m_dsStateNone;
    ComPtr<ID3D11DepthStencilState> m_dsStateShadow;

    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11Buffer> m_constantBuffer;
    ComPtr<ID3D11SamplerState> m_sampler;
    ComPtr<ID3D11BlendState> m_blendStateAlpha;
    ComPtr<ID3D11BlendState> m_blendStateAdditive;
    ComPtr<ID3D11BlendState> m_blendStateSubtractive;

    ID3D11ShaderResourceView* m_whiteTexture = nullptr;

    // Colored triangle pipeline (per-vertex color, no texture)
    ComPtr<ID3D11VertexShader> m_colorVS;
    ComPtr<ID3D11PixelShader> m_colorPS;
    ComPtr<ID3D11InputLayout> m_colorInputLayout;
    ComPtr<ID3D11Buffer> m_colorVB;  // large dynamic VB for batched tris
    static const int kMaxColorVerts = 4096;

    // Textured triangle pipeline (per-vertex color * texture)
    ComPtr<ID3D11VertexShader> m_texTriVS;
    ComPtr<ID3D11PixelShader> m_texTriPS;
    ComPtr<ID3D11InputLayout> m_texTriInputLayout;
    ComPtr<ID3D11Buffer> m_texTriVB;
    static const int kMaxTexTriVerts = 4096;

    struct QueuedSpriteCmd {
        enum class Kind : uint8_t {
            Sprite,
            TexturedTriangles,
            ColoredTriangles,
            SolidRect,
        };
        Kind kind = Kind::Sprite;
        SpriteCmd sprite{};
        TexturedTriCmd texturedTriangles{};
        ColorTriCmd colorTriangles{};
        SolidRectCmd solidRect{};
        int layer = 0;
        uint64_t order = 0;
    };

    std::vector<QueuedSpriteCmd> m_spriteQueue;
    uint64_t m_spriteOrder = 0;

    int m_width;
    int m_height;
};
