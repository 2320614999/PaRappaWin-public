#include "pr/pr_stage2_cd_command_device.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <thread>

namespace {
using D=PrStage2CdCommandDevice::Device;
uint64_t checks=0;
void Check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
template<class F>void Reject(F f){bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}Check(rejected,"Expected invalid CD transaction rejection");}
void W(D& d,uint32_t p,uint32_t v){Check(d.TryWrite(p,1u,v),"Unrecognized CD port");}
uint32_t R(D& d,uint32_t p){uint32_t v=0;Check(d.TryRead(p,1u,v),"Unrecognized CD read");return v;}
void Bank(D& d,uint32_t bank){W(d,0x1F801800u,bank);}
void Ack(D& d){Bank(d,1u);W(d,0x1F801803u,7u);}
void Send(D& d,uint8_t op,std::initializer_list<uint8_t> params={}){Bank(d,0u);for(auto b:params)W(d,0x1F801802u,b);W(d,0x1F801801u,op);}
void Simple(D& d,uint8_t op,std::initializer_list<uint8_t> params={}){
    Send(d,op,params);Check(d.Service(),"Command not executed");Check(d.Flags()==3u,"Command did not acknowledge");Ack(d);
}
uint8_t Bcd(uint32_t v){return uint8_t((v/10u)*16u+v%10u);}
void Contract(const std::filesystem::path& disc,const std::filesystem::path& out){
    using C=PrStage2InterruptDevice::Controller;
    C irq;D d(disc,irq);uint32_t untouched=0xAABBCCDDu;
    Check(!d.TryRead(0x1F801814u,4u,untouched)&&untouched==0xAABBCCDDu,"CD claimed GPU register");
    for(uint32_t alias:{0u,0x80000000u,0xA0000000u})for(uint32_t p=0x1F801800u;p<0x1F801804u;++p)
        for(uint32_t width:{2u,4u,8u}){Reject([&]{d.TryRead(p|alias,width,untouched);});Reject([&]{d.TryWrite(p|alias,width,0u);});}
    for(uint32_t alias:{0u,0x80000000u,0xA0000000u}){
        Check(d.TryWrite(0x1F801800u|alias,1u,3u),"Indexed alias write");Check((R(d,0x1F801800u)&3u)==3u,"Indexed aliases disconnected");
    }
    irq.TryWrite(0x1F801074u,2u,4u);
    Send(d,1u);Check(d.Commands()==1u&&d.Executed()==0u&&d.Responses()==0u,"Write executed the command");
    for(unsigned n=0;n<128u;++n){Check(R(d,0x1F801800u)&128u,"Pending command lost BUSYSTS");Check(d.Executed()==0u&&irq.Pending()==0u,"Register read pumped a command");}
    Check(d.Service()&&d.Flags()==3u&&d.Executed()==1u&&irq.Pending()==0u,"Masked command state/IRQ incorrect");
    Bank(d,1u);W(d,0x1F801802u,1u);Check(irq.Pending()==4u,"Enabling mask did not expose latched reply");
    irq.TryWrite(0x1F801070u,2u,0u);Check(irq.Pending()==0u,"CPU IRQ ack failed");
    Check(d.Flags()==3u,"CPU IRQ acknowledgment lost CD reply");
    W(d,0x1F801803u,1u);Check(d.Flags()==2u,"Partial acknowledgment collapsed interrupt code");
    W(d,0x1F801802u,2u);Check(irq.Pending()==4u,"Bitwise INTSTS mask misunderstood");Ack(d);irq.TryWrite(0x1F801070u,2u,0u);
    for(uint32_t mask=0;mask<32u;++mask){
        Bank(d,1u);W(d,0x1F801802u,mask);Send(d,1u);Check(d.Service(),"Nop Service failed");
        Check((irq.Pending()!=0u)==((mask&3u)!=0u),"Mask used type-index rather than INTSTS bits");Ack(d);irq.TryWrite(0x1F801070u,2u,0u);
    }
    Send(d,10u);Check(d.Service()&&d.Flags()==3u,"Init first response missing");
    const auto before=d.Responses();Check(!d.Service()&&d.Responses()==before,"Second Init response bypassed first ack");
    Ack(d);Check(d.Flags()==0u&&d.Responses()==before,"Ack itself executed/published second reply");
    Check(d.Service()&&d.Flags()==2u&&d.Mode()==0x20u,"Init second phase or mode incorrect");Ack(d);
    Simple(d,12u);Simple(d,13u,{1u,2u});Check(d.FilterFile()==1u&&d.FilterChannel()==2u,"Filter state missing");
    Simple(d,14u,{0u});Check(d.Mode()==0u,"Setmode state missing");
    Send(d,2u,{0xAAu,0u,0u});Check(d.Service()&&d.Flags()==5u,"Invalid BCD acknowledged as success");Ack(d);
    Send(d,13u,{1u});Check(d.Service()&&d.Flags()==5u,"Wrong parameter count did not fail");Ack(d);
    Bank(d,0u);for(uint32_t n=0;n<16u;++n)W(d,0x1F801802u,n);Check(!(R(d,0x1F801800u)&16u),"Full FIFO still write-ready");Reject([&]{W(d,0x1F801802u,99u);});
    Bank(d,1u);W(d,0x1F801803u,64u);Check(R(d,0x1F801800u)&8u,"Explicit parameter FIFO clear failed");
    // Fixed location comes from the actual Stage2 movie descriptor, not a
    // stand-in movie frame. Python independently checks all 2352 source bytes.
    constexpr uint32_t lba=241038u;const uint32_t absolute=lba+150u;
    Simple(d,2u,{Bcd(absolute/4500u),Bcd(absolute/75u%60u),Bcd(absolute%75u)});
    Check(d.Location()==lba&&d.SectorReads()==0u,"Setloc read or moved actual sector data");
    Simple(d,27u);Check(d.Reading()&&d.SectorReads()==0u,"ReadS immediately forged a sector");
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    for(unsigned n=0;n<100u;++n){R(d,0x1F801800u);Check(d.SectorReads()==0u,"Status polling secretly read a sector");}
    Check(d.Service()&&d.SectorReads()==1u&&d.Flags()==1u&&d.LastSector()==lba,"Real first movie sector missing");
    std::ofstream raw(out/"actual_cd_sector.bin",std::ios::binary);raw.write(reinterpret_cast<const char*>(d.RawSector().data()),2352);Check(bool(raw),"Raw sector evidence write failed");raw.close();
    Ack(d);Bank(d,0u);W(d,0x1F801803u,128u);
    for(uint32_t i=0;i<2048u;++i)Check(R(d,0x1F801802u)==d.RawSector()[24u+i],"Data FIFO did not consume actual sector bytes");
    Check(!(R(d,0x1F801800u)&64u),"Drained sector still ready");Check(d.SectorReads()==1u,"Data reads performed extra disk IO");
    std::cout<<"cd-device-contract 32-masks 2-init-responses 1-real-sector 2048-data-bytes no-read-pump\n";
    std::cout<<"cd-device-observation "<<d.Commands()<<' '<<d.Executed()<<' '<<d.Responses()<<' '<<d.SectorReads()<<' '<<checks<<'\n';
    // 请求过的 FIFO 必须继续持有；未请求的数据只能在 INT1 确认后被下一实际扇区替换。
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    Check(!d.Service()&&d.LastSector()==lba&&d.UnrequestedSectorsReplaced()==0u,"Requested FIFO was overwritten");
    d.ReleaseConsumedSector();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    Check(d.Service()&&d.Flags()==1u&&d.LastSector()==lba+1u,"Second actual sector not published");
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    Check(!d.Service()&&d.LastSector()==lba+1u,"Unacknowledged data interrupt was overwritten");
    Ack(d);Check(d.Service()&&d.LastSector()==lba+2u&&d.Flags()==1u&&d.UnrequestedSectorsReplaced()==1u,"Unrequested sector blocked continuous reading");
    std::ofstream unrequested(out/"actual_after_unrequested.bin",std::ios::binary);
    unrequested.write(reinterpret_cast<const char*>(d.RawSector().data()),2352);
    Check(bool(unrequested),"Unrequested-sector successor evidence write failed");
    Ack(d);Bank(d,0u);W(d,0x1F801803u,128u);
    const uint32_t first=R(d,0x1F801802u);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    Check(!d.Service()&&d.LastSector()==lba+2u&&first==d.RawSector()[24u],"Partial FIFO transfer lost ownership");
    std::cout<<"cd-unrequested-contract 3-real-sectors 1-replacement ack-before-read requested-fifo-retained\n";
}
void MechanicalStartup(const std::filesystem::path& disc){
    PrStage2InterruptDevice::Controller irq;
    auto now=D::Clock::time_point{};
    D d(disc,irq,[&]{return now;});d.EnableMechanicalTiming();
    constexpr uint32_t lba=241038u,absolute=lba+150u;
    Simple(d,2u,{Bcd(absolute/4500u),Bcd(absolute/75u%60u),Bcd(absolute%75u)});
    Simple(d,14u,{0xC8u});
    Send(d,27u);Check(d.Service()&&d.Flags()==3u,"ReadS startup acknowledgment missing");
    Bank(d,0);Check(R(d,0x1F801801u)==0x42u,"ReadS falsely reported reading before seek");Ack(d);
    now+=std::chrono::milliseconds(700);
    for(unsigned i=0;i<100u;++i)Check(!d.Service()&&d.SectorReads()==0u,"Startup consumed a sector before the drive was ready");
    Send(d,1u);Check(d.Service()&&d.Flags()==3u,"Readiness query stalled behind mechanical wait");
    Bank(d,0);Check(R(d,0x1F801801u)==0x42u,"Readiness query bypassed the original waiting loop");Ack(d);
    now+=std::chrono::seconds(3);
    Check(d.Service()&&d.SectorReads()==1u&&d.LastSector()==lba&&d.Flags()==1u,"Prepared drive did not deliver the real sector");
    Bank(d,0);Check(R(d,0x1F801801u)==0x22u,"First real sector did not publish reading status");Ack(d);
    // A seek aborted by Stop must never release its pending first sector.
    Simple(d,27u);Send(d,8u);Check(d.Service()&&d.Flags()==3u,"Stop ack missing");Ack(d);
    Check(d.Service()&&d.Flags()==2u,"Stop completion missing");Ack(d);
    now+=std::chrono::seconds(10);
    Check(!d.Service()&&d.SectorReads()==1u&&!d.Reading(),"Aborted startup resumed reading");
    std::cout<<"cd-startup-contract seek-before-read real-first-sector query-during-seek stop-cancels-start\n";
}
void HeldStartup(const std::filesystem::path& disc){
    PrStage2InterruptDevice::Controller irq;
    auto now=D::Clock::time_point{};
    D d(disc,irq,[&]{return now;});d.EnableMechanicalTiming();
    constexpr uint32_t lba=241038u,absolute=lba+150u;
    for(unsigned attempt=0;attempt<2u;++attempt){
        const auto before=d.SectorReads();
        d.HoldNextStreamStart();
        Simple(d,2u,{Bcd(absolute/4500u),Bcd(absolute/75u%60u),Bcd(absolute%75u)});
        Simple(d,14u,{0xC8u});Simple(d,27u);
        if(attempt==0u)now+=std::chrono::seconds(10);
        Check(!d.Service()&&d.SectorReads()==before,"Held startup consumed a sector");
        Send(d,1u);Check(d.Service()&&d.Flags()==3u,"Held startup blocked status commands");
        Bank(d,0);Check(R(d,0x1F801801u)==0x42u,"Held startup published playing status");Ack(d);
        d.ReleaseStreamStart();
        if(attempt==1u){
            Check(!d.Service()&&d.SectorReads()==before,"Release bypassed physical startup deadline");
            now+=std::chrono::seconds(4);
        }
        Check(d.Service()&&d.LastSector()==lba&&d.SectorReads()==before+1u,
              "Startup lost the first sector after release/retry");Ack(d);
        if(attempt==0u){
            Check(!d.Service()&&d.SectorReads()==before+1u,"Held startup accumulated playback debt");
            now+=std::chrono::microseconds(6667);
            Check(d.Service()&&d.LastSector()==lba+1u,"Released stream cadence did not resume");Ack(d);
        }
        Send(d,8u);Check(d.Service()&&d.Flags()==3u,"Stop ack missing");Ack(d);
        Check(d.Service()&&d.Flags()==2u,"Stop completion missing");Ack(d);
    }
    d.HoldNextStreamStart();Simple(d,27u);
    Send(d,8u);Check(d.Service()&&d.Flags()==3u,"Held stop ack missing");Ack(d);
    Check(d.Service()&&d.Flags()==2u,"Held stop completion missing");Ack(d);
    now+=std::chrono::seconds(10);
    Check(!d.Service()&&!d.Reading(),"Cancelled held startup read a sector");
    d.HoldNextStreamStart(); // Stop released the previous host-owned hold.
    std::cout<<"cd-startup-hold-contract status-only first-sector no-catchup retry seek-deadline stop\n";
}
}
int main(int argc,char**argv){try{Check(argc==3,"Arguments: disc output");const auto disc=std::filesystem::u8path(argv[1]);Contract(disc,std::filesystem::u8path(argv[2]));MechanicalStartup(disc);HeldStartup(disc);return 0;}
catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}}
