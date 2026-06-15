#include "d3d11_renderer.h"
#include "logger.h"
#include <algorithm>
#include <vector>
#include <objbase.h>
#include <wincodec.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
 #pragma comment(lib, "windowscodecs.lib")
 #pragma comment(lib, "ole32.lib")

static const char* g_shaderCode = R"(
cbuffer Constants : register(b0) {
    float4 screenSize;  // width, height, 1/width, 1/height
    float4 tint;        // rgba multiply
};

struct VS_INPUT {
    float2 pos : POSITION;
    float2 uv : TEXCOORD0;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

PS_INPUT VSMain(VS_INPUT input) {
    PS_INPUT output;
    // Convert from pixel coords to NDC
    float2 ndc;
    ndc.x = (input.pos.x * screenSize.z) * 2.0 - 1.0;
    ndc.y = 1.0 - (input.pos.y * screenSize.w) * 2.0;
    output.pos = float4(ndc, 0, 1);
    output.uv = input.uv;
    return output;
}

float4 PSMain(PS_INPUT input) : SV_TARGET {
    return tex.Sample(samp, input.uv) * tint;
}
)";

// Per-vertex color shader for batched colored triangles (no texture)
static const char* g_colorShaderCode = R"(
cbuffer Constants : register(b0) {
    float4 screenSize;
    float4 tint;
};

struct VS_INPUT {
    float2 pos : POSITION;
    float4 color : COLOR;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

PS_INPUT VSColor(VS_INPUT input) {
    PS_INPUT output;
    float2 ndc;
    ndc.x = (input.pos.x * screenSize.z) * 2.0 - 1.0;
    ndc.y = 1.0 - (input.pos.y * screenSize.w) * 2.0;
    output.pos = float4(ndc, 0, 1);
    output.color = input.color;
    return output;
}

float4 PSColor(PS_INPUT input) : SV_TARGET {
    return input.color;
}
)";

// Textured triangle shader: per-vertex color * texture sample
static const char* g_texTriShaderCode = R"(
cbuffer Constants : register(b0) {
    float4 screenSize;
    float4 tint;
};

struct VS_INPUT {
    float2 pos : POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR;
    float perspectiveW : TEXCOORD1;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    noperspective float4 color : COLOR;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

PS_INPUT VSTexTri(VS_INPUT input) {
    PS_INPUT output;
    float2 ndc;
    ndc.x = (input.pos.x * screenSize.z) * 2.0 - 1.0;
    ndc.y = 1.0 - (input.pos.y * screenSize.w) * 2.0;
    float w = max(input.perspectiveW, 1.0);
    output.pos = float4(ndc * w, 0, w);
    output.uv = input.uv;
    output.color = input.color;
    return output;
}

float4 PSTexTri(PS_INPUT input) : SV_TARGET {
    float4 texColor = tex.Sample(samp, input.uv);
    // PSX color key: palette entry 0 (alpha=0) is fully transparent → discard
    if (texColor.a < 0.01) discard;
    // Non-transparent pixels: multiply texture RGB by vertex color, keep vertex alpha
    float4 result;
    result.rgb = texColor.rgb * input.color.rgb;
    result.a = input.color.a;
    return result;
}
)";

D3D11Renderer::D3D11Renderer() : m_width(0), m_height(0) {}

D3D11Renderer::~D3D11Renderer() {
    Shutdown();
}

