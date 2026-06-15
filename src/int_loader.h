#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <memory>

// INT file block types (from int_extract.py)
enum class IntBlockType : uint32_t {
    Tim = 1,    // TIM texture
    Vab = 2,    // VAB audio
    Mem = 3,    // Raw memory block
    EoF = 0xFFFFFFFF  // End of file marker
};

struct IntFileEntry {
    std::string name;
    IntBlockType type;
    uint32_t offset;
    uint32_t size;
    std::vector<uint8_t> data;
};

struct IntArchive {
    std::string path;
    std::vector<IntFileEntry> entries;
    std::map<std::string, size_t> nameIndex;  // name -> entry index

    const IntFileEntry* Find(const std::string& name) const {
        auto it = nameIndex.find(name);
        if (it != nameIndex.end()) {
            return &entries[it->second];
        }
        return nullptr;
    }
};

class IntLoader {
public:
    // Load an INT archive from file
    static bool Load(const std::string& path, IntArchive& archive);

    // Load an INT archive from memory
    static bool LoadFromMemory(const uint8_t* data, size_t size, IntArchive& archive);

private:
    static const uint32_t HEADER_SIZE = 8192;
    static const uint32_t SECTOR_SIZE = 2048;
    static const uint32_t ENTRY_SIZE = 20;  // size(4) + name(16)
};
