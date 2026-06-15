#define NOMINMAX
#include "pr_mime.h"
#include "logger.h"

#include <cstring>
#include <algorithm>
#include <cmath>
#include <deque>

// ============================================================
// VDF Parser
// ============================================================
namespace VdfParser {

bool Parse(const uint8_t* data, size_t size, VdfData& out) {
    if (!data || size < 4) return false;

    out = VdfData{};
    out.keys = *(const uint32_t*)data;

    if (out.keys == 0 || out.keys > 1000) {
        Log::Printf("VDF: invalid keys=%u", out.keys);
        return false;
    }

    size_t off = 4;
    out.keyList.resize(out.keys);

    for (uint32_t k = 0; k < out.keys; k++) {
        if (off + 12 > size) {
            Log::Printf("VDF: truncated at key %u (off=%zu size=%zu)", k, off, size);
            out.keyList.resize(k);
            out.keys = k;
            break;
        }

        VdfKey& key = out.keyList[k];
        key.obj     = *(const uint32_t*)(data + off);
        key.vertTop = *(const uint32_t*)(data + off + 4);
        key.nVert   = *(const uint32_t*)(data + off + 8);
        off += 12;

        if (key.nVert > 10000) {
            Log::Printf("VDF: key %u nVert=%u too large", k, key.nVert);
            return false;
        }

        size_t vecBytes = key.nVert * 8; // SVECTOR = 8 bytes
        if (off + vecBytes > size) {
            Log::Printf("VDF: truncated key %u verts (need %zu, have %zu)", k, vecBytes, size - off);
            return false;
        }

        key.deltas.resize(key.nVert);
        for (uint32_t v = 0; v < key.nVert; v++) {
            const int16_t* p = (const int16_t*)(data + off);
            key.deltas[v].x   = p[0];
            key.deltas[v].y   = p[1];
            key.deltas[v].z   = p[2];
            key.deltas[v].pad = p[3];
            off += 8;
        }
    }

    Log::Printf("VDF: parsed keys=%u totalSize=%zu", out.keys, off);
    return true;
}

} // namespace VdfParser

// ============================================================
// DAT Parser
// ============================================================
namespace DatParser {

bool Parse(const uint8_t* data, size_t size, DatData& out) {
    if (!data || size < 2) return false;

    out = DatData{};
    out.keys = *(const uint16_t*)data;

    if (out.keys == 0 || out.keys > 1000) {
        Log::Printf("DAT: invalid keys=%u", out.keys);
        return false;
    }

    out.keyList.resize(out.keys);
    size_t off = 2;

    for (uint16_t k = 0; k < out.keys; k++) {
        if (off + 2 > size) {
            Log::Printf("DAT: truncated at key %u frame count", k);
            return false;
        }

        DatKey& key = out.keyList[k];
        key.frames = *(const uint16_t*)(data + off);
        off += 2;
        out.maxFrames = (std::max)(out.maxFrames, key.frames);

        const size_t inflBytes = static_cast<size_t>(key.frames) * 2u;
        if (off + inflBytes > size) {
            Log::Printf("DAT: truncated at key %u influences (frames=%u off=%zu size=%zu)",
                        k,
                        key.frames,
                        off,
                        size);
            return false;
        }

        key.influence.resize(key.frames);
        for (uint32_t i = 0; i < key.frames; i++) {
            key.influence[i] = *(const int16_t*)(data + off + i * 2);
        }

        off += inflBytes;
    }

    Log::Printf("DAT: parsed keys=%u maxFrames=%u totalSize=%zu",
                out.keys, out.maxFrames, off);
    return true;
}

} // namespace DatParser

