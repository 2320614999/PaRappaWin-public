#pragma once

#include <cstdint>
#include <vector>

class XaDecoder {
public:
    XaDecoder();
    ~XaDecoder();

    void Reset();

    bool DecodeSector(const uint8_t* data, size_t size, uint8_t coding);

    const std::vector<int16_t>& GetSamples() const { return m_samples; }
    void ClearSamples() { m_samples.clear(); }

    uint32_t GetSampleRate() const { return m_sampleRate; }
    bool IsStereo() const { return m_stereo; }

private:
    void DecodeBlock(const uint8_t* src, int16_t* dst, int blkIdx, bool isRight);

    std::vector<int16_t> m_samples;

    int32_t m_prevL[2] = {0, 0};
    int32_t m_prevR[2] = {0, 0};

    uint32_t m_sampleRate = 37800;
    bool m_stereo = true;
};
