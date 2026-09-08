#pragma once
#include "tim_decoder.h"
#include "d3d11_renderer.h"
#include <cstddef>
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
    std::unordered_map<uint16_t, ID3D11ShaderResourceView*> psxAbr0StpClutSrvs;
    std::unordered_map<uint16_t, ID3D11ShaderResourceView*> psxAbr1StpClutSrvs;
    bool dirty = true;
};

struct PsxVramAtlasUploadResult {
    bool rendererKnown = false;
    bool complete = false;
    uint32_t tpageCount = 0u;
    uint32_t dirtyTpageCount = 0u;
    uint32_t createdTpageCount = 0u;
    uint32_t updatedTpageCount = 0u;
    uint32_t failedTpageCount = 0u;
    uint32_t readyTpageCount = 0u;
};

struct PsxVramAtlasStandaloneTimView {
    std::size_t imageIndex = 0u;
    int paletteRow = 0;
    ID3D11ShaderResourceView* srv = nullptr;
    D3D11Renderer* renderer = nullptr;
};

class PsxVramAtlas {
public:
    static const int TPAGE_W = 256;
    static const int TPAGE_H = 256;

    PsxVramAtlas();
    ~PsxVramAtlas();
    PsxVramAtlas(PsxVramAtlas&& other) noexcept;
    PsxVramAtlas& operator=(PsxVramAtlas&& other) noexcept;
    PsxVramAtlas(const PsxVramAtlas&) = delete;
    PsxVramAtlas& operator=(const PsxVramAtlas&) = delete;

    // Clear all tpage textures
    void Clear();

    // Register a tpage that will be needed (from TMD primitive data)
    void RegisterTpage(uint16_t tpage);

    // Register a CLUT row that will be needed by a TMD primitive.
    void RegisterClut(uint16_t clut);

    // Load a TIM texture, automatically placing it in the correct tpage texture(s).
    // raw = raw TIM file bytes, size = byte count
    // Preflight performs the same validation without logging or changing atlas state.
    bool CanLoadTim(const uint8_t* raw,
                    size_t size,
                    const std::string& name = "",
                    bool uploadClut = true,
                    bool filterToRequiredClutRow = true) const;

    bool LoadTim(const uint8_t* raw,
                 size_t size,
                 const std::string& name = "",
                 bool uploadClut = true,
                 bool filterToRequiredClutRow = true);

    // Overlay the authoritative words written by the PSX LoadImage path.
    // TIM replay establishes the atlas shape, while this projection preserves
    // later overlapping/partial VRAM writes and palette changes exactly as
    // they exist in the native scene loader.
    bool ApplyPartialVramWords(const uint16_t* words,
                               const uint8_t* known,
                               size_t wordCount);

    // Upload all dirty tpage textures to GPU
    PsxVramAtlasUploadResult UploadAll(D3D11Renderer* renderer);

    // Get the D3D11 texture for a specific tpage (returns nullptr if not loaded)
    ID3D11ShaderResourceView* GetTpageSRV(uint16_t tpage) const;

    // Get a tpage texture recolored with the primitive's CLUT. Used by TMD
    // rendering so multiple CLUT rows on one tpage keep PSX colors.
    ID3D11ShaderResourceView* GetTpageSRV(uint16_t tpage,
                                          uint16_t clut,
                                          D3D11Renderer* renderer);

    // Materialize one TIM upload independently from the shared final VRAM
    // page.  Scene0's native menu uploads overlapping TIMs and draws them
    // before a later upload overwrites the same VRAM rectangle, so a final
    // atlas page cannot reproduce those sprites by itself.
    ID3D11ShaderResourceView* GetStandaloneTimSRV(
        uint16_t orgX,
        uint16_t orgY,
        uint16_t width,
        uint16_t height,
        uint16_t clutX,
        uint16_t clutY,
        D3D11Renderer* renderer);

    // Get the exact CLUT texture used with normal alpha blending for PSX
    // ABR0. STP pixels carry alpha 0.5; non-STP pixels remain opaque.
    ID3D11ShaderResourceView* GetTpagePsxAbr0StpSRV(
        uint16_t tpage,
        uint16_t clut,
        D3D11Renderer* renderer);

    // Get the exact CLUT texture used with BlendMode::PsxAbr1Stp. STP pixels
    // retain RGB with alpha 0; non-STP pixels remain opaque.
    ID3D11ShaderResourceView* GetTpagePsxAbr1StpSRV(
        uint16_t tpage,
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

    // Convert PSX byte UVs to normalized host-atlas coordinates (0..1).
    // Each byte addresses a texel; use its centre so point sampling cannot
    // select an adjacent atlas texel at an integer page boundary.
    static void UVtoNormalized(uint8_t u, uint8_t v, float& out_u, float& out_v);

    int GetLoadedCount() const { return m_loadedCount; }
    int GetTpageCount() const { return (int)m_tpages.size(); }
    bool CanResolveTpageClut(uint16_t tpage, uint16_t clut) const;

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
    bool CanLoadTimTarget(uint16_t tpage,
                          uint16_t clutX,
                          uint16_t clutY,
                          uint16_t clutH,
                          bool filterToRequiredClutRow) const;
    ID3D11ShaderResourceView* BuildClutSrv(TpageTexture& tp,
                                           uint16_t clut,
                                           D3D11Renderer* renderer);
    ID3D11ShaderResourceView* BuildPsxAbr0StpClutSrv(
        TpageTexture& tp,
        uint16_t clut,
        D3D11Renderer* renderer);
    ID3D11ShaderResourceView* BuildPsxAbr1StpClutSrv(
        TpageTexture& tp,
        uint16_t clut,
        D3D11Renderer* renderer);
    void MoveFrom(PsxVramAtlas&& other) noexcept;
    void DestroyStandaloneTimViews();

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
    std::vector<TimImage> m_standaloneTimImages;
    std::vector<std::string> m_standaloneTimNames;
    std::vector<PsxVramAtlasStandaloneTimView> m_standaloneTimViews;
    D3D11Renderer* m_standaloneTimRenderer = nullptr;
    D3D11Renderer* m_renderer = nullptr;
    int m_loadedCount = 0;
};