// ============================================================
// TOD Parser
// ============================================================
namespace TodParser {

bool Parse(const uint8_t* data, size_t size, TodData& out) {
    if (!data || size < 8) return false;

    out = TodData{};

    // tod+0: 4-byte header (skip)
    // tod+4: u32 blockCount
    out.rawBlockCount = *(const uint32_t*)(data + 4);
    out.blockCount = out.rawBlockCount;
    out.rawBytes.assign(data, data + size);

    if (out.blockCount == 0 || out.blockCount > 10000) {
        Log::Printf("TOD: invalid blockCount=%u", out.blockCount);
        return false;
    }

    size_t off = 8;
    out.blocks.resize(out.blockCount);

    for (uint32_t b = 0; b < out.blockCount; b++) {
        if (off + 8 > size) {
            Log::Printf("TOD: truncated at block %u", b);
            out.blocks.resize(b);
            out.blockCount = b;
            break;
        }

        const size_t blockStart = off;
        TodBlock& block = out.blocks[b];
        block.rawOffset = static_cast<uint32_t>(blockStart);
        block.unk0        = *(const uint16_t*)(data + off);
        block.cmdCount    = *(const uint16_t*)(data + off + 2);
        block.triggerTime = *(const uint32_t*)(data + off + 4);
        off += 8;

        block.commands.resize(block.cmdCount);
        for (uint16_t c = 0; c < block.cmdCount; c++) {
            if (off + 4 > size) {
                Log::Printf("TOD: truncated at block %u cmd %u", b, c);
                block.commands.resize(c);
                break;
            }

            TodCommand& cmd = block.commands[c];
            cmd.header = *(const uint32_t*)(data + off);
            uint32_t lenDwords = (cmd.header >> 24) & 0xFF;
            if (lenDwords == 0) lenDwords = 1;
            off += 4;

            // Read remaining dwords (len-1)
            uint32_t extraDwords = lenDwords - 1;
            if (off + extraDwords * 4 > size) {
                Log::Printf("TOD: truncated cmd data at block %u cmd %u", b, c);
                break;
            }
            cmd.data.resize(extraDwords);
            for (uint32_t d = 0; d < extraDwords; d++) {
                cmd.data[d] = *(const uint32_t*)(data + off);
                off += 4;
            }
        }
        block.rawSize = static_cast<uint32_t>(off - blockStart);
    }

    // Diagnostic: command type distribution
    int typeCounts[16] = {};
    int totalCmds = 0;
    for (const auto& blk : out.blocks) {
        for (const auto& cmd : blk.commands) {
            uint16_t ct = (cmd.header >> 16) & 0xF;
            if (ct < 16) typeCounts[ct]++;
            totalCmds++;
        }
    }
    std::string dist;
    for (int i = 0; i < 16; i++) {
        if (typeCounts[i] > 0) {
            if (!dist.empty()) dist += ", ";
            dist += "type" + std::to_string(i) + "=" + std::to_string(typeCounts[i]);
        }
    }
    Log::Printf("TOD: parsed blocks=%u cmds=%d totalSize=%zu [%s]",
                out.blockCount, totalCmds, off, dist.c_str());
    return true;
}

} // namespace TodParser

