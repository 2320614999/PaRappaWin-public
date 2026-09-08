#include "pr_ss0_mdec_output_direct.h"

#include <algorithm>
#include <array>
#include <limits>

namespace PrSS0MdecOutputDirect {
namespace {

// 0x8005D858..0x8005D898. The UV table at 0x8005D898 is byte-identical.
static constexpr std::array<uint8_t, 64> kCurrentScusQuantTable = {{
    0x02u, 0x10u, 0x10u, 0x13u, 0x10u, 0x13u, 0x16u, 0x16u,
    0x16u, 0x16u, 0x16u, 0x16u, 0x1Au, 0x18u, 0x1Au, 0x1Bu,
    0x1Bu, 0x1Bu, 0x1Au, 0x1Au, 0x1Au, 0x1Au, 0x1Bu, 0x1Bu,
    0x1Bu, 0x1Du, 0x1Du, 0x1Du, 0x22u, 0x22u, 0x22u, 0x1Du,
    0x1Du, 0x1Du, 0x1Bu, 0x1Bu, 0x1Du, 0x1Du, 0x20u, 0x20u,
    0x22u, 0x22u, 0x25u, 0x26u, 0x25u, 0x23u, 0x23u, 0x22u,
    0x23u, 0x26u, 0x26u, 0x28u, 0x28u, 0x28u, 0x30u, 0x30u,
    0x2Eu, 0x2Eu, 0x38u, 0x38u, 0x3Au, 0x45u, 0x45u, 0x53u,
}};

// 0x8005D8DC..0x8005D95C, signed little-endian halfwords.
static constexpr std::array<int16_t, 64> kCurrentScusScaleTable = {{
    23170, 23170, 23170, 23170, 23170, 23170, 23170, 23170,
    32138, 27245, 18204,  6392, -6393,-18205,-27246,-32139,
    30273, 12539,-12540,-30274,-30274,-12540, 12539, 30273,
    27245, -6393,-32139,-18205, 18204, 32138,  6392,-27246,
    23170,-23171,-23171, 23170, 23170,-23171,-23171, 23170,
    18204,-32139,  6392, 27245,-27246, -6393, 32138,-18205,
    12539,-30274, 30273,-12540,-12540, 30273,-30274, 12539,
     6392,-18205, 27245,-32139, 32138,-27246, 18204, -6393,
}};

// Reverse of the PSX MDEC zigzag matrix: scan index -> raster index.
static constexpr std::array<uint8_t, 64> kZagZig = {{
     0,  1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63,
}};

static uint16_t ReadLe16(const uint8_t* bytes)
{
    return static_cast<uint16_t>(bytes[0]) |
           static_cast<uint16_t>(static_cast<uint16_t>(bytes[1]) << 8u);
}

static uint32_t ReadLe32(const uint8_t* bytes)
{
    return static_cast<uint32_t>(bytes[0]) |
           (static_cast<uint32_t>(bytes[1]) << 8u) |
           (static_cast<uint32_t>(bytes[2]) << 16u) |
           (static_cast<uint32_t>(bytes[3]) << 24u);
}

static void WriteLe16(uint8_t* bytes, uint16_t value)
{
    bytes[0] = static_cast<uint8_t>(value & 0xFFu);
    bytes[1] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
}

static int32_t Signed10(uint16_t value)
{
    const uint32_t raw = value & 0x03FFu;
    return (raw & 0x0200u) != 0u
        ? static_cast<int32_t>(raw) - 0x0400
        : static_cast<int32_t>(raw);
}

static int32_t ClampCoefficient(int64_t value)
{
    return static_cast<int32_t>(std::max<int64_t>(
        -1024, std::min<int64_t>(1023, value)));
}

static int32_t RoundIdctPass(int64_t sum)
{
    // The public PSX MDEC description identifies the scale-table matrix and
    // two-pass shape but explicitly leaves hardware rounding imperfectly
    // known. This deterministic rounding is therefore not claimed bit-exact.
    if (sum >= 0) {
        return static_cast<int32_t>((sum + 0x7FFFll) / 0x10000ll);
    }
    return -static_cast<int32_t>((-sum + 0x8000ll) / 0x10000ll);
}

static void Idct(const std::array<int32_t, 64>& coefficients,
                 std::array<int32_t, 64>& samples)
{
    std::array<int32_t, 64> first{};
    for (uint32_t y = 0; y < 8u; ++y) {
        for (uint32_t x = 0; x < 8u; ++x) {
            int64_t sum = 0;
            for (uint32_t z = 0; z < 8u; ++z) {
                sum += static_cast<int64_t>(coefficients[y + z * 8u]) *
                       kCurrentScusScaleTable[x + z * 8u];
            }
            first[x + y * 8u] = RoundIdctPass(sum);
        }
    }
    for (uint32_t y = 0; y < 8u; ++y) {
        for (uint32_t x = 0; x < 8u; ++x) {
            int64_t sum = 0;
            for (uint32_t z = 0; z < 8u; ++z) {
                sum += static_cast<int64_t>(first[y + z * 8u]) *
                       kCurrentScusScaleTable[x + z * 8u];
            }
            samples[x + y * 8u] = RoundIdctPass(sum);
        }
    }
}

class RleReader {
public:
    RleReader(const uint8_t* bytes, size_t halfwords)
        : bytes_(bytes), halfwords_(halfwords)
    {
    }

