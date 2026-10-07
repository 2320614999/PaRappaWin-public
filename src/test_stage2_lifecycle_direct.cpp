#include "pr/pr_stage2_lifecycle_direct.h"
#include "pr/pr_stage2_int_loader_direct.h"
#include "pr/pr_stage2_tim_direct.h"
#include "pr/pr_stage2_gpu_direct.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>

// Receives an independent original-instruction trace from the Python verifier.
// No game process, asset writes, or memory-card backend is linked here.
struct Mutation { uint32_t address, width, value; };
struct NativeExit {};
struct NativeBreak {};
struct ObservedUnsupportedPrimitive {};
struct ObservedReferenceAddressLimit { uint32_t address, width; };
struct CallReceipt {
    uint32_t function;
    int32_t result;
    uint32_t resultHigh;
    PrStage2LifecycleDirect::Vector32 vectorResult;
    PrStage2LifecycleDirect::Matrix32 matrixResult;
    uint32_t localCountResult;
    std::vector<uint32_t> arguments;
    std::vector<Mutation> mutations;
};
struct TestServices : PrStage2LifecycleDirect::Services {
    std::map<uint32_t, uint8_t> memory;
    std::vector<CallReceipt> calls;
    size_t cursor = 0;
    bool stopOnUnsupportedPrimitive = false;
    bool stopOnReferenceAddressLimit = false;
    bool traceIo = false;
    std::map<uint32_t,std::vector<uint32_t>> ioSequences;
    std::map<uint32_t,size_t> ioPositions;
    PrPsxGteDirect::MatrixRegisters gte;
    PrPsxGteDirect::MatrixRegisters& MatrixGte() override { return gte; }
    void PrintGte() const {
        std::cout << "matrix-gte";
        for (auto v : gte.matrix.words) std::cout << ' ' << v;
        std::cout << ' ' << gte.vectorXY0 << ' ' << static_cast<uint32_t>(gte.vectorZ0);
        for (auto v : gte.ir) std::cout << ' ' << static_cast<uint32_t>(v);
        for (auto v : gte.mac) std::cout << ' ' << static_cast<uint32_t>(v);
        std::cout << ' ' << gte.flags;
        for (size_t i = 0; i < 2; ++i)
            std::cout << ' ' << gte.vectorXY12[i] << ' ' << static_cast<uint32_t>(gte.vectorZ12[i]);
        for (auto v : gte.rgb) std::cout << ' ' << v;
        for (auto v : gte.sxy) std::cout << ' ' << v;
        for (auto v : gte.sz) std::cout << ' ' << v;
        std::cout << ' ' << static_cast<uint32_t>(gte.mac0) << ' ' << static_cast<uint32_t>(gte.ir0)
                  << ' ' << gte.otz << ' ' << static_cast<uint32_t>(gte.ofx) << ' ' << static_cast<uint32_t>(gte.ofy)
                  << ' ' << static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(gte.h)))
                  << ' ' << static_cast<uint32_t>(static_cast<int32_t>(gte.dqa)) << ' ' << static_cast<uint32_t>(gte.dqb)
                  << ' ' << static_cast<uint32_t>(static_cast<int32_t>(gte.zsf3))
                  << ' ' << static_cast<uint32_t>(static_cast<int32_t>(gte.zsf4)) << '\n';
    }
    void Put(Mutation m) {
        for (uint32_t i = 0; i < m.width; ++i)
            memory[m.address + i] = static_cast<uint8_t>(m.value >> (i * 8));
    }
    uint32_t Read(uint32_t a, uint32_t n) {
        if (stopOnReferenceAddressLimit &&
            !(a >= 0x80000000u && a <= 0x80200000u - n) &&
            !(a >= 0x1F800000u && a <= 0x1F810000u - n))
            throw ObservedReferenceAddressLimit{a,n};
        uint32_t v = 0;
        for (uint32_t i = 0; i < n; ++i) {
            const auto it = memory.find(a + i);
            if (it == memory.end()) { std::cerr << "unseeded read " << a << '\n'; std::exit(2); }
            v |= uint32_t(it->second) << (8 * i);
        }
        const auto sequence=ioSequences.find(a);
        if (sequence!=ioSequences.end()) {
            if (n!=4u) { std::cerr<<"wrong MMIO sequence width"; std::exit(2); }
            const size_t index=ioPositions[a]++;
            v=sequence->second[index<sequence->second.size()?index:sequence->second.size()-1u];
        }
        if (traceIo && a>=0x1F800000u && a<=0x1F810000u-n)
            std::cout<<"read "<<a<<' '<<n<<' '<<v<<'\n';
        return v;
    }
    uint8_t Read8(uint32_t a) override { return static_cast<uint8_t>(Read(a, 1)); }
    uint16_t Read16(uint32_t a) override { return static_cast<uint16_t>(Read(a, 2)); }
    uint32_t Read32(uint32_t a) override { return Read(a, 4); }
    uint32_t ReadPad80035510() override {
        // The instruction reference stops at the original 80035510 leaf.
        // Bind the native PAD seam at that same boundary; the lower 80048A00
        // snapshot is not part of this isolated lifecycle comparison.
        return static_cast<uint32_t>(Call(0x80035510u, {1u}));
    }
    void Write8(uint32_t a, uint8_t v) override {
        Put({a, 1, v}); std::cout << "write " << a << " 1 " << uint32_t(v) << '\n';
    }
    void Write16(uint32_t a, uint16_t v) override {
        Put({a, 2, v}); std::cout << "write " << a << " 2 " << v << '\n';
    }
    void Write32(uint32_t a, uint32_t v) override {
        Put({a, 4, v}); std::cout << "write " << a << " 4 " << v << '\n';
    }
    int32_t Call(uint32_t f, std::initializer_list<uint32_t> args) override {
        if (cursor >= calls.size()) { std::cerr << "unexpected call " << f; std::exit(3); }
        const auto& expected = calls[cursor++];
        if (f != expected.function || std::vector<uint32_t>(args) != expected.arguments) {
            std::cerr << "call mismatch #" << cursor << " actual=" << f
                      << " expected=" << expected.function << '\n';
            for (auto a : args) std::cerr << a << ' ';
            std::exit(4);
        }
        std::cout << "call " << f;
        for (auto a : args) std::cout << ' ' << a;
        std::cout << '\n';
        for (auto m : expected.mutations) Put(m);
        if (stopOnUnsupportedPrimitive && f == 0x80047FFCu &&
            args.size() == 3u && *args.begin() == 0x80012204u)
            throw ObservedUnsupportedPrimitive{};
        return expected.result;
    }
    [[noreturn]] void Exit(uint32_t f, std::initializer_list<uint32_t> args) override {
        Call(f, args);
        throw NativeExit{};
    }
    [[noreturn]] void Break(uint32_t instruction, uint32_t code) override {
        PrintGte();
        std::cout << "break " << instruction << ' ' << code << '\n';
        throw NativeBreak{};
    }
    PrStage2LifecycleDirect::Words64 Call64(uint32_t f, std::initializer_list<uint32_t> args) override {
        const uint32_t low = static_cast<uint32_t>(Call(f, args));
        return {low, calls[cursor - 1].resultHigh};
    }
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32 input) override {
        Call(0x8003A3DCu, {input.x, input.y, input.z});
        return calls[cursor - 1].vectorResult;
    }
    int32_t SetCdLocation800367A4(uint32_t location) override {
        return Call(0x800367A4u,{2u,location,0u});
    }
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect r,uint32_t source) override {
        return Call(0x80044D64u,{r.x,r.y,r.width,r.height,source});
    }
};

