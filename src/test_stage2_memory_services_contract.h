#pragma once
#include "pr/pr_stage2_memory_services.h"
#include <array>
#include <iostream>
#include <vector>

namespace MemoryContract {
using Args=std::initializer_list<uint32_t>;
uint32_t checks=0;
void Require(bool v,const char* why) {++checks;if(!v)throw std::runtime_error(why);}
template<class F> void Reject(F fn) {
    bool caught=false;try {fn();}catch(const std::exception&){caught=true;}
    Require(caught,"invalid memory operation succeeded");
}
struct Probe : PrStage2MemoryServices::Services {
    std::vector<std::array<uint32_t,3>> events;
    uint32_t deviceValue=0xA1B2C3D4u;
    bool deviceFailure=false,reenter=false;
    PrPsxGteDirect::MatrixRegisters gte{};
    uint32_t Device(uint32_t op,uint32_t a,uint32_t v=0) {
        events.push_back({op,a,v});
        if(deviceFailure)throw std::runtime_error("explicit device failure");
        if(reenter) { reenter=false;Write32(0xA065D82Cu,0x81234567u); }
        return deviceValue;
    }
    uint8_t ReadDevice8(uint32_t a) override {return uint8_t(Device(1,a));}
    uint16_t ReadDevice16(uint32_t a) override {return uint16_t(Device(2,a));}
    uint32_t ReadDevice32(uint32_t a) override {return Device(4,a);}
    void WriteDevice8(uint32_t a,uint8_t v) override {Device(17,a,v);}
    void WriteDevice16(uint32_t a,uint16_t v) override {Device(18,a,v);}
    void WriteDevice32(uint32_t a,uint32_t v) override {Device(20,a,v);}
    int32_t CallExternal(uint32_t,Args) override {throw std::runtime_error("unbound external call");}
    void CallExternalVoid(uint32_t,Args) override {throw std::runtime_error("unbound external void");}
    PrStage2LifecycleDirect::Words64 Call64(uint32_t,Args) override {throw std::runtime_error("unbound wide call");}
    PrStage2LifecycleDirect::Vector32 NormalizeVector8003A3DC(PrStage2LifecycleDirect::Vector32) override {throw std::runtime_error("unbound normalize");}
    int32_t SetCdLocation800367A4(uint32_t) override {throw std::runtime_error("unbound CD");}
    int32_t LoadImage80044D64(PrStage2LifecycleDirect::ImageRect,uint32_t) override {throw std::runtime_error("unbound upload");}
    PrPsxGteDirect::MatrixRegisters& MatrixGte() override {return gte;}
    [[noreturn]] void Exit(uint32_t,Args) override {throw std::runtime_error("native exit");}
    [[noreturn]] void Break(uint32_t,uint32_t) override {throw std::runtime_error("native break");}
};

void Run() {
    Probe s,other;
    std::vector<uint32_t> aliases;
    for(uint32_t segment:{0u,0x80000000u,0xA0000000u})
        for(uint32_t mirror:{0u,0x200000u,0x400000u,0x600000u})aliases.push_back(segment+mirror);
    for(uint32_t a:aliases) {
        Reject([&]{s.Read8(a+0x110u);});Reject([&]{s.Read16(a+0x110u);});Reject([&]{s.Read32(a+0x110u);});
    }
    Require(s.events.empty(),"unknown RAM delegated to device");
    s.Write8(0x80000110u,0x12u);Reject([&]{s.Read16(0x110u);});
    s.Write16(0xA0000111u,0x3456u);Reject([&]{s.Read32(0x110u);});
    s.Write8(0x600113u,0x78u);Require(s.Read32(0x110u)==0x78345612u,"partial initialization/endian mismatch");
    Reject([&]{other.Read8(0x110u);});
    uint32_t value=0x12345678u;
    for(uint32_t writer:aliases)for(uint32_t offset:{0x1000u,0x1001u,0x1002u,0x1003u,0x1FFFFCu}) {
        value=value*1664525u+1013904223u;s.Write32(writer+offset,value);
        for(uint32_t reader:aliases) {
            Require(s.Read32(reader+offset)==value,"RAM alias word mismatch");
            for(uint32_t i=0;i<4u;++i)Require(s.Read8(reader+offset+i)==uint8_t(value>>(8*i)),"RAM byte lanes mismatch");
            Require(s.Read16(reader+offset+1u)==uint16_t(value>>8u),"unaligned RAM halfword mismatch");
        }
    }
    for(uint32_t segment:{0u,0x80000000u,0xA0000000u})for(uint32_t mirror:{0u,0x200000u,0x400000u}) {
        const auto a=segment+mirror+0x1FFFFEu;s.Write32(a,0x12345678u);
        Require(s.Read16(segment+0x1FFFFEu)==0x5678u&&s.Read16(segment)==0x1234u,"physical mirror boundary lost lanes");
        Require(s.Read32(a)==0x12345678u,"physical mirror wrap read differs");
    }
    for(uint32_t end:{0x00800000u,0x80800000u,0xA0800000u}) {
        s.Write16(end-2u,0xABCDu);
        Reject([&]{s.Write32(end-2u,0);});Reject([&]{s.Read32(end-2u);});
        Require(s.Read16(end-2u)==0xABCDu,"failed crossing write changed RAM");
    }
    for(uint32_t begin:{0x80000000u,0xA0000000u}) {
        Reject([&]{s.Read32(begin-1u);});Reject([&]{s.Write32(begin-1u,0);});
    }
    Reject([&]{s.Read32(0xFFFFFFFFu);});Reject([&]{s.Write32(0xFFFFFFFEu,0);});
    Require(s.events.empty(),"RAM boundary failure reached device");
    for(uint32_t a:{0x1F801074u,0x1F8010F4u,0x9F801814u,0xBF801814u,0x1F800000u,0x800000u,0x80800000u,0xBFC00000u}) {
        s.events.clear();
        Require(s.Read8(a)==0xD4u&&s.Read16(a)==0xC3D4u&&s.Read32(a)==0xA1B2C3D4u,"device return width changed");
        s.Write8(a,0x56);s.Write16(a,0x789A);s.Write32(a,0xBCDEF012);
        Require(s.events==std::vector<std::array<uint32_t,3>>{{1,a,0},{2,a,0},{4,a,0},{17,a,0x56},{18,a,0x789A},{20,a,0xBCDEF012}},"device transaction split, masked or reordered");
        s.events.clear();s.deviceFailure=true;Reject([&]{s.Read32(a);});
        Require(s.events.size()==1u,"failed device read retried");s.events.clear();Reject([&]{s.Write16(a,7);});
        Require(s.events==std::vector<std::array<uint32_t,3>>{{18,a,7}},"failed device write retried");s.deviceFailure=false;
    }
    for(uint32_t base:aliases) {
        s.Write32(base+0x5D82Cu,0x12345678u);
        const PrStage2LifecycleDirect::GpuCommandSource local{0u,std::make_shared<const uint8_t>(0)};
        s.WriteGpuCommandSource(local);
        Reject([&]{s.Read32(base+0x5D82Cu);});Reject([&]{s.Write16(base+0x5D82Cu,0);});
        Require(s.ReadGpuCommandSource().local==local.local,"rejected partial overwrite lost identity");
        s.Write32(base+0x5D82Cu,0x87654321u);
        Require(!s.ReadGpuCommandSource().local&&s.Read32(0x8005D82Cu)==0x87654321u,"whole alias write failed to replace identity");
    }
    const PrStage2LifecycleDirect::GpuCommandSource local{0u,std::make_shared<const uint8_t>(0)};
    s.WriteGpuCommandSource(local);s.reenter=true;s.Write32(0x1F8010F4u,0);
    Require(!s.ReadGpuCommandSource().local&&s.Read32(0x8005D82Cu)==0x81234567u,"device reentry source change lost");
    s.events.clear();s.Call(0x8001C1E8u,{});
    Require(s.Read32(0x8EDE0u)==0x8003B9C8u&&s.Read32(0xA068EED0u)==0x8003E26Cu&&s.Read32(0x8008EDD8u)==0,"native table did not use owned RAM");
    s.Call(0x800473C0u,{0xA0202001u,0x1234A5u,9u});
    for(uint32_t i=0;i<9;++i)Require(s.Read8(0x80002001u+i)==0xA5u,"native fill bypassed RAM");
    Require(s.events.empty(),"native RAM helper escaped to external device");
    s.Write32(0x80057008u,0x1F801074u);s.deviceValue=0xFEDCu;
    Require(s.Call(0x800358C0u,{0xABCD8123u})==0xFEDC,"native mask return lost zero extension");
    Require(s.events==std::vector<std::array<uint32_t,3>>{{2,0x1F801074,0},{18,0x1F801074,0x8123}},"native IRQ mask changed MMIO width/order");
    std::cout<<"memory-contract "<<checks<<" 12\n";
}
}