// ============================================================
// MIMe Engine State
// ============================================================
namespace MimeEngine {

struct NamedVdf { std::string name; VdfData data; };
struct NamedDat { std::string name; DatData data; };
struct NamedTod { std::string name; TodData data; };

// Channels keep raw pointers to loaded VDF/DAT/TOD payloads. These containers
// therefore need stable element addresses across late runtime loads.
static std::deque<NamedVdf> s_vdfs;
static std::deque<NamedDat> s_dats;
static std::deque<NamedTod> s_tods;
static MimeChannel s_channels[kMaxMimeChannels];
static TodPlayback s_todState;

void Init() {
    s_vdfs.clear();
    s_dats.clear();
    s_tods.clear();
    ClearAllChannels();
    ClearTod();
}

int LoadVdf(const std::string& name, const uint8_t* data, size_t size) {
    NamedVdf nv;
    nv.name = name;
    if (!VdfParser::Parse(data, size, nv.data)) return -1;
    int idx = (int)s_vdfs.size();
    s_vdfs.push_back(std::move(nv));
    return idx;
}

int LoadDat(const std::string& name, const uint8_t* data, size_t size) {
    NamedDat nd;
    nd.name = name;
    if (!DatParser::Parse(data, size, nd.data)) return -1;
    int idx = (int)s_dats.size();
    s_dats.push_back(std::move(nd));
    return idx;
}

int LoadTod(const std::string& name, const uint8_t* data, size_t size) {
    NamedTod nt;
    nt.name = name;
    if (!TodParser::Parse(data, size, nt.data)) return -1;
    int idx = (int)s_tods.size();
    s_tods.push_back(std::move(nt));
    return idx;
}

const VdfData* GetVdf(int idx) {
    if (idx < 0 || idx >= (int)s_vdfs.size()) return nullptr;
    return &s_vdfs[idx].data;
}

const DatData* GetDat(int idx) {
    if (idx < 0 || idx >= (int)s_dats.size()) return nullptr;
    return &s_dats[idx].data;
}

const TodData* GetTod(int idx) {
    if (idx < 0 || idx >= (int)s_tods.size()) return nullptr;
    return &s_tods[idx].data;
}

const VdfData* GetVdfByName(const std::string& name) {
    for (auto& v : s_vdfs) {
        if (v.name == name) return &v.data;
    }
    return nullptr;
}

const DatData* GetDatByName(const std::string& name) {
    for (auto& d : s_dats) {
        if (d.name == name) return &d.data;
    }
    return nullptr;
}

const TodData* GetTodByName(const std::string& name) {
    for (auto& t : s_tods) {
        if (t.name == name) return &t.data;
    }
    return nullptr;
}

void SetChannel(int chan, const VdfData* vdf, const DatData* dat,
                uint32_t currentFrame, uint32_t speed, bool loop) {
    if (chan < 0 || chan >= kMaxMimeChannels) return;
    auto& ch = s_channels[chan];
    ch.active = (vdf != nullptr && dat != nullptr);
    ch.vdf = vdf;
    ch.dat = dat;
    ch.startFrame = currentFrame;
    ch.speed = speed;
    ch.loop = loop;
    if (ch.active) {
        Log::Printf("MIMe: chan %d set vdf(keys=%u) dat(keys=%u) speed=0x%X loop=%d",
                     chan, vdf->keys, dat->keys, speed, loop ? 1 : 0);
    }
}

void ClearChannel(int chan) {
    if (chan < 0 || chan >= kMaxMimeChannels) return;
    s_channels[chan] = MimeChannel{};
}

void ClearAllChannels() {
    for (int i = 0; i < kMaxMimeChannels; i++)
        s_channels[i] = MimeChannel{};
}

MimeChannel& GetChannel(int chan) {
    static MimeChannel dummy;
    if (chan < 0 || chan >= kMaxMimeChannels) return dummy;
    return s_channels[chan];
}

} // namespace MimeEngine (temporarily close for struct member defs)

// ---- TodPlayback / TodObjectTransform member functions ----

const TodObjectTransform* TodPlayback::FindObject(uint16_t objId) const {
    for (int i = 0; i < objectCount; i++)
        if (objects[i].objId == objId) return &objects[i];
    return nullptr;
}

TodObjectTransform* TodPlayback::FindOrAddObject(uint16_t objId) {
    for (int i = 0; i < objectCount; i++)
        if (objects[i].objId == objId) return &objects[i];
    if (objectCount >= kMaxTodObjects) return nullptr;
    auto* p = &objects[objectCount++];
    *p = TodObjectTransform{};
    p->objId = objId;
    return p;
}

float TodObjectTransform::GetRotY() const {
    float m00 = (float)matrix.m[0][0] / 4096.0f;
    float m02 = (float)matrix.m[0][2] / 4096.0f;
    return std::atan2(m02, m00);
}

float TodObjectTransform::GetRotZ() const {
    float m00 = (float)matrix.m[0][0] / 4096.0f;
    float m10 = (float)matrix.m[1][0] / 4096.0f;
    return std::atan2(m10, m00);
}