bool D3D11Renderer::Initialize(HWND hwnd, int width, int height) {
    m_width = width;
    m_height = height;

    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 2;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
        nullptr, 0, D3D11_SDK_VERSION,
        &scd, &m_swapChain, &m_device, &featureLevel, &m_context);

    if (FAILED(hr)) return false;

    // Create render target view
    ComPtr<ID3D11Texture2D> backBuffer;
    hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) return false;

    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_rtv);
    if (FAILED(hr)) return false;

    {
        D3D11_TEXTURE2D_DESC dsDesc = {};
        dsDesc.Width = (UINT)width;
        dsDesc.Height = (UINT)height;
        dsDesc.MipLevels = 1;
        dsDesc.ArraySize = 1;
        dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dsDesc.SampleDesc.Count = 1;
        dsDesc.SampleDesc.Quality = 0;
        dsDesc.Usage = D3D11_USAGE_DEFAULT;
        dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        hr = m_device->CreateTexture2D(&dsDesc, nullptr, &m_depthStencil);
        if (FAILED(hr)) return false;
        hr = m_device->CreateDepthStencilView(m_depthStencil.Get(), nullptr, &m_dsv);
        if (FAILED(hr)) return false;

        D3D11_DEPTH_STENCIL_DESC dsn = {};
        dsn.DepthEnable = FALSE;
        dsn.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        dsn.DepthFunc = D3D11_COMPARISON_ALWAYS;
        dsn.StencilEnable = FALSE;
        dsn.StencilReadMask = 0xFF;
        dsn.StencilWriteMask = 0xFF;
        dsn.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        dsn.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        dsn.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        dsn.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        dsn.BackFace = dsn.FrontFace;
        hr = m_device->CreateDepthStencilState(&dsn, &m_dsStateNone);
        if (FAILED(hr)) return false;

        D3D11_DEPTH_STENCIL_DESC dss = dsn;
        dss.StencilEnable = TRUE;
        dss.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
        dss.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR_SAT;
        dss.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
        dss.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR_SAT;
        hr = m_device->CreateDepthStencilState(&dss, &m_dsStateShadow);
        if (FAILED(hr)) return false;
    }

    // Set viewport
    D3D11_VIEWPORT vp = {};
    vp.Width = (float)width;
    vp.Height = (float)height;
    vp.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &vp);

    if (!CreateShaders()) return false;
    if (!CreateBuffers()) return false;

    // Create 1x1 white texture for DrawRect
    uint32_t white = 0xFFFFFFFF;
    m_whiteTexture = CreateTexture(&white, 1, 1);

    return true;
}

void D3D11Renderer::Shutdown() {
    if (m_whiteTexture) {
        DestroyTexture(m_whiteTexture);
        m_whiteTexture = nullptr;
    }
    if (m_context) m_context->ClearState();
}

