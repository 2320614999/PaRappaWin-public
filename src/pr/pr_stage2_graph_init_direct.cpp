#include "pr_stage2_graph_init_direct.h"

namespace PrStage2GraphInitDirect {
namespace {
uint32_t U(int32_t v) { return static_cast<uint32_t>(v); }
int32_t S(uint32_t v) {
    return v <= 0x7FFFFFFFu ? static_cast<int32_t>(v)
        : static_cast<int32_t>(static_cast<int64_t>(v) - 0x100000000LL);
}
void CopyMatrixTripleGroups(Services& s, uint32_t dst, uint32_t src) {
    for (uint32_t offset : {0u, 12u}) {
        const uint32_t a = s.Read32(src + offset);
        const uint32_t b = s.Read32(src + offset + 4u);
        const uint32_t c = s.Read32(src + offset + 8u);
        s.Write32(dst + offset, a);
        s.Write32(dst + offset + 4u, b);
        s.Write32(dst + offset + 8u, c);
    }
    const uint32_t a = s.Read32(src + 24u), b = s.Read32(src + 28u);
    s.Write32(dst + 24u, a);
    s.Write32(dst + 28u, b);
}
}

void InitGte80040D20(Services& s) {
    // 80040D6C enables the PSX COP2/BIOS path before these CTC2 writes.
    // The native arithmetic owner needs no CPU enable bit or BIOS patch;
    // its register state must still receive every original initial value.
    auto& g=s.MatrixGte();
    g.zsf3=341;g.zsf4=256;g.h=1000;
    g.dqa=-4194;g.dqb=20971520;g.ofx=0;g.ofy=0;
    g.farColor={0,0,0}; // SetFarColor(0,0,0), control 21..23.
    s.Write16(0x800917ACu,0u);
    s.Write16(0x800917AAu,0u);
}

int32_t DetectGpu800472E4(Services& s, uint32_t mode) {
    s.Write32(s.Read32(0x8005D808u), 0x10000007u);
    const uint32_t data = s.Read32(0x8005D804u);
    if ((s.Read32(data) & 0x00FFFFFFu) == 2u) {
        if ((mode & 8u) == 0u) return 3;
        s.Write32(s.Read32(0x8005D808u), 0x09000001u);
        return 4;
    }
    const uint32_t status = s.Read32(s.Read32(0x8005D808u));
    s.Write32(data, (status & 0x3FFFu) | 0xE1001000u);
    const uint32_t freshData = s.Read32(0x8005D804u);
    const uint32_t freshStatus = s.Read32(0x8005D808u);
    s.Read32(freshData); // Original discarded GPUREAD still has device effects.
    if ((s.Read32(freshStatus) & 0x1000u) == 0u) return 0;
    if ((mode & 8u) == 0u) return 1;
    s.Write32(freshStatus, 0x20000504u);
    return 2;
}

int32_t ResetGpu80046EC0(Services& s, uint32_t mode) {
    const uint32_t mask = U(s.Call(0x800358C0u, {0u}));
    s.Write32(0x8005D83Cu, 0u);
    const uint32_t tail = s.Read32(0x8005D83Cu);
    s.Write32(0x8005D848u, mask);
    s.Write32(0x8005D838u, tail);
    const uint32_t kind = mode & 7u;
    if (kind == 0u || kind == 1u) {
        s.Write32(s.Read32(0x8005D814u), 0x401u);
        const uint32_t priority = s.Read32(0x8005D824u);
        s.Write32(priority, s.Read32(priority) | 0x800u);
        if (kind == 0u) {
            s.Write32(s.Read32(0x8005D808u), 0u);
            PrStage2GpuDirect::FillBytes800473C0(s, 0x8008EB98u, 0u, 0x100u);
            PrStage2GpuDirect::FillBytes800473C0(s, 0x80094448u, 0u, 0x1800u);
        } else {
            s.Write32(s.Read32(0x8005D808u), 0x02000000u);
            s.Write32(s.Read32(0x8005D808u), 0x01000000u);
        }
    }
    s.Call(0x800358C0u, {s.Read32(0x8005D848u)});
    return kind == 0u ? DetectGpu800472E4(s, mode) : 0;
}

int32_t ResetGraph800446A0(Services& s, uint32_t mode) {
    const uint32_t kind = mode & 7u;
    if (kind != 0u && kind != 3u) {
        if (s.Read8(0x8005D736u) >= 2u)
            s.Call(s.Read32(0x8005D730u), {0x8001245Cu, mode});
        const uint32_t driver = s.Read32(0x8005D72Cu);
        const uint32_t reset = s.Read32(driver + 0x34u);
        return reset == 0x80046EC0u ? ResetGpu80046EC0(s, 1u) : s.Call(reset, {1u});
    }
    s.Call(0x80047FFCu, {0x8001243Cu, 0x8005D6ECu, 0x8005D734u});
    PrStage2GpuDirect::FillBytes800473C0(s, 0x8005D734u, 0u, 128u);
    s.Call(0x80035744u, {}); // Original callback ownership, not a local reset substitute.
    s.Call(0x80048950u, {s.Read32(0x8005D72Cu) & 0x00FFFFFFu});
    const int32_t version = ResetGpu80046EC0(s, kind != 0u ? 1u : 0u);
    s.Write8(0x8005D734u, static_cast<uint8_t>(version));
    const uint32_t firstIndex = s.Read8(0x8005D734u);
    s.Write8(0x8005D735u, 1u);
    const uint16_t width = s.Read16(0x8005D7B4u + 4u * firstIndex);
    const uint32_t secondIndex = s.Read8(0x8005D734u);
    s.Write16(0x8005D738u, width);
    s.Write16(0x8005D73Au, s.Read16(0x8005D7C8u + 4u * secondIndex));
    PrStage2GpuDirect::FillBytes800473C0(s, 0x8005D744u, 0xFFFFFFFFu, 92u);
    PrStage2GpuDirect::FillBytes800473C0(s, 0x8005D7A0u, 0xFFFFFFFFu, 20u);
    return s.Read8(0x8005D734u);
}

int32_t InitGraphEnvironment8003FC14(Services& s, uint32_t width, uint32_t height,
    uint32_t flags, uint32_t dither, uint32_t rgb24) {
    ResetGraph800446A0(s, 0u);
    for (uint32_t address : {0x8009173Au, 0x80091738u, 0x80091742u, 0x80091740u,
                             0x8009173Eu, 0x8009173Cu, 0x80091744u})
        s.Write16(address, 0u);
    s.Write8(0x80091746u, static_cast<uint8_t>(dither));
    s.Write8(0x80091747u, 0u);
    s.Write8(0x80091748u, 0u);
    PrStage2GpuDirect::PutDrawEnvironment80045114(s, 0x80091730u);
    s.Write16(0x80091790u, 0u);
    s.Write16(0x80091792u, 0u);
    s.Write16(0x80091794u, static_cast<uint16_t>(width));
    s.Write16(0x80091796u, static_cast<uint16_t>(height));
    for (uint32_t address : {0x80091798u, 0x8009179Au, 0x8009179Cu, 0x8009179Eu})
        s.Write16(address, 0u);
    if (PrStage2GpuDirect::VideoMode8003623C(s) == 1) {
        s.Write16(0x8009179Au, 24u);
        s.Write8(0x800917A2u, 1u);
    }
    // In NTSC the original does NOT clear the previous 917A2 byte here.
    s.Write8(0x800917A0u, static_cast<uint8_t>(flags & 1u));
    s.Write16(0x800965A0u, static_cast<uint16_t>(flags & 4u));
    s.Write8(0x800917A1u, static_cast<uint8_t>(rgb24));
    return PrStage2GpuDirect::PutDisplayEnvironment800452EC(s, 0x80091790u);
}

int32_t InitClearPacket800442F4(Services& s, uint32_t packet) {
    s.Write8(packet + 3u, 3u);
    s.Write8(packet + 7u, 2u);
    return 2;
}

int32_t InitGeometry8003FDE4(Services& s, uint32_t width, uint32_t height) {
    s.Write32(0x8009182Cu, height & 0xFFFFu);
    const uint32_t scaledHeight = s.Read32(0x8009182Cu) << 14u;
    s.Write32(0x800917FCu, width & 0xFFFFu);
    const int32_t divisor = S(s.Read32(0x800917FCu));
    if (divisor == 0) s.Break(0x8003FE1Cu, 7u);
    if (scaledHeight == 0x80000000u && divisor == -1) s.Break(0x8003FE34u, 6u);
    const int32_t ratio = S(scaledHeight) / divisor;
    for (uint32_t address : {0x8009183Cu, 0x8009183Au, 0x80091842u,
                             0x8009183Eu, 0x80091846u, 0x80091844u})
        s.Write16(address, 0u);
    for (uint32_t address : {0x80091854u, 0x80091850u, 0x8009184Cu}) s.Write32(address, 0u);
    for (uint32_t address : {0x80091838u, 0x80091840u, 0x80091848u}) s.Write16(address, 4096u);
    // Leave the template's padding halfword untouched; copy in original groups.
    for (uint32_t offset = 0; offset < 32u; offset += 8u) {
        const uint32_t a = s.Read32(0x80091838u + offset);
        const uint32_t b = s.Read32(0x8009183Cu + offset);
        s.Write32(0x800928A8u + offset, a);
        s.Write32(0x800928ACu + offset, b);
    }
    CopyMatrixTripleGroups(s, 0x80091700u, 0x80091838u);
    s.Write16(0x80091710u, 0u);
    s.Write16(0x80091708u, 0u);
    s.Write16(0x80091700u, 0u);
    CopyMatrixTripleGroups(s, 0x800917B0u, 0x80091700u);
    for (uint32_t address : {0x8008EEF0u, 0x8008EEF2u, 0x8008EEF4u, 0x8008EEF6u,
                             0x800901C6u, 0x800901C4u, 0x800928D2u})
        s.Write16(address, 0u);
    // MULT/MFHI divides the full signed quotient by 3, then SH truncates.
    s.Write16(0x800928B0u, static_cast<uint16_t>(ratio / 3));
    s.Write16(0x800928D0u, 0u);
    const uint16_t freshWidth = s.Read16(0x800917FCu);
    const uint16_t freshHeight = s.Read16(0x8009182Cu);
    s.Write16(0x800928D4u, freshWidth);
    s.Write16(0x800928D6u, freshHeight);
    InitClearPacket800442F4(s, 0x8008A730u);
    InitClearPacket800442F4(s, 0x8008A740u);
    s.Write32(0x8009658Cu, 1u);
    return 1;
}

int32_t InitGraph8003FB9C(Services& s, uint32_t width, uint32_t height,
    uint32_t flags, uint32_t dither, uint32_t rgb24) {
    width &= 0xFFFFu; height &= 0xFFFFu;
    InitGraphEnvironment8003FC14(s, width, height, flags & 0xFFFFu,
        dither & 0xFFFFu, rgb24 & 0xFFFFu);
    s.CallVoid(0x80040D20u, {});
    s.Write16(0x80096590u, 0u);
    InitGeometry8003FDE4(s, width, height);
    PrStage2GpuDirect::ApplyDrawClip800402E0(s);
    return PrStage2GpuDirect::ApplyFrameOffset800401AC(s);
}

int32_t SetDoubleBufferOffsets80040AE4(Services& s, uint32_t x0, uint32_t y0,
    uint32_t x1, uint32_t y1) {
    const bool software = s.Read16(0x800965A0u) != 0u;
    s.Write16(0x8008ECA8u, static_cast<uint16_t>(x0));
    s.Write16(0x8008ECAAu, static_cast<uint16_t>(x1));
    s.Write16(0x8008ECACu, static_cast<uint16_t>(y0));
    s.Write16(0x8008ECAEu, static_cast<uint16_t>(y1));
    s.Write16(0x8008EEF0u, static_cast<uint16_t>(software ? 0u : x0));
    s.Write16(0x8008EEF2u, static_cast<uint16_t>(software ? 0u : x1));
    s.Write16(0x8008EEF4u, static_cast<uint16_t>(software ? 0u : y0));
    s.Write16(0x8008EEF6u, static_cast<uint16_t>(software ? 0u : y1));
    PrStage2GpuDirect::ApplyDrawClip800402E0(s);
    return PrStage2GpuDirect::ApplyFrameOffset800401AC(s);
}

int32_t SetScreenCenter80040B84(Services& s) {
    const int32_t width = S(s.Read32(0x800917FCu));
    const int32_t height = S(s.Read32(0x8009182Cu));
    s.Write16(0x800901C4u, static_cast<uint16_t>(width / 2));
    s.Write16(0x800901C6u, static_cast<uint16_t>(height / 2));
    PrStage2GpuDirect::ApplyFrameOffset800401AC(s);
    s.Write32(0x80095D04u, 10u);
    s.Write32(0x80095D00u, 0u);
    s.Write32(0x800928A4u, 0x3FFFu);
    return 0x3FFF;
}
}
