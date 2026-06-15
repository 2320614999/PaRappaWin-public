#include "pr_tmd.h"
#include "logger.h"
#include <cstring>
#include <map>

namespace TmdParser {

static uint16_t ReadLe16(const uint8_t* p) {
    uint16_t v = 0;
    std::memcpy(&v, p, 2);
    return v;
}

static uint32_t ReadLe32(const uint8_t* p) {
    uint32_t v = 0;
    std::memcpy(&v, p, 4);
    return v;
}

static void ApplyTmdPrimitiveGpuFlags(TmdPrimitive& out) {
    out.semiTransparent = (out.mode & 0x02u) != 0u;
    out.textureRaw = (out.mode & 0x01u) != 0u;
    out.abr = (uint8_t)((out.tpage >> 5) & 0x03u);
    out.tpageColorMode = (uint8_t)((out.tpage >> 7) & 0x03u);
    out.clutX = (uint16_t)((out.clut & 0x003Fu) << 4);
    out.clutY = (uint16_t)((out.clut >> 6) & 0x01FFu);
}

static void ParseTmdFlatTriangle(const uint8_t* pp,
                                 uint8_t olen,
                                 uint8_t ilen,
                                 uint8_t flag,
                                 uint8_t mode,
                                 TmdPrimitive& out) {
    out = TmdPrimitive{};
    out.mode = mode;
    out.flag = flag;
    out.ilen = ilen;
    out.olen = olen;
    out.textured = false;
    out.quad = false;
    out.r = pp[0];
    out.g = pp[1];
    out.b = pp[2];
    out.pad1 = pp[3];
    out.colorWords[0] = ReadLe32(pp + 0);
    out.colorWords[1] = out.colorWords[0];
    out.colorWords[2] = out.colorWords[0];
    std::memcpy(&out.v0_idx, pp + 4, 2);
    std::memcpy(&out.v1_idx, pp + 6, 2);
    std::memcpy(&out.v2_idx, pp + 8, 2);
    std::memcpy(&out.pad2, pp + 10, 2);
    ApplyTmdPrimitiveGpuFlags(out);
}

static void ParseTmdFlatTriangleNL(const uint8_t* pp,
                                   uint8_t olen,
                                   uint8_t ilen,
                                   uint8_t flag,
                                   uint8_t mode,
                                   TmdPrimitive& out) {
    ParseTmdFlatTriangle(pp, olen, ilen, flag, mode, out);
    out.v0_idx = ReadLe16(pp + 6);
    out.v1_idx = ReadLe16(pp + 8);
    out.v2_idx = ReadLe16(pp + 10);
}

static void ParseTmdFlatQuadNF(const uint8_t* pp,
                               uint8_t olen,
                               uint8_t ilen,
                               uint8_t flag,
                               uint8_t mode,
                               TmdPrimitive& out) {
    out = TmdPrimitive{};
    out.mode = mode;
    out.flag = flag;
    out.ilen = ilen;
    out.olen = olen;
    out.textured = false;
    out.quad = true;
    out.r = pp[0];
    out.g = pp[1];
    out.b = pp[2];
    out.pad1 = pp[3];
    out.colorWords[0] = ReadLe32(pp + 0);
    out.colorWords[1] = out.colorWords[0];
    out.colorWords[2] = out.colorWords[0];
    out.colorWords[3] = out.colorWords[0];
    std::memcpy(&out.v0_idx, pp + 4, 2);
    std::memcpy(&out.v1_idx, pp + 6, 2);
    std::memcpy(&out.v2_idx, pp + 8, 2);
    std::memcpy(&out.v3_idx, pp + 10, 2);
    ApplyTmdPrimitiveGpuFlags(out);
}

static void ParseTmdFlatQuadNL(const uint8_t* pp,
                               uint8_t olen,
                               uint8_t ilen,
                               uint8_t flag,
                               uint8_t mode,
                               TmdPrimitive& out) {
    out = TmdPrimitive{};
    out.mode = mode;
    out.flag = flag;
    out.ilen = ilen;
    out.olen = olen;
    out.textured = false;
    out.quad = true;
    out.r = pp[0];
    out.g = pp[1];
    out.b = pp[2];
    out.pad1 = pp[3];
    out.colorWords[0] = ReadLe32(pp + 0);
    out.colorWords[1] = out.colorWords[0];
    out.colorWords[2] = out.colorWords[0];
    out.colorWords[3] = out.colorWords[0];
    std::memcpy(&out.pad2, pp + 4, 2);
    std::memcpy(&out.v0_idx, pp + 6, 2);
    std::memcpy(&out.v1_idx, pp + 8, 2);
    std::memcpy(&out.v2_idx, pp + 10, 2);
    std::memcpy(&out.v3_idx, pp + 12, 2);
    ApplyTmdPrimitiveGpuFlags(out);
}

static void ParseTmdTexturedTriangle(const uint8_t* pp,
                                     uint8_t olen,
                                     uint8_t ilen,
                                     uint8_t flag,
                                     uint8_t mode,
                                     TmdPrimitive& out) {
    out = TmdPrimitive{};
    out.mode = mode;
    out.flag = flag;
    out.ilen = ilen;
    out.olen = olen;
    out.textured = true;
    out.quad = false;
    out.u0 = pp[0];
    out.v0 = pp[1];
    std::memcpy(&out.clut, pp + 2, 2);
    out.u1 = pp[4];
    out.v1 = pp[5];
    std::memcpy(&out.tpage, pp + 6, 2);
    out.u2 = pp[8];
    out.v2 = pp[9];
    std::memcpy(&out.pad0, pp + 10, 2);
    out.r = pp[12];
    out.g = pp[13];
    out.b = pp[14];
    out.pad1 = pp[15];
    out.colorWords[0] = ReadLe32(pp + 12);
    out.colorWords[1] = out.colorWords[0];
    out.colorWords[2] = out.colorWords[0];
    std::memcpy(&out.v0_idx, pp + 16, 2);
    std::memcpy(&out.v1_idx, pp + 18, 2);
    std::memcpy(&out.v2_idx, pp + 20, 2);
    std::memcpy(&out.pad2, pp + 22, 2);
    ApplyTmdPrimitiveGpuFlags(out);
}

static void ParseTmdTexturedTriangleNL(const uint8_t* pp,
                                       uint8_t olen,
                                       uint8_t ilen,
                                       uint8_t flag,
                                       uint8_t mode,
                                       TmdPrimitive& out) {
    out = TmdPrimitive{};
    out.mode = mode;
    out.flag = flag;
    out.ilen = ilen;
    out.olen = olen;
    out.textured = true;
    out.quad = false;
    out.u0 = pp[0];
    out.v0 = pp[1];
    std::memcpy(&out.clut, pp + 2, 2);
    out.u1 = pp[4];
    out.v1 = pp[5];
    std::memcpy(&out.tpage, pp + 6, 2);
    out.u2 = pp[8];
    out.v2 = pp[9];
    std::memcpy(&out.pad0, pp + 10, 2);
    out.r = 0x80;
    out.g = 0x80;
    out.b = 0x80;
    out.colorWords[0] = 0x00808080u;
    out.colorWords[1] = out.colorWords[0];
    out.colorWords[2] = out.colorWords[0];
    std::memcpy(&out.pad2, pp + 12, 2);
    std::memcpy(&out.v0_idx, pp + 14, 2);
    std::memcpy(&out.v1_idx, pp + 16, 2);
    std::memcpy(&out.v2_idx, pp + 18, 2);
    ApplyTmdPrimitiveGpuFlags(out);
}

static void ParseTmdTexturedQuad(const uint8_t* pp,
                                 uint8_t olen,
                                 uint8_t ilen,
                                 uint8_t flag,
                                 uint8_t mode,
                                 TmdPrimitive& out) {
    out = TmdPrimitive{};
    out.mode = mode;
    out.flag = flag;
    out.ilen = ilen;
    out.olen = olen;
    out.textured = true;
    out.quad = true;
    out.u0 = pp[0];
    out.v0 = pp[1];
    std::memcpy(&out.clut, pp + 2, 2);
    out.u1 = pp[4];
    out.v1 = pp[5];
    std::memcpy(&out.tpage, pp + 6, 2);
    out.u2 = pp[8];
    out.v2 = pp[9];
    std::memcpy(&out.pad0, pp + 10, 2);
    out.u3 = pp[12];
    out.v3 = pp[13];
    out.r = pp[16];
    out.g = pp[17];
    out.b = pp[18];
    out.pad1 = pp[19];
    out.colorWords[0] = ReadLe32(pp + 16);
    out.colorWords[1] = out.colorWords[0];
    out.colorWords[2] = out.colorWords[0];
    out.colorWords[3] = out.colorWords[0];
    std::memcpy(&out.v0_idx, pp + 20, 2);
    std::memcpy(&out.v1_idx, pp + 22, 2);
    std::memcpy(&out.v2_idx, pp + 24, 2);
    std::memcpy(&out.v3_idx, pp + 26, 2);
    ApplyTmdPrimitiveGpuFlags(out);
}

static void ParseTmdTexturedQuadNL(const uint8_t* pp,
                                   uint8_t olen,
                                   uint8_t ilen,
                                   uint8_t flag,
                                   uint8_t mode,
                                   TmdPrimitive& out) {
    out = TmdPrimitive{};
    out.mode = mode;
    out.flag = flag;
    out.ilen = ilen;
    out.olen = olen;
    out.textured = true;
    out.quad = true;
    out.u0 = pp[0];
    out.v0 = pp[1];
    std::memcpy(&out.clut, pp + 2, 2);
    out.u1 = pp[4];
    out.v1 = pp[5];
    std::memcpy(&out.tpage, pp + 6, 2);
    out.u2 = pp[8];
    out.v2 = pp[9];
    std::memcpy(&out.pad0, pp + 10, 2);
    out.u3 = pp[12];
    out.v3 = pp[13];
    out.r = 0x80;
    out.g = 0x80;
    out.b = 0x80;
    out.colorWords[0] = 0x00808080u;
    out.colorWords[1] = out.colorWords[0];
    out.colorWords[2] = out.colorWords[0];
    out.colorWords[3] = out.colorWords[0];
    std::memcpy(&out.pad2, pp + 16, 2);
    std::memcpy(&out.v0_idx, pp + 18, 2);
    std::memcpy(&out.v1_idx, pp + 20, 2);
    std::memcpy(&out.v2_idx, pp + 22, 2);
    std::memcpy(&out.v3_idx, pp + 24, 2);
    ApplyTmdPrimitiveGpuFlags(out);
}

static void ParseTmdGouraudTriangleNL(const uint8_t* pp,
                                      uint8_t olen,
                                      uint8_t ilen,
                                      uint8_t flag,
                                      uint8_t mode,
                                      TmdPrimitive& out) {
    out = TmdPrimitive{};
    out.mode = mode;
    out.flag = flag;
    out.ilen = ilen;
    out.olen = olen;
    out.textured = false;
    out.quad = false;
    out.r = pp[0];
    out.g = pp[1];
    out.b = pp[2];
    out.pad1 = pp[3];
    out.colorWords[0] = ReadLe32(pp + 0);
    out.colorWords[1] = out.colorWords[0];
    out.colorWords[2] = out.colorWords[0];
    out.v0_idx = ReadLe16(pp + 6);
    out.v1_idx = ReadLe16(pp + 10);
    out.v2_idx = ReadLe16(pp + 14);
    ApplyTmdPrimitiveGpuFlags(out);
}

static void ParseTmdGouraudTriangleN(const uint8_t* pp,
                                     uint8_t olen,
                                     uint8_t ilen,
                                     uint8_t flag,
                                     uint8_t mode,
                                     TmdPrimitive& out) {
    out = TmdPrimitive{};
    out.mode = mode;
    out.flag = flag;
    out.ilen = ilen;
    out.olen = olen;
    out.textured = false;
    out.quad = false;
    out.r = pp[0];
    out.g = pp[1];
    out.b = pp[2];
    out.pad1 = pp[3];
    out.colorWords[0] = ReadLe32(pp + 0);
    out.colorWords[1] = ReadLe32(pp + 4);
    out.colorWords[2] = ReadLe32(pp + 8);
    out.v0_idx = ReadLe16(pp + 12);
    out.v1_idx = ReadLe16(pp + 14);
    out.v2_idx = ReadLe16(pp + 16);
    ApplyTmdPrimitiveGpuFlags(out);
}

static void ParseTmdGouraudQuadNL(const uint8_t* pp,
                                  uint8_t olen,
                                  uint8_t ilen,
                                  uint8_t flag,
                                  uint8_t mode,
                                  TmdPrimitive& out) {
    out = TmdPrimitive{};
    out.mode = mode;
    out.flag = flag;
    out.ilen = ilen;
    out.olen = olen;
    out.textured = false;
    out.quad = true;
    out.r = pp[0];
    out.g = pp[1];
    out.b = pp[2];
    out.pad1 = pp[3];
    out.colorWords[0] = ReadLe32(pp + 0);
    out.colorWords[1] = out.colorWords[0];
    out.colorWords[2] = out.colorWords[0];
    out.colorWords[3] = out.colorWords[0];
    out.v0_idx = ReadLe16(pp + 6);
    out.v1_idx = ReadLe16(pp + 10);
    out.v2_idx = ReadLe16(pp + 14);
    out.v3_idx = ReadLe16(pp + 18);
    ApplyTmdPrimitiveGpuFlags(out);
}

static void ParseTmdGouraudQuadN(const uint8_t* pp,
                                 uint8_t olen,
                                 uint8_t ilen,
                                 uint8_t flag,
                                 uint8_t mode,
                                 TmdPrimitive& out) {
    out = TmdPrimitive{};
    out.mode = mode;
    out.flag = flag;
    out.ilen = ilen;
    out.olen = olen;
    out.textured = false;
    out.quad = true;
    out.r = pp[0];
    out.g = pp[1];
    out.b = pp[2];
    out.pad1 = pp[3];
    out.colorWords[0] = ReadLe32(pp + 0);
    out.colorWords[1] = ReadLe32(pp + 4);
    out.colorWords[2] = ReadLe32(pp + 8);
    out.colorWords[3] = ReadLe32(pp + 12);
    out.v0_idx = ReadLe16(pp + 16);
    out.v1_idx = ReadLe16(pp + 18);
    out.v2_idx = ReadLe16(pp + 20);
    out.v3_idx = ReadLe16(pp + 22);
    ApplyTmdPrimitiveGpuFlags(out);
}

static void ParseTmdTexturedGouraudTriangleNL(const uint8_t* pp,
                                              uint8_t olen,
                                              uint8_t ilen,
                                              uint8_t flag,
                                              uint8_t mode,
                                              TmdPrimitive& out) {
    ParseTmdTexturedTriangleNL(pp, olen, ilen, flag, mode, out);
    out.v0_idx = ReadLe16(pp + 14);
    out.v1_idx = ReadLe16(pp + 18);
    out.v2_idx = ReadLe16(pp + 22);
}

static void ParseTmdTexturedGouraudTriangleN(const uint8_t* pp,
                                             uint8_t olen,
                                             uint8_t ilen,
                                             uint8_t flag,
                                             uint8_t mode,
                                             TmdPrimitive& out) {
    ParseTmdTexturedTriangle(pp, olen, ilen, flag, mode, out);
    out.colorWords[0] = ReadLe32(pp + 12);
    out.colorWords[1] = ReadLe32(pp + 16);
    out.colorWords[2] = ReadLe32(pp + 20);
    out.v0_idx = ReadLe16(pp + 24);
    out.v1_idx = ReadLe16(pp + 26);
    out.v2_idx = ReadLe16(pp + 28);
}

static void ParseTmdTexturedGouraudQuadNL(const uint8_t* pp,
                                          uint8_t olen,
                                          uint8_t ilen,
                                          uint8_t flag,
                                          uint8_t mode,
                                          TmdPrimitive& out) {
    ParseTmdTexturedQuadNL(pp, olen, ilen, flag, mode, out);
    out.v0_idx = ReadLe16(pp + 18);
    out.v1_idx = ReadLe16(pp + 22);
    out.v2_idx = ReadLe16(pp + 26);
    out.v3_idx = ReadLe16(pp + 30);
}

static void ParseTmdTexturedGouraudQuadN(const uint8_t* pp,
                                         uint8_t olen,
                                         uint8_t ilen,
                                         uint8_t flag,
                                         uint8_t mode,
                                         TmdPrimitive& out) {
    ParseTmdTexturedQuad(pp, olen, ilen, flag, mode, out);
    out.colorWords[0] = ReadLe32(pp + 16);
    out.colorWords[1] = ReadLe32(pp + 20);
    out.colorWords[2] = ReadLe32(pp + 24);
    out.colorWords[3] = ReadLe32(pp + 28);
    out.v0_idx = ReadLe16(pp + 32);
    out.v1_idx = ReadLe16(pp + 34);
    out.v2_idx = ReadLe16(pp + 36);
    out.v3_idx = ReadLe16(pp + 38);
}

static bool TryParseSupportedPrimitive(const uint8_t* pp,
                                       uint8_t olen,
                                       uint8_t ilen,
                                       uint8_t flag,
                                       uint8_t mode,
                                       TmdPrimitive& out) {
    const uint8_t psxMode = mode & 0xFDu;
    if (psxMode == 0x20u && ilen == 3) {
        ParseTmdFlatTriangleNL(pp, olen, ilen, flag, mode, out);
        return true;
    }
    if (psxMode == 0x21u && ilen == 3) {
        ParseTmdFlatTriangle(pp, olen, ilen, flag, mode, out);
        return true;
    }
    if (psxMode == 0x28u && ilen == 4) {
        ParseTmdFlatQuadNL(pp, olen, ilen, flag, mode, out);
        return true;
    }
    if (psxMode == 0x29u && ilen == 3) {
        ParseTmdFlatQuadNF(pp, olen, ilen, flag, mode, out);
        return true;
    }
    if (psxMode == 0x24u && ilen == 5) {
        ParseTmdTexturedTriangleNL(pp, olen, ilen, flag, mode, out);
        return true;
    }
    if ((mode == 0x25 || mode == 0x27) && ilen == 6) {
        ParseTmdTexturedTriangle(pp, olen, ilen, flag, mode, out);
        return true;
    }
    if (psxMode == 0x2Cu && ilen == 7) {
        ParseTmdTexturedQuadNL(pp, olen, ilen, flag, mode, out);
        return true;
    }
    if ((mode & 0xFDu) == 0x2Du && ilen == 7) {
        ParseTmdTexturedQuad(pp, olen, ilen, flag, mode, out);
        return true;
    }
    if (psxMode == 0x30u && ilen == 4) {
        ParseTmdGouraudTriangleNL(pp, olen, ilen, flag, mode, out);
        return true;
    }
    if (psxMode == 0x31u && ilen == 5) {
        ParseTmdGouraudTriangleN(pp, olen, ilen, flag, mode, out);
        return true;
    }
    if (psxMode == 0x34u && ilen == 6) {
        ParseTmdTexturedGouraudTriangleNL(pp, olen, ilen, flag, mode, out);
        return true;
    }
    if (psxMode == 0x35u && ilen == 8) {
        ParseTmdTexturedGouraudTriangleN(pp, olen, ilen, flag, mode, out);
        return true;
    }
    if (psxMode == 0x38u && ilen == 5) {
        ParseTmdGouraudQuadNL(pp, olen, ilen, flag, mode, out);
        return true;
    }
    if (psxMode == 0x39u && ilen == 6) {
        ParseTmdGouraudQuadN(pp, olen, ilen, flag, mode, out);
        return true;
    }
    if (psxMode == 0x3Cu && ilen == 8) {
        ParseTmdTexturedGouraudQuadNL(pp, olen, ilen, flag, mode, out);
        return true;
    }
    if (psxMode == 0x3Du && ilen == 10) {
        ParseTmdTexturedGouraudQuadN(pp, olen, ilen, flag, mode, out);
        return true;
    }
    return false;
}

bool Parse(const uint8_t* data, size_t size, TmdModel& out) {
    if (!data || size < 12) return false;

    // TMD header: id(4) flags(4) nobj(4)
    uint32_t id, flags, nobj;
    std::memcpy(&id, data + 0, 4);
    std::memcpy(&flags, data + 4, 4);
    std::memcpy(&nobj, data + 8, 4);

    if (id != 0x00000041) return false;  // TMD magic
    if (nobj == 0 || nobj > 256) return false;

    out.id = id;
    out.flags = flags;
    out.objects.resize(nobj);

    // Object table starts at offset 12
    // Each object entry: 7 * 4 = 28 bytes
    //   vert_ofs(4) vert_count(4) norm_ofs(4) norm_count(4) prim_ofs(4) prim_count(4) scale(4)
    const size_t objTableOfs = 12;
    const size_t objEntrySize = 28;
    if (objTableOfs + nobj * objEntrySize > size) return false;

    // fixp flag: if flags & 1, offsets are absolute; else relative to obj table start
    const bool fixp = (flags & 1) != 0;
    const size_t baseOfs = fixp ? 0 : objTableOfs;

    int totalVerts = 0, totalParsed = 0, totalSkipped = 0;
    std::map<uint8_t, int> parsedModes;
    std::map<uint8_t, int> skippedModes;

    for (uint32_t oi = 0; oi < nobj; ++oi) {
        const uint8_t* oe = data + objTableOfs + oi * objEntrySize;
        uint32_t vertOfs, vertCount, normOfs, normCount, primOfs, primCount;
        int32_t scale;
        std::memcpy(&vertOfs,   oe + 0, 4);
        std::memcpy(&vertCount, oe + 4, 4);
        std::memcpy(&normOfs,   oe + 8, 4);
        std::memcpy(&normCount, oe + 12, 4);
        std::memcpy(&primOfs,   oe + 16, 4);
        std::memcpy(&primCount, oe + 20, 4);
        std::memcpy(&scale,     oe + 24, 4);

        TmdObject& obj = out.objects[oi];
        obj.scale = scale;
        totalVerts += (int)vertCount;

        // Parse vertices (8 bytes each: x,y,z,pad as int16)
        {
            size_t vOfs = baseOfs + vertOfs;
            if (vOfs + vertCount * 8 > size) return false;
            obj.vertices.resize(vertCount);
            for (uint32_t vi = 0; vi < vertCount; ++vi) {
                const uint8_t* vp = data + vOfs + vi * 8;
                std::memcpy(&obj.vertices[vi].x, vp + 0, 2);
                std::memcpy(&obj.vertices[vi].y, vp + 2, 2);
                std::memcpy(&obj.vertices[vi].z, vp + 4, 2);
                std::memcpy(&obj.vertices[vi].pad, vp + 6, 2);
            }
        }

        // Parse primitives - walk the primitive packet stream
        {
            size_t pOfs = baseOfs + primOfs;
            obj.rawPrimitivePackets.clear();
            obj.rawPrimitivePackets.reserve(primCount);
            obj.primitives.clear();
            obj.primitives.reserve(primCount);

            size_t cursor = pOfs;
            for (uint32_t pi = 0; pi < primCount; ++pi) {
                if (cursor + 4 > size) break;
                uint8_t olen = data[cursor + 0];
                uint8_t ilen = data[cursor + 1];
                uint8_t flag = data[cursor + 2];
                uint8_t mode = data[cursor + 3];

                size_t packetSize = 4 + (size_t)ilen * 4;
                if (cursor + packetSize > size) break;

                TmdRawPrimitivePacket raw{};
                raw.rawPrimitiveIndex = pi;
                raw.rawPacketOffset = static_cast<uint32_t>(cursor);
                raw.rawPacketByteSize = static_cast<uint32_t>(packetSize);
                raw.olen = olen;
                raw.ilen = ilen;
                raw.flag = flag;
                raw.mode = mode;

                TmdPrimitive prim;
                if (TryParseSupportedPrimitive(data + cursor + 4,
                                               olen,
                                               ilen,
                                               flag,
                                               mode,
                                               prim)) {
                    prim.rawPacketKnown = true;
                    prim.rawPrimitiveIndex = pi;
                    prim.rawPacketOffset = static_cast<uint32_t>(cursor);
                    prim.rawPacketByteSize =
                        static_cast<uint32_t>(packetSize);
                    raw.parsed = true;
                    raw.parsedPrimitiveIndex =
                        static_cast<uint32_t>(obj.primitives.size());
                    obj.primitives.push_back(prim);
                    ++totalParsed;
                    parsedModes[mode]++;
                } else {
                    ++totalSkipped;
                    skippedModes[mode]++;
                }
                obj.rawPrimitivePackets.push_back(raw);

                cursor += packetSize;
            }
        }
    }

    Log::Printf("TMD: %u objs, %d verts, %d prims(parsed), %d skipped",
                nobj, totalVerts, totalParsed, totalSkipped);
    for (auto& [m, cnt] : parsedModes) {
        Log::Printf("  TMD parsed mode=0x%02X count=%d", (int)m, cnt);
    }
    for (auto& [m, cnt] : skippedModes) {
        Log::Printf("  TMD skip mode=0x%02X count=%d", (int)m, cnt);
    }

    return true;
}

} // namespace TmdParser
