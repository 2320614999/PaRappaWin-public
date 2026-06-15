#pragma once
#include <cstdint>
#include <vector>

// PSX TMD (Table Model Data) parser for PaRappa Win.
// The Stage1 scene uses a mixed PSX primitive subset carried through
// sub_800428B0/GsTMDfast*: flat/textured triangles and quads, including
// no-light (*NL) record layouts.

struct TmdVertex {
    int16_t x, y, z;
    int16_t pad;
};

struct TmdRawPrimitivePacket {
    uint32_t rawPrimitiveIndex = 0;
    uint32_t rawPacketOffset = 0;
    uint32_t rawPacketByteSize = 0;
    uint8_t olen = 0;
    uint8_t ilen = 0;
    uint8_t flag = 0;
    uint8_t mode = 0;
    bool parsed = false;
    uint32_t parsedPrimitiveIndex = 0;
};

struct TmdPrimitive {
    bool rawPacketKnown = false;
    uint32_t rawPrimitiveIndex = 0;
    uint32_t rawPacketOffset = 0;
    uint32_t rawPacketByteSize = 0;
    uint8_t mode = 0;
    uint8_t flag = 0;
    uint8_t ilen = 0;
    uint8_t olen = 0;
    bool textured = false;
    bool quad = false;
    bool semiTransparent = false;
    bool textureRaw = false;
    uint8_t abr = 0;
    uint8_t tpageColorMode = 0;
    uint16_t clutX = 0;
    uint16_t clutY = 0;

    uint8_t u0 = 0, v0 = 0;
    uint8_t u1 = 0, v1 = 0;
    uint8_t u2 = 0, v2 = 0;
    uint8_t u3 = 0, v3 = 0;
    uint16_t clut = 0;   // CBA: CLUT address in VRAM
    uint16_t tpage = 0;  // TSB: texture page
    uint16_t pad0 = 0;
    uint8_t r = 0x80, g = 0x80, b = 0x80;
    uint8_t pad1 = 0;
    uint32_t colorWords[4] = {};
    uint16_t v0_idx = 0;
    uint16_t v1_idx = 0;
    uint16_t v2_idx = 0;
    uint16_t v3_idx = 0;
    uint16_t pad2 = 0;
};

struct TmdObject {
    std::vector<TmdVertex> vertices;
    std::vector<TmdRawPrimitivePacket> rawPrimitivePackets;
    std::vector<TmdPrimitive> primitives;
    int32_t scale;
};

struct TmdModel {
    uint32_t id;
    uint32_t flags;
    std::vector<TmdObject> objects;
};

namespace TmdParser {
    bool Parse(const uint8_t* data, size_t size, TmdModel& out);
}
