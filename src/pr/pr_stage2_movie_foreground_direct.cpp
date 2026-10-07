#include "pr_stage2_movie_foreground_direct.h"
#include "pr_stage2_cd_command_direct.h"
#include "pr_stage2_int_loader_direct.h"
#include <stdexcept>

namespace PrStage2MovieForegroundDirect {
namespace {
int32_t S(uint32_t v){return v<0x80000000u?int32_t(v):int32_t(int64_t(v)-0x100000000LL);}
uint32_t H(uint16_t v){return v<0x8000u?uint32_t(v):uint32_t(v)|0xFFFF0000u;}
uint32_t Asr(uint32_t v,unsigned bits){return !bits?v:(v>>bits)|((v&0x80000000u)?(~0u<<(32u-bits)):0u);}
constexpr uint32_t Control=0x8006ED78u,WorkBuffer=0x8006EDA8u;
void RequireReply(const PrStage2CdCommandDirect::Reply& r){
    if(!r.copied)throw std::logic_error("Source movie attempted to consume an unproduced CD reply");
}
}
// 只有原视频回调仍安装时才检查 STR ring；音频流仍走正常 CD 数据中断。
bool CanAcceptHostStreamSector(Services& s,const std::array<uint8_t,2352>& raw){
    return s.Read32(0x800570FCu)!=0x80039240u || CanAcceptHostVideoSector(s,raw);
}
bool CanAcceptHostVideoSector(Services& s,const std::array<uint8_t,2352>& raw){
    const auto half=[&](uint32_t at){return uint32_t(raw[at])|(uint32_t(raw[at+1u])<<8u);};
    if(half(24u)!=0x160u||(raw[18]&6u)!=2u)
        throw std::invalid_argument("Native movie receiver requires a real STR video sector");
    if(s.Read32(0x80092858u))return false;
    const uint32_t slots=s.Read32(0x801C3868u),index=s.Read32(0x80095C50u);
    const uint32_t count=half(30u),part=half(28u),ring=s.Read32(0x800965ACu);
    if(slots<2u||slots>2048u||index>=slots||!count||count>=slots||part>=count)
        throw std::invalid_argument("Native video frame cannot fit the configured source ring");
    if(s.Read16(ring+(index<<5u)))return false;
    // 80039670 checks this same wrap condition after its header transfer.
    // Waiting before delivery avoids its full-ring rejection: do not replay
    // the callback, overwrite the live ring or drop part of the next frame.
    if(part==0u&&slots-index-1u<count&&s.Read32(0x8009659Cu))
        return s.Read16(ring)==0u;
    return true;
}
void LocationQuery::Request(Services& s,const std::array<uint8_t,8>& header){
    if(pending_||faulted_)throw std::logic_error("Native location query already owned or faulted");
    const auto bcd=[](uint8_t v){return (v&15u)<10u && (v>>4u)<10u;};
    if(!bcd(header[0])||!bcd(header[1])||!bcd(header[2])||header[1]>=0x60u||header[2]>=0x75u||header[3]!=2u)
        throw std::invalid_argument("Native location query requires an actual Mode2 sector header");
    // These are SDK-owned RAM fields, not emulated device registers. The
    // command is accepted here; its actual bytes become visible on Service.
    s.Write8(0x800573D4u,0u);s.Write8(0x80057119u,16u);
    header_=header;pending_=true;++requests_;
}
bool LocationQuery::Service(Services& s,bool allowed){
    if(faulted_)throw std::logic_error("Native location query is faulted");
    if(!pending_||!allowed)return false;
    try{
        for(uint32_t i=0;i<8u;++i)s.Write8(0x800882F8u+i,header_[i]);
        s.Write8(0x800573D4u,2u);pending_=false;++replies_;
        const uint32_t callback=s.Read32(0x800570F8u);
        if(callback)s.CallVoid(callback,{2u,0x800882F8u});
        return true;
    }catch(...){faulted_=true;throw;}
}
void SelectText80024C84(Services& s,uint32_t row){
    s.Write16(0x8008ECFAu,0u);s.Write32(0x8008ECE4u,0u);
    s.Write32(0x8006EDBCu,0u);s.Write32(0x8006ED60u,0u);
    if(row){
        const uint32_t language=H(s.Read16(0x800916D8u));
        const uint32_t count=s.Read32(row+24u),events=s.Read32(row+20u);
        const uint32_t selected=s.Read32(row+(language<<2u)),base=s.Read32(row);
        s.Write32(0x8006EDACu,count);s.Write32(0x8006EDB0u,events);
        s.Write32(0x8006EDA4u,base);s.Write32(0x8006EDB4u,selected);
    }else{
        s.Write32(0x8006EDACu,0u);s.Write32(0x8006EDB0u,0u);
    }
    // The null-row branch leaves V0 unspecified. Its caller discards it.
}
void EndText80024CF0(Services&,uint32_t){ /* Original JR RA / NOP. */ }
int32_t UpdateText80024CF8(Services& s,uint32_t work){
    const int32_t primary=S(H(s.Read16(0x8008ECF8u)));
    if(primary>0){
        const uint16_t next=uint16_t(primary-1);s.Write16(0x8008ECF8u,next);
        if(!next){s.Write32(0x8008ECE0u,0u);s.Write32(work+264u,0u);}
    }
    const int32_t subtitle=S(H(s.Read16(0x8008ECFAu)));
    if(subtitle>0){
        const uint16_t next=uint16_t(subtitle-1);s.Write16(0x8008ECFAu,next);
        if(!next){s.Write32(0x8008ECE4u,0u);s.Write32(work+268u,0u);}
    }
    const uint32_t index=s.Read32(0x8006EDBCu),count=s.Read32(0x8006EDACu);
    uint32_t event=0;
    if(S(index)<S(count)){
        const uint32_t table=s.Read32(0x8006EDB0u);
        const uint32_t frame=s.Read8(work+7u),at=table+(index<<4u);
        const uint32_t minute=s.Read16(at),second=s.Read8(at+2u),tick=s.Read8(at+3u);
        const uint32_t eventTime=(minute<<16u)+(second<<8u)+tick;
        const uint32_t nowMinute=s.Read16(work+4u),nowSecond=s.Read8(work+6u);
        const uint32_t now=(nowMinute<<16u)+(nowSecond<<8u)+frame;
        if(S(eventTime-now)<=0){s.Write32(0x8006EDBCu,index+1u);event=at;}
    }
    if(!event)return 0;
    const uint32_t language=H(s.Read16(0x800916D8u));
    const int32_t text=S(H(s.Read16(event+(language<<1u)+6u)));
    if(text>0){
        const uint32_t table=s.Read32(0x8006EDB4u),value=s.Read32(table+uint32_t(text)*4u);
        s.Write32(0x8008ECE4u,value);s.Write16(0x8008ECFAu,s.Read16(event+4u));
    }
    const uint32_t value=s.Read32(0x8008ECE4u);s.Write32(work+268u,value);return S(work);
}
int32_t Ready8001A750(Services& s){
    // 800364D0 is the unchanged forwarding wrapper to this same Sync body.
    // Copy the real reply into owned native storage, not a fake PSX stack slot.
    const auto reply=PrStage2CdCommandDirect::SyncReply80037070(s,1u);
    if(reply.status!=2)return 0;
    RequireReply(reply);
    if(reply.bytes[0]&0x20u)return 1;
    s.Call(0x80036678u,{1u,0u});return 0;
}
int32_t RefreshLocation8001A3C8(Services& s){
    const auto reply=PrStage2CdCommandDirect::SyncReply80037070(s,1u);
    if(reply.status==5){
        RequireReply(reply);
        if(reply.bytes[0]&0x10u){s.Write32(0x80049428u,1u);return 0;}
    }
    if(reply.status!=2)return 0;
    RequireReply(reply);
    if(s.Call(0x800363A4u,{})==13)return 0;
    s.Write32(0x80049428u,0u);
    for(uint32_t i=0;i<3u;++i)s.Write8(0x800493F4u+i,reply.bytes[i]);
    return 1;
}
int32_t RequestLocation8001A280(Services& s){
    const uint32_t error=s.Read32(0x80049428u);
    return error?S(error):s.Call(0x80036678u,{16u,0u});
}
int32_t LastCommand800363A4(Services& s){return s.Read8(0x80057119u);}
int32_t MediaError8001A3B8(Services& s){return S(s.Read32(0x80049428u));}
int32_t Position8001A7A4(Services& s,uint32_t start){
    const uint32_t first=uint32_t(s.Call(0x80036A78u,{0x800493F4u}));
    s.Write32(0x80049404u,first+150u);
    const uint32_t second=uint32_t(s.Call(0x80036A78u,{0x800493F4u}));
    return S(second-start);
}
int32_t AtEnd8001A7F8(Services& s,uint32_t record){
    const uint32_t tail=s.Read32(record+44u),length=s.Read32(record+8u),position=s.Read32(0x80049404u);
    return S(position)<S(tail+length)?0:1;
}
int32_t Prepare8001EC54(Services& s,uint32_t work,uint32_t mode){
    const uint32_t buffer=uint32_t(s.Call(0x8004019Cu,{}));
    const uint32_t pool=s.Read32(0x8006ED50u+(buffer<<2u));s.Write32(WorkBuffer,buffer);
    s.CallVoid(0x80040F90u,{pool});
    s.Call(0x80040CC8u,{0u,0u,0x80087288u+20u*s.Read32(WorkBuffer)});
    if(mode==2u){s.Call(0x8001FCBCu,{8u,0u});return s.Call(0x8001FDC0u,{0u});}
    if(s.Read16(0x800916DCu)){
        const uint32_t saved=s.Read32(WorkBuffer),text=s.Read32(work+268u);
        return s.Call(0x8001DB00u,{text,0x80087288u+20u*saved});
    }
    return s.Call(0x8001CE30u,{5u});
}
int32_t Submit8001ED3C(Services& s,uint32_t){return s.Call(0x80040CA4u,{0x80087288u+20u*s.Read32(WorkBuffer)});}
int32_t Swap8001ED74(Services& s){return s.Call(0x80040370u,{});}
int32_t DecodeIfReady80027528(Services& s){
    const uint32_t control=s.Read32(Control);
    if(!s.Read32(control+32u))return s.Call(0x800273A4u,{});
    const uint32_t available=s.Read32(control+28u);
    return available?S(available):s.Call(0x800273A4u,{});
}
int32_t Blit8002756C(Services& s){
    if(s.Read32(0x8006ED88u)<2u)return 1;
    const int32_t buffer=s.Call(0x8004019Cu,{});
    const uint32_t y=s.Read32(s.Read32(Control)+24u);
    const uint32_t control=s.Read32(Control),height=s.Read32(control+12u);
    PrStage2LifecycleDirect::ImageRect rect{0u,uint16_t(y+(buffer?240u:0u)),16u,uint16_t(height)};
    const uint32_t width=s.Read32(control+8u),image=s.Read32(control+4u);
    const int32_t columns=S(width)/16;
    int32_t result=S(control);uint32_t x=0;
    for(int32_t left=columns;left>0;--left){
        const uint32_t current=s.Read32(Control),origin=s.Read32(current+20u);
        rect.x=uint16_t(origin+x);
        const uint32_t h=s.Read32(current+12u),rounded=S(h+15u)<0?h+30u:h+15u;
        const uint32_t offset=((x<<4u)*Asr(rounded,4u))<<1u;
        x+=16u;result=s.LoadImage80044D64(rect,image+offset);
    }
    return result;
}
int32_t Caption8001DB00(Services& s,uint32_t text,uint32_t table){
    s.CallVoid(0x8001B730u,{24u,184u,0u});s.Call(0x8001B954u,{text,480u,table});
    return s.Call(0x8001C864u,{5u});
}
int32_t ReadPad80035510(Services& s,uint32_t){
    return S(s.ReadPad80035510());
}
bool TryCall(Services& s,uint32_t fn,std::initializer_list<uint32_t> args,int32_t& result){
    size_t n;
    switch(fn){
    case 0x80024C84u:case 0x80024CF0u:throw std::invalid_argument("Movie void call has no portable scalar return");
    case 0x80024CF8u:case 0x8001A7A4u:case 0x8001A7F8u:case 0x8001ED3Cu:case 0x80035510u:case 0x80036A78u:n=1;break;
    case 0x8001EC54u:case 0x8001DB00u:n=2;break;
    case 0x8001A750u:case 0x8001A3C8u:case 0x8001A280u:case 0x800363A4u:case 0x8001A3B8u:case 0x8001ED74u:case 0x80027528u:case 0x8002756Cu:n=0;break;
    default:return false;
    }
    if(args.size()!=n)throw std::invalid_argument("Movie foreground argument count");const auto a=args.begin();
    switch(fn){
    case 0x80024CF8u:result=UpdateText80024CF8(s,a[0]);break;
    case 0x8001A750u:result=Ready8001A750(s);break;
    case 0x8001A3C8u:result=RefreshLocation8001A3C8(s);break;
    case 0x8001A280u:result=RequestLocation8001A280(s);break;
    case 0x800363A4u:result=LastCommand800363A4(s);break;
    case 0x8001A3B8u:result=MediaError8001A3B8(s);break;
    case 0x8001A7A4u:result=Position8001A7A4(s,a[0]);break;
    case 0x8001A7F8u:result=AtEnd8001A7F8(s,a[0]);break;
    case 0x8001EC54u:result=Prepare8001EC54(s,a[0],a[1]);break;
    case 0x8001ED3Cu:result=Submit8001ED3C(s,a[0]);break;
    case 0x8001ED74u:result=Swap8001ED74(s);break;
    case 0x80027528u:result=DecodeIfReady80027528(s);break;
    case 0x8002756Cu:result=Blit8002756C(s);break;
    case 0x8001DB00u:result=Caption8001DB00(s,a[0],a[1]);break;
    case 0x80035510u:result=ReadPad80035510(s,a[0]);break;
    case 0x80036A78u:result=PrStage2IntLoaderDirect::LocationToLba80036A78(s,a[0]);break;
    }
    return true;
}
bool TryCallVoid(Services& s,uint32_t fn,std::initializer_list<uint32_t> args){
    if(fn==0x80024C84u||fn==0x80024CF0u){
        if(args.size()!=1u)throw std::invalid_argument("Movie text call argument count");
        if(fn==0x80024C84u)SelectText80024C84(s,*args.begin());else EndText80024CF0(s,*args.begin());
        return true;
    }
    int32_t ignored;return TryCall(s,fn,args,ignored);
}
}