namespace MimeEngine { // reopen

// ---- MimeEngine TOD functions ----

static int s_todLogCycle = 0;  // only log TOD commands on first cycle

void SetTod(const TodData* tod, bool loop) {
    s_todState = TodPlayback{};
    s_todState.active = (tod != nullptr);
    s_todState.tod = tod;
    s_todState.currentBlock = 0;
    s_todState.frameCounter = 0;
    s_todState.loop = loop;
    s_todLogCycle = 0;
}

void ClearTod() {
    s_todState = TodPlayback{};
}

TodPlayback& GetTodPlayback() {
    return s_todState;
}

// Parse PSX MATRIX from 8 dwords: 3x3 int16 rotation + int32[3] translation
static void ParseCoordMatrix(const std::vector<uint32_t>& data, TodCoordMatrix& out) {
    // PSX MATRIX layout in dwords:
    // dw0: m[0][0] | (m[0][1] << 16)
    // dw1: m[0][2] | (m[1][0] << 16)
    // dw2: m[1][1] | (m[1][2] << 16)
    // dw3: m[2][0] | (m[2][1] << 16)
    // dw4: m[2][2] | (pad << 16)
    // dw5: t[0]
    // dw6: t[1]
    // dw7: t[2]
    if (data.size() < 8) return;
    out.m[0][0] = (int16_t)(data[0] & 0xFFFF);
    out.m[0][1] = (int16_t)(data[0] >> 16);
    out.m[0][2] = (int16_t)(data[1] & 0xFFFF);
    out.m[1][0] = (int16_t)(data[1] >> 16);
    out.m[1][1] = (int16_t)(data[2] & 0xFFFF);
    out.m[1][2] = (int16_t)(data[2] >> 16);
    out.m[2][0] = (int16_t)(data[3] & 0xFFFF);
    out.m[2][1] = (int16_t)(data[3] >> 16);
    out.m[2][2] = (int16_t)(data[4] & 0xFFFF);
    out.t[0] = (int32_t)data[5];
    out.t[1] = (int32_t)data[6];
    out.t[2] = (int32_t)data[7];
}

void TickTod() {
    if (!s_todState.active || !s_todState.tod) return;

    s_todState.frameCounter++;
    const TodData& tod = *s_todState.tod;

    // Process blocks whose triggerTime <= current frame
    while (s_todState.currentBlock < (int)tod.blocks.size()) {
        const TodBlock& block = tod.blocks[s_todState.currentBlock];
        if (s_todState.frameCounter < block.triggerTime) break;

        for (const auto& cmd : block.commands) {
            uint16_t cmdType = (cmd.header >> 16) & 0xF;
            uint16_t cmdFlags = (cmd.header >> 20) & 0xF;
            uint16_t cmdLen  = (cmd.header >> 24) & 0xFF;
            uint16_t objId   = cmd.header & 0xFFFF;

            // Log only on first TOD cycle to avoid spam
            if (s_todLogCycle == 0 && s_todState.currentBlock < 3) {
                std::string dataStr;
                for (size_t di = 0; di < cmd.data.size() && di < 12; di++) {
                    if (di > 0) dataStr += ",";
                    dataStr += std::to_string((int32_t)cmd.data[di]);
                }
                Log::Printf("TOD cmd: blk=%d type=%d flags=0x%X len=%d objId=%d data=[%s]",
                    s_todState.currentBlock, cmdType, cmdFlags, cmdLen, objId, dataStr.c_str());
            }

            switch (cmdType) {
            case 0: {
                // Object attribute (visibility, color, flags)
                // data[0]: flags (bit31 = set-color?), data[1]: extra
                if (cmd.data.size() >= 2) {
                    // Find or create attribute slot
                    for (int i = 0; i < kMaxTodObjects; i++) {
                        if (s_todState.objAttrs[i].valid &&
                            i < s_todState.objectCount &&
                            s_todState.objects[i].objId == objId) {
                            s_todState.objAttrs[i].flags = cmd.data[0];
                            s_todState.objAttrs[i].color = cmd.data[1];
                            break;
                        }
                    }
                }
                break;
            }
            case 2: {
                // PSX sub_80028054 falls through for this command type.
                break;
            }
            case 3: {
                // PSX sub_80028054 falls through for this command type.
                break;
            }
            case 4: {
                // COORD matrix: PSX MATRIX (3x3 rotation + translation)
                // 8 data dwords = PSX MATRIX structure
                if (cmd.data.size() >= 8) {
                    TodObjectTransform* obj = s_todState.FindOrAddObject(objId);
                    if (obj) {
                        ParseCoordMatrix(cmd.data, obj->matrix);
                        obj->valid = true;
                    }
                    // Update legacy compat fields for obj 1
                    if (objId == 1) {
                        const TodObjectTransform* o = s_todState.FindObject(1);
                        if (o) {
                            s_todState.rotY = o->GetRotY();
                            s_todState.tx = o->GetTx();
                            s_todState.ty = o->GetTy();
                            s_todState.tz = o->GetTz();
                        }
                    }
                }
                break;
            }
            case 7: {
                // Camera/viewpoint setup
                // data[0]: projection distance
                // data[1..3]: position (or reference point)
                // data[4..6]: target (or second reference)
                if (cmd.data.size() >= 7) {
                    auto& cam = s_todState.camera;
                    cam.valid    = true;
                    cam.projDist = (int32_t)cmd.data[0];
                    cam.pos[0]   = (int32_t)cmd.data[1];
                    cam.pos[1]   = (int32_t)cmd.data[2];
                    cam.pos[2]   = (int32_t)cmd.data[3];
                    cam.target[0]= (int32_t)cmd.data[4];
                    cam.target[1]= (int32_t)cmd.data[5];
                    cam.target[2]= (int32_t)cmd.data[6];
                }
                break;
            }
            case 8: {
                // Object selector / hierarchy marker
                // No data (len=1), just marks object context for display list
                break;
            }
            case 9: {
                // Viewport/projection setup
                // data[0]: param (e.g. 700), data[1]: flags, data[2]: far (e.g. 1000000)
                if (cmd.data.size() >= 3) {
                    auto& vp = s_todState.viewport;
                    vp.valid  = true;
                    vp.param0 = (int32_t)cmd.data[0];
                    vp.param1 = (int32_t)cmd.data[1];
                    vp.param2 = (int32_t)cmd.data[2];
                }
                break;
            }
            default:
                break;
            }
        }
        s_todState.currentBlock++;
    }

    // Loop: when all blocks consumed, reset timeline
    if (s_todState.loop &&
        s_todState.currentBlock >= (int)tod.blocks.size()) {
        s_todState.currentBlock = 0;
        s_todState.frameCounter = 0;
        s_todLogCycle++;
    }
}

// ============================================================
// Apply MIMe deformation
// ============================================================
void ApplyDeformation(const std::vector<TmdVertex>& baseVerts,
                      std::vector<TmdVertex>& outVerts,
                      uint32_t currentFrame,
                      uint32_t channelMask,
                      uint32_t subFrame8) {
    // Start with base vertices
    outVerts = baseVerts;

    for (int chan = 0; chan < kMaxMimeChannels; chan++) {
        if (!(channelMask & (1u << chan))) continue;
        const MimeChannel& ch = s_channels[chan];
        if (!ch.active || !ch.vdf || !ch.dat) continue;
        if (currentFrame < ch.startFrame) {
            continue;
        }

        uint32_t deltaFrames = currentFrame - ch.startFrame;
        uint32_t frameIndex = (deltaFrames * ch.speed) >> 8;

        const VdfData& vdf = *ch.vdf;
        const DatData& dat = *ch.dat;

        if (ch.loop && dat.maxFrames != 0u) {
            frameIndex %= dat.maxFrames;
        }

        // VDF and DAT must have same number of keys
        uint32_t numKeys = std::min((uint32_t)vdf.keys, (uint32_t)dat.keys);

        for (uint32_t k = 0; k < numKeys; k++) {
            const VdfKey& vk = vdf.keyList[k];
            const DatKey& dk = dat.keyList[k];

            uint32_t inflSize = (uint32_t)dk.influence.size();
            if (inflSize == 0) continue;
            const uint32_t maxIdx = inflSize - 1u;
            const uint32_t sampleFrame = (std::min)(frameIndex, maxIdx);
            int32_t infl = dk.influence[sampleFrame];
            if (subFrame8 != 0u && sampleFrame < maxIdx) {
                const int32_t nextInfl = dk.influence[sampleFrame + 1u];
                infl += (int32_t)((((int64_t)nextInfl - infl) * (int64_t)subFrame8 + 127) / 255);
            }

            if (infl == 0) continue;

            // Apply vertex deltas weighted by influence
            // PSX uses GTE gpl12: delta * infl / 4096
            for (uint32_t v = 0; v < vk.nVert; v++) {
                uint32_t vertIdx = vk.vertTop + v;
                if (vertIdx >= outVerts.size()) break;

                const VdfVertexDelta& delta = vk.deltas[v];
                TmdVertex& ov = outVerts[vertIdx];

                // Fixed-point multiply: (delta * infl) >> 12
                ov.x += (int16_t)(((int32_t)delta.x * infl) >> 12);
                ov.y += (int16_t)(((int32_t)delta.y * infl) >> 12);
                ov.z += (int16_t)(((int32_t)delta.z * infl) >> 12);
            }
        }
    }
}

std::string DumpInfo() {
    std::string out;
    out += "=== MIMe Engine ===\n";
    out += "VDF files: " + std::to_string(s_vdfs.size()) + "\n";
    for (size_t i = 0; i < s_vdfs.size(); i++) {
        out += "  [" + std::to_string(i) + "] " + s_vdfs[i].name
             + " keys=" + std::to_string(s_vdfs[i].data.keys) + "\n";
    }
    out += "DAT files: " + std::to_string(s_dats.size()) + "\n";
    for (size_t i = 0; i < s_dats.size(); i++) {
        const auto& d = s_dats[i].data;
        out += "  [" + std::to_string(i) + "] " + s_dats[i].name
             + " keys=" + std::to_string(d.keys);
        if (!d.keyList.empty()) {
            out += " frames[0]=" + std::to_string(d.keyList[0].frames);
        }
        out += "\n";
    }
    out += "TOD files: " + std::to_string(s_tods.size()) + "\n";
    for (size_t i = 0; i < s_tods.size(); i++) {
        out += "  [" + std::to_string(i) + "] " + s_tods[i].name
             + " blocks=" + std::to_string(s_tods[i].data.blockCount) + "\n";
    }
    out += "Channels:\n";
    for (int i = 0; i < kMaxMimeChannels; i++) {
        const auto& ch = s_channels[i];
        out += "  chan" + std::to_string(i) + ": "
             + (ch.active ? "ACTIVE" : "idle");
        if (ch.active) {
            out += " speed=0x" + std::to_string(ch.speed)
                 + " loop=" + (ch.loop ? "Y" : "N");
        }
        out += "\n";
    }
    if (s_todState.active) {
        out += "TOD: block=" + std::to_string(s_todState.currentBlock)
             + "/" + std::to_string(s_todState.tod ? (int)s_todState.tod->blockCount : 0)
             + " frame=" + std::to_string(s_todState.frameCounter) + "\n";
        for (int i = 0; i < s_todState.objectCount; i++) {
            const auto& o = s_todState.objects[i];
            if (!o.valid) continue;
            const auto& m = o.matrix;
            out += "  obj" + std::to_string(o.objId)
                 + " m=[" + std::to_string(m.m[0][0]) + "," + std::to_string(m.m[0][1]) + "," + std::to_string(m.m[0][2])
                 + "|" + std::to_string(m.m[1][0]) + "," + std::to_string(m.m[1][1]) + "," + std::to_string(m.m[1][2])
                 + "|" + std::to_string(m.m[2][0]) + "," + std::to_string(m.m[2][1]) + "," + std::to_string(m.m[2][2])
                 + "] t=(" + std::to_string(m.t[0]) + "," + std::to_string(m.t[1]) + "," + std::to_string(m.t[2]) + ")\n";
        }
        if (s_todState.camera.valid) {
            const auto& c = s_todState.camera;
            out += "  cam projDist=" + std::to_string(c.projDist)
                 + " pos=(" + std::to_string(c.pos[0]) + "," + std::to_string(c.pos[1]) + "," + std::to_string(c.pos[2]) + ")"
                 + " tgt=(" + std::to_string(c.target[0]) + "," + std::to_string(c.target[1]) + "," + std::to_string(c.target[2]) + ")\n";
        }
        if (s_todState.viewport.valid) {
            const auto& v = s_todState.viewport;
            out += "  viewport p0=" + std::to_string(v.param0) + " p1=" + std::to_string(v.param1) + " p2=" + std::to_string(v.param2) + "\n";
        }
    }
    return out;
}

void Reset() {
    s_vdfs.clear();
    s_dats.clear();
    s_tods.clear();
    ClearAllChannels();
    ClearTod();
}

} // namespace MimeEngine
