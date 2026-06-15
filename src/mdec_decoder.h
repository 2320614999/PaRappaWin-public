#pragma once

#include <cstdint>
#include <vector>
#include <array>

// MDEC (Motion Decoder) - PSX video decoder
// Based on DuckStation's MDEC implementation (stenzek/duckstation)
// Implements: RLE decoding, IDCT, YCbCr to RGB conversion

class MdecDecoder {
public:
    MdecDecoder();
    ~MdecDecoder();

    // Initialize decoder for given dimensions
    bool Initialize(uint16_t width, uint16_t height);
    void Shutdown();

    // Decode a frame from MDEC bitstream
    // Returns RGBA pixel data (width * height * 4 bytes)
    // quantScale and version come from STR sector header
    bool DecodeFrame(const uint8_t* bitstream, size_t bitstreamSize,
                     uint32_t frameNo, uint16_t quantScale, uint16_t version);

    // Get decoded frame data
    const std::vector<uint32_t>& GetPixels() const { return m_pixels; }
    uint16_t GetWidth() const { return m_width; }
    uint16_t GetHeight() const { return m_height; }

    // Statistics
    uint32_t GetDecodedFrameCount() const { return m_decodedFrameCount; }

private:
    struct VlcStats {
        uint32_t blocks = 0;
        uint32_t eob = 0;
        uint32_t escape = 0;
        uint32_t unknownSkip = 0;
        uint32_t acCodes = 0;
    };

    // STR frame header parsing
    struct StrFrameHeader {
        uint16_t mdecCodeCount;  // Number of 32-byte blocks
        uint16_t magic;          // Should be 0x3800
        uint16_t qscale;         // Quantization scale
        uint16_t version;        // Frame version (2 or 3)
    };

    // Bit reader for VLC decoding
    class BitReader {
    public:
        BitReader(const uint8_t* data, size_t size);
        BitReader(const uint8_t* data, size_t size, bool msbFirst);
        uint32_t PeekBits(int n);
        uint32_t ReadBits(int n);
        int32_t ReadSignedBits(int n);
        void SkipBits(int n);
        bool HasBits(int n) const;
        size_t GetBitPos() const { return m_bitPos; }
    private:
        const uint8_t* m_data;
        size_t m_size;
        size_t m_bitPos = 0;
        bool m_msbFirst = true;
    };

    // Core decoding functions
    bool ParseFrameHeader(const uint8_t* data, size_t size, StrFrameHeader& header);
    bool DecodeFrame_v2v3(const uint8_t* data, size_t size, const StrFrameHeader& header);
    bool DecodeBlock_VLC(BitReader& bits, int16_t* blk, int qscale, bool isV3, bool isChroma, int& prevDC);
    void IDCT(int16_t* blk);
    void YUVToRGB(uint32_t mbX, uint32_t mbY,
                  const std::array<int16_t, 64>& Crblk,
                  const std::array<int16_t, 64>& Cbblk,
                  const std::array<int16_t, 64>& Yblk,
                  uint32_t blockIdx);

    // Quantization tables
    std::array<uint8_t, 64> m_iqY;   // Luminance
    std::array<uint8_t, 64> m_iqUV;  // Chrominance

    // Scale table for IDCT
    std::array<int16_t, 64> m_scaleTable;

    // Working buffers
    std::array<std::array<int16_t, 64>, 6> m_blocks;  // Cr, Cb, Y0, Y1, Y2, Y3
    std::array<uint32_t, 256> m_blockRGB;  // 16x16 RGB output

    std::vector<uint32_t> m_pixels;  // RGBA output
    uint16_t m_width = 0;
    uint16_t m_height = 0;
    bool m_initialized = false;
    uint32_t m_decodedFrameCount = 0;
    bool m_bitOrderDetermined = false;
    bool m_useMsbFirst = true;

    bool m_collectVlcStats = false;
    uint32_t m_vlcStatsFrameNo = 0;
    VlcStats m_vlcStats;
};