bool D3D11Renderer::CreateShaders() {
    ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;

    HRESULT hr = D3DCompile(g_shaderCode, strlen(g_shaderCode), nullptr, nullptr, nullptr,
                            "VSMain", "vs_5_0", 0, 0, &vsBlob, &errorBlob);
    if (FAILED(hr)) return false;

    hr = D3DCompile(g_shaderCode, strlen(g_shaderCode), nullptr, nullptr, nullptr,
                    "PSMain", "ps_5_0", 0, 0, &psBlob, &errorBlob);
    if (FAILED(hr)) return false;

    hr = m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
                                       nullptr, &m_vertexShader);
    if (FAILED(hr)) return false;

    hr = m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
                                      nullptr, &m_pixelShader);
    if (FAILED(hr)) return false;

    // Input layout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = m_device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(),
                                      vsBlob->GetBufferSize(), &m_inputLayout);
    if (FAILED(hr)) return false;

    // Colored triangle shaders (per-vertex color, no texture)
    {
        ComPtr<ID3DBlob> cvs, cps, cerr;
        hr = D3DCompile(g_colorShaderCode, strlen(g_colorShaderCode), nullptr, nullptr, nullptr,
                        "VSColor", "vs_5_0", 0, 0, &cvs, &cerr);
        if (FAILED(hr)) return false;
        hr = D3DCompile(g_colorShaderCode, strlen(g_colorShaderCode), nullptr, nullptr, nullptr,
                        "PSColor", "ps_5_0", 0, 0, &cps, &cerr);
        if (FAILED(hr)) return false;
        hr = m_device->CreateVertexShader(cvs->GetBufferPointer(), cvs->GetBufferSize(), nullptr, &m_colorVS);
        if (FAILED(hr)) return false;
        hr = m_device->CreatePixelShader(cps->GetBufferPointer(), cps->GetBufferSize(), nullptr, &m_colorPS);
        if (FAILED(hr)) return false;
        D3D11_INPUT_ELEMENT_DESC colorLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        hr = m_device->CreateInputLayout(colorLayout, 2, cvs->GetBufferPointer(),
                                          cvs->GetBufferSize(), &m_colorInputLayout);
        if (FAILED(hr)) return false;

        // Large dynamic VB for batched colored triangles
        D3D11_BUFFER_DESC cvbd = {};
        cvbd.Usage = D3D11_USAGE_DYNAMIC;
        cvbd.ByteWidth = sizeof(ColorVertex) * kMaxColorVerts;
        cvbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        cvbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        hr = m_device->CreateBuffer(&cvbd, nullptr, &m_colorVB);
        if (FAILED(hr)) return false;
    }

    // Textured triangle pipeline (position + UV + color)
    {
        ComPtr<ID3DBlob> tvs, tps, terr;
        hr = D3DCompile(g_texTriShaderCode, strlen(g_texTriShaderCode), nullptr, nullptr, nullptr,
                        "VSTexTri", "vs_5_0", 0, 0, &tvs, &terr);
        if (FAILED(hr)) return false;
        hr = D3DCompile(g_texTriShaderCode, strlen(g_texTriShaderCode), nullptr, nullptr, nullptr,
                        "PSTexTri", "ps_5_0", 0, 0, &tps, &terr);
        if (FAILED(hr)) return false;
        hr = m_device->CreateVertexShader(tvs->GetBufferPointer(), tvs->GetBufferSize(), nullptr, &m_texTriVS);
        if (FAILED(hr)) return false;
        hr = m_device->CreatePixelShader(tps->GetBufferPointer(), tps->GetBufferSize(), nullptr, &m_texTriPS);
        if (FAILED(hr)) return false;
        D3D11_INPUT_ELEMENT_DESC texTriLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        hr = m_device->CreateInputLayout(texTriLayout, 4, tvs->GetBufferPointer(),
                                          tvs->GetBufferSize(), &m_texTriInputLayout);
        if (FAILED(hr)) return false;

        D3D11_BUFFER_DESC tvbd = {};
        tvbd.Usage = D3D11_USAGE_DYNAMIC;
        tvbd.ByteWidth = sizeof(TexturedVertex) * kMaxTexTriVerts;
        tvbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        tvbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        hr = m_device->CreateBuffer(&tvbd, nullptr, &m_texTriVB);
        if (FAILED(hr)) return false;
    }

    // Sampler state
    D3D11_SAMPLER_DESC sd = {};
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    hr = m_device->CreateSamplerState(&sd, &m_sampler);
    if (FAILED(hr)) return false;

    // Blend state for alpha
    D3D11_BLEND_DESC bd = {};
    bd.RenderTarget[0].BlendEnable = TRUE;
    bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    hr = m_device->CreateBlendState(&bd, &m_blendStateAlpha);
    if (FAILED(hr)) return false;

    // Blend state for additive
    D3D11_BLEND_DESC bdAdd = bd;
    bdAdd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bdAdd.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    bdAdd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    hr = m_device->CreateBlendState(&bdAdd, &m_blendStateAdditive);
    if (FAILED(hr)) return false;

    D3D11_BLEND_DESC bdSub = bd;
    bdSub.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bdSub.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    bdSub.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
    hr = m_device->CreateBlendState(&bdSub, &m_blendStateSubtractive);
    if (FAILED(hr)) return false;

    return true;
}

bool D3D11Renderer::CreateBuffers() {
    // Vertex buffer for a quad (6 vertices)
    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DYNAMIC;
    vbd.ByteWidth = sizeof(Vertex) * 6;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = m_device->CreateBuffer(&vbd, nullptr, &m_vertexBuffer);
    if (FAILED(hr)) return false;

    // Constant buffer
    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = 32;  // float4 screenSize + float4 tint
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = m_device->CreateBuffer(&cbd, nullptr, &m_constantBuffer);
    if (FAILED(hr)) return false;

    return true;
}

void D3D11Renderer::BeginFrame(float r, float g, float b) {
    float clearColor[4] = { r, g, b, 1.0f };
    m_context->OMSetRenderTargets(1, m_rtv.GetAddressOf(), m_dsv.Get());
    m_context->ClearRenderTargetView(m_rtv.Get(), clearColor);
    if (m_dsv) {
        m_context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        if (m_dsStateNone) {
            m_context->OMSetDepthStencilState(m_dsStateNone.Get(), 0);
        }
    }

    // Update constant buffer
    D3D11_MAPPED_SUBRESOURCE mapped;
    m_context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    float* cb = (float*)mapped.pData;
    cb[0] = (float)m_width;
    cb[1] = (float)m_height;
    cb[2] = 1.0f / m_width;
    cb[3] = 1.0f / m_height;
    cb[4] = 1.0f;
    cb[5] = 1.0f;
    cb[6] = 1.0f;
    cb[7] = 1.0f;
    m_context->Unmap(m_constantBuffer.Get(), 0);

    // Set pipeline state
    m_context->IASetInputLayout(m_inputLayout.Get());
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    m_context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    m_context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    m_context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());
    m_context->OMSetBlendState(m_blendStateAlpha.Get(), nullptr, 0xFFFFFFFF);
}

