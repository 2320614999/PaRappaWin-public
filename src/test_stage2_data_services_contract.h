#pragma once
#include "pr/pr_stage2_data_services.h"
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <vector>

namespace DataContract {
using Args = std::initializer_list<uint32_t>;
uint32_t checks = 0;
void Require(bool ok, const char* message) {
    ++checks;
    if (!ok) throw std::runtime_error(message);
}
template<class F> void Reject(F operation) {
    bool rejected = false;
    try { operation(); } catch (const std::exception&) { rejected = true; }
    Require(rejected, "invalid cold-data operation succeeded");
}
struct Probe : PrStage2DataServices::Services {
    explicit Probe(const std::filesystem::path& p) : Services(p) {}
    PrPsxGteDirect::MatrixRegisters gte{};
    uint8_t ReadDevice8(uint32_t) override { throw std::runtime_error("unbound device8"); }
    uint16_t ReadDevice16(uint32_t) override { throw std::runtime_error("unbound device16"); }
    uint32_t ReadDevice32(uint32_t) override { throw std::runtime_error("unbound device32"); }
    void WriteDevice8(uint32_t,uint8_t) override { throw std::runtime_error("unbound device8"); }
    void WriteDevice16(uint32_t,uint16_t) override { throw std::runtime_error("unbound device16"); }
    void WriteDevice32(uint32_t,uint32_t) override { throw std::runtime_error("unbound device32"); }
    int32_t CallExternal(uint32_t,Args) override { throw std::runtime_error("unbound external call"); }
    void CallExternalVoid(uint32_t,Args) override { throw std::runtime_error("unbound external void"); }
    PrStage2LifecycleDirect::Words64 Call64(uint32_t,Args) override { throw std::runtime_error("unbound Call64"); }
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32) override { throw std::runtime_error("unbound normalize"); }
    int32_t SetCdLocation800367A4(uint32_t) override { throw std::runtime_error("unbound CD"); }
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect,uint32_t) override { throw std::runtime_error("unbound upload"); }
    PrPsxGteDirect::MatrixRegisters& MatrixGte() override { return gte; }
    [[noreturn]] void Exit(uint32_t,Args) override { throw std::runtime_error("unbound exit"); }
    [[noreturn]] void Break(uint32_t,uint32_t) override { throw std::runtime_error("unbound break"); }
};
struct InjectedWriteFailure : std::runtime_error {
    InjectedWriteFailure() : std::runtime_error("injected clear write failure") {}
};
struct Trace : PrStage2LifecycleDirect::Services {
    uint32_t count=0, first=0, last=0, failAfter=std::numeric_limits<uint32_t>::max();
    uint64_t hash=14695981039346656037ull;
    void Write32(uint32_t a,uint32_t v) override {
        if (count==failAfter) throw InjectedWriteFailure{};
        if (count>=400000u) throw std::runtime_error("clear loop did not terminate");
        if (count==0u) first=a;
        ++count; last=a;
        for (uint32_t word : {a,4u,v}) for (uint32_t i=0;i<4u;++i)
            hash=(hash^uint8_t(word>>(i*8u)))*1099511628211ull;
    }
    uint8_t Read8(uint32_t) override { throw std::runtime_error("clear unexpectedly read RAM"); }
    uint16_t Read16(uint32_t) override { throw std::runtime_error("clear unexpectedly read RAM"); }
    uint32_t Read32(uint32_t) override { throw std::runtime_error("clear unexpectedly read RAM"); }
    void Write8(uint32_t,uint8_t) override { throw std::runtime_error("clear lost word access width"); }
    void Write16(uint32_t,uint16_t) override { throw std::runtime_error("clear lost word access width"); }
    int32_t Call(uint32_t,Args) override { throw std::runtime_error("clear unexpectedly called dependency"); }
    PrStage2LifecycleDirect::Words64 Call64(uint32_t,Args) override { throw std::runtime_error("unbound Call64"); }
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32) override { throw std::runtime_error("unbound normalize"); }
    int32_t SetCdLocation800367A4(uint32_t) override { throw std::runtime_error("unbound CD"); }
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect,uint32_t) override { throw std::runtime_error("unbound upload"); }
    PrPsxGteDirect::MatrixRegisters& MatrixGte() override { throw std::runtime_error("clear touched GTE"); }
    [[noreturn]] void Exit(uint32_t,Args) override { throw std::runtime_error("clear unexpectedly exited"); }
    [[noreturn]] void Break(uint32_t,uint32_t) override { throw std::runtime_error("clear unexpectedly trapped"); }
};
std::vector<uint8_t> Read(const std::filesystem::path& path) {
    std::ifstream input(path,std::ios::binary);
    if (!input) throw std::runtime_error("test input missing");
    return {std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>()};
}
void Write(const std::filesystem::path& path,const std::vector<uint8_t>& data) {
    std::ofstream output(path,std::ios::binary|std::ios::trunc);
    output.write(reinterpret_cast<const char*>(data.data()),static_cast<std::streamsize>(data.size()));
    if (!output) throw std::runtime_error("test file write failed");
}
void Run(const std::filesystem::path& scus,const std::filesystem::path& scratch) {
    Trace trace;
    PrStage2DataServices::ClearScusBss80028590(trace);
    Require(trace.count==348910u && trace.first==0x8006ECB8u && trace.last==0x801C386Cu,
            "clear word count or endpoint mismatch");
    std::cout<<"bss-trace "<<trace.count<<' '<<trace.first<<' '<<trace.last<<' '<<trace.hash<<'\n';
    for (uint32_t index : {0u,1u,348909u}) {
        Trace broken; broken.failAfter=index;
        bool propagated=false;
        try { PrStage2DataServices::ClearScusBss80028590(broken); }
        catch (const InjectedWriteFailure&) { propagated=true; }
        Require(propagated && broken.count==index,"clear retried or swallowed write failure");
    }
    const auto raw=Read(scus);
    Require(raw.size()==391168u,"test original file length");
    Probe memory(scus), independent(scus);
    // Expectations are independent of production constants. The verifier also
    // binds the full original file and interprets the nine original loop words.
    uint64_t loadedHash=14695981039346656037ull;
    for (uint32_t address=0x80010000u;address<0x801C3870u;++address) {
        const auto actual=memory.Read8(address);
        const uint8_t expected=address<0x8006ECB8u ? raw[0x800u+address-0x80010000u] : 0u;
        Require(actual==expected,"cold data byte differs from payload/BSS ownership");
        loadedHash=(loadedHash^actual)*1099511628211ull;
    }
    uint32_t aliases=0;
    for (uint32_t segment : {0u,0x80000000u,0xA0000000u})
        for (uint32_t mirror : {0u,0x200000u,0x400000u,0x600000u}) {
            const auto a=segment+mirror;
            for (uint32_t offset : {0x10000u,0x55F78u,0x57000u,0x5D82Cu,0x6ECB4u,0x6ECB8u,0x1C386Cu})
                Require(memory.Read32(a+offset)==memory.Read32(0x80000000u+offset),"cold alias data differs");
            for (uint32_t offset : {0u,0xA0u,0x100u,0xFFFCu,0x1C3870u,0x1FFFFCu})
                Reject([&] { memory.Read32(a+offset); });
            ++aliases;
        }
    memory.Write32(0x80010000u,0x11223344u);
    Require(independent.Read32(0x80010000u)!=memory.Read32(0x80010000u),"cold instances share RAM");
    memory.Call(0x8001C1E8u,{});
    Require(memory.Read32(0x8008EDE0u)==0x8003B9C8u,"native dispatcher did not use cold RAM");
    const auto before=Read(scus);
    Require(before==raw,"loading changed original file");
    std::filesystem::create_directories(scratch);
    uint32_t invalid=0;
    const auto missing=scratch/"missing-scus";
    Require(!std::filesystem::exists(missing),"test missing path already exists");
    Reject([&] { Probe rejected(missing); }); ++invalid;
    const auto bad=scratch/"invalid-scus.bin";
    for (size_t size : {size_t(0),size_t(7),size_t(2047),size_t(2048),raw.size()-1u,raw.size()+1u}) {
        auto data=raw; data.resize(size); Write(bad,data);
        Reject([&] { Probe rejected(bad); }); ++invalid;
    }
    for (size_t offset : {size_t(0),size_t(7),size_t(0x10),size_t(0x14),size_t(0x18),size_t(0x1C),
                         size_t(0x20),size_t(0x24),size_t(0x28),size_t(0x2C),size_t(0x30),size_t(0x34)}) {
        auto data=raw; data[offset]^=0x80u; Write(bad,data);
        Reject([&] { Probe rejected(bad); }); ++invalid;
    }
    for (size_t index=0;index<9u;++index) {
        auto data=raw; data[0x800u+0x18590u+4u*index]^=0x40u; Write(bad,data);
        Reject([&] { Probe rejected(bad); }); ++invalid;
    }
    // Header-valid data changes are actually loaded, not silently replaced by
    // cached original bytes. This is a structural validator, not a whole-file
    // cryptographic authentication boundary.
    auto altered=raw; altered[0x800u]^=0x41u;
    const auto unicode=scratch/std::filesystem::u8path(u8"原始数据 副本.bin");
    Write(unicode,altered); Probe changed(unicode);
    Require(changed.Read8(0x80010000u)==altered[0x800u],"loader cached or ignored source data");
    Require(Read(unicode)==altered && Read(scus)==raw,"loader changed a disk source");
    std::cout<<"cold-data "<<0x1C3870u-0x10000u<<' '<<loadedHash<<' '<<aliases<<' '<<invalid<<'\n';
    std::cout<<"data-contract "<<checks<<'\n';
}
}
