#include "xa_decoder.h"
#include <algorithm>

static const int32_t kK0[4] = { 0, 240, 460, 392 };
static const int32_t kK1[4] = { 0,   0,-208,-220 };

XaDecoder::XaDecoder() {
    Reset();
}

XaDecoder::~XaDecoder() = default;

void XaDecoder::Reset() {
    m_samples.clear();
    m_prevL[0] = m_prevL[1] = 0;
    m_prevR[0] = m_prevR[1] = 0;
    m_sampleRate = 37800;
    m_stereo = true;
}

bool XaDecoder::DecodeSector(const uint8_t* data, size_t size, uint8_t coding) {
    if (!data || size < 2304) {
        return false;
    }

    const uint8_t monoStereo = coding & 0x03;
    m_stereo = (monoStereo == 0x01);

    const uint8_t rateCode = (coding >> 2) & 0x03;
    if (rateCode == 0x00) m_sampleRate = 37800;
    else if (rateCode == 0x01) m_sampleRate = 18900;
    else return false;

    const uint8_t bitsPerSample = (coding >> 4) & 0x03;
    if (bitsPerSample != 0x00) {
        return false;
    }

    m_samples.clear();
    const int soundGroups = 18;
    const int soundUnits = 8;
    const int samplesPerUnit = 28;
    if (m_stereo) {
        m_samples.reserve(soundGroups * 4 * samplesPerUnit * 2);
    } else {
        m_samples.reserve(soundGroups * soundUnits * samplesPerUnit);
    }

    auto paramIndex = [](int su) -> int {
        return (su < 4) ? su : (su + 4);
    };

    auto decodeNibble = [](int nibble, int shift, int filter, int32_t* prev) -> int16_t {
        if (shift < 0) shift = 0;
        if (shift > 12) shift = 12;
        if (filter < 0 || filter > 3) filter = 0;

        const int sn = (nibble >= 8) ? (nibble - 16) : nibble;
        int32_t s = (int32_t)sn << (12 - shift);
        s += (prev[0] * kK0[filter] + prev[1] * kK1[filter] + 128) >> 8;
        s = std::clamp(s, (int32_t)-32768, (int32_t)32767);
        prev[1] = prev[0];
        prev[0] = s;
        return (int16_t)s;
    };

    for (int sg = 0; sg < soundGroups; sg++) {
        const uint8_t* sgData = data + sg * 128;
        const uint8_t* params = sgData;
        const uint8_t* sampleBytes = sgData + 16;

        if (m_stereo) {
            for (int pair = 0; pair < 4; pair++) {
                const int suL = pair * 2;
                const int suR = suL + 1;
                const uint8_t spL = params[paramIndex(suL)];
                const uint8_t spR = params[paramIndex(suR)];
                const int shiftL = spL & 0x0F;
                const int shiftR = spR & 0x0F;
                const int filterL = (spL >> 4) & 0x03;
                const int filterR = (spR >> 4) & 0x03;

                for (int sd = 0; sd < samplesPerUnit; sd++) {
                    const uint8_t b = sampleBytes[sd * 4 + pair];
                    const int nibL = b & 0x0F;
                    const int nibR = (b >> 4) & 0x0F;
                    const int16_t l = decodeNibble(nibL, shiftL, filterL, m_prevL);
                    const int16_t r = decodeNibble(nibR, shiftR, filterR, m_prevR);
                    m_samples.push_back(l);
                    m_samples.push_back(r);
                }
            }
        } else {
            for (int su = 0; su < soundUnits; su++) {
                const uint8_t sp = params[paramIndex(su)];
                const int shift = sp & 0x0F;
                const int filter = (sp >> 4) & 0x03;
                const int group = su / 2;

                for (int sd = 0; sd < samplesPerUnit; sd++) {
                    const uint8_t b = sampleBytes[sd * 4 + group];
                    const int nib = (su & 1) ? ((b >> 4) & 0x0F) : (b & 0x0F);
                    const int16_t pcm = decodeNibble(nib, shift, filter, m_prevL);
                    m_samples.push_back(pcm);
                }
            }
        }
    }

    return !m_samples.empty();
}
