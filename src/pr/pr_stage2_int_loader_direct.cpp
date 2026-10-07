#include "pr_stage2_int_loader_direct.h"
#include "pr_stage2_tim_direct.h"

#include <array>

namespace PrStage2IntLoaderDirect {
namespace {
constexpr uint32_t kHeapDepth = 0x8006EB84u;
constexpr uint32_t kHeapEnd = 0x8006EDC4u;
constexpr uint32_t kHeapCursor = 0x8006EDC8u;
constexpr uint32_t kResourceTable = 0x80091858u;
int32_t Signed(uint32_t value) {
    return value <= 0x7FFFFFFFu ? static_cast<int32_t>(value)
        : static_cast<int32_t>(static_cast<int64_t>(value) - 0x100000000LL);
}
uint32_t Push(Services& s, uint32_t bytes) {
    const auto result = static_cast<uint32_t>(PrStage2LifecycleDirect::PushResourceHeap80025A70(s,bytes));
    if (!result) s.Exit(0x80047F3Cu,{1u});
    return result;
}
// A location may refer to real shared RAM or AC18's six private words.
// Local writes stay local, just like the original stack stores.
struct LocationRef {
    Services& memory;
    uint32_t address;
    std::array<uint32_t,6>* local = nullptr;
    uint8_t Read(uint32_t offset) const {
        return local ? static_cast<uint8_t>((*local)[offset/4u] >> (8u*(offset%4u)))
                     : memory.Read8(address+offset);
    }
    void Write(uint32_t offset, uint8_t value) const {
        if (!local) { memory.Write8(address+offset,value); return; }
        const uint32_t shift = 8u*(offset%4u);
        auto& word = (*local)[offset/4u];
        word = (word & ~(0xFFu << shift)) | (uint32_t(value) << shift);
    }
};
int32_t LocationToLba(LocationRef location) {
    const uint32_t minute = location.Read(0), second = location.Read(1);
    const uint32_t seconds = 60u*(10u*(minute>>4u)+(minute&15u))
                          + 10u*(second>>4u)+(second&15u);
    const uint32_t frame = location.Read(2);
    return Signed(75u*seconds+10u*(frame>>4u)+(frame&15u)-150u);
}
void LbaToLocation(uint32_t lba, LocationRef location) {
    const int32_t biased = Signed(lba+150u); // ADDIU wraps before signed divides.
    const int32_t seconds = biased/75;
    const auto bcd = [](int32_t value) {
        return static_cast<uint8_t>(16*(value/10)+value%10);
    };
    // Original stores second, frame, then minute (including negative inputs).
    location.Write(1,bcd(seconds%60));
    location.Write(2,bcd(biased%75));
    location.Write(0,bcd(seconds/60));
}
int32_t SeekFile(Services& s, LocationRef location, uint32_t offset) {
    const uint32_t lba = static_cast<uint32_t>(LocationToLba(location))+offset;
    LbaToLocation(lba,location);
    const uint32_t minute = location.Read(0), second = location.Read(1), frame = location.Read(2);
    return s.SetCdLocation800367A4(minute | (second<<8u) | (frame<<16u)) != 0;
}
}

int32_t LocationToLba80036A78(Services& s, uint32_t location) {
    return LocationToLba({s,location});
}
int32_t LbaToLocation80036974(Services& s, uint32_t lba, uint32_t location) {
    LbaToLocation(lba,{s,location});
    return Signed(location);
}
int32_t SeekFile8001A89C(Services& s, uint32_t descriptor, uint32_t offset) {
    return SeekFile(s,{s,descriptor},offset);
}
int32_t FindFile8001A2B0(Services& s, uint32_t descriptor, uint32_t path) {
    if (!path) return 0;
    for (uint32_t attempt = 0; attempt < 4u; ++attempt)
        if (s.Call(0x800381F8u,{descriptor,path})) return Signed(descriptor);
    return 0;
}
int32_t OpenFile8001A324(Services& s, uint32_t record) {
    if (s.Read32(record+12u) == 1u) return 0;
    const uint32_t path = s.Read32(record);
    if (!path) return 0;
    if (!FindFile8001A2B0(s,record+16u,path)) return -1;
    const uint32_t lba = static_cast<uint32_t>(LocationToLba80036A78(s,record+16u));
    const uint32_t size = s.Read32(record+20u);
    s.Write32(record+40u,lba);
    s.Write32(record+12u,1u);
    s.Write32(record+44u,lba+(size>>11u));
    return 0;
}

int32_t FindHeapEntry80025BFC(Services& s, uint32_t address) {
    const int32_t depth = Signed(s.Read32(kHeapDepth));
    if (depth <= 0) return 0;
    uint32_t index = 1u;
    do {
        if (s.Read32(kResourceTable + 4u*index) == address) return Signed(index);
        ++index;
    } while (Signed(index) <= depth);
    return 0;
}

int32_t SplitHeapEntry80025BBC(Services& s, uint32_t address, uint32_t index, uint32_t bytes) {
    const uint32_t end = s.Read32(kHeapEnd);
    const uint32_t next = address + bytes;
    if (next >= end) return 0;
    s.Write32(kResourceTable + 4u*index,address);
    s.Write32(kHeapDepth,index);
    s.Write32(kHeapCursor,next);
    return 1;
}

int32_t ReadSectors8001A818(Services& s, uint32_t destination, uint32_t sectors, uint32_t mode) {
    const uint32_t flags = mode == 1u ? 128u : 0u;
    while (s.Call(0x80038FC0u,{sectors,destination,flags}) == 0) {}
    s.Call(0x80035560u,{3u});
    int32_t result;
    do { result = s.Call(0x800390C8u,{1u,0u}); } while (result > 0);
    return result == 0 ? Signed(sectors) : 0;
}

namespace {
int32_t ParseInt(Services& s, LocationRef descriptor, uint32_t readMode, uint32_t loadingSound) {
    using namespace PrStage2LifecycleDirect;
    if (!SeekFile(s,descriptor,0u)) return 0;
    for (;;) {
        const auto header = static_cast<uint32_t>(AllocatePacketHeap80025B28(s,8192u));
        if (!header) s.Exit(0x80047F3Cu,{1u});
        if (!ReadSectors8001A818(s,header,4u,readMode)) return 0;
        if (!SeekFile(s,descriptor,4u)) return 0;
        const uint32_t type = s.Read32(header);
        uint32_t sectors = 0u;
        if (type == 1u) {
            uint32_t payload = Push(s,s.Read32(header+8u)<<11u);
            if (!ReadSectors8001A818(s,payload,s.Read32(header+8u),readMode)) return 0;
            uint32_t count = 0u, entry = header+16u;
            if (Signed(s.Read32(header+4u)) > 0) {
                do {
                    PrStage2TimDirect::UploadTim8001AE7C(s,payload);
                    const uint32_t bytes = s.Read32(entry);
                    entry += 20u; ++count;
                    const uint32_t limit = s.Read32(header+4u);
                    payload += bytes;
                    if (Signed(count) >= Signed(limit)) break;
                } while (true);
            }
            PopResourceHeap80025AF8(s);
            sectors = s.Read32(header+8u);
        } else if (type == 2u) {
            const uint32_t bank = Push(s,s.Read32(header+16u));
            const uint32_t samples = Push(s,s.Read32(header+36u));
            if (!ReadSectors8001A818(s,bank,s.Read32(header+8u),readMode)) {
                PopResourceHeap80025AF8(s); PopResourceHeap80025AF8(s);
                return 0;
            }
            s.Call(0x80027120u,{});
            s.Call(0x80026E4Cu,{});
            if (s.Call(0x80027078u,{bank})) {
                s.Call(0x800270D4u,{samples});
                s.Call(0x800270FCu,{1u});
            }
            PopResourceHeap80025AF8(s);
            sectors = s.Read32(header+8u);
            if (!sectors) return 0;
            if (loadingSound == 1u) {
                ResetAudio80026FA4(s);
                PlayCue80026EF8(s,s.Read32(0x80094410u));
                FlushAudio80026ECC(s);
            }
        } else if (type == 3u) {
            uint32_t payload = Push(s,s.Read32(header+8u)<<11u);
            if (!ReadSectors8001A818(s,payload,s.Read32(header+8u),readMode)) return 0;
            const uint32_t first = static_cast<uint32_t>(FindHeapEntry80025BFC(s,payload));
            uint32_t count = 0u, entry = header+16u;
            if (first && Signed(s.Read32(header+4u)) > 0) {
                do {
                    const uint32_t bytes = s.Read32(entry);
                    const int32_t split = SplitHeapEntry80025BBC(s,payload,first+count,bytes);
                    ++count;
                    if (!split) return 0;
                    // Re-read after the split: header and table may overlap.
                    const uint32_t nextBytes = s.Read32(entry);
                    entry += 20u;
                    const uint32_t limit = s.Read32(header+4u);
                    payload += nextBytes;
                    if (Signed(count) >= Signed(limit)) break;
                } while (true);
            }
            sectors = s.Read32(header+8u);
        } else {
            if (type == 0xFFFFFFFFu) return 1;
            s.Write32(0x8006ED28u,1u); // GP+744 error flag.
            return 0;
        }
        if (!sectors) return 0;
        // The original ignores this seek's return, unlike the two earlier
        // seek gates. The next read still occurs after a failed tail seek.
        SeekFile(s,descriptor,sectors);
    }
}
}
int32_t ParseInt8001A8F0(Services& s, uint32_t descriptor, uint32_t readMode, uint32_t loadingSound) {
    return ParseInt(s,{s,descriptor},readMode,loadingSound);
}
int32_t LoadInt8001AC18(Services& s, uint32_t record, uint32_t loadingSound) {
    int32_t result = 0;
    std::array<uint32_t,6> descriptor{};
    for (uint32_t attempt = 0; attempt < 4u; ++attempt) {
        PrStage2LifecycleDirect::ResetResourceHeap80025A34(s);
        if (OpenFile8001A324(s,record) >= 0) {
            const uint32_t first = s.Read32(record+16u), second = s.Read32(record+20u);
            const uint32_t third = s.Read32(record+24u), fourth = s.Read32(record+28u);
            descriptor[0] = first; descriptor[1] = second;
            descriptor[2] = third; descriptor[3] = fourth;
            const uint32_t fifth = s.Read32(record+32u), sixth = s.Read32(record+36u);
            descriptor[4] = fifth; descriptor[5] = sixth;
            result = ParseInt(s,{s,0u,&descriptor},attempt == 0u ? 1u : 0u,loadingSound);
            if (result == 1) break;
        }
    }
    return result;
}
}
