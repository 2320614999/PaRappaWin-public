#include "pr_stage2_tim_direct.h"
#include "pr_stage2_gpu_direct.h"
#include "logger.h"
#include <array>

namespace PrStage2TimDirect {
namespace {
int32_t Signed(uint32_t v) {
    return v <= 0x7FFFFFFFu ? static_cast<int32_t>(v)
        : static_cast<int32_t>(static_cast<int64_t>(v)-0x100000000LL);
}
// GsGetTimInfo can write shared RAM or the uploaders' private GSIMAGE.
// Keep the original interleaved reads/stores, including aliased RAM outputs.
struct InfoRef {
    Services& s;
    uint32_t address;
    std::array<uint32_t,7>* local = nullptr;
    void Word(uint32_t offset,uint32_t value) {
        if (local) (*local)[offset/4u] = value;
        else s.Write32(address+offset,value);
    }
    void Half(uint32_t offset,uint16_t value) {
        if (!local) { s.Write16(address+offset,value); return; }
        auto& word = (*local)[offset/4u]; const uint32_t shift = (offset%4u)*8u;
        word = (word&~(0xFFFFu<<shift)) | (uint32_t(value)<<shift);
    }
};
int32_t GetInfo(Services& s,uint32_t input,InfoRef out) {
    const uint32_t flags = s.Read32(input);
    out.Word(0u,flags);
    if ((flags&8u) != 0u) {
        uint32_t clut = input+4u;
        uint32_t pixel = clut+(s.Read32(clut)&~3u);
        clut += 4u;
        out.Half(16u,s.Read16(clut));
        uint16_t value = s.Read16(clut+2u); clut += 4u;
        out.Half(18u,value);
        value = s.Read16(clut); pixel += 4u;
        out.Half(20u,value);
        value = s.Read16(clut+2u); clut += 4u;
        out.Word(24u,clut); out.Half(22u,value);
        out.Half(4u,s.Read16(pixel));
        value = s.Read16(pixel+2u); pixel += 4u;
        out.Half(6u,value);
        out.Half(8u,s.Read16(pixel));
        value = s.Read16(pixel+2u); pixel += 4u;
        out.Word(12u,pixel); out.Half(10u,value);
        return Signed(pixel);
    }
    uint32_t pixel = input+8u;
    out.Half(4u,s.Read16(pixel));
    uint16_t value = s.Read16(pixel+2u); pixel += 4u;
    out.Half(6u,value);
    out.Half(8u,s.Read16(pixel));
    value = s.Read16(pixel+2u); pixel += 4u;
    out.Word(12u,pixel); out.Half(10u,value);
    return value;
}
PrStage2LifecycleDirect::ImageRect ImageRect(const std::array<uint32_t,7>& info,
                                             uint32_t index) {
    return {static_cast<uint16_t>(info[index]),static_cast<uint16_t>(info[index]>>16u),
            static_cast<uint16_t>(info[index+1u]),static_cast<uint16_t>(info[index+1u]>>16u)};
}
}
int32_t GetTimInfo80040EAC(Services& s,uint32_t flags,uint32_t output) {
    return GetInfo(s,flags,{s,output});
}
int32_t UploadTim8001AE7C(Services& s,uint32_t tim) {
    std::array<uint32_t,7> info{};
    GetInfo(s,tim+4u,{s,0u,&info});
    s.LoadImage80044D64(ImageRect(info,1u),info[3]);
    PrStage2GpuDirect::DrawSync80044B3C(s,0u);
    if ((info[0]&8u) == 0u) return 0;
    s.LoadImage80044D64(ImageRect(info,4u),info[6]);
    return PrStage2GpuDirect::DrawSync80044B3C(s,0u);
}
int32_t GetClut80043EBC(uint32_t x,uint32_t y) {
    // SRA x,4 followed by ANDI 63: only these six low bits survive, so the
    // unsigned shift has the exact same result without signed-shift rules.
    return static_cast<int32_t>(((y<<6u)|((x>>4u)&63u))&0xFFFFu);
}
int32_t UploadClut800431E0(Services& s,uint32_t source,uint32_t x,uint32_t y) {
    s.LoadImage80044D64({static_cast<uint16_t>(x),static_cast<uint16_t>(y),16u,1u},source);
    // Original ignores the GPU return and computes the ID from full a1/a2,
    // not from a new validation/clamp of the truncated private RECT.
    return GetClut80043EBC(x,y);
}
int32_t UploadRuntimeTim8001ADEC(Services& s,uint32_t tim,uint32_t uploadClut) {
    std::array<uint32_t,7> info{};
    GetInfo(s,tim+4u,{s,0u,&info});
    static uint32_t diagnosticCount = 0u;
    if (diagnosticCount < 256u) {
        const auto image = ImageRect(info,1u);
        const uint16_t clutX = static_cast<uint16_t>(info[4u]);
        const uint16_t clutY = static_cast<uint16_t>(info[4u] >> 16u);
        Log::Printf("S2 runtime TIM upload tim=%08X image=%u,%u %ux%u clut=%u,%u uploadClut=%u",
                    tim, static_cast<unsigned>(image.x), static_cast<unsigned>(image.y),
                    static_cast<unsigned>(image.width), static_cast<unsigned>(image.height),
                    static_cast<unsigned>(clutX), static_cast<unsigned>(clutY),
                    static_cast<unsigned>(uploadClut));
        ++diagnosticCount;
    }
    const int32_t result = s.LoadImage80044D64(ImageRect(info,1u),info[3]);
    if (uploadClut == 0u) return result;
    if ((info[0]&8u) == 0u) return 0;
    const auto extend = [](uint16_t value) {
        return uint32_t(value) | ((value&0x8000u) ? 0xFFFF0000u : 0u);
    };
    // Unlike 1AE7C: no DrawSync, always 16x1 CLUT, and return its CLUT ID.
    const int32_t clut = UploadClut800431E0(s,info[6],extend(static_cast<uint16_t>(info[4])),
                                            extend(static_cast<uint16_t>(info[4]>>16u)));
    if (diagnosticCount <= 256u) {
        Log::Printf("S2 runtime TIM CLUT result=%d source=%08X", clut, info[6]);
    }
    return clut;
}
}
