#pragma once
#include "pr/pr_stage2_interrupt_device.h"
#include <array>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>

namespace InterruptDeviceContract {
using Device = PrStage2InterruptDevice::Controller;
using Source = PrStage2InterruptDevice::Source;
using Boundary = PrStage2InterruptDevice::DmaBoundary;
inline uint64_t checks = 0;
inline void Check(bool value, const char* message) {
    ++checks;
    if (!value) throw std::runtime_error(message);
}
inline uint32_t Read(const Device& d, uint32_t a, uint32_t width) {
    uint32_t result = 0xDEADBEEFu;
    Check(d.TryRead(a,width,result), "interrupt device did not claim a register");
    return result;
}
inline void Write(Device& d,uint32_t a,uint32_t width,uint32_t value) {
    Check(d.TryWrite(a,width,value),"interrupt device did not claim a write");
}
inline std::array<uint32_t,5> Snapshot(const Device& d) {
    return {Read(d,0x1F801070u,2),Read(d,0x1F801074u,2),
        Read(d,0x1F8010F0u,4),Read(d,0x1F8010F4u,4),d.Pending()};
}
template<class F> void RejectUnchanged(Device& d,F f) {
    const auto before = Snapshot(d);
    bool rejected = false;
    try { f(); } catch (const std::invalid_argument&) { rejected = true; }
    Check(rejected && Snapshot(d)==before,"invalid device access changed state or was accepted");
}

// Independent boolean/bitset specification. It does not reuse the production
// register decoding, masks, edge helper, or DICR master calculation.
struct Oracle {
    std::bitset<11> status,mask,lines;
    std::bitset<7> flags,enabled,slices;
    bool master = false, error = false;
    uint32_t dpcr = 0x07654321u;
    void Line(unsigned i,bool on) {
        if (on && !lines[i]) status.set(i);
        lines[i] = on;
    }
    void DmaLine() { Line(3,error || (master && flags.any())); }
    void Control(uint32_t v) {
        for(unsigned i=0;i<7;++i) {
            enabled[i]=(v&(1u<<(16+i)))!=0;
            slices[i]=(v&(1u<<i))!=0;
            if(v&(1u<<(24+i)))flags.reset(i);
        }
        master=(v&(1u<<23))!=0;error=(v&(1u<<15))!=0;
        DmaLine();
    }
    void Complete(unsigned i,bool last) {
        if(master && enabled[i] && (last || slices[i]))flags.set(i);
        DmaLine();
    }
    uint32_t ControlValue() const {
        uint32_t v=error?32768u:0u;
        if(master)v+=8388608u;
        for(unsigned i=0;i<7;++i) {
            if(slices[i])v+=1u<<i;
            if(enabled[i])v+=1u<<(16+i);
            if(flags[i])v+=1u<<(24+i);
        }
        if(error || (master && flags.any()))v+=0x80000000u;
        return v;
    }
    void Compare(const Device& d) const {
        Check(Read(d,0x1F801070u,2)==status.to_ulong(),"oracle I_STAT mismatch");
        Check(Read(d,0x1F801074u,2)==mask.to_ulong(),"oracle I_MASK mismatch");
        Check(Read(d,0x1F8010F0u,4)==dpcr,"oracle DPCR mismatch");
        Check(Read(d,0x1F8010F4u,4)==ControlValue(),"oracle DICR mismatch");
        Check(d.Pending()==(status&mask).to_ulong(),"oracle pending mismatch");
    }
};
inline void Run() {
    checks=0;
    Device fresh;
    Check(Snapshot(fresh)==std::array<uint32_t,5>{0,0,0x07654321u,0,0},"device cold state");
    uint32_t edgeCases=0;
    for(unsigned i=0;i<11;++i) {
        if(i==3)continue;
        for(unsigned mask=0;mask<2048;++mask) {
            Device d;const auto source=static_cast<Source>(i);const unsigned bit=1u<<i;
            Write(d,0x1F801074u,2,mask);
            d.SetLine(source,true);
            Check(Read(d,0x1F801070u,2)==bit && d.Pending()==(bit&mask),"masked edge lost or delivered");
            Write(d,0x1F801070u,2,~bit);
            d.SetLine(source,true);
            Check(Read(d,0x1F801070u,2)==0 && d.Pending()==0,"held-high line relatches");
            d.SetLine(source,false);d.SetLine(source,true);
            Write(d,0x1F801074u,2,0xFFFFFFFFu);
            Check(Read(d,0x1F801070u,2)==bit && d.Pending()==bit && Read(d,0x1F801074u,2)==2047u,"unmask/valid bits");
            ++edgeCases;
        }
    }
    uint32_t completionCases=0;
    for(unsigned mask=0;mask<128;++mask)for(unsigned master=0;master<2;++master)
    for(unsigned channel=0;channel<7;++channel)for(unsigned slice=0;slice<2;++slice)
    for(unsigned last=0;last<2;++last) {
        Device d;Oracle o;
        const uint32_t control=(mask<<16)|(master<<23)|(slice<<channel);
        Write(d,0x1F8010F4u,4,control);o.Control(control);
        d.NotifyDma(channel,last?Boundary::TransferComplete:Boundary::BlockComplete);
        o.Complete(channel,last!=0);o.Compare(d);++completionCases;
    }
    // Every latched-flag set x every channel-enable set x master x forced IRQ.
    // A later channel disable does NOT cancel an already-latched flag.
    uint32_t truthCases=0;
    for(unsigned flags=0;flags<128;++flags)for(unsigned enabled=0;enabled<128;++enabled)
    for(unsigned master=0;master<2;++master)for(unsigned error=0;error<2;++error) {
        Device d;Write(d,0x1F8010F4u,4,0x00FF0000u);
        for(unsigned i=0;i<7;++i)if(flags&(1u<<i))d.NotifyDma(i,Boundary::TransferComplete);
        Write(d,0x1F801070u,2,0);
        const uint32_t control=(enabled<<16)|(master<<23)|(error<<15);
        Write(d,0x1F8010F4u,4,control);
        const bool high=error!=0 || (master!=0 && flags!=0);
        Check(Read(d,0x1F8010F4u,4)==(control|(flags<<24)|(high?0x80000000u:0)),"latched DMA truth table");
        Check(Read(d,0x1F801070u,2)==((flags==0&&error!=0)?8u:0u),"DICR non-edge retriggered I_STAT");
        ++truthCases;
    }
    {
        Device d;Write(d,0x1F8010F4u,4,0x00860000u);
        d.NotifyDma(1,Boundary::TransferComplete);d.NotifyDma(2,Boundary::TransferComplete);
        Write(d,0x1F801074u,2,8);Check(d.Pending()==8,"DMA event not latched");
        Write(d,0x1F801070u,2,0);const auto before=Snapshot(d);
        for(unsigned i=0;i<32;++i)Check(Snapshot(d)==before,"read generated an event");
        Write(d,0x1F8010F4u,4,0x02860000u);
        Check(Read(d,0x1F8010F4u,4)==0x84860000u && d.Pending()==0,"acknowledgement destroyed another flag");
        d.NotifyDma(1,Boundary::TransferComplete);Check(d.Pending()==0,"high DMA line generated another edge");
        Write(d,0x1F8010F4u,4,0x06860000u);
        Check(Read(d,0x1F8010F4u,4)==0x00860000u,"DMA flags not write-one-to-clear");
        d.NotifyDma(2,Boundary::TransferComplete);Check(d.Pending()==8,"new DMA edge missing");
        Write(d,0x1F8010F4u,4,0x7F000000u);
        Check(d.Pending()==8,"DICR acknowledge incorrectly cleared I_STAT");
    }
    {
        Device d;Write(d,0x1F8010F4u,4,0x80007F80u);
        Check(Read(d,0x1F8010F4u,4)==0 && Read(d,0x1F801070u,2)==0,"readonly/reserved DMA bits are writable");
        d.NotifyDmaBusError();Check(Read(d,0x1F8010F4u,4)==0x80008000u,"DMA bus error did not force master");
        Write(d,0x1F801070u,2,0);d.NotifyDmaBusError();Check(Read(d,0x1F801070u,2)==0,"held bus error retriggered");
        Write(d,0x1F8010F4u,4,0);d.NotifyDmaBusError();Check(Read(d,0x1F801070u,2)==8,"new bus error edge missing");
    }
    uint32_t rejected=0;
    for(uint32_t alias:{0u,0x80000000u,0xA0000000u}) {
        Device d;
        for(uint32_t reg:{0x1F801070u,0x1F801074u,0x1F8010F0u,0x1F8010F4u})
        for(unsigned offset=0;offset<4;++offset)for(unsigned width:{0u,1u,2u,3u,4u,8u})for(bool write:{false,true}) {
            const bool irq=reg<0x1F801080u;
            const bool good=offset==0 && (irq?(width==2 || (write&&width==4)):width==4);
            if(good)continue;
            uint32_t sentinel=0xDEADBEEFu;
            RejectUnchanged(d,[&]{if(write)d.TryWrite((reg|alias)+offset,width,0xFFFFFFFFu);
                else d.TryRead((reg|alias)+offset,width,sentinel);});
            Check(sentinel==0xDEADBEEFu,"invalid read replaced caller result");++rejected;
        }
        Write(d,0x1F801074u|alias,4,0xF123FFFFu);
        Check(Read(d,0x1F801074u,2)==2047,"IRQ alias masking");
        Write(d,0x1F8010F0u|alias,4,0x89ABCDEFu);
        Check(Read(d,0x1F8010F0u,4)==0x89ABCDEFu,"DMA alias register");
        d.SetLine(Source::Gpu,true);Write(d,0x1F801070u|alias,4,0xFFFFFFFDu);
        Check(Read(d,0x1F801070u,2)==0,"IRQ alias acknowledgement");
    }
    {
        Device d;const auto before=Snapshot(d);
        for(uint32_t a:{0x1F801078u,0x1F8010F8u,0x1F8010A8u,0x1F801814u,0xDF801070u,0x3F801074u,0xFFFFFFFFu}) {
            uint32_t sentinel=17;
            Check(!d.TryRead(a,4,sentinel)&&sentinel==17&&!d.TryWrite(a,4,0xFFFFFFFFu)&&Snapshot(d)==before,"unknown transaction claimed/mutated");
        }
        for(uint32_t source:{3u,11u,31u,0xFFFFFFFFu}) {
            RejectUnchanged(d,[&]{d.SetLine(static_cast<Source>(source),true);});++rejected;
        }
        for(uint32_t ch:{7u,31u,0xFFFFFFFFu}) {
            RejectUnchanged(d,[&]{d.NotifyDma(ch,Boundary::TransferComplete);});++rejected;
        }
        RejectUnchanged(d,[&]{d.NotifyDma(0,static_cast<Boundary>(99));});++rejected;
        Device other;d.SetLine(Source::Spu,true);Check(Snapshot(other)==before,"device instances share events");
    }
    std::mt19937 rng(0x1F8010F4u);Device device;Oracle oracle;
    constexpr uint32_t RandomSteps=12000;
    for(unsigned step=0;step<RandomSteps;++step) {
        const uint32_t value=rng();
        const unsigned channel=rng()%7, op=rng()%8;
        switch(op) {
        case 0:Write(device,0x1F801074u,2,value);oracle.mask=std::bitset<11>(value);break;
        case 1:Write(device,0x1F801070u,2,value);oracle.status&=std::bitset<11>(value);break;
        case 2:{unsigned source=rng()%10;if(source>=3)++source;bool high=(value&1)!=0;
            device.SetLine(static_cast<Source>(source),high);oracle.Line(source,high);break;}
        case 3:Write(device,0x1F8010F4u,4,value);oracle.Control(value);break;
        case 4:device.NotifyDma(channel,Boundary::TransferComplete);oracle.Complete(channel,true);break;
        case 5:device.NotifyDma(channel,Boundary::BlockComplete);oracle.Complete(channel,false);break;
        case 6:device.NotifyDmaBusError();oracle.error=true;oracle.DmaLine();break;
        case 7:Write(device,0x1F8010F0u,4,value);oracle.dpcr=value;break;
        }
        oracle.Compare(device);
    }
    std::cout<<"interrupt-device-contract "<<edgeCases<<' '<<completionCases<<' '<<truthCases<<' '
             <<RandomSteps<<' '<<rejected<<' '<<checks<<'\n';
}
}
