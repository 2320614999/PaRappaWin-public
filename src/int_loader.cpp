#include "int_loader.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <filesystem>

static uint32_t align4(uint32_t x) {
    return (x + 3) & ~3;
}

bool IntLoader::Load(const std::string& path, IntArchive& archive) {
    std::ifstream file(std::filesystem::u8path(path), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return false;
    }

    size_t fileSize = file.tellg();
    file.seekg(0);

    std::vector<uint8_t> data(fileSize);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    file.close();

    archive.path = path;
    return LoadFromMemory(data.data(), data.size(), archive);
}

bool IntLoader::LoadFromMemory(const uint8_t* data, size_t size, IntArchive& archive) {
    archive.entries.clear();
    archive.nameIndex.clear();

    size_t offset = 0;

    while (offset + HEADER_SIZE <= size) {
        const uint8_t* header = data + offset;

        // Block header: type(4), files(4), sectors(4), unk(4)
        uint32_t blockType = *reinterpret_cast<const uint32_t*>(header + 0);
        uint32_t numFiles = *reinterpret_cast<const uint32_t*>(header + 4);
        uint32_t sectors = *reinterpret_cast<const uint32_t*>(header + 8);

        // End of file marker
        if (blockType == static_cast<uint32_t>(IntBlockType::EoF)) {
            break;
        }

        IntBlockType type = static_cast<IntBlockType>(blockType);

        // Validate
        const uint32_t maxEntries = (HEADER_SIZE - 16) / ENTRY_SIZE;
        if (numFiles > maxEntries) {
            break;
        }

        // Parse file entries starting at offset 16
        // Each entry: size(4), name(16) = 20 bytes
        const uint8_t* blockData = data + offset + HEADER_SIZE;
        uint32_t dataSize = sectors * SECTOR_SIZE;

        if (offset + HEADER_SIZE + dataSize > size) {
            break;  // Prevent overflow
        }

        uint32_t dataCursor = 0;

        for (uint32_t i = 0; i < numFiles; i++) {
            const uint8_t* entryPtr = header + 16 + i * ENTRY_SIZE;

            uint32_t entrySize = *reinterpret_cast<const uint32_t*>(entryPtr + 0);

            char nameRaw[17] = {0};
            memcpy(nameRaw, entryPtr + 4, 16);

            std::string name(nameRaw);
            // Trim trailing spaces and nulls
            while (!name.empty() && (name.back() == ' ' || name.back() == '\0')) {
                name.pop_back();
            }

            if (name.empty()) {
                name = "file_" + std::to_string(i);
            }

            if (entrySize == 0) {
                IntFileEntry entry;
                entry.name = name;
                entry.type = type;
                entry.offset = static_cast<uint32_t>(offset + HEADER_SIZE + dataCursor);
                entry.size = 0;
                if (archive.nameIndex.find(name) == archive.nameIndex.end()) {
                    archive.nameIndex[name] = archive.entries.size();
                }
                archive.entries.push_back(std::move(entry));
                continue;
            }

            IntFileEntry entry;
            entry.name = name;
            entry.type = type;
            entry.offset = static_cast<uint32_t>(offset + HEADER_SIZE + dataCursor);
            entry.size = entrySize;

            // Copy data
            if (dataCursor + entrySize > dataSize) {
                break;
            }
            entry.data.resize(entrySize);
            memcpy(entry.data.data(), blockData + dataCursor, entrySize);

            archive.nameIndex[name] = archive.entries.size();
            archive.entries.push_back(std::move(entry));

            dataCursor = align4(dataCursor + entrySize);
        }

        offset += HEADER_SIZE + dataSize;
    }

    return !archive.entries.empty();
}
