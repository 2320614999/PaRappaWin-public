#include "pr_stage2_movie_cd_direct.h"
#include "pr_stage2_int_loader_direct.h"
#include <stdexcept>

namespace PrStage2MovieCdDirect {
namespace {
uint32_t H(uint16_t v){return v<0x8000u?uint32_t(v):uint32_t(v)|0xFFFF0000u;}
int32_t Command(Services& s,uint32_t command,const PrStage2CdParameters::View& params,uint32_t output,bool nonblocking,bool wait){
    const auto issue=[&](uint32_t op,uint32_t nonblock){
        if(params.local){
            auto* engine=dynamic_cast<PrStage2CdParameters::Engine*>(&s);
            if(!engine)throw std::logic_error("Private CD parameter transport is unbound");
            return engine->CallPrivateCdCommand(op,params,output,nonblock);
        }
        return s.Call(0x800375BCu,{op,params.address,output,nonblock});
    };
    const uint32_t op=command&255u;
    const uint32_t saved=s.Read32(0x800570F8u);
    const uint32_t table=0x80057078u+4u*op;
    int32_t retries=3;
    for(;;){
        s.Write32(0x800570F8u,0u);
        if(op!=1u&&(s.Read8(0x80057108u)&0x10u))s.Call(0x800375BCu,{1u,0u,0u,0u});
        bool retry=false;
        if(params.Present()&&s.Read32(table)!=0u)retry=issue(2u,0u)!=0;
        if(!retry){
            // Restore the entry value, NOT zero or the callback currently in
            // the slot. The original old value lives across all four attempts.
            s.Write32(0x800570F8u,saved);
            if(issue(op,nonblocking?1u:0u)==0)
                return wait?(s.Call(0x80037070u,{0u,output})==2?1:0):1;
        }
        if(--retries==-1){s.Write32(0x800570F8u,saved);return 0;}
        s.AwaitDeviceProgress(0x1F801800u);
    }
}
}
int32_t Control800367A4(Services& s,uint32_t command,uint32_t params,uint32_t output){return Command(s,command,{s,params},output,false,true);}
int32_t ControlCommand80036540(Services& s,uint32_t command,uint32_t params,uint32_t output){return Command(s,command,{s,params},output,false,false);}
int32_t ControlNonblocking80036678(Services& s,uint32_t command,uint32_t params){return Command(s,command,{s,params},0u,true,false);}
int32_t ControlPrivate80036540(Services& s,uint32_t command,const PrStage2CdParameters::View& params,uint32_t output){
    if(!params.local||!params.validBytes||&params.memory!=&s)throw std::invalid_argument("Invalid private CD parameter carrier");
    return Command(s,command,params,output,false,false);
}
int32_t Sync800364D0(Services& s,uint32_t mode,uint32_t output){return s.Call(0x80037070u,{mode,output});}
// 只更新滤波参数的声道字节，文件号保持原值；返回原非阻塞命令结果。
int32_t SelectAudioChannel8001A654(Services& s,uint32_t channel){
    s.Write8(0x8004940Du,uint8_t(channel));
    return s.Call(0x80036678u,{13u,0x8004940Cu});
}
int32_t WaitCleanup8001A694(Services& s){
    while(s.Call(0x800367A4u,{8u,0u,0x80049414u})==0)
        s.AwaitDeviceProgress(0x1F801800u);
    // Preserve the prior callback return, not the command's success value.
    return s.Call(0x80036510u,{0u});
}
int32_t OpenStream8001A4D0(Services& s,uint32_t record,uint32_t mode){
    const uint32_t present=s.Read32(record);if(present==0u)return 0;
    // The original ULW/USW are two instructions each, even for the aligned
    // records used here. Preserve both reads and both full destination writes.
    const uint32_t at=record+16u,shift=(at&3u)*8u;
    const uint32_t upper=s.Read32((at+3u)&~3u),lower=s.Read32(at&~3u);
    const uint32_t location=shift?(lower>>shift)|(upper<<(32u-shift)):lower;
    s.Write32(0x800493ECu,location);s.Write32(0x800493ECu,location);
    s.Write32(0x800493FCu,uint32_t(PrStage2IntLoaderDirect::LocationToLba80036A78(s,0x800493ECu)));
    s.CallVoid(0x8001A478u,{H(s.Read16(record+6u))});
    for(;;){
        for(;;){
            while(s.Call(0x800367A4u,{2u,0x800493ECu,0x80049414u})==0)s.AwaitDeviceProgress(0x1F801800u);
            if(s.Call(0x800364D0u,{0u,0u})==2)break;
            s.AwaitDeviceProgress(0x1F801800u);
        }
        s.Write8(0x8004940Cu,1u);
        const uint16_t channel=s.Read16(record+4u);
        s.Write16(0x8004940Eu,0u);s.Write8(0x8004940Du,uint8_t(channel));
        for(;;){
            while(s.Call(0x800367A4u,{13u,0x8004940Cu,0x80049414u})==0)s.AwaitDeviceProgress(0x1F801800u);
            if(s.Call(0x800364D0u,{0u,0u})==2)break;
            s.AwaitDeviceProgress(0x1F801800u);
        }
        s.Write32(0x80049424u,mode==1u?16u:4u);
        while(s.Call(0x800391ACu,{mode==1u?456u:72u})==0)s.AwaitDeviceProgress(0x1F801800u);
        s.Call(0x80035560u,{3u});
        if(s.Call(0x800364D0u,{0u,0u})==2)break;
        s.AwaitDeviceProgress(0x1F801800u);
    }
    const uint32_t cadence=s.Read32(0x80049424u);
    s.Write32(0x80049410u,1u);
    s.Write32(0x80049420u,0u-cadence); // Original LW/NEGU, not folded zero.
    return s.Call(0x80036678u,{1u,0u});
}
bool TryCall(Services& s,uint32_t fn,std::initializer_list<uint32_t> args,int32_t& result){
    size_t n;
    switch(fn){
    case 0x8001A694u:n=0;break;
    case 0x8001A654u:n=1;break;
    case 0x8001A4D0u:case 0x80036678u:case 0x800364D0u:n=2;break;
    case 0x800367A4u:case 0x80036540u:n=3;break;
    default:return false;
    }
    if(args.size()!=n)throw std::invalid_argument("Movie CD original argument count");const auto a=args.begin();
    switch(fn){
    case 0x8001A694u:result=WaitCleanup8001A694(s);break;
    case 0x8001A654u:result=SelectAudioChannel8001A654(s,a[0]);break;
    case 0x8001A4D0u:result=OpenStream8001A4D0(s,a[0],a[1]);break;
    case 0x800364D0u:result=Sync800364D0(s,a[0],a[1]);break;
    case 0x800367A4u:result=Control800367A4(s,a[0],a[1],a[2]);break;
    case 0x80036540u:result=ControlCommand80036540(s,a[0],a[1],a[2]);break;
    case 0x80036678u:result=ControlNonblocking80036678(s,a[0],a[1]);break;
    }
    return true;
}
}
