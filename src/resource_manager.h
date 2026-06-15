#pragma once
#include "int_loader.h"
#include "tim_decoder.h"
#include "d3d11_renderer.h"
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct TextureResource {
    std::string name;
    TimImage tim;
    ID3D11ShaderResourceView* srv = nullptr;
};

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();

    void SetRenderer(D3D11Renderer* renderer) { m_renderer = renderer; }

    // Load INT archive and extract all TIM textures
    bool LoadIntArchive(const std::string& path);

    // Load INT archive and extract only TIM textures (no MEM entries, no memOrder changes).
    // If keyPrefix is non-empty, textures will be stored under "{keyPrefix}{name}".
    bool LoadIntArchiveTimOnly(const std::string& path, const std::string& keyPrefix);

    bool LoadTimFromBytes(const uint8_t* data, size_t size, const std::string& keyPrefix);

    // Get a loaded texture by name (without extension)
    TextureResource* GetTexture(const std::string& name);

    // Create D3D11 texture from TIM data (lazy loading)
    ID3D11ShaderResourceView* GetTextureView(const std::string& name);

    const std::vector<uint8_t>* GetMem(const std::string& name) const;

    const std::vector<uint8_t>* GetTimRaw(const std::string& name) const;

    // Find a TIM by its VRAM placement and CLUT placement (PSX spriteTemplate matching)
    TextureResource* FindTextureByTimHeader(int bpp,
                                           int16_t orgX, int16_t orgY,
                                           uint32_t width, uint32_t height,
                                           int16_t clutX, int16_t clutY);

    // Get statistics
    size_t GetTextureCount() const { return m_textures.size(); }
    size_t GetMemCount() const { return m_mem.size(); }
    uint32_t GetGeneration() const { return m_generation; }

    std::vector<std::string> GetTextureNames() const;

    std::vector<std::string> GetMemNames() const;

    // Get Mem file names in INT file insertion order (for PSX dword_80091858 index mapping)
    const std::vector<std::string>& GetMemNamesOrdered() const { return m_memOrder; }

    std::vector<std::string> GetTimRawNames() const;

    void Clear();

private:
    D3D11Renderer* m_renderer = nullptr;
    std::map<std::string, std::unique_ptr<TextureResource>> m_textures;
    std::map<std::string, std::vector<uint8_t>> m_mem;
    std::map<std::string, std::vector<uint8_t>> m_timRaw;
    std::vector<std::string> m_memOrder;    // Mem filenames in INT insertion order
    std::vector<std::string> m_timRawOrder; // TIM keys in INT/load insertion order
    uint32_t m_generation = 0;  // 资源变更计数，用于触发背景重选
};