void D3D11Renderer::EndFrame() {
    m_swapChain->Present(0, 0);  // VSync enabled
}

ID3D11ShaderResourceView* D3D11Renderer::CreateTexture(const uint32_t* rgba, int width, int height) {
    D3D11_TEXTURE2D_DESC td = {};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DYNAMIC;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = rgba;
    initData.SysMemPitch = width * 4;

    ComPtr<ID3D11Texture2D> tex;
    HRESULT hr = m_device->CreateTexture2D(&td, &initData, &tex);
    if (FAILED(hr)) return nullptr;

    ID3D11ShaderResourceView* srv = nullptr;
    hr = m_device->CreateShaderResourceView(tex.Get(), nullptr, &srv);
    if (FAILED(hr)) return nullptr;

    return srv;
}

void D3D11Renderer::UpdateTexture(ID3D11ShaderResourceView* srv, const uint32_t* rgba, int width, int height) {
    if (!srv) return;

    ComPtr<ID3D11Resource> resource;
    srv->GetResource(&resource);

    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = m_context->Map(resource.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (SUCCEEDED(hr)) {
        for (int y = 0; y < height; y++) {
            memcpy((uint8_t*)mapped.pData + y * mapped.RowPitch,
                   rgba + y * width, width * 4);
        }
        m_context->Unmap(resource.Get(), 0);
    } else {
        Log::Printf("UpdateTexture: Map FAILED hr=0x%08X srv=%p", (unsigned)hr, (void*)srv);
    }
}

void D3D11Renderer::DestroyTexture(ID3D11ShaderResourceView* srv) {
    if (srv) srv->Release();
}

void D3D11Renderer::DrawSprite(ID3D11ShaderResourceView* texture,
                                float x, float y, float w, float h,
                                float u0, float v0, float u1, float v1) {
    DrawSpriteTint(texture, x, y, w, h, u0, v0, u1, v1, 1.0f, 1.0f, 1.0f, 1.0f, BlendMode::Alpha);
}

void D3D11Renderer::DrawSpriteTint(ID3D11ShaderResourceView* texture,
                                    float x, float y, float w, float h,
                                    float u0, float v0, float u1, float v1,
                                    float r, float g, float b, float a,
                                    BlendMode blend) {
    if (!texture) return;

    // Update constant buffer tint
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        m_context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        float* cb = (float*)mapped.pData;
        cb[0] = (float)m_width;
        cb[1] = (float)m_height;
        cb[2] = 1.0f / m_width;
        cb[3] = 1.0f / m_height;
        cb[4] = r;
        cb[5] = g;
        cb[6] = b;
        cb[7] = a;
        m_context->Unmap(m_constantBuffer.Get(), 0);
    }

    // Blend state
    if (blend == BlendMode::Additive) {
        m_context->OMSetBlendState(m_blendStateAdditive.Get(), nullptr, 0xFFFFFFFF);
    } else if (blend == BlendMode::Subtractive) {
        m_context->OMSetBlendState(m_blendStateSubtractive.Get(), nullptr, 0xFFFFFFFF);
    } else {
        m_context->OMSetBlendState(m_blendStateAlpha.Get(), nullptr, 0xFFFFFFFF);
    }

    // Update vertex buffer
    Vertex vertices[6] = {
        { x,     y,     u0, v0 },
        { x + w, y,     u1, v0 },
        { x,     y + h, u0, v1 },
        { x + w, y,     u1, v0 },
        { x + w, y + h, u1, v1 },
        { x,     y + h, u0, v1 },
    };

    D3D11_MAPPED_SUBRESOURCE mappedVB;
    m_context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVB);
    memcpy(mappedVB.pData, vertices, sizeof(vertices));
    m_context->Unmap(m_vertexBuffer.Get(), 0);

    // Draw
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    m_context->PSSetShaderResources(0, 1, &texture);
    m_context->Draw(6, 0);
}

