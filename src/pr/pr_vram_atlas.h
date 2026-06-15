#pragma once
#include "tim_decoder.h"
#include "d3d11_renderer.h"
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

// Per-tpage texture atlas for PSX TMD rendering.
// Each unique tpage used by TMD primitives gets a 256x256 RGBA texture.
// TIM files are decoded and placed at their correct offset within the tpage texture.
//
// PSX tpage layout:
//   tpage index = (TSB & 0xF)         -> X base in 64-halfword steps
//   tpage Y     = ((TSB >> 4) & 1)    -> 0 or 256
//   color mode  = ((TSB >> 7) & 3)    -> 0=4bit, 1=8bit, 2=15bit
//
// Texture dimensions per tpage (always 256x256 texels):
//   4-bit:  64 halfwords wide  (64 * 4 = 256 texels)
//   8-bit: 128 halfwords wide  (128 * 2 = 256 texels)
//   15-bit: 256 halfwords wide (256 * 1 = 256 texels)

struct TpageTexture {
    uint16_t tpage = 0;
    int colorMode = 0;                 // 0=4bit, 1=8bit, 2=15bit
    int baseHW = 0;                    // base X in halfword units
    int baseY = 0;                     // base Y (0 or 256)
    std::vector<uint8_t> indexedPixels; // 256 * 256 palette indices for 4/8-bit TMD sampling
    std::vector<uint32_t> pixels;      // 256 * 256 RGBA
    ID3D11ShaderResourceView* srv = nullptr;
    std::unordered_map<uint16_t, ID3D11ShaderResourceView*> clutSrvs;
    bool dirty = true;
};

class PsxVramAtlas {
public:
    static const int TPAGE_W = 256;
    static const int TPAGE_H = 256;

    PsxVramAtlas();
    ~PsxVramAtlas();

    // Clear all tpage textures
    void Clear();

    // Register a tpage that will be needed (from TMD primitive data)
    void RegisterTpage(uint16_t tpage);

    // Register a CLUT row that will be needed by a TMD primitive.
    void RegisterClut(uint16_t clut);

    // Load a TIM texture, automatically placing it in the correct tpage texture(s).
    // raw = raw TIM file bytes, size = byte count
    bool LoadTim(const uint8_t* raw,
                 size_t size,
                 const std::string& name = "",
                 bool uploadClut = true,
                 bool filterToRequiredClutRow = true);

    // Upload all dirty tpage textures to GPU
    void UploadAll(D3D11Renderer* renderer);

    // Get the D3D11 texture for a specific tpage (returns nullptr if not loaded)
    ID3D11ShaderResourceView* GetTpageSRV(uint16_t tpage) const;

    // Get a tpage texture recolored with the primitive's CLUT. Used by TMD
    // rendering so multiple CLUT rows on one tpage keep PSX colors.
    ID3D11ShaderResourceView* GetTpageSRV(uint16_t tpage,
                                          uint16_t clut,
                                          D3D11Renderer* renderer);

    bool TryGetClutHasStpBits(uint16_t clut, bool& outHasStpBits) const;

    bool CopyRgbaRect(uint16_t tpage,
                      uint16_t clut,
                      int x,
                      int y,
                      int w,
                      int h,
                      std::vector<uint32_t>& out) const;

    // Convert TMD UV to normalized texture UV (0..1)
    // Simply: out_u = u / 256.0f, out_v = v / 256.0f
    static void UVtoNormalized(uint8_t u, uint8_t v, float& out_u, float& out_v);

    int GetLoadedCount() const { return m_loadedCount; }
    int GetTpageCount() const { return (int)m_tpages.size(); }

    // Debug: dump a tpage's pixel buffer as PNG
    bool DumpTpagePixels(uint16_t tpage, D3D11Renderer* renderer) const;

    // Debug: fill tpage with solid color
    void FillTpageColor(uint16_t tpage, uint32_t rgba);

private:
    // Find or create a tpage texture entry
    TpageTexture& GetOrCreateTpage(uint16_t tpage);

    void DestroyClutSrvs(TpageTexture& tp);
    void RegisterClutRows(const uint16_t* clut, int clutW, int clutH, int clutX, int clutY);
    bool TimHasRequiredClutRow(int clutX, int clutY, int clutH) const;
    ID3D11ShaderResourceView* BuildClutSrv(TpageTexture& tp,
                                           uint16_t clut,
                                           D3D11Renderer* renderer);

    // Decode 4-bit TIM pixels into a tpage texture
    void DecodeTim4bit(TpageTexture& tp,
                       const uint8_t* pixelData, size_t pixelBytes,
                       int imgX, int imgY, int imgW_hw, int imgH,
                       const uint16_t* clut, int clutSize);

    // Decode 8-bit TIM pixels into a tpage texture
    void DecodeTim8bit(TpageTexture& tp,
                       const uint8_t* pixelData, size_t pixelBytes,
                       int imgX, int imgY, int imgW_hw, int imgH,
                       const uint16_t* clut, int clutSize);

    std::unordered_map<uint16_t, TpageTexture> m_tpages;
    std::unordered_map<uint16_t, std::vector<uint16_t>> m_cluts;
    std::unordered_set<uint16_t> m_requiredCluts;
    D3D11Renderer* m_renderer = nullptr;
    int m_loadedCount = 0;
};
