#include "pr_stage2_data_services.h"
#include <array>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace PrStage2DataServices {
namespace {
constexpr uint32_t HeaderBytes = 0x800u;
constexpr uint32_t LoadAddress = 0x80010000u;
constexpr uint32_t PayloadBytes = 0x5F000u;
constexpr uint32_t StartupAddress = 0x80028590u;

uint32_t Word(const std::vector<uint8_t>& data, size_t offset) {
    return uint32_t(data.at(offset)) | (uint32_t(data.at(offset + 1u)) << 8u) |
        (uint32_t(data.at(offset + 2u)) << 16u) | (uint32_t(data.at(offset + 3u)) << 24u);
}

std::vector<uint8_t> ReadScusData(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) throw std::runtime_error("Cannot open SCUS native data image: " + path.u8string());
    const auto length = file.tellg();
    if (length != std::streampos(HeaderBytes + PayloadBytes))
        throw std::runtime_error("SCUS native data image size mismatch");
    std::vector<uint8_t> data(HeaderBytes + PayloadBytes);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));
    if (!file || file.gcount() != static_cast<std::streamsize>(data.size()))
        throw std::runtime_error("Incomplete SCUS native data image read");
    if (file.peek() != std::char_traits<char>::eof() || file.bad())
        throw std::runtime_error("SCUS native data image changed during read");

    constexpr std::array<uint8_t, 8> magic{{'P','S','-','X',' ','E','X','E'}};
    for (size_t i = 0; i < magic.size(); ++i)
        if (data[i] != magic[i]) throw std::runtime_error("Invalid SCUS native data image signature");
    // This HAL supports the source layout used by this direct port, not an
    // arbitrary PS-X executable loader. Header stack/GP values are validated
    // but never applied to the native host's registers or stack.
    if (Word(data, 0x10u) != StartupAddress || Word(data, 0x14u) != 0u ||
        Word(data, 0x18u) != LoadAddress || Word(data, 0x1Cu) != PayloadBytes ||
        Word(data, 0x20u) != 0u || Word(data, 0x24u) != 0u ||
        Word(data, 0x28u) != 0u || Word(data, 0x2Cu) != 0u ||
        Word(data, 0x30u) != 0x801FFFF0u || Word(data, 0x34u) != 0u)
        throw std::runtime_error("Unsupported SCUS native data layout");
    // Layout guard only; these words are never interpreted by the product.
    // The translated clear loop below must agree with the supplied revision.
    constexpr std::array<uint32_t, 9> prologue{{
        0x3C028007u, 0x2442ECB8u, 0x3C03801Cu, 0x24633870u,
        0xAC400000u, 0x24420004u, 0x0043082Bu, 0x1420FFFCu, 0u}};
    for (size_t i = 0; i < prologue.size(); ++i)
        if (Word(data, HeaderBytes + StartupAddress - LoadAddress + i * 4u) != prologue[i])
            throw std::runtime_error("SCUS startup clear layout differs from its native translation");
    return data;
}

std::vector<uint8_t> ReadNativeDataPack(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file || file.tellg() != std::streampos(PayloadBytes))
        throw std::runtime_error("S2 native data pack size mismatch: " + path.u8string());
    std::vector<uint8_t> data(PayloadBytes);
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));
    if (!file || file.gcount() != static_cast<std::streamsize>(data.size()) ||
        file.peek() != std::char_traits<char>::eof() || file.bad())
        throw std::runtime_error("Incomplete S2 native data pack read: " + path.u8string());
    return data;
}

std::filesystem::path ResolveDataSource(const std::filesystem::path& requested) {
    if (std::filesystem::is_regular_file(requested)) return requested;
    const std::array<std::filesystem::path, 3> candidates{{
        requested.parent_path() / "S2" / "S2_NATIVE_DATA.BIN",
        requested.parent_path() / "S2_NATIVE_DATA.BIN",
        std::filesystem::current_path() / "S2" / "S2_NATIVE_DATA.BIN"}};
    for (const auto& candidate : candidates)
        if (std::filesystem::is_regular_file(candidate)) return candidate;
    throw std::runtime_error("Neither SCUS nor the S2 native data pack is available");
}
}

void ClearScusBss80028590(PrStage2LifecycleDirect::Services& services) {
    uint32_t address = 0x8006ECB8u;
    do {
        services.Write32(address, 0u);
        address += 4u;
    } while (address < 0x801C3870u);
}

Services::Services(const std::filesystem::path& scusPath) {
    // Read and validate everything before touching this fresh RAM owner.
    // Public word writes preserve the existing private-source-slot semantics.
    const auto source = ResolveDataSource(scusPath);
    if (source.filename() == "S2_NATIVE_DATA.BIN") {
        const auto data = ReadNativeDataPack(source);
        for (uint32_t offset = 0u; offset < PayloadBytes; offset += 4u)
            Write32(LoadAddress + offset, Word(data, offset));
    } else {
        const auto data = ReadScusData(source);
        for (uint32_t offset = 0u; offset < PayloadBytes; offset += 4u)
            Write32(LoadAddress + offset, Word(data, HeaderBytes + offset));
    }
    ClearScusBss80028590(*this);
}
}