void D3D11Renderer::DrawRect(float x, float y, float w, float h,
                              float r, float g, float b, float a) {
    if (!m_whiteTexture) return;

    // Update white texture with the desired color
    uint8_t rb = (uint8_t)(r * 255.0f);
    uint8_t gb = (uint8_t)(g * 255.0f);
    uint8_t bb = (uint8_t)(b * 255.0f);
    uint8_t ab = (uint8_t)(a * 255.0f);
    uint32_t color = (ab << 24) | (bb << 16) | (gb << 8) | rb;
    UpdateTexture(m_whiteTexture, &color, 1, 1);

    DrawSpriteTint(m_whiteTexture, x, y, w, h, 0, 0, 1, 1, 1.0f, 1.0f, 1.0f, 1.0f, BlendMode::Alpha);
}

void D3D11Renderer::DrawTriangle(float x0, float y0, float x1, float y1, float x2, float y2,
                                  float r, float g, float b, float a) {
    if (!m_whiteTexture) return;

    // Tint via constant buffer
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        m_context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        float* cb = (float*)mapped.pData;
        cb[0] = (float)m_width;
        cb[1] = (float)m_height;
        cb[2] = 1.0f / m_width;
        cb[3] = 1.0f / m_height;
        cb[4] = r;
        cb[5] = g;
        cb[6] = b;
        cb[7] = a;
        m_context->Unmap(m_constantBuffer.Get(), 0);
    }

    m_context->OMSetBlendState(m_blendStateAlpha.Get(), nullptr, 0xFFFFFFFF);

    // 3 vertices for triangle, duplicated to fill 6-vertex buffer (draw 3)
    Vertex vertices[6] = {
        { x0, y0, 0.0f, 0.0f },
        { x1, y1, 0.0f, 0.0f },
        { x2, y2, 0.0f, 0.0f },
        { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
    };

    D3D11_MAPPED_SUBRESOURCE mappedVB;
    m_context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVB);
    memcpy(mappedVB.pData, vertices, sizeof(vertices));
    m_context->Unmap(m_vertexBuffer.Get(), 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    m_context->PSSetShaderResources(0, 1, &m_whiteTexture);
    m_context->Draw(3, 0);
}

void D3D11Renderer::DrawTriangleBatch(const ColorVertex* vertices,
                                      int vertexCount,
                                      BlendMode blend) {
    if (!m_colorVB || !m_colorVS || !m_colorPS || vertexCount <= 0) return;
    if (vertexCount > kMaxColorVerts) vertexCount = kMaxColorVerts;

    // Update screen size in constant buffer
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        m_context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        float* cb = (float*)mapped.pData;
        cb[0] = (float)m_width;
        cb[1] = (float)m_height;
        cb[2] = 1.0f / m_width;
        cb[3] = 1.0f / m_height;
        cb[4] = 1.0f; cb[5] = 1.0f; cb[6] = 1.0f; cb[7] = 1.0f;
        m_context->Unmap(m_constantBuffer.Get(), 0);
    }

    // Upload vertex data
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        m_context->Map(m_colorVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, vertices, sizeof(ColorVertex) * vertexCount);
        m_context->Unmap(m_colorVB.Get(), 0);
    }

    // Switch to color pipeline
    m_context->IASetInputLayout(m_colorInputLayout.Get());
    m_context->VSSetShader(m_colorVS.Get(), nullptr, 0);
    m_context->PSSetShader(m_colorPS.Get(), nullptr, 0);
    if (blend == BlendMode::Additive) {
        m_context->OMSetBlendState(m_blendStateAdditive.Get(), nullptr, 0xFFFFFFFF);
    } else if (blend == BlendMode::Subtractive) {
        m_context->OMSetBlendState(m_blendStateSubtractive.Get(), nullptr, 0xFFFFFFFF);
    } else {
        m_context->OMSetBlendState(m_blendStateAlpha.Get(), nullptr, 0xFFFFFFFF);
    }

    UINT stride = sizeof(ColorVertex);
    UINT offset = 0;
    m_context->IASetVertexBuffers(0, 1, m_colorVB.GetAddressOf(), &stride, &offset);
    m_context->Draw(vertexCount, 0);

    // Restore sprite pipeline
    m_context->IASetInputLayout(m_inputLayout.Get());
    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
}

void D3D11Renderer::BeginShadowStencil() {
    if (!m_context || !m_dsv || !m_dsStateShadow) return;
    m_context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_STENCIL, 1.0f, 0);
    m_context->OMSetDepthStencilState(m_dsStateShadow.Get(), 0);
}

