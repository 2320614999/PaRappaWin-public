#pragma once
#include "pr/pr_stage2_int_loader_direct.h"
#include "pr/pr_stage2_disc_file_device.h"
#include "pr/pr_stage2_vram_device.h"
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

// Isolated disk integration test. CD lookup/seek/read use the real source
// module and raw BIN. TIM writes actual VRAM; SPU/VBlank remain observations;
// this executable never starts the game or links a memory-card backend.
struct DiskServices : PrStage2LifecycleDirect::Services {
    std::vector<uint8_t> ram = std::vector<uint8_t>(0x200000u);
    PrStage2DiscFileDevice::Device disc;
    PrStage2VramDevice::Device vram;
    PrPsxGteDirect::MatrixRegisters gte;
    uint32_t vab = 0, waits = 0, gpuSyncQueries = 0, gpuStatusReads = 0;
    DiskServices(const char* binary, const char* discPath) : disc(discPath) {
        std::ifstream source(binary,std::ios::binary);
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(source)),{});
        if (data.size() < 0x800u || std::string(data.begin(),data.begin()+8) != "PS-X EXE")
            throw std::runtime_error("invalid source SCUS");
        auto word = [&](uint32_t a) { return uint32_t(data[a]) | (uint32_t(data[a+1])<<8u)
            | (uint32_t(data[a+2])<<16u) | (uint32_t(data[a+3])<<24u); };
        const uint32_t address = word(0x18), size = word(0x1C);
        if (size > data.size()-0x800u) throw std::runtime_error("truncated source SCUS");
        for (uint32_t i = 0; i < size; ++i) Write8(address+i,data[0x800u+i]);
    }
    uint32_t Offset(uint32_t a,uint32_t n) {
        if (a < 0x80000000u || a > 0x80200000u-n) throw std::runtime_error("RAM range");
        return a-0x80000000u;
    }
    uint32_t Read(uint32_t a,uint32_t n) {
        const uint32_t p = Offset(a,n); uint32_t v = 0;
        for (uint32_t i = 0; i < n; ++i) v |= uint32_t(ram[p+i])<<(8u*i);
        return v;
    }
    void Write(uint32_t a,uint32_t n,uint32_t v) {
        const uint32_t p = Offset(a,n);
        for (uint32_t i = 0; i < n; ++i) ram[p+i] = static_cast<uint8_t>(v>>(8u*i));
    }
    uint8_t Read8(uint32_t a) override { return static_cast<uint8_t>(Read(a,1)); }
    uint16_t Read16(uint32_t a) override { return static_cast<uint16_t>(Read(a,2)); }
    uint32_t Read32(uint32_t a) override {
        // Explicit synchronous-upload fixture. DrawSync's original queue
        // code runs; only the completed device status is supplied here.
        if (a==0x1F8010A8u) return 0u;
        if (a==0x1F801814u) { ++gpuStatusReads; return 0x04000000u; }
        return Read(a,4);
    }
    void Write8(uint32_t a,uint8_t v) override { Write(a,1,v); }
    void Write16(uint32_t a,uint16_t v) override { Write(a,2,v); }
    void Write32(uint32_t a,uint32_t v) override { Write(a,4,v); }
    int32_t SetCdLocation800367A4(uint32_t value) override { return disc.SetLocation(value); }
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect r,uint32_t source) override {
        const int32_t result = vram.UploadImageWords(*this,r,source);
        uint64_t hash = 14695981039346656037ull;
        for (uint32_t y = 0; y < r.height; ++y) {
            for (uint32_t x = 0; x < r.width; ++x) {
                const uint16_t word = vram.Words()[(r.y+y)*1024u+r.x+x];
                hash = (hash^static_cast<uint8_t>(word))*1099511628211ull;
                hash = (hash^static_cast<uint8_t>(word>>8u))*1099511628211ull;
            }
        }
        std::cout << "upload " << r.x << ' ' << r.y << ' ' << r.width << ' ' << r.height << ' ' << hash << '\n';
        return result;
    }
    int32_t Call(uint32_t fn,std::initializer_list<uint32_t> args) override {
        std::vector<uint32_t> a(args);
        if (fn == 0x800381F8u && a.size() == 2) return disc.Lookup(*this,a[0],a[1]);
        if (fn == 0x80038FC0u && a.size() == 3) return disc.StartRead(*this,a[0],a[1],a[2]);
        if (fn == 0x800390C8u && a == std::vector<uint32_t>{1,0}) return disc.PollRead();
        if (fn == 0x80035560u && a == std::vector<uint32_t>{3}) { ++waits; return 0; }
        if (fn == 0x80035560u && a == std::vector<uint32_t>{0xFFFFFFFFu}) { ++gpuSyncQueries; return 0; }
        if (fn == 0x80027120u && a.empty()) { ++vab; return 0; }
        if (fn == 0x80026E4Cu && a.empty()) return 0;
        if (fn == 0x80027078u && a.size() == 1) return 1;
        if (fn == 0x800270D4u && a.size() == 1) return 0;
        if (fn == 0x800270FCu && a == std::vector<uint32_t>{1}) return 0;
        throw std::runtime_error("unexpected device call "+std::to_string(fn));
    }
    PrStage2LifecycleDirect::Words64 Call64(uint32_t,std::initializer_list<uint32_t>) override { throw std::runtime_error("unexpected Call64"); }
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32) override { throw std::runtime_error("unexpected normalize"); }
    PrPsxGteDirect::MatrixRegisters& MatrixGte() override { return gte; }
    [[noreturn]] void Exit(uint32_t,std::initializer_list<uint32_t>) override { throw std::runtime_error("unexpected native exit"); }
    [[noreturn]] void Break(uint32_t,uint32_t) override { throw std::runtime_error("unexpected native break"); }
};