// Compatibility check against the existing three-vertex projection API.
// Both use shared arithmetic: this is a regression check, not hardware proof.
int CheckProjectionRegisters() {
    using namespace PrPsxGteDirect;
    uint32_t seed = 0x8003B88Cu;
    const auto random = [&]() { seed = seed * 1664525u + 1013904223u; return seed; };
    for (uint32_t trial = 0; trial < 4096u; ++trial) {
        MatrixRegisters g{};
        for (auto& word : g.matrix.words) word = random();
        if (trial < 2048u) g.matrix.words = {{0x1000u,0u,0x1000u,0u,0x1000u,
            random()%201u-100u,random()%201u-100u,random()%4000u}};
        g.ofx = static_cast<int32_t>(random()); g.ofy = static_cast<int32_t>(random());
        g.h = static_cast<uint16_t>(random()); g.dqa = static_cast<int16_t>(random());
        g.dqb = static_cast<int32_t>(random());
        Rtpt3ExactInput280030 input{};
        input.matrixKnown = input.controlKnown = input.verticesKnown = true;
        input.matrix = g.matrix;
        input.control.geomScreenKnown = input.control.geomOffsetKnown = input.control.depthCueKnown = true;
        input.control.geomScreen = g.h; input.control.geomOffsetX = g.ofx; input.control.geomOffsetY = g.ofy;
        input.control.depthCueA = g.dqa; input.control.depthCueB = g.dqb;
        for (uint32_t i = 0; i < 3u; ++i) {
            const uint32_t xy = random(); const auto z = static_cast<int16_t>(random());
            input.vertices[i] = {static_cast<int16_t>(xy),static_cast<int16_t>(xy>>16u),z,0};
            if (i == 0u) { g.vectorXY0 = xy; g.vectorZ0 = z; }
            else { g.vectorXY12[i-1u] = xy; g.vectorZ12[i-1u] = z; }
        }
        const auto previous = ExecuteRtpt3Exact280030(input);
        ExecutePerspective(g,true);
        if (!previous.known || previous.flagAfterRtpt != g.flags || previous.ir0 != g.ir0) return 8;
        for (uint32_t i = 0; i < 3u; ++i)
            if (previous.sxy[i].word != g.sxy[i] || previous.szAfterRtpt.sz[i+1u] != g.sz[i+1u]) return 8;
    }
    std::cout << "projection register compatibility: 4096\n";
    return 0;
}