void D3D11Renderer::EndShadowStencil() {
    if (!m_context || !m_dsStateNone) return;
    m_context->OMSetDepthStencilState(m_dsStateNone.Get(), 0);
}

void D3D11Renderer::DrawTexturedTriangleBatch(ID3D11ShaderResourceView* texture,
                                              const TexturedVertex* vertices,
                                              int vertexCount,
                                              BlendMode blend) {
    if (!m_texTriVB || !m_texTriVS || !m_texTriPS || !texture || vertexCount <= 0) return;
    if (vertexCount > kMaxTexTriVerts) vertexCount = kMaxTexTriVerts;

    // Update screen size in constant buffer
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        m_context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        float* cb = (float*)mapped.pData;
        cb[0] = (float)m_width;
        cb[1] = (float)m_height;
        cb[2] = 1.0f / m_width;
        cb[3] = 1.0f / m_height;
        cb[4] = 1.0f; cb[5] = 1.0f; cb[6] = 1.0f; cb[7] = 1.0f;
        m_context->Unmap(m_constantBuffer.Get(), 0);
    }

    // Upload vertex data
    {
        D3D11_MAPPED_SUBRESOURCE mapped;
        m_context->Map(m_texTriVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        memcpy(mapped.pData, vertices, sizeof(TexturedVertex) * vertexCount);
        m_context->Unmap(m_texTriVB.Get(), 0);
    }

    // Switch to textured triangle pipeline
    m_context->IASetInputLayout(m_texTriInputLayout.Get());
    m_context->VSSetShader(m_texTriVS.Get(), nullptr, 0);
    m_context->PSSetShader(m_texTriPS.Get(), nullptr, 0);
    m_context->PSSetShaderResources(0, 1, &texture);
    m_context->PSSetSamplers(0, 1, m_sampler.GetAddressOf());
    if (blend == BlendMode::Additive) {
        m_context->OMSetBlendState(m_blendStateAdditive.Get(), nullptr, 0xFFFFFFFF);
    } else if (blend == BlendMode::Subtractive) {
        m_context->OMSetBlendState(m_blendStateSubtractive.Get(), nullptr, 0xFFFFFFFF);
    } else {
        m_context->OMSetBlendState(m_blendStateAlpha.Get(), nullptr, 0xFFFFFFFF);
    }

    UINT stride = sizeof(TexturedVertex);
    UINT offset = 0;
    m_context->IASetVertexBuffers(0, 1, m_texTriVB.GetAddressOf(), &stride, &offset);
    m_context->Draw(vertexCount, 0);

    // Restore sprite pipeline
    m_context->IASetInputLayout(m_inputLayout.Get());
    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
}

void D3D11Renderer::SubmitSprite(const SpriteCmd& cmd) {
    SpriteCmd c = cmd;
    if (c.order == 0) {
        c.order = ++m_spriteOrder;
    } else {
        if (c.order > m_spriteOrder) {
            m_spriteOrder = c.order;
        }
    }
    QueuedSpriteCmd queued{};
    queued.kind = QueuedSpriteCmd::Kind::Sprite;
    queued.sprite = c;
    queued.layer = c.layer;
    queued.order = c.order;
    m_spriteQueue.push_back(queued);
}

void D3D11Renderer::SubmitTexturedTriangles(const TexturedTriCmd& cmd) {
    TexturedTriCmd c = cmd;
    if (c.vertexCount <= 0 || c.texture == nullptr) {
        return;
    }
    if (c.vertexCount > 6) {
        c.vertexCount = 6;
    }
    if (c.order == 0) {
        c.order = ++m_spriteOrder;
    } else {
        if (c.order > m_spriteOrder) {
            m_spriteOrder = c.order;
        }
    }
    QueuedSpriteCmd queued{};
    queued.kind = QueuedSpriteCmd::Kind::TexturedTriangles;
    queued.texturedTriangles = c;
    queued.layer = c.layer;
    queued.order = c.order;
    m_spriteQueue.push_back(queued);
}

