// MDEC Decoder - Based on jpsxdec STR format documentation
// Implements VLC decoding for STR v2/v3 format
// Reference: https://github.com/m35/jpsxdec/blob/readme/jpsxdec/PlayStation1_STR_format.txt

#include "mdec_decoder.h"
#include "logger.h"
#include <cstring>
#include <algorithm>
#include <cmath>

// Zigzag scan order for IDCT (MPEG1/JPEG standard)
static constexpr uint8_t s_zigzag[64] = {
     0,  1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

// PSX default quantization table
static constexpr uint8_t s_defaultQuantTable[64] = {
     2, 16, 19, 22, 26, 27, 29, 34,
    16, 16, 22, 24, 27, 29, 34, 37,
    19, 22, 26, 27, 29, 34, 34, 38,
    22, 22, 26, 27, 29, 34, 37, 40,
    22, 26, 27, 29, 32, 35, 40, 48,
    26, 27, 29, 32, 35, 40, 48, 58,
    26, 27, 29, 34, 38, 46, 56, 69,
    27, 29, 35, 38, 46, 56, 69, 83
};

// IDCT scale matrix (based on DuckStation)
static constexpr int16_t s_defaultScaleTable[64] = {
    23170, 23170, 23170, 23170, 23170, 23170, 23170, 23170,
    32138, 27246, 18205,  6393, -6393,-18205,-27246,-32138,
    30274, 12540,-12540,-30274,-30274,-12540, 12540, 30274,
    27246, -6393,-32138,-18205, 18205, 32138,  6393,-27246,
    23170,-23170,-23170, 23170, 23170,-23170,-23170, 23170,
    18205,-32138,  6393, 27246,-27246, -6393, 32138,-18205,
    12540,-30274, 30274,-12540,-12540, 30274,-30274, 12540,
     6393,-18205, 27246,-32138, 32138,-27246, 18205, -6393
};

// VLC AC coefficient table (simplified - escape code fallback)
// Format: { code_bits, code_length, run, level }
struct VlcEntry {
    uint16_t code;
    uint8_t len;
    uint8_t run;
    int16_t level;
};

// Common VLC codes (subset - uses escape for others)
static const VlcEntry s_vlcTable[] = {
    // EOB
    {0b10, 2, 0xFF, 0},           // End of block
    {0x3, 2, 0, 1},
    {0x3, 3, 1, 1},
    {0x4, 4, 0, 2},
    {0x5, 4, 2, 1},
    {0x5, 5, 0, 3},
    {0x6, 5, 4, 1},
    {0x7, 5, 3, 1},
    {0x4, 6, 7, 1},
    {0x5, 6, 6, 1},
    {0x6, 6, 1, 2},
    {0x7, 6, 5, 1},
    {0x4, 7, 2, 2},
    {0x5, 7, 9, 1},
    {0x6, 7, 0, 4},
    {0x7, 7, 8, 1},
    {0x20, 8, 13, 1},
    {0x21, 8, 0, 6},
    {0x22, 8, 12, 1},
    {0x23, 8, 11, 1},
    {0x24, 8, 3, 2},
    {0x25, 8, 1, 3},
    {0x26, 8, 0, 5},
    {0x27, 8, 10, 1},
    {0x8, 10, 16, 1},
    {0x9, 10, 5, 2},
    {0xA, 10, 0, 7},
    {0xB, 10, 2, 3},
    {0xC, 10, 1, 4},
    {0xD, 10, 15, 1},
    {0xE, 10, 14, 1},
    {0xF, 10, 4, 2},
    {0x10, 12, 0, 11},
    {0x11, 12, 8, 2},
    {0x12, 12, 4, 3},
    {0x13, 12, 0, 10},
    {0x14, 12, 2, 4},
    {0x15, 12, 7, 2},
    {0x16, 12, 21, 1},
    {0x17, 12, 20, 1},
    {0x18, 12, 0, 9},
    {0x19, 12, 19, 1},
    {0x1A, 12, 18, 1},
    {0x1B, 12, 1, 5},
    {0x1C, 12, 3, 3},
    {0x1D, 12, 0, 8},
    {0x1E, 12, 6, 2},
    {0x1F, 12, 17, 1},
    {0x10, 13, 10, 2},
    {0x11, 13, 9, 2},
    {0x12, 13, 5, 3},
    {0x13, 13, 3, 4},
    {0x14, 13, 2, 5},
    {0x15, 13, 1, 7},
    {0x16, 13, 1, 6},
    {0x17, 13, 0, 15},
    {0x18, 13, 0, 14},
    {0x19, 13, 0, 13},
    {0x1A, 13, 0, 12},
    {0x1B, 13, 26, 1},
    {0x1C, 13, 25, 1},
    {0x1D, 13, 24, 1},
    {0x1E, 13, 23, 1},
    {0x1F, 13, 22, 1},
    {0x10, 14, 0, 31},
    {0x11, 14, 0, 30},
    {0x12, 14, 0, 29},
    {0x13, 14, 0, 28},
    {0x14, 14, 0, 27},
    {0x15, 14, 0, 26},
    {0x16, 14, 0, 25},
    {0x17, 14, 0, 24},
    {0x18, 14, 0, 23},
    {0x19, 14, 0, 22},
    {0x1A, 14, 0, 21},
    {0x1B, 14, 0, 20},
    {0x1C, 14, 0, 19},
    {0x1D, 14, 0, 18},
    {0x1E, 14, 0, 17},
    {0x1F, 14, 0, 16},
    {0x10, 15, 0, 40},
    {0x11, 15, 0, 39},
    {0x12, 15, 0, 38},
    {0x13, 15, 0, 37},
    {0x14, 15, 0, 36},
    {0x15, 15, 0, 35},
    {0x16, 15, 0, 34},
    {0x17, 15, 0, 33},
    {0x18, 15, 0, 32},
    {0x19, 15, 1, 14},
    {0x1A, 15, 1, 13},
    {0x1B, 15, 1, 12},
    {0x1C, 15, 1, 11},
    {0x1D, 15, 1, 10},
    {0x1E, 15, 1, 9},
    {0x1F, 15, 1, 8},
    {0x10, 16, 1, 18},
    {0x11, 16, 1, 17},
    {0x12, 16, 1, 16},
    {0x13, 16, 1, 15},
    {0x14, 16, 6, 3},
    {0x15, 16, 16, 2},
    {0x16, 16, 15, 2},
    {0x17, 16, 14, 2},
    {0x18, 16, 13, 2},
    {0x19, 16, 12, 2},
    {0x1A, 16, 11, 2},
    {0x1B, 16, 31, 1},
    {0x1C, 16, 30, 1},
    {0x1D, 16, 29, 1},
    {0x1E, 16, 28, 1},
    {0x1F, 16, 27, 1},
};
static constexpr size_t s_vlcTableSize = sizeof(s_vlcTable) / sizeof(s_vlcTable[0]);

// Sign extend helper
template<int bits, typename T>
static inline T SignExtend(T value) {
    constexpr T mask = T(1) << (bits - 1);
    return (value ^ mask) - mask;
}

// BitReader implementation
MdecDecoder::BitReader::BitReader(const uint8_t* data, size_t size)
    : m_data(data), m_size(size), m_bitPos(0), m_msbFirst(true) {}

MdecDecoder::BitReader::BitReader(const uint8_t* data, size_t size, bool msbFirst)
    : m_data(data), m_size(size), m_bitPos(0), m_msbFirst(msbFirst) {}

uint32_t MdecDecoder::BitReader::PeekBits(int n) {
    if (n <= 0 || n > 24) return 0;

    uint32_t result = 0;
    size_t bitIdx = m_bitPos;

    for (int i = 0; i < n; i++) {
        const size_t wordIdx = bitIdx / 16;
        const size_t byteIdx = wordIdx * 2;
        const int bitInWord = m_msbFirst ? (15 - int(bitIdx % 16)) : int(bitIdx % 16);

        if (byteIdx + 1 >= m_size) break;

        const uint16_t w = uint16_t(m_data[byteIdx]) | (uint16_t(m_data[byteIdx + 1]) << 8);
        const uint8_t bit = uint8_t((w >> bitInWord) & 1);
        result = (result << 1) | bit;
        bitIdx++;
    }

    return result;
}

uint32_t MdecDecoder::BitReader::ReadBits(int n) {
    uint32_t result = PeekBits(n);
    m_bitPos += n;
    return result;
}

int32_t MdecDecoder::BitReader::ReadSignedBits(int n) {
    uint32_t val = ReadBits(n);
    // Sign extend
    if (val & (1u << (n - 1))) {
        val |= ~((1u << n) - 1);
    }
    return (int32_t)val;
}

void MdecDecoder::BitReader::SkipBits(int n) {
    m_bitPos += n;
}

bool MdecDecoder::BitReader::HasBits(int n) const {
    return (m_bitPos + n) <= (m_size * 8);
}

MdecDecoder::MdecDecoder() = default;
MdecDecoder::~MdecDecoder() { Shutdown(); }

bool MdecDecoder::Initialize(uint16_t width, uint16_t height) {
    if (width == 0 || height == 0) {
        Log::Printf("MdecDecoder: Invalid dimensions %ux%u", width, height);
        return false;
    }

    m_width = width;
    m_height = height;
    m_pixels.resize((size_t)width * height);
    m_initialized = true;
    m_decodedFrameCount = 0;
    m_bitOrderDetermined = false;
    m_useMsbFirst = true;

    // Initialize quantization tables
    for (size_t i = 0; i < 64; i++) {
        const size_t raster = s_zigzag[i];
        m_iqY[i] = s_defaultQuantTable[raster];
        m_iqUV[i] = s_defaultQuantTable[raster];
    }

    // Initialize scale table
    std::copy(std::begin(s_defaultScaleTable), std::end(s_defaultScaleTable), m_scaleTable.begin());

    Log::Printf("MdecDecoder: Initialized %ux%u (real MDEC)", width, height);
    return true;
}

void MdecDecoder::Shutdown() {
    m_pixels.clear();
    m_width = 0;
    m_height = 0;
    m_initialized = false;
}

bool MdecDecoder::DecodeFrame(const uint8_t* bitstream, size_t bitstreamSize,
                               uint32_t frameNo, uint16_t quantScale, uint16_t version) {
    if (!m_initialized) {
        Log::Printf("MdecDecoder: Not initialized");
        return false;
    }

    // Clear output to black
    std::fill(m_pixels.begin(), m_pixels.end(), 0xFF000000u);

    m_collectVlcStats = true;
    m_vlcStatsFrameNo = frameNo;
    m_vlcStats = {};

    StrFrameHeader header;
    const uint8_t* mdecData = bitstream;
    size_t mdecSize = bitstreamSize;
    if (ParseFrameHeader(bitstream, bitstreamSize, header)) {
        if (mdecSize > 8) {
            mdecData = bitstream + 8;
            mdecSize = bitstreamSize - 8;
        }
    } else {
        // Fallback: construct header from parameters (from STR sector header)
        header.mdecCodeCount = 0;  // Not used
        header.magic = 0x3800;
        header.qscale = quantScale;
        header.version = version;
    }

    // Log first frame info
    if (m_decodedFrameCount == 0) {
        Log::Printf("MdecDecoder: First frame - qscale=%u version=%u", quantScale, version);
    }

    if (!DecodeFrame_v2v3(mdecData, mdecSize, header)) {
        // Partial decode is OK
    }

    m_collectVlcStats = false;
    if (m_vlcStats.unknownSkip != 0 || frameNo <= 3) {
        Log::Printf("MdecDecoder: VLC stats frame=%u blocks=%u eob=%u escape=%u unknownSkip=%u acCodes=%u",
                    m_vlcStatsFrameNo,
                    m_vlcStats.blocks,
                    m_vlcStats.eob,
                    m_vlcStats.escape,
                    m_vlcStats.unknownSkip,
                    m_vlcStats.acCodes);
    }

    m_decodedFrameCount++;
    return true;
}

bool MdecDecoder::ParseFrameHeader(const uint8_t* data, size_t size, StrFrameHeader& header) {
    if (size < 8) return false;

    // Read as little-endian
    header.mdecCodeCount = data[0] | (data[1] << 8);
    header.magic = data[2] | (data[3] << 8);
    header.qscale = data[4] | (data[5] << 8);
    header.version = data[6] | (data[7] << 8);

    // Log first frame header
    if (m_decodedFrameCount == 0) {
        Log::Printf("MdecDecoder: Frame header - codes=%u magic=0x%04X qscale=%u version=%u",
                    header.mdecCodeCount, header.magic, header.qscale, header.version);
    }

    return true;
}

bool MdecDecoder::DecodeFrame_v2v3(const uint8_t* data, size_t size, const StrFrameHeader& header) {
    auto probeBitOrder = [&](bool msbFirst) -> int64_t {
        BitReader probeBits(data, size, msbFirst);
        const uint32_t mbWidth = (m_width + 15) / 16;
        const uint32_t mbHeight = (m_height + 15) / 16;
        const uint32_t mbTotal = mbWidth * mbHeight;
        const uint32_t mbProbe = (mbTotal < 8) ? mbTotal : 8;
        const bool isV3 = (header.version == 3);

        int prevCrDC = 0;
        int prevCbDC = 0;
        int prevYDC = 0;
        int64_t score = 0;

        uint32_t seen = 0;
        for (uint32_t mbX = 0; mbX < mbWidth && seen < mbProbe; mbX++) {
            for (uint32_t mbY = 0; mbY < mbHeight && seen < mbProbe; mbY++) {
                for (int block = 0; block < 6; block++) {
                    std::array<int16_t, 64> tmp;
                    std::fill(tmp.begin(), tmp.end(), int16_t(0));

                    const bool isChroma = (block < 2);
                    int& prevDC = isChroma ? (block == 0 ? prevCrDC : prevCbDC) : prevYDC;
                    if (!DecodeBlock_VLC(probeBits, tmp.data(), header.qscale, isV3, isChroma, prevDC)) {
                        return -1;
                    }

                    for (int i = 1; i < 64; i++) {
                        score += std::abs(int(tmp[i]));
                    }
                }
                seen++;
            }
        }
        return score;
    };

    if (!m_bitOrderDetermined) {
        if (header.version == 2) {
            m_useMsbFirst = true;
            m_bitOrderDetermined = true;
            Log::Printf("MdecDecoder: BitOrder forced -> msb (v2)");
        } else {
            const bool prevCollect = m_collectVlcStats;
            m_collectVlcStats = false;
            const int64_t scoreMsb = probeBitOrder(true);
            const int64_t scoreLsb = probeBitOrder(false);
            m_collectVlcStats = prevCollect;
            m_useMsbFirst = (scoreMsb >= scoreLsb);
            m_bitOrderDetermined = true;
            Log::Printf("MdecDecoder: BitOrder probe msb=%lld lsb=%lld -> %s", (long long)scoreMsb, (long long)scoreLsb,
                        m_useMsbFirst ? "msb" : "lsb");
        }
    }

    BitReader bits(data, size, m_useMsbFirst);

    // Calculate macroblock dimensions
    const uint32_t mbWidth = (m_width + 15) / 16;
    const uint32_t mbHeight = (m_height + 15) / 16;
    bool isV3 = (header.version == 3);
    int prevCrDC = 0;
    int prevCbDC = 0;
    int prevYDC = 0;

    // Decode each macroblock (column-major order)
    for (uint32_t mbX = 0; mbX < mbWidth; mbX++) {
        for (uint32_t mbY = 0; mbY < mbHeight; mbY++) {
            // Decode 6 blocks: Cr, Cb, Y0, Y1, Y2, Y3
            for (int block = 0; block < 6; block++) {
                std::fill(m_blocks[block].begin(), m_blocks[block].end(), int16_t(0));

                const bool isChroma = (block < 2);
                int& prevDC = isChroma ? (block == 0 ? prevCrDC : prevCbDC) : prevYDC;
                if (!DecodeBlock_VLC(bits, m_blocks[block].data(), header.qscale, isV3, isChroma, prevDC)) {
                    // End of data or error
                    return true;
                }

                // Apply IDCT
                IDCT(m_blocks[block].data());
            }

            // Convert YCbCr to RGB
            YUVToRGB(0, 0, m_blocks[0], m_blocks[1], m_blocks[2], 0);  // Y0: top-left
            YUVToRGB(8, 0, m_blocks[0], m_blocks[1], m_blocks[3], 1);  // Y1: top-right
            YUVToRGB(0, 8, m_blocks[0], m_blocks[1], m_blocks[4], 2);  // Y2: bottom-left
            YUVToRGB(8, 8, m_blocks[0], m_blocks[1], m_blocks[5], 3);  // Y3: bottom-right

            // Copy to output
            const uint32_t pixelX = mbX * 16;
            const uint32_t pixelY = mbY * 16;

            for (uint32_t y = 0; y < 16 && (pixelY + y) < m_height; y++) {
                for (uint32_t x = 0; x < 16 && (pixelX + x) < m_width; x++) {
                    const size_t srcIdx = y * 16 + x;
                    const size_t dstIdx = (pixelY + y) * m_width + (pixelX + x);
                    m_pixels[dstIdx] = m_blockRGB[srcIdx];
                }
            }
        }
    }

    return true;
}

bool MdecDecoder::DecodeBlock_VLC(BitReader& bits, int16_t* blk, int qscale, bool isV3, bool isChroma, int& prevDC) {
    // VLC decoding for STR v2/v3 format
    // Reference: jpsxdec PlayStation1_STR_format.txt

    if (m_collectVlcStats) {
        m_vlcStats.blocks++;
    }

    if (!bits.HasBits(isV3 ? 2 : 10))
        return false;

    auto decodeV3DcDiff = [&](bool chroma, int& outDiff) -> bool {
        struct DcCode {
            uint16_t code;
            uint8_t len;
            uint8_t nbits;
        };

        static constexpr DcCode chromaCodes[] = {
            {0xFE, 8, 8},
            {0x7E, 7, 7},
            {0x3E, 6, 6},
            {0x1E, 5, 5},
            {0x0E, 4, 4},
            {0x06, 3, 3},
            {0x02, 2, 2},
            {0x01, 2, 1},
            {0x00, 2, 0},
        };

        static constexpr DcCode lumaCodes[] = {
            {0x7E, 7, 8},
            {0x3E, 6, 7},
            {0x1E, 5, 6},
            {0x0E, 4, 5},
            {0x06, 3, 4},
            {0x05, 3, 3},
            {0x01, 2, 2},
            {0x00, 2, 1},
            {0x04, 3, 0},
        };

        const DcCode* codes = chroma ? chromaCodes : lumaCodes;
        const size_t count = chroma ? (sizeof(chromaCodes) / sizeof(chromaCodes[0])) : (sizeof(lumaCodes) / sizeof(lumaCodes[0]));

        for (size_t i = 0; i < count; i++) {
            const DcCode& c = codes[i];
            if (!bits.HasBits(c.len))
                return false;
            if (bits.PeekBits(c.len) != c.code)
                continue;

            bits.SkipBits(c.len);
            if (c.nbits == 0) {
                outDiff = 0;
                return true;
            }

            if (!bits.HasBits(c.nbits))
                return false;

            const uint32_t sign = bits.ReadBits(1);
            const uint32_t data = (c.nbits > 1) ? bits.ReadBits(c.nbits - 1) : 0;

            int diff;
            if (sign == 0)
                diff = int(data) - ((1 << c.nbits) - 1);
            else
                diff = int(data) + (1 << (c.nbits - 1));

            outDiff = diff * 4;
            return true;
        }

        return false;
    };

    const auto& qt = isChroma ? m_iqUV : m_iqY;

    // Read DC coefficient
    int32_t dc = 0;
    if (isV3) {
        int diff = 0;
        if (!decodeV3DcDiff(isChroma, diff))
            return false;
        dc = prevDC + diff;
        prevDC = int(dc);
    } else {
        dc = bits.ReadSignedBits(10);
    }

    // Dequantize DC (DC is not multiplied by qscale)
    int32_t dcCoeff;
    if (qscale == 0) {
        dcCoeff = dc * 2;
    } else {
        dcCoeff = dc * qt[0];
    }
    blk[0] = static_cast<int16_t>(std::clamp(dcCoeff, -1024, 1023));

    // Read AC coefficients using VLC
    int k = 0;  // Current coefficient index (0 = DC, 1-63 = AC)

    while (k < 63 && bits.HasBits(2)) {
        // Check for EOB (End of Block) - code "10"
        if (bits.PeekBits(2) == 0b10) {
            bits.SkipBits(2);
            if (m_collectVlcStats) {
                m_vlcStats.eob++;
            }
            break;
        }

        int run = 0;
        int level = 0;
        bool found = false;

        // Try to match VLC table entries
        for (size_t i = 1; i < s_vlcTableSize; i++) {  // Skip EOB entry
            const VlcEntry& entry = s_vlcTable[i];
            const int fullLen = int(entry.len) + 1;  // include sign bit
            if (bits.HasBits(fullLen)) {
                const uint32_t peek = bits.PeekBits(fullLen);
                if ((peek >> 1) == entry.code) {
                    const uint32_t sign = peek & 1;
                    bits.SkipBits(fullLen);
                    run = entry.run;
                    level = sign ? -entry.level : entry.level;
                    found = true;
                    break;
                }
            }
        }

        // If not found in table, check for escape code "000001"
        if (!found) {
            if (bits.HasBits(6) && bits.PeekBits(6) == 0b000001) {
                bits.SkipBits(6);
                // Read 6-bit run and 10-bit signed level
                if (bits.HasBits(16)) {
                    run = bits.ReadBits(6);
                    level = bits.ReadSignedBits(10);
                    found = true;
                    if (m_collectVlcStats) {
                        m_vlcStats.escape++;
                    }
                }
            }
        }

        if (!found) {
            // Unknown code - skip one bit and try again
            bits.SkipBits(1);
            if (m_collectVlcStats) {
                m_vlcStats.unknownSkip++;
            }
            continue;
        }

        if (m_collectVlcStats) {
            m_vlcStats.acCodes++;
        }

        // Apply run (skip zeros) and store level
        k += run + 1;
        if (k < 64) {
            const int idx = s_zigzag[k];
            int32_t coeff;
            if (qscale == 0) {
                coeff = level * 2;
            } else {
                coeff = (level * qt[k] * qscale + 4) / 8;
            }
            blk[idx] = static_cast<int16_t>(std::clamp(coeff, -1024, 1023));
        }
    }

    return true;
}

void MdecDecoder::IDCT(int16_t* blk) {
    // Two-pass IDCT (based on DuckStation's IDCT_Old)
    static float t[8][8];
    static bool tInit = false;
    if (!tInit) {
        const float pi = 3.14159265358979323846f;
        for (int u = 0; u < 8; u++) {
            const float Cu = (u == 0) ? 0.7071067811865476f : 1.0f;
            for (int x = 0; x < 8; x++) {
                t[u][x] = Cu * std::cos(((2.0f * float(x) + 1.0f) * float(u) * pi) / 16.0f);
            }
        }
        tInit = true;
    }

    float tmp[64];
    for (int v = 0; v < 8; v++) {
        for (int x = 0; x < 8; x++) {
            float sum = 0.0f;
            for (int u = 0; u < 8; u++) {
                sum += float(blk[v * 8 + u]) * t[u][x];
            }
            tmp[v * 8 + x] = sum;
        }
    }

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            float sum = 0.0f;
            for (int v = 0; v < 8; v++) {
                sum += tmp[v * 8 + x] * t[v][y];
            }

            sum *= 0.25f;
            const float rounded = sum + ((sum >= 0.0f) ? 0.5f : -0.5f);
            const int32_t result = (int32_t)rounded;
            blk[y * 8 + x] = static_cast<int16_t>(std::clamp(result, -128, 127));
        }
    }
}