int main(int argc, char** argv) {
    if (argc == 2 && std::strcmp(argv[1],"--projection-register-check") == 0) return CheckProjectionRegisters();
    uint32_t function, entry;
    int32_t scene;
    size_t count;
    if (!(std::cin >> function >> entry >> scene >> count) || count > 4000000u) return 1;
    TestServices s;
    for (size_t i = 0; i < count; ++i) {
        Mutation m{};
        if (!(std::cin >> m.address >> m.width >> m.value) || m.width == 0u || m.width > 4u) return 1;
        s.Put(m);
    }
    if (!(std::cin >> count) || count > 4000000u) return 1;
    for (size_t i = 0; i < count; ++i) {
        CallReceipt c{}; size_t arguments = 0, mutations = 0;
        if (!(std::cin >> c.function >> c.result >> c.resultHigh
                      >> c.vectorResult.x >> c.vectorResult.y >> c.vectorResult.z)) return 1;
        for (auto& word : c.matrixResult.words) if (!(std::cin >> word)) return 1;
        if (!(std::cin >> c.localCountResult >> arguments) || arguments > 16u) return 1;
        for (size_t j = 0; j < arguments; ++j) {
            uint32_t a = 0;
            if (!(std::cin >> a)) return 1;
            c.arguments.push_back(a);
        }
        if (!(std::cin >> mutations) || mutations > 4000000u) return 1;
        for (size_t j = 0; j < mutations; ++j) {
            Mutation m{};
            if (!(std::cin >> m.address >> m.width >> m.value) || m.width == 0u || m.width > 4u) return 1;
            c.mutations.push_back(m);
        }
        s.calls.push_back(c);
    }
    size_t sequenceCount = 0;
    if (!(std::cin >> sequenceCount) || sequenceCount > 1000000u ||
        (sequenceCount != 0u && function != 0x8001B000u)) return 1;
    std::vector<uint32_t> sequenceFrames(sequenceCount);
    for (auto& frame : sequenceFrames) if (!(std::cin >> frame)) return 1;
    uint32_t observations = 0;
    if (!(std::cin >> observations) || observations > 7u ||
        ((observations & 1u) && function != 0x8001AF1Cu)) return 1;
    const bool bindThenDraw = (observations & 1u) != 0u;
    s.stopOnUnsupportedPrimitive = (observations & 2u) != 0u;
    s.stopOnReferenceAddressLimit = (observations & 4u) != 0u;
    uint32_t gteInitial[41];
    for (auto& value : gteInitial) if (!(std::cin >> value)) return 1;
    for (size_t i = 0; i < 8; ++i) s.gte.matrix.words[i] = gteInitial[i];
    s.gte.vectorXY0 = gteInitial[8]; s.gte.vectorZ0 = static_cast<int32_t>(gteInitial[9]);
    for (size_t i = 0; i < 3; ++i) {
        s.gte.ir[i] = static_cast<int32_t>(gteInitial[10 + i]);
        s.gte.mac[i] = static_cast<int32_t>(gteInitial[13 + i]);
    }
    s.gte.flags = gteInitial[16];
    for (size_t i = 0; i < 2; ++i) {
        s.gte.vectorXY12[i] = gteInitial[17 + 2*i];
        s.gte.vectorZ12[i] = static_cast<int32_t>(gteInitial[18 + 2*i]);
    }
    for (size_t i = 0; i < 3; ++i) { s.gte.rgb[i] = gteInitial[21+i]; s.gte.sxy[i] = gteInitial[24+i]; }
    for (size_t i = 0; i < 4; ++i) s.gte.sz[i] = static_cast<uint16_t>(gteInitial[27+i]);
    s.gte.mac0 = static_cast<int32_t>(gteInitial[31]); s.gte.ir0 = static_cast<int32_t>(gteInitial[32]);
    s.gte.otz = static_cast<uint16_t>(gteInitial[33]);
    s.gte.ofx = static_cast<int32_t>(gteInitial[34]); s.gte.ofy = static_cast<int32_t>(gteInitial[35]);
    s.gte.h = static_cast<uint16_t>(gteInitial[36]); s.gte.dqa = static_cast<int16_t>(gteInitial[37]);
    s.gte.dqb = static_cast<int32_t>(gteInitial[38]); s.gte.zsf3 = static_cast<int16_t>(gteInitial[39]);
    s.gte.zsf4 = static_cast<int16_t>(gteInitial[40]);
    // Observation-only native INT readback. Expected bytes/hashes/addresses
    // are never supplied to C++; resolve slots from the actual loaded table.
    size_t residentCount = 0;
    if (!(std::cin >> residentCount) || residentCount > 1024u ||
        (residentCount && function != 0x8001A8F0u && function != 0x8001AC18u)) return 1;
    std::vector<std::pair<uint32_t,uint32_t>> residentRegions;
    uint32_t residentBytes = 0;
    for (size_t i = 0; i < residentCount; ++i) {
        uint32_t slot, length;
        if (!(std::cin >> slot >> length) || slot == 0u || slot > 1024u ||
            length > 0x200000u - residentBytes) return 1;
        residentRegions.emplace_back(slot,length);
        residentBytes += length;
    }
    uint32_t ioTrace=0,ioCount=0;
    if (!(std::cin>>ioTrace>>ioCount) || ioTrace>1u || ioCount>1024u) return 1;
    s.traceIo=ioTrace!=0u;
    for (uint32_t i=0;i<ioCount;++i) {
        uint32_t address=0,length=0;
        if (!(std::cin>>address>>length) || address<0x1F800000u || address>0x1F80FFFCu ||
            length==0u || length>4096u || s.ioSequences.count(address)) return 1;
        auto& sequence=s.ioSequences[address]; sequence.resize(length);
        for (auto& value:sequence) if (!(std::cin>>value)) return 1;
    }
    if (!std::cin) return 5;
    int32_t result = 0;
    const auto arg = [&](uint32_t index) { return s.Read32(entry + 4u * index); };
    try { switch (function) {
    case 0x80043EBCu: result = PrStage2TimDirect::GetClut80043EBC(arg(0),arg(1)); break;
    case 0x800431E0u: result = PrStage2TimDirect::UploadClut800431E0(s,arg(0),arg(1),arg(2)); break;
    case 0x8001ADECu: result = PrStage2TimDirect::UploadRuntimeTim8001ADEC(s,arg(0),arg(1)); break;
    case 0x80040EACu: result = PrStage2TimDirect::GetTimInfo80040EAC(s,arg(0),arg(1)); break;
    case 0x8001AE7Cu: result = PrStage2TimDirect::UploadTim8001AE7C(s,arg(0)); break;
    case 0x80036A78u: result = PrStage2IntLoaderDirect::LocationToLba80036A78(s,arg(0)); break;
    case 0x80036974u: result = PrStage2IntLoaderDirect::LbaToLocation80036974(s,arg(0),arg(1)); break;
    case 0x8001A89Cu: result = PrStage2IntLoaderDirect::SeekFile8001A89C(s,arg(0),arg(1)); break;
    case 0x8001A2B0u: result = PrStage2IntLoaderDirect::FindFile8001A2B0(s,arg(0),arg(1)); break;
    case 0x8001A324u: result = PrStage2IntLoaderDirect::OpenFile8001A324(s,arg(0)); break;
    case 0x800450A0u: result = PrStage2GpuDirect::DrawOrderingTable800450A0(s,arg(0)); break;
    case 0x80046840u: result = PrStage2GpuDirect::StartLinkedDma80046840(s,arg(0)); break;
    case 0x800468E0u: result = PrStage2GpuDirect::Enqueue800468E0(s,arg(0),arg(1),arg(2),arg(3)); break;
    case 0x80046BC4u: result = PrStage2GpuDirect::DrainQueue80046BC4(s); break;
    case 0x80047144u: result = PrStage2GpuDirect::ResetTimeout80047144(s); break;
    case 0x80047178u: result = PrStage2GpuDirect::CheckTimeout80047178(s); break;
    case 0x80044B3Cu: result = PrStage2GpuDirect::DrawSync80044B3C(s,arg(0)); break;
    case 0x80046FFCu: result = PrStage2GpuDirect::SyncQueue80046FFC(s,arg(0)); break;
    case 0x80044FA8u: result = PrStage2GpuDirect::ClearOrderingTable80044FA8(s,arg(0),arg(1)); break;
    case 0x80045FC4u: result = PrStage2GpuDirect::StartOtcDma80045FC4(s,arg(0),arg(1)); break;
    case 0x800460ACu: result = PrStage2GpuDirect::ClearImage800460AC(s,arg(0),arg(1)); break;
    case 0x8004688Cu: result = PrStage2GpuDirect::ReadGpuInfo8004688C(s,arg(0)); break;
    case 0x8004019Cu: result = PrStage2GpuDirect::CurrentDrawBuffer8004019C(s); break;
    case 0x80040F90u: PrStage2GpuDirect::SetWorkBase80040F90(s,arg(0)); break;
    case 0x80040370u: result = PrStage2GpuDirect::SwapBuffers80040370(s); break;
    case 0x80040C74u: PrStage2GpuDirect::SetProjection80040C74(s,arg(0)); break;
    case 0x800452ECu: result = PrStage2GpuDirect::PutDisplayEnvironment800452EC(s,arg(0)); break;
    case 0x80044AA0u: result = PrStage2GpuDirect::SetDisplayMask80044AA0(s,arg(0)); break;
    case 0x800402E0u: result = PrStage2GpuDirect::ApplyDrawClip800402E0(s); break;
    case 0x800401ACu: result = PrStage2GpuDirect::ApplyFrameOffset800401AC(s); break;
    case 0x8003623Cu: result = PrStage2GpuDirect::VideoMode8003623C(s); break;
    case 0x800402C0u: PrStage2GpuDirect::SetGeomOffset800402C0(s,arg(0),arg(1)); break;
    case 0x80040C94u: PrStage2GpuDirect::SetGeomScreen80040C94(s,arg(0)); break;
    case 0x80045114u: result = PrStage2GpuDirect::PutDrawEnvironment80045114(s,arg(0)); break;
    case 0x80045EF0u: result = PrStage2GpuDirect::DisplayX80045EF0(s,arg(0)); break;
    case 0x800467B4u: result = PrStage2GpuDirect::WriteGpuControl800467B4(s,arg(0)); break;
    case 0x800473C0u: result = PrStage2GpuDirect::FillBytes800473C0(s,arg(0),arg(1),arg(2)); break;
    case 0x8004598Cu: result = PrStage2GpuDirect::BuildDrawEnvironment8004598C(s,arg(0),arg(1)); break;
    case 0x80045C8Cu: result = PrStage2GpuDirect::PackDrawAreaStart80045C8C(s,arg(0),arg(1)); break;
    case 0x80045D58u: result = PrStage2GpuDirect::PackDrawAreaEnd80045D58(s,arg(0),arg(1)); break;
    case 0x80045E24u: result = PrStage2GpuDirect::PackDrawOffset80045E24(s,arg(0),arg(1)); break;
    case 0x80045C30u: result = PrStage2GpuDirect::PackDrawMode80045C30(s,arg(0),arg(1),arg(2)); break;
    case 0x80045E6Cu: result = PrStage2GpuDirect::PackTextureWindow80045E6C(s,arg(0)); break;
    case 0x8001AC18u: result = PrStage2IntLoaderDirect::LoadInt8001AC18(s,arg(0),arg(1)); break;
    case 0x80025BFCu: result = PrStage2IntLoaderDirect::FindHeapEntry80025BFC(s,arg(0)); break;
    case 0x80025BBCu: result = PrStage2IntLoaderDirect::SplitHeapEntry80025BBC(s,arg(0),arg(1),arg(2)); break;
    case 0x8001A818u: result = PrStage2IntLoaderDirect::ReadSectors8001A818(s,arg(0),arg(1),arg(2)); break;
    case 0x8001A8F0u: result = PrStage2IntLoaderDirect::ParseInt8001A8F0(s,arg(0),arg(1),arg(2)); break;
    case 0x8003B88Cu: result = PrStage2LifecycleDirect::DrawNf3_8003B88C(s,arg(0),arg(1),arg(2),arg(3),arg(4),arg(5)); break;
    case 0x8003CFDCu: result = PrStage2LifecycleDirect::DrawTnf3_8003CFDC(s,arg(0),arg(1),arg(2),arg(3),arg(4),arg(5)); break;
    case 0x8003BD9Cu: result = PrStage2LifecycleDirect::DrawNf4_8003BD9C(s,arg(0),arg(1),arg(2),arg(3),arg(4),arg(5)); break;
    case 0x8003D58Cu: result = PrStage2LifecycleDirect::DrawTnf4_8003D58C(s,arg(0),arg(1),arg(2),arg(3),arg(4),arg(5)); break;
    case 0x8003C36Cu: result = PrStage2LifecycleDirect::DrawNg3_8003C36C(s,arg(0),arg(1),arg(2),arg(3),arg(4),arg(5)); break;
    case 0x8003DC2Cu: result = PrStage2LifecycleDirect::DrawTng3_8003DC2C(s,arg(0),arg(1),arg(2),arg(3),arg(4),arg(5)); break;
    case 0x80041A68u: result = PrStage2LifecycleDirect::ResolveWorldMatrix80041A68(s, arg(0), arg(1)); break;
    case 0x8004075Cu: result = PrStage2LifecycleDirect::ComposeMatrixLeft8004075C(s, arg(0), arg(1)); break;
    case 0x800406D8u: result = PrStage2LifecycleDirect::ComposeMatrixRight800406D8(s, arg(0), arg(1)); break;
    case 0x80040884u: result = PrStage2LifecycleDirect::MultiplyMatrixLeft80040884(s, arg(0), arg(1)); break;
    case 0x80040994u: result = PrStage2LifecycleDirect::MultiplyMatrixRight80040994(s, arg(0), arg(1)); break;
    case 0x8003ABA4u: result = PrStage2LifecycleDirect::ApplyMatrixLongVector8003ABA4(s, arg(0), arg(1), arg(2)); break;
    case 0x8003F6B0u: PrStage2LifecycleDirect::LoadRotation8003F6B0(s, arg(0)); break;
    case 0x8003F6E0u: PrStage2LifecycleDirect::LoadTranslation8003F6E0(s, arg(0)); break;
    case 0x80040544u: PrStage2LifecycleDirect::LoadGteMatrix80040544(s, arg(0)); break;
    case 0x8001B084u: PrStage2LifecycleDirect::DrawModels8001B084(s, arg(0), static_cast<int32_t>(arg(1)), arg(2), static_cast<int32_t>(arg(3))); break;
    case 0x800428B0u: PrStage2LifecycleDirect::SubmitModel800428B0(s, arg(0), arg(1), arg(2), arg(3)); break;
    case 0x80028054u: result = PrStage2LifecycleDirect::ApplyTodCommand80028054(s, arg(0), arg(1)); break;
    case 0x8003B3CCu: result = PrStage2LifecycleDirect::RotateMatrixY8003B3CC(s, arg(0), arg(1)); break;
    case 0x8003B22Cu: result = PrStage2LifecycleDirect::RotateMatrixX8003B22C(s, arg(0), arg(1)); break;
    case 0x8003B56Cu: result = PrStage2LifecycleDirect::RotateMatrixZ8003B56C(s, arg(0), arg(1)); break;
    case 0x8003B0FCu: result = PrStage2LifecycleDirect::ScaleMatrix8003B0FC(s, arg(0), arg(1)); break;
    case 0x8003B0CCu: result = PrStage2LifecycleDirect::TranslateMatrix8003B0CC(s, arg(0), arg(1)); break;
    case 0x80040BFCu: result = PrStage2LifecycleDirect::MapModelData80040BFC(s, arg(0)); break;
    case 0x8004274Cu: result = PrStage2LifecycleDirect::GroupModelPrimitives8004274C(s, arg(0), arg(1), arg(2)); break;
    case 0x8001AF1Cu:
        result = PrStage2LifecycleDirect::BindModel8001AF1C(s, arg(0), arg(1), arg(2));
        if (bindThenDraw)
            PrStage2LifecycleDirect::DrawModels8001B084(s, arg(1), result, 0x801D29A8u, 10);
        break;
    case 0x8001AFD8u: result = PrStage2LifecycleDirect::InitAnimationCursor8001AFD8(s, arg(0), arg(1), arg(2)); break;
    case 0x8004049Cu: result = PrStage2LifecycleDirect::InitCoordinate8004049C(s, arg(0), arg(1)); break;
    case 0x80028504u: result = PrStage2LifecycleDirect::ApplyAnimationBlock80028504(s, arg(0), arg(1), arg(2), arg(3)); break;
    case 0x8001B000u:
        if (sequenceFrames.empty()) {
            result = PrStage2LifecycleDirect::AdvanceAnimation8001B000(s, arg(0), arg(1), arg(2), arg(3));
        } else {
            const uint32_t countAddress = arg(1), cursorAddress = arg(2), descriptor = arg(3);
            for (uint32_t frame : sequenceFrames) {
                result = PrStage2LifecycleDirect::AdvanceAnimation8001B000(s, frame, countAddress, cursorAddress, descriptor);
                s.PrintGte();
                std::cout << "return " << result << '\n';
            }
        }
        break;
    case 0x8001E33Cu: PrStage2LifecycleDirect::BindDrawBuffers8001E33C(s, arg(0), arg(1)); break;
    case 0x8001EEACu: result = PrStage2LifecycleDirect::FillDrawFlags8001EEAC(s, arg(0)); break;
    case 0x8001EEE8u: result = PrStage2LifecycleDirect::ResetDrawFlags8001EEE8(s); break;
    case 0x80040CC8u: result = PrStage2LifecycleDirect::ClearOrderingTable80040CC8(s, arg(0), arg(1), arg(2)); break;
    case 0x80040CA4u: result = PrStage2LifecycleDirect::SubmitOrderingTable80040CA4(s, arg(0)); break;
    case 0x8001E374u: result = PrStage2LifecycleDirect::ClearWorkOrderingTable8001E374(s, arg(0)); break;
    case 0x8001E3B0u: result = PrStage2LifecycleDirect::SubmitWorkOrderingTable8001E3B0(s, arg(0)); break;
    case 0x8004401Cu: result = PrStage2LifecycleDirect::LinkPrimitive8004401C(s, arg(0), arg(1)); break;
    case 0x80040060u: result = PrStage2LifecycleDirect::EnqueueClearPrimitive80040060(s, arg(0), arg(1), arg(2), arg(3)); break;
    case 0x80025A00u: result = PrStage2LifecycleDirect::ResetHeapPointers80025A00(s); break;
    case 0x80025A34u: result = PrStage2LifecycleDirect::ResetResourceHeap80025A34(s); break;
    case 0x80025A70u: result = PrStage2LifecycleDirect::PushResourceHeap80025A70(s, arg(0)); break;
    case 0x80025AF8u: result = PrStage2LifecycleDirect::PopResourceHeap80025AF8(s); break;
    case 0x80025B28u: result = PrStage2LifecycleDirect::AllocatePacketHeap80025B28(s, arg(0)); break;
    case 0x80030544u: result = PrStage2LifecycleDirect::AllocateVoice80030544(s); break;
    case 0x80030C90u: result = PrStage2LifecycleDirect::PrepareVoiceRegisters80030C90(s); break;
    case 0x800315C8u: result = PrStage2LifecycleDirect::ComputePitch800315C8(s, arg(0), arg(1)); break;
    case 0x800307ACu: result = PrStage2LifecycleDirect::SetRegularVoice800307AC(s, arg(0), arg(1)); break;
    case 0x80030EA4u: result = PrStage2LifecycleDirect::SetNoiseVoice80030EA4(s, arg(0)); break;
    case 0x800353E8u: result = PrStage2LifecycleDirect::SetNoiseMask800353E8(s, arg(0), arg(1)); break;
    case 0x8003540Cu: result = PrStage2LifecycleDirect::UpdateRegisterMask8003540C(s, arg(0), arg(1), arg(2), arg(3)); break;
    case 0x80031A28u: result = PrStage2LifecycleDirect::AdvanceVolumeRamp80031A28(s, arg(0)); break;
    case 0x80031F28u: result = PrStage2LifecycleDirect::AdvancePanRamp80031F28(s, arg(0)); break;
    case 0x80032B00u: result = PrStage2LifecycleDirect::CommitAudio80032B00(s); break;
    case 0x80025C64u: result = PrStage2LifecycleDirect::CopyBytes80025C64(s, arg(0), arg(1), static_cast<int32_t>(arg(2))); break;
    case 0x8002F13Cu: result = PrStage2LifecycleDirect::SelectAudioBank8002F13C(s, arg(0), arg(1)); break;
    case 0x800345E4u: result = PrStage2LifecycleDirect::StopVoice800345E4(s, arg(0), arg(1), arg(2), arg(3), arg(4)); break;
    case 0x80034240u: result = PrStage2LifecycleDirect::PlayVoice80034240(s, arg(0), arg(1), arg(2), arg(3), arg(4), arg(5), arg(6)); break;
    case 0x80026FC4u: PrStage2LifecycleDirect::ReplaceCue80026FC4(s, arg(0)); break;
    case 0x80026EF8u: result = PrStage2LifecycleDirect::PlayCue80026EF8(s, arg(0)); break;
    case 0x800351B8u: result = PrStage2LifecycleDirect::ResetVoices800351B8(s); break;
    case 0x80026FA4u: result = PrStage2LifecycleDirect::ResetAudio80026FA4(s); break;
    case 0x8002EFF4u: result = PrStage2LifecycleDirect::FlushAudioDriver8002EFF4(s); break;
    case 0x80026ECCu: result = PrStage2LifecycleDirect::FlushAudio80026ECC(s); break;
    case 0x801C85CCu: result = PrStage2LifecycleDirect::ApplyInputRow801C85CC(s, 0x801C3640u, entry); break;
    case 0x80014614u: result = PrStage2LifecycleDirect::JudgeInput80014614(s, 0x801C3640u); break;
    case 0x80024BF4u: result = PrStage2LifecycleDirect::IsJudgeBlocked80024BF4(s, 0x801C3640u); break;
    case 0x80024B54u: result = PrStage2LifecycleDirect::DecodeInput80024B54(static_cast<uint32_t>(scene)); break;
    case 0x801C7884u: result = PrStage2LifecycleDirect::ResetEventTimeline801C7884(s, 0x801C3640u, static_cast<uint32_t>(scene)); break;
    case 0x801C78FCu: result = PrStage2LifecycleDirect::PublishEventText801C78FC(s, 0x801C3640u, entry); break;
    case 0x801C7A24u: result = PrStage2LifecycleDirect::AdvanceResourceQueue801C7A24(s, 0x801C3640u, static_cast<uint32_t>(scene)); break;
    case 0x801C7B20u: result = PrStage2LifecycleDirect::RestoreSecondResourcePair801C7B20(s, 0x801C3640u); break;
    case 0x801C7BC0u: result = PrStage2LifecycleDirect::RestoreFirstResourcePair801C7BC0(s, 0x801C3640u); break;
    case 0x801C7C54u: result = PrStage2LifecycleDirect::DispatchEventResources801C7C54(s, 0x801C3640u, entry); break;
    case 0x801C870Cu: result = PrStage2LifecycleDirect::UpdateEvents801C870C(s, 0x801C3640u); break;
    case 0x801C9ABCu: result = PrStage2LifecycleDirect::DrawModelsAfterFirst801C9ABC(s, 0x801D5C70u, scene,
        s.Read32(entry + 12u), static_cast<int32_t>(s.Read32(entry + 16u))); break;
    case 0x801C9B5Cu: PrStage2LifecycleDirect::DrawPreparedScene801C9B5C(s); break;
    case 0x801C9E18u: PrStage2LifecycleDirect::AnimateAndDrawScene801C9E18(s); break;
    case 0x801CA57Cu: PrStage2LifecycleDirect::PrepareFrame801CA57C(s, 0x801C3640u, scene); break;
    case 0x801C6858u: result = PrStage2LifecycleDirect::GameClock801C6858(s); break;
    case 0x801C9A00u: result = PrStage2LifecycleDirect::AllocatePackets801C9A00(s); break;
    case 0x801C9A64u: result = PrStage2LifecycleDirect::ConfigureModel801C9A64(s, static_cast<uint32_t>(scene), s.Read32(entry + 12u), entry); break;
    case 0x801CB284u: result = PrStage2LifecycleDirect::InitResources801CB284(s); break;
    case 0x801CB170u: result = PrStage2LifecycleDirect::Present801CB170(s); break;
    case 0x801C9644u: result = PrStage2LifecycleDirect::InputFeedback801C9644(s, 0x801C3640u); break;
    case 0x801C9730u: result = PrStage2LifecycleDirect::SpecialPose801C9730(s, 0x801C3640u); break;
    case 0x801C7958u: result = PrStage2LifecycleDirect::InitDemo801C7958(s); break;
    case 0x801C97ECu: result = PrStage2LifecycleDirect::InitGlobals801C97EC(s); break;
    case 0x801C657Cu: result = PrStage2LifecycleDirect::InitScene801C657C(s, entry, scene); break;
    case 0x801C74E4u: result = PrStage2LifecycleDirect::RunScene801C74E4(s, scene); break;
    case 0x801C6A3Cu: result = PrStage2LifecycleDirect::InitMovie801C6A3C(s, entry, scene); break;
    case 0x801C66C8u: result = PrStage2LifecycleDirect::UpdateMovieClock801C66C8(s, entry, 0x801C3640u); break;
    case 0x801C6804u: result = PrStage2LifecycleDirect::MovieFrame801C6804(s, entry, 0x801C3640u); break;
    case 0x801C6CDCu: result = PrStage2LifecycleDirect::InitGame801C6CDC(s, entry, 0x801C3640u); break;
    case 0x801C6D58u: result = PrStage2LifecycleDirect::RunGame801C6D58(s, entry, 0x801C3640u, scene); break;
    case 0x801C6AB8u: result = PrStage2LifecycleDirect::RunMovie801C6AB8(s, entry, 0x801C3640u, static_cast<uint32_t>(scene)); break;
    default: return 6;
    } } catch (const ObservedReferenceAddressLimit& limit) {
        if (s.cursor != s.calls.size()) return 7;
        s.PrintGte();
        std::cout << "observed reference address limit " << limit.address << ' ' << limit.width << '\n';
        return 0;
    } catch (const ObservedUnsupportedPrimitive&) {
        if (s.cursor != s.calls.size()) return 7;
        s.PrintGte();
        std::cout << "observed unsupported primitive\n";
        return 0;
    } catch (const NativeExit&) {
        if (s.cursor != s.calls.size()) return 7;
        s.PrintGte();
        std::cout << "terminal exit\n";
        return 0;
    } catch (const NativeBreak&) {
        return s.cursor == s.calls.size() ? 0 : 7;
    }
    if (s.cursor != s.calls.size()) return 7;
    if (!sequenceFrames.empty()) return 0;
    for (const auto& region : residentRegions) {
        const uint32_t address = s.Read32(0x80091858u + 4u*region.first);
        if (address < 0x80000000u || address > 0x80200000u-region.second) return 9;
        uint64_t hash = 14695981039346656037ull;
        for (uint32_t i = 0; i < region.second; ++i)
            hash = (hash ^ s.Read8(address+i))*1099511628211ull;
        std::cout << "resident " << region.first << ' ' << address << ' '
                  << region.second << ' ' << hash << '\n';
    }
    if (!residentRegions.empty())
        std::cout << "resident-state " << s.Read32(0x8006EB84u) << ' '
                  << s.Read32(0x8006EDC8u) << '\n';
    s.PrintGte();
    if (function == 0x80026FC4u || function == 0x8001E33Cu || function == 0x8001B084u ||
        function == 0x800428B0u || function == 0x801C9B5Cu || function == 0x801C9E18u ||
        function == 0x801CA57Cu || function == 0x8003F6B0u || function == 0x8003F6E0u ||
        function == 0x80040544u || function == 0x80040F90u || function == 0x80040C74u ||
        function == 0x80040C94u || function == 0x800402C0u) std::cout << "return void\n";
    else std::cout << "return " << result << '\n';
    return 0;
}