    bool Read(uint16_t& value)
    {
        if (offset_ >= halfwords_) {
            return false;
        }
        value = ReadLe16(bytes_ + offset_ * sizeof(uint16_t));
        ++offset_;
        return true;
    }

    uint32_t consumed() const
    {
        return static_cast<uint32_t>(offset_);
    }

private:
    const uint8_t* bytes_ = nullptr;
    size_t halfwords_ = 0;
    size_t offset_ = 0;
};

static bool DecodeBlock(RleReader& reader,
                        std::array<int32_t, 64>& samples)
{
    std::array<int32_t, 64> coefficients{};
    uint16_t word = 0;
    do {
        if (!reader.Read(word)) {
            return false;
        }
    } while (word == 0xFE00u);

    const uint32_t qScale = (word >> 10u) & 0x3Fu;
    uint32_t k = 0u;
    int64_t value = qScale == 0u
        ? static_cast<int64_t>(Signed10(word)) * 2ll
        : static_cast<int64_t>(Signed10(word)) *
              kCurrentScusQuantTable[0];
    coefficients[qScale == 0u ? 0u : kZagZig[0]] =
        ClampCoefficient(value);

    for (;;) {
        if (!reader.Read(word)) {
            return false;
        }
        if (word == 0xFE00u) {
            break;
        }
        k += ((word >> 10u) & 0x3Fu) + 1u;
        if (k > 63u) {
            return false;
        }
        if (qScale == 0u) {
            value = static_cast<int64_t>(Signed10(word)) * 2ll;
            coefficients[k] = ClampCoefficient(value);
        } else {
            value = (static_cast<int64_t>(Signed10(word)) *
                     kCurrentScusQuantTable[k] * qScale + 4ll) / 8ll;
            coefficients[kZagZig[k]] = ClampCoefficient(value);
        }
    }

    Idct(coefficients, samples);
    return true;
}

static int32_t ClampSigned8(int32_t value)
{
    return std::max(-128, std::min(127, value));
}

static int32_t ScaleChroma(int32_t value, int32_t numerator)
{
    const int64_t product = static_cast<int64_t>(value) * numerator;
    return product >= 0
        ? static_cast<int32_t>(product / 65536ll)
        : -static_cast<int32_t>((-product) / 65536ll);
}

static uint16_t Pack15bpp(int32_t y, int32_t cb, int32_t cr)
{
    // Fixed-point forms of the documented 1.402, 0.3437, 0.7143 and
    // 1.772 conversion coefficients. Exact hardware rounding is a non-claim.
    const int32_t red = ClampSigned8(y + ScaleChroma(cr, 91881)) + 128;
    const int32_t green = ClampSigned8(
        y - ScaleChroma(cb, 22525) - ScaleChroma(cr, 46812)) + 128;
    const int32_t blue = ClampSigned8(y + ScaleChroma(cb, 116130)) + 128;
    return static_cast<uint16_t>(
        0x8000u |
        (static_cast<uint32_t>(red) >> 3u) |
        ((static_cast<uint32_t>(green) >> 3u) << 5u) |
        ((static_cast<uint32_t>(blue) >> 3u) << 10u));
}

static void StoreLumaBlock(
    const std::array<int32_t, 64>& cr,
    const std::array<int32_t, 64>& cb,
    const std::array<int32_t, 64>& yBlock,
    uint32_t xBase,
    uint32_t yBase,
    uint8_t* macroblock)
{
    for (uint32_t y = 0; y < 8u; ++y) {
        for (uint32_t x = 0; x < 8u; ++x) {
            const uint32_t outX = xBase + x;
            const uint32_t outY = yBase + y;
            const uint32_t chromaIndex =
                (outX >> 1u) + (outY >> 1u) * 8u;
            const uint16_t pixel = Pack15bpp(
                yBlock[x + y * 8u], cb[chromaIndex], cr[chromaIndex]);
            WriteLe16(
                macroblock + (outX + outY * 16u) * sizeof(uint16_t),
                pixel);
        }
    }
}

static uint64_t Fnv1a64(const uint8_t* bytes, size_t size)
{
    uint64_t hash = 1469598103934665603ull;
    for (size_t index = 0; index < size; ++index) {
        hash ^= bytes[index];
        hash *= 1099511628211ull;
    }
    return hash;
}

} // namespace

Mdec15bppResult ExecuteMdec15bppCurrentScus(
    const uint8_t* commandAndRle,
    size_t inputBytes,
    uint16_t width,
    uint16_t height,
    uint8_t* output,
    size_t outputBytes)
{
    Mdec15bppResult result{};
    if (commandAndRle == nullptr || output == nullptr ||
        inputBytes < sizeof(uint32_t) || width == 0u || height == 0u) {
        return result;
    }

    const uint32_t command = ReadLe32(commandAndRle);
    const uint32_t commandKind = command >> 29u;
    const uint32_t depth = (command >> 27u) & 3u;
    const bool signedOutput = (command & 0x04000000u) != 0u;
    const bool bit15 = (command & 0x02000000u) != 0u;
    const uint32_t inputWords = command & 0xFFFFu;
    const uint64_t commandBytes = sizeof(uint32_t) +
        static_cast<uint64_t>(inputWords) * sizeof(uint32_t);
    const uint32_t macroblockColumns =
        (static_cast<uint32_t>(width) + 15u) >> 4u;
    const uint32_t macroblockRows =
        (static_cast<uint32_t>(height) + 15u) >> 4u;
    const uint64_t macroblockCount64 =
        static_cast<uint64_t>(macroblockColumns) * macroblockRows;
    const uint64_t requiredOutput64 = macroblockCount64 * 512u;
    if (commandKind != 1u || depth != 3u || signedOutput || !bit15 ||
        inputWords == 0u || commandBytes > inputBytes ||
        macroblockCount64 == 0u ||
        macroblockCount64 > (std::numeric_limits<uint32_t>::max)() ||
        requiredOutput64 > outputBytes ||
        requiredOutput64 > (std::numeric_limits<uint32_t>::max)()) {
        return result;
    }

    RleReader reader(
        commandAndRle + sizeof(uint32_t),
        static_cast<size_t>(inputWords) * 2u);
    std::array<int32_t, 64> cr{};
    std::array<int32_t, 64> cb{};
    std::array<int32_t, 64> y1{};
    std::array<int32_t, 64> y2{};
    std::array<int32_t, 64> y3{};
    std::array<int32_t, 64> y4{};
    for (uint32_t macroblock = 0u;
         macroblock < static_cast<uint32_t>(macroblockCount64);
         ++macroblock) {
        if (!DecodeBlock(reader, cr) || !DecodeBlock(reader, cb) ||
            !DecodeBlock(reader, y1) || !DecodeBlock(reader, y2) ||
            !DecodeBlock(reader, y3) || !DecodeBlock(reader, y4)) {
            return Mdec15bppResult{};
        }
        uint8_t* destination = output +
            static_cast<size_t>(macroblock) * 512u;
        StoreLumaBlock(cr, cb, y1, 0u, 0u, destination);
        StoreLumaBlock(cr, cb, y2, 8u, 0u, destination);
        StoreLumaBlock(cr, cb, y3, 0u, 8u, destination);
        StoreLumaBlock(cr, cb, y4, 8u, 8u, destination);
    }

    result.commandWord = command;
    result.inputWordCount = inputWords;
    result.inputHalfwordsConsumed = reader.consumed();
    result.width = width;
    result.height = height;
    result.macroblockColumns = macroblockColumns;
    result.macroblockRows = macroblockRows;
    result.macroblocksDecoded = static_cast<uint32_t>(macroblockCount64);
    result.outputBytesWritten = static_cast<uint32_t>(requiredOutput64);
    result.outputFnv1a64 = Fnv1a64(output, result.outputBytesWritten);
    result.output15bpp = true;
    result.outputUnsigned = true;
    result.outputBit15Set = true;
    result.dma1VerticalMacroblockLayout = true;
    result.currentScusQuantTableAuthority = true;
    result.currentScusScaleTableAuthority = true;
    result.currentIdaCommandAuthority = true;
    result.documentedPsxMdecAlgorithmAuthority = true;
    result.mdecHardwareBitExactAuthority = false;
    result.dmaCompletionCallbackTimingAuthority = false;
    result.psxHardwareMmioAuthority = false;
    result.hostProjection = false;
    result.replayValueAuthority = false;
    result.oldWinS0Authority = false;
    result.stage2PlusAuthority = false;
    result.comod2Authority = false;
    result.known = true;
    result.executed = true;
    return result;
}

} // namespace PrSS0MdecOutputDirect
