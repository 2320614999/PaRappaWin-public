#include "pr_stage2_cd_command_direct.h"
#include <array>
#include <stdexcept>

namespace PrStage2CdCommandDirect {
namespace {
int32_t S(uint32_t v){return v<0x80000000u?int32_t(v):int32_t(int64_t(v)-0x100000000LL);}
constexpr uint32_t Status=0x800573D4u,Ready=0x800573D5u;
void CopyResponse(Services& s,uint32_t destination,uint32_t source){
    if(destination)for(uint32_t i=0;i<8u;++i){const uint8_t b=s.Read8(source+i);s.Write8(destination+i,b);}
}
void CopyLocal(Services& s,uint32_t destination,const std::array<uint8_t,8>& response){
    for(uint32_t i=0;i<8u;++i)s.Write8(destination+i,response[i]);
}
void BeginTimeout(Services& s,uint32_t label){
    const uint32_t now=uint32_t(s.Call(0x80035560u,{0xFFFFFFFFu}));
    s.Write32(0x80088310u,now+960u);s.Write32(0x80088314u,0u);s.Write32(0x80088318u,label);
}
bool Timeout(Services& s){
    const int32_t now=s.Call(0x80035560u,{0xFFFFFFFFu});
    const int32_t deadline=S(s.Read32(0x80088310u));
    bool expired=deadline<now;
    if(!expired){const uint32_t old=s.Read32(0x80088314u);s.Write32(0x80088314u,old+1u);expired=S(old)>0x3C0000;}
    if(!expired)return false;
    s.Call(0x80047F4Cu,{0x80011D28u});
    const uint32_t sync=s.Read8(Status),ready=s.Read8(Ready),label=s.Read32(0x80088318u);
    const uint32_t readyName=s.Read32(0x8005719Cu+4u*ready);
    const uint32_t command=s.Read8(0x80057119u);
    const uint32_t commandName=s.Read32(0x8005711Cu+4u*command),syncName=s.Read32(0x8005719Cu+4u*sync);
    s.Call(0x80047FFCu,{0x80011D38u,label,commandName,syncName,readyName});
    s.Call(0x80037A8Cu,{});return true;
}
void Callbacks(Services& s,uint32_t events){
    if(events&4u){const uint32_t callback=s.Read32(0x800570FCu);if(callback)s.Call(callback,{s.Read8(Ready),0x80088300u});}
    // The ready callback may replace the sync callback; re-read afterwards.
    if(events&2u){const uint32_t callback=s.Read32(0x800570F8u);if(callback)s.Call(callback,{s.Read8(Status),0x800882F8u});}
}
uint32_t Drain(Services& s){
    const uint32_t port=s.Read32(0x800573BCu);const uint8_t index=s.Read8(port)&3u;
    for(;;){const uint32_t events=uint32_t(s.Call(0x80036AF8u,{}));if(!events)break;Callbacks(s,events);}
    const uint32_t finalPort=s.Read32(0x800573BCu);s.Write8(finalPort,index);return finalPort;
}
}
int32_t CheckCallback80035898(Services& s){return s.Read16(0x80055F7Au);}
int32_t Dispatch80038118(Services& s){return S(Drain(s));}
int32_t ReadyCallback80036528(Services& s,uint32_t callback){const uint32_t old=s.Read32(0x800570FCu);s.Write32(0x800570FCu,callback);return S(old);}
int32_t SyncCallback80036510(Services& s,uint32_t callback){const uint32_t old=s.Read32(0x800570F8u);s.Write32(0x800570F8u,callback);return S(old);}
int32_t DmaCallback80036930(Services& s,uint32_t callback){return s.Call(0x800357A4u,{3u,callback});}
int32_t Reset80036430(Services& s){return s.Call(0x80037A8Cu,{});}
int32_t Rebind80037C60(Services& s){
    for(uint32_t p:{0x800570FCu,0x800570F8u,0x8005710Cu,0x80057108u})s.Write32(p,0u);
    s.Call(0x80035744u,{});return s.Call(0x80035774u,{2u,0x80038118u});
}
int32_t Initialize80037CB0(Services& s){
    s.Call(0x80047F4Cu,{0x80011E18u});s.Call(0x80047FFCu,{0x80011E24u,0x800573D8u});
    s.Write8(0x80057119u,0u);s.Write8(0x80057118u,0u);
    Rebind80037C60(s); // Identical inline original writes/calls, not an extra PSX call.
    Reset80037A8C(s); // Original inlines this byte-MMIO reset block too.
    s.Call(0x800375BCu,{1u,0u,0u,0u});
    if(s.Read32(0x80057108u)&0x10u)s.Call(0x800375BCu,{1u,0u,0u,0u});
    if(s.Call(0x800375BCu,{10u,0u,0u,0u})!=0)return -1;
    if(s.Call(0x800375BCu,{12u,0u,0u,0u})!=0)return -1;
    return s.Call(0x80037070u,{0u,0u})==2?0:-1;
}
int32_t Reset80037A8C(Services& s){
    s.Write8(s.Read32(0x800573BCu),1u);
    while(s.Read8(s.Read32(0x800573C8u))&7u){
        s.Write8(s.Read32(0x800573BCu),1u);s.Write8(s.Read32(0x800573C8u),7u);s.Write8(s.Read32(0x800573C4u),7u);
        // Plain MMIO observations never manufacture a response. This hook
        // retains a host continuation only when the original loop must wait.
    }
    s.Write8(0x800573D6u,0u);const uint8_t end=s.Read8(0x800573D6u);s.Write8(Ready,end);
    const uint32_t port=s.Read32(0x800573BCu);s.Write8(Status,2u);s.Write8(port,0u);
    s.Write8(s.Read32(0x800573C8u),0u);s.Write32(s.Read32(0x800573CCu),0x1325u);return 0x1325;
}
namespace {
template<class Copy>int32_t SyncCore(Services& s,uint32_t nonblocking,Copy copy){
    BeginTimeout(s,0x80011DB0u);
    for(;;){
        if(Timeout(s))return -1;
        if(s.Call(0x80035898u,{})!=0)Drain(s);
        const uint32_t result=s.Read8(Status);
        if(result==2u||result==5u){s.Write8(Status,2u);copy();return int32_t(result);}
        if(nonblocking)return 0;
        s.AwaitDeviceProgress(0x1F801800u);
    }
}
}
int32_t Sync80037070(Services& s,uint32_t nonblocking,uint32_t output){
    return SyncCore(s,nonblocking,[&]{CopyResponse(s,output,0x800882F8u);});
}
Reply SyncReply80037070(Services& s,uint32_t nonblocking){
    Reply reply;
    reply.status=SyncCore(s,nonblocking,[&]{
        for(uint32_t i=0;i<8u;++i)reply.bytes[i]=s.Read8(0x800882F8u+i);
        reply.copied=true;
    });
    return reply;
}
int32_t CommandParameters800375BC(Services& s,uint32_t command,const PrStage2CdParameters::View& params,uint32_t output,uint32_t nonblocking){
    const uint32_t op=command&255u;
    if(S(s.Read32(0x80057104u))>=2){const uint32_t name=s.Read32(0x8005711Cu+4u*op);s.Call(0x80047FFCu,{0x80011DC4u,name});}
    if(s.Read32(0x8005733Cu+4u*op)!=0u&&!params.Present()){
        if(S(s.Read32(0x80057104u))>0){const uint32_t name=s.Read32(0x8005711Cu+4u*op);s.Call(0x80047FFCu,{0x80011DCCu,name});}
        return -2;
    }
    // Preserve the original ignored result: a previous Sync error does not
    // silently turn into a newly successful command or skip its MMIO writes.
    s.Call(0x80037070u,{0u,0u});
    if(op==2u)for(uint32_t i=0;i<4u;++i){const uint8_t b=params.Read(i);s.Write8(0x80057114u+i,b);}
    s.Write8(Status,0u);
    if(s.Read32(0x8005723Cu+4u*op))s.Write8(Ready,0u);
    s.Write8(s.Read32(0x800573BCu),0u);
    const uint32_t countAddress=0x8005733Cu+4u*op;
    if(S(s.Read32(countAddress))>0){
        uint32_t i=0u;
        do{const uint32_t port=s.Read32(0x800573C4u);const uint8_t b=params.Read(i);s.Write8(port,b);++i;}
        while(S(i)<S(s.Read32(countAddress)));
    }
    const uint32_t commandPort=s.Read32(0x800573C0u);s.Write8(0x80057119u,uint8_t(command));s.Write8(commandPort,uint8_t(command));
    if(nonblocking)return 0;
    // Original reads the reply between storing timeout count and its label.
    const uint32_t now=uint32_t(s.Call(0x80035560u,{0xFFFFFFFFu}));s.Write32(0x80088310u,now+960u);s.Write32(0x80088314u,0u);
    uint32_t ready=s.Read8(Status);s.Write32(0x80088318u,0x80011DDCu);
    while(!ready){
        if(Timeout(s))return -1;
        if(s.Call(0x80035898u,{})!=0)Drain(s);
        ready=s.Read8(Status);if(!ready)s.AwaitDeviceProgress(0x1F801800u);
    }
    if(s.Read8(Status)==2u&&op==14u){const uint8_t mode=params.Read(0u);s.Write8(0x80057118u,mode);}
    CopyResponse(s,output,0x800882F8u);return s.Read8(Status)==5u?-1:0;
}
int32_t Command800375BC(Services& s,uint32_t command,uint32_t params,uint32_t output,uint32_t nonblocking){
    return CommandParameters800375BC(s,command,{s,params},output,nonblocking);
}
int32_t Interrupt80036AF8(Services& s){
    s.Write8(s.Read32(0x800573BCu),1u);
    const uint32_t flagsPort=s.Read32(0x800573C8u);
    uint8_t kind=s.Read8(flagsPort)&7u;
    if(!kind)return 0;
    while(kind!=(s.Read8(flagsPort)&7u))kind=s.Read8(flagsPort)&7u;
    // Private response storage never escapes and has no fabricated PSX address.
    std::array<uint8_t,8> response{};uint32_t count=0u;
    while(s.Read8(s.Read32(0x800573BCu))&0x20u){
        response[count]=s.Read8(s.Read32(0x800573C0u));if(++count==8u)break;
    }
    s.Write8(s.Read32(0x800573BCu),1u);s.Write8(s.Read32(0x800573C8u),7u);s.Write8(s.Read32(0x800573C4u),7u);
    uint32_t error=0u;
    bool update=true;
    if(kind==3u){const uint32_t op=s.Read8(0x80057119u);update=s.Read32(0x800572BCu+4u*op)!=0u;}
    if(update){
        const uint32_t previous=s.Read32(0x80057108u);
        if(!(previous&0x10u)&&(response[0]&0x10u)){const uint32_t lid=s.Read32(0x80057110u);s.Write32(0x80057110u,lid+1u);}
        error=response[0]&0x1Du;s.Write32(0x80057108u,response[0]);s.Write32(0x8005710Cu,response[1]);
    }
    if(kind==5u){
        s.Call(0x80047F4Cu,{0x80011D54u});
        if(S(s.Read32(0x80057104u))>0){
            const uint32_t op=s.Read8(0x80057119u),status=s.Read32(0x80057108u),detail=s.Read32(0x8005710Cu);
            const uint32_t name=s.Read32(0x8005711Cu+4u*op);s.Call(0x80047FFCu,{0x80011D60u,name,status,detail});
        }
    }
    switch(kind){
    case 3u:
        if(error){s.Write8(Status,5u);CopyLocal(s,0x800882F8u,response);return 2;}
        {const uint32_t op=s.Read8(0x80057119u);const bool second=s.Read32(0x800571BCu+4u*op)!=0u;
        s.Write8(Status,second?3u:2u);CopyLocal(s,0x800882F8u,response);return second?1:2;}
    case 2u:s.Write8(Status,error?5u:2u);CopyLocal(s,0x800882F8u,response);return 2;
    case 1u:
        if(error&&count==1u)error=0u;
        s.Write8(Ready,error?5u:1u);CopyLocal(s,0x80088300u,response);
        s.Write8(s.Read32(0x800573BCu),0u);s.Write8(s.Read32(0x800573C8u),0u);return 4;
    case 4u:
        s.Write8(0x800573D6u,4u);s.Write8(Ready,s.Read8(0x800573D6u));
        CopyLocal(s,0x80088308u,response);CopyLocal(s,0x80088300u,response);return 4;
    case 5u:
        s.Write8(Ready,5u);s.Write8(Status,s.Read8(Ready));CopyLocal(s,0x800882F8u,response);CopyLocal(s,0x80088300u,response);return 6;
    default:s.Call(0x80047F4Cu,{0x80011D7Cu});s.Call(0x80047FFCu,{0x80011D90u,kind});return 0;
    }
}
bool TryCall(Services& s,uint32_t fn,std::initializer_list<uint32_t> args,int32_t& result){
    size_t n;
    switch(fn){
    case 0x800375BCu:n=4;break;
    case 0x80037070u:n=2;break;
    case 0x80036510u:case 0x80036528u:case 0x80036930u:n=1;break;
    case 0x80035898u:case 0x80036AF8u:case 0x80038118u:case 0x80037A8Cu:case 0x80036430u:case 0x80037C60u:case 0x80037CB0u:n=0;break;
    default:return false;
    }
    if(args.size()!=n)throw std::invalid_argument("CD command original argument count");const auto a=args.begin();
    switch(fn){
    case 0x800375BCu:result=Command800375BC(s,a[0],a[1],a[2],a[3]);break;
    case 0x80037070u:result=Sync80037070(s,a[0],a[1]);break;
    case 0x80036528u:result=ReadyCallback80036528(s,a[0]);break;
    case 0x80036510u:result=SyncCallback80036510(s,a[0]);break;
    case 0x80036930u:result=DmaCallback80036930(s,a[0]);break;
    case 0x80035898u:result=CheckCallback80035898(s);break;
    case 0x80036AF8u:result=Interrupt80036AF8(s);break;
    case 0x80038118u:result=Dispatch80038118(s);break;
    case 0x80037A8Cu:result=Reset80037A8C(s);break;
    case 0x80036430u:result=Reset80036430(s);break;
    case 0x80037C60u:result=Rebind80037C60(s);break;
    case 0x80037CB0u:result=Initialize80037CB0(s);break;
    }
    return true;
}
}