void MdecDecoder::YUVToRGB(uint32_t xx, uint32_t yy,
                           const std::array<int16_t, 64>& Crblk,
                           const std::array<int16_t, 64>& Cbblk,
                           const std::array<int16_t, 64>& Yblk,
                           uint32_t blockIdx) {
    // BT.601 YCbCr to RGB conversion (based on DuckStation)
    // Note: PSX MDEC outputs unsigned values (0-255)
    const int16_t addval = 0x80;  // Convert from signed to unsigned

    for (uint32_t y = 0; y < 8; y++) {
        for (uint32_t x = 0; x < 8; x++) {
            // Chrominance is 2x2 subsampled
            const uint32_t crCbX = ((x + xx) / 2) % 8;
            const uint32_t crCbY = ((y + yy) / 2) % 8;
            const uint32_t crCbIdx = crCbX + crCbY * 8;

            int16_t Cr = Crblk[crCbIdx];
            int16_t Cb = Cbblk[crCbIdx];
            int16_t Y = Yblk[x + y * 8];

            // YCbCr to RGB conversion
            // R = Y + 1.402 * Cr
            // G = Y - 0.3437 * Cb - 0.7143 * Cr
            // B = Y + 1.772 * Cb
            int16_t R = static_cast<int16_t>(1.402f * float(Cr));
            int16_t B = static_cast<int16_t>(1.772f * float(Cb));
            int16_t G = static_cast<int16_t>((-0.3437f * float(Cb)) + (-0.7143f * float(Cr)));

            R = static_cast<int16_t>(std::clamp(int(Y) + R, -128, 127)) + addval;
            G = static_cast<int16_t>(std::clamp(int(Y) + G, -128, 127)) + addval;
            B = static_cast<int16_t>(std::clamp(int(Y) + B, -128, 127)) + addval;

            // Pack as ABGR (D3D11 format)
            const uint32_t outX = xx + x;
            const uint32_t outY = yy + y;
            m_blockRGB[outY * 16 + outX] =
                0xFF000000u |                        // Alpha
                (uint32_t(B & 0xFF) << 16) |         // Blue
                (uint32_t(G & 0xFF) << 8) |          // Green
                uint32_t(R & 0xFF);                  // Red
        }
    }
}