void D3D11Renderer::SubmitColorTriangles(const ColorTriCmd& cmd) {
    ColorTriCmd c = cmd;
    if (c.vertexCount <= 0) {
        return;
    }
    if (c.vertexCount > 6) {
        c.vertexCount = 6;
    }
    if (c.order == 0) {
        c.order = ++m_spriteOrder;
    } else {
        if (c.order > m_spriteOrder) {
            m_spriteOrder = c.order;
        }
    }
    QueuedSpriteCmd queued{};
    queued.kind = QueuedSpriteCmd::Kind::ColoredTriangles;
    queued.colorTriangles = c;
    queued.layer = c.layer;
    queued.order = c.order;
    m_spriteQueue.push_back(queued);
}

void D3D11Renderer::SubmitSolidRect(const SolidRectCmd& cmd) {
    SolidRectCmd c = cmd;
    if (c.w <= 0.0f || c.h <= 0.0f) {
        return;
    }
    if (c.order == 0) {
        c.order = ++m_spriteOrder;
    } else {
        if (c.order > m_spriteOrder) {
            m_spriteOrder = c.order;
        }
    }
    QueuedSpriteCmd queued{};
    queued.kind = QueuedSpriteCmd::Kind::SolidRect;
    queued.solidRect = c;
    queued.layer = c.layer;
    queued.order = c.order;
    m_spriteQueue.push_back(queued);
}

void D3D11Renderer::FlushSprites() {
    if (m_spriteQueue.empty()) {
        return;
    }

    std::stable_sort(m_spriteQueue.begin(), m_spriteQueue.end(), [](const QueuedSpriteCmd& a, const QueuedSpriteCmd& b) {
        if (a.layer != b.layer) {
            return a.layer < b.layer;
        }
        return a.order < b.order;
    });

    for (const auto& c : m_spriteQueue) {
        if (c.kind == QueuedSpriteCmd::Kind::TexturedTriangles) {
            DrawTexturedTriangleBatch(c.texturedTriangles.texture,
                                      c.texturedTriangles.vertices,
                                      c.texturedTriangles.vertexCount,
                                      c.texturedTriangles.blend);
        } else if (c.kind == QueuedSpriteCmd::Kind::ColoredTriangles) {
            DrawTriangleBatch(c.colorTriangles.vertices,
                              c.colorTriangles.vertexCount,
                              c.colorTriangles.blend);
        } else if (c.kind == QueuedSpriteCmd::Kind::SolidRect) {
            const SolidRectCmd& r = c.solidRect;
            DrawRect(r.x, r.y, r.w, r.h, r.r, r.g, r.b, r.a);
        } else {
            const SpriteCmd& s = c.sprite;
            DrawSpriteTint(s.texture, s.x, s.y, s.w, s.h, s.u0, s.v0, s.u1, s.v1, s.r, s.g, s.b, s.a, s.blend);
        }
    }

    m_spriteQueue.clear();
    m_spriteOrder = 0;
}

void D3D11Renderer::ClearSprites() {
    m_spriteQueue.clear();
    m_spriteOrder = 0;
}

bool D3D11Renderer::SaveScreenshot(const std::wstring& filename) {
    if (!m_device || !m_context || !m_swapChain) return false;

    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) return false;

    D3D11_TEXTURE2D_DESC desc;
    backBuffer->GetDesc(&desc);

    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.MiscFlags = 0;

    ComPtr<ID3D11Texture2D> staging;
    hr = m_device->CreateTexture2D(&desc, nullptr, &staging);
    if (FAILED(hr)) return false;

    m_context->CopyResource(staging.Get(), backBuffer.Get());

    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = m_context->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) return false;

    const int width = (int)desc.Width;
    const int height = (int)desc.Height;
    const UINT dstStride = width * 4;
    std::vector<uint8_t> bgra(dstStride * height);

    for (int y = 0; y < height; y++) {
        const uint8_t* srcRow = (const uint8_t*)mapped.pData + (size_t)y * mapped.RowPitch;
        uint8_t* dstRow = bgra.data() + (size_t)y * dstStride;
        for (int x = 0; x < width; x++) {
            const size_t s = (size_t)x * 4;
            const size_t d = (size_t)x * 4;
            dstRow[d + 0] = srcRow[s + 2];
            dstRow[d + 1] = srcRow[s + 1];
            dstRow[d + 2] = srcRow[s + 0];
            dstRow[d + 3] = 255;
        }
    }

    m_context->Unmap(staging.Get(), 0);

    const HRESULT hrCom = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool didInitCom = SUCCEEDED(hrCom);
    if (FAILED(hrCom) && hrCom != RPC_E_CHANGED_MODE) return false;

    bool success = false;
    do {
        ComPtr<IWICImagingFactory> factory;
        hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&factory));
        if (FAILED(hr)) break;

        ComPtr<IWICStream> stream;
        hr = factory->CreateStream(&stream);
        if (FAILED(hr)) break;

        hr = stream->InitializeFromFilename(filename.c_str(), GENERIC_WRITE);
        if (FAILED(hr)) break;

        ComPtr<IWICBitmapEncoder> encoder;
        hr = factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder);
        if (FAILED(hr)) break;

        hr = encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);
        if (FAILED(hr)) break;

        ComPtr<IWICBitmapFrameEncode> frame;
        ComPtr<IPropertyBag2> props;
        hr = encoder->CreateNewFrame(&frame, &props);
        if (FAILED(hr)) break;

        hr = frame->Initialize(props.Get());
        if (FAILED(hr)) break;

        hr = frame->SetSize((UINT)width, (UINT)height);
        if (FAILED(hr)) break;

        WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
        hr = frame->SetPixelFormat(&format);
        if (FAILED(hr) || format != GUID_WICPixelFormat32bppBGRA) break;

        hr = frame->WritePixels((UINT)height, dstStride, (UINT)bgra.size(), bgra.data());
        if (FAILED(hr)) break;

        hr = frame->Commit();
        if (FAILED(hr)) break;

        hr = encoder->Commit();
        if (FAILED(hr)) break;

        success = true;
    } while (false);

    if (didInitCom) CoUninitialize();
    return success;
}

bool D3D11Renderer::SaveRgbaPng(const std::wstring& filename, const uint32_t* rgba, int width, int height) {
    if (!rgba || width <= 0 || height <= 0) return false;

    const UINT dstStride = (UINT)width * 4;
    std::vector<uint8_t> bgra((size_t)dstStride * (size_t)height);

    for (int y = 0; y < height; y++) {
        const uint32_t* srcRow = rgba + (size_t)y * (size_t)width;
        uint8_t* dstRow = bgra.data() + (size_t)y * (size_t)dstStride;
        for (int x = 0; x < width; x++) {
            const uint32_t c = srcRow[x];
            const uint8_t r = (uint8_t)(c & 0xFF);
            const uint8_t g = (uint8_t)((c >> 8) & 0xFF);
            const uint8_t b = (uint8_t)((c >> 16) & 0xFF);
            const uint8_t a = (uint8_t)((c >> 24) & 0xFF);
            const size_t d = (size_t)x * 4;
            dstRow[d + 0] = b;
            dstRow[d + 1] = g;
            dstRow[d + 2] = r;
            dstRow[d + 3] = a;
        }
    }

    const HRESULT hrCom = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool didInitCom = SUCCEEDED(hrCom);
    if (FAILED(hrCom) && hrCom != RPC_E_CHANGED_MODE) return false;

    bool success = false;
    HRESULT hr;
    do {
        ComPtr<IWICImagingFactory> factory;
        hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&factory));
        if (FAILED(hr)) break;

        ComPtr<IWICStream> stream;
        hr = factory->CreateStream(&stream);
        if (FAILED(hr)) break;

        hr = stream->InitializeFromFilename(filename.c_str(), GENERIC_WRITE);
        if (FAILED(hr)) break;

        ComPtr<IWICBitmapEncoder> encoder;
        hr = factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder);
        if (FAILED(hr)) break;

        hr = encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);
        if (FAILED(hr)) break;

        ComPtr<IWICBitmapFrameEncode> frame;
        ComPtr<IPropertyBag2> props;
        hr = encoder->CreateNewFrame(&frame, &props);
        if (FAILED(hr)) break;

        hr = frame->Initialize(props.Get());
        if (FAILED(hr)) break;

        hr = frame->SetSize((UINT)width, (UINT)height);
        if (FAILED(hr)) break;

        WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
        hr = frame->SetPixelFormat(&format);
        if (FAILED(hr) || format != GUID_WICPixelFormat32bppBGRA) break;

        hr = frame->WritePixels((UINT)height, dstStride, (UINT)bgra.size(), bgra.data());
        if (FAILED(hr)) break;

        hr = frame->Commit();
        if (FAILED(hr)) break;

        hr = encoder->Commit();
        if (FAILED(hr)) break;

        success = true;
    } while (false);

    if (didInitCom) CoUninitialize();
    return success;
}
