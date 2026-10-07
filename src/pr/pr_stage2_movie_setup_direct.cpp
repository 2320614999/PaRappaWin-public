#include "pr_stage2_movie_setup_direct.h"
#include <stdexcept>

namespace PrStage2MovieSetupDirect {
namespace {
int32_t S(uint32_t v){return v<0x80000000u?int32_t(v):int32_t(int64_t(v)-0x100000000LL);}
uint32_t H(uint32_t v){v&=65535u;return v<32768u?v:v|0xFFFF0000u;}
constexpr uint32_t Control=0x8006ED78u; // Original GP+338h, NOT 800965A8.
}
void Volume8001A478(Services& s,uint32_t volume){
    const uint32_t value=H(volume);
    s.CallVoid(0x8002AB24u,{0u,value,value});
}
void Mute8001A4A4(Services& s,uint32_t mode){s.CallVoid(0x8001A478u,{mode==1u?0u:127u});}
int32_t Allocate80027238(Services& s,uint32_t bytes,uint32_t /*unusedLabel*/){
    const uint32_t address=uint32_t(s.Call(0x80025A70u,{bytes}));
    if(address==0u)s.Exit(0x80047F3Cu,{1u});
    s.CallVoid(0x80025C44u,{address,bytes});
    return S(address);
}
int32_t Decoder80027288(Services& s,uint32_t kind){
    const uint32_t row=0x80054614u+24u*kind;
    s.Write32(Control,uint32_t(s.Call(0x80027238u,{36u,0x8001136Cu})));
    const uint32_t sector=uint32_t(s.Call(0x80027238u,{0x10000u,0x80011378u}));
    const uint32_t vlcBytes=s.Read32(row+12u);
    s.Write32(Control+4u,sector);
    const uint32_t vlc=uint32_t(s.Call(0x80027238u,{vlcBytes,0x80011388u}));
    const uint32_t imageBytes=s.Read32(row+16u);
    s.Write32(Control+8u,vlc);
    s.Write32(Control+12u,uint32_t(s.Call(0x80027238u,{imageBytes,0x80011394u})));
    s.Call(0x800473ECu,{0u});
    s.CallVoid(0x8003624Cu,{s.Read32(Control+4u),32u});
    s.Call(0x80047658u,{0x80027220u});
    const uint32_t control=s.Read32(Control);
    const uint32_t finalVlc=s.Read32(Control+8u),finalImage=s.Read32(Control+12u);
    s.Write32(control,finalVlc);s.Write32(control+4u,finalImage);
    for(uint32_t p:{8u,12u,16u,28u,32u})s.Write32(control+p,0u);
    s.Write32(control+20u,H(s.Read16(row+4u)));
    const uint32_t y=H(s.Read16(row+6u));
    s.Write32(control+24u,y);
    if(kind==2u)return 2;
    const uint16_t subtitles=s.Read16(0x800916DCu);
    const uint32_t adjusted=y+23u; // Executed delay slot even when subtitles != 0.
    if(subtitles==0u)s.Write32(control+24u,adjusted);
    return S(adjusted);
}
int32_t OutputCallback80027220(Services& s){
    const uint32_t p=s.Read32(Control);s.Write32(p+32u,0u);return S(p);
}
void RingSlots8003954C(Services& s,uint32_t first,uint32_t count){
    // Its zero-count V0 is unspecified. Callers here discard it explicitly.
    for(uint32_t i=0;i<count;++i){const uint32_t p=(first+i)<<5u;s.Write32(s.Read32(0x800965ACu)+p,0u);}
}
void ClearRing80039260(Services& s){
    const uint32_t count=s.Read32(0x801C3868u);
    for(uint32_t p:{0x80095C58u,0x80095C54u,0x80095C50u,0x80092858u})s.Write32(p,0u);
    s.CallVoid(0x8003954Cu,{0u,count});
    s.Write32(0x80091640u,0u);s.Write16(0x8008ECD4u,0u);s.Write32(0x8008ECA4u,0u);
}
void SetRing8003624C(Services& s,uint32_t buffer,uint32_t count){
    s.Write32(0x800965ACu,buffer);s.Write32(0x801C3868u,count);s.CallVoid(0x80039260u,{});
}
int32_t SetOutputCallback80047658(Services& s,uint32_t callback){return s.Call(0x800357A4u,{1u,callback});}
int32_t StopReset80027664(Services& s){
    s.Call(0x80047658u,{0u});s.CallVoid(0x800392C0u,{});
    s.Call(0x80025AF8u,{});s.Call(0x80025AF8u,{});s.Call(0x80025AF8u,{});
    return s.Call(0x80025AF8u,{});
}
int32_t Reset800473EC(Services& s,uint32_t mode){
    if(mode==0u)s.Call(0x80035744u,{});
    return s.Call(0x8004767Cu,{mode});
}
int32_t ResetLower8004767C(Services& s,uint32_t mode){
    if(mode>1u)return s.Call(0x80047FFCu,{0x8001263Cu,mode});
    s.Write32(s.Read32(0x8005D990u),0x80000000u);
    s.Write32(s.Read32(0x8005D964u),0u);
    s.Write32(s.Read32(0x8005D970u),0u);
    if(mode==1u){
        const uint32_t output=s.Read32(0x8005D970u),control=s.Read32(0x8005D990u);
        (void)s.Read32(output);s.Write32(control,0x60000000u);return 0x60000000;
    }
    s.Write32(s.Read32(0x8005D990u),0x60000000u);
    s.Call(0x80047778u,{0x8005D854u,32u});
    return s.Call(0x80047778u,{0x8005D8D8u,32u});
}
int32_t InputWait8004789C(Services& s){
    uint32_t remaining=0x100000u;
    uint32_t status=s.Read32(s.Read32(0x8005D990u));
    while(status&0x20000000u){
        if(--remaining==0xFFFFFFFFu){s.Call(0x800479CCu,{0x80012658u});return -1;}
        s.AwaitDeviceProgress(0x1F801824u);
        status=s.Read32(s.Read32(0x8005D990u));
    }
    return 0;
}
int32_t Input80047778(Services& s,uint32_t packet,uint32_t words){
    s.Call(0x8004789Cu,{});
    const uint32_t dpcr=s.Read32(0x8005D994u);s.Write32(dpcr,s.Read32(dpcr)|0x88u);
    s.Write32(s.Read32(0x8005D95Cu),packet+4u);
    s.Write32(s.Read32(0x8005D960u),((words>>5u)<<16u)|32u);
    const uint32_t port=s.Read32(0x8005D98Cu),command=s.Read32(packet);s.Write32(port,command);
    const uint32_t channel=s.Read32(0x8005D964u);s.Write32(channel,0x01000201u);return S(channel);
}
bool TryCall(Services& s,uint32_t fn,std::initializer_list<uint32_t> args,int32_t& result){
    size_t n;
    switch(fn){
    case 0x8001A478u:case 0x8001A4A4u:case 0x8003624Cu:case 0x80039260u:case 0x8003954Cu:
        throw std::invalid_argument("Movie setup void call cannot manufacture a scalar return");
    case 0x80027238u:case 0x80047778u:n=2;break;
    case 0x80027288u:case 0x800473ECu:case 0x8004767Cu:case 0x80047658u:case 0x80025A70u:n=1;break;
    case 0x80027220u:case 0x8004789Cu:case 0x80027664u:case 0x80025AF8u:n=0;break;
    default:return false;
    }
    if(args.size()!=n)throw std::invalid_argument("Movie setup argument count");const auto a=args.begin();
    switch(fn){
    case 0x80025A70u:result=PrStage2LifecycleDirect::PushResourceHeap80025A70(s,a[0]);break;
    case 0x80025AF8u:result=PrStage2LifecycleDirect::PopResourceHeap80025AF8(s);break;
    case 0x80027664u:result=StopReset80027664(s);break;
    case 0x80027238u:result=Allocate80027238(s,a[0],a[1]);break;
    case 0x80027288u:result=Decoder80027288(s,a[0]);break;
    case 0x80027220u:result=OutputCallback80027220(s);break;
    case 0x800473ECu:result=Reset800473EC(s,a[0]);break;
    case 0x8004767Cu:result=ResetLower8004767C(s,a[0]);break;
    case 0x80047658u:result=SetOutputCallback80047658(s,a[0]);break;
    case 0x8004789Cu:result=InputWait8004789C(s);break;
    case 0x80047778u:result=Input80047778(s,a[0],a[1]);break;
    }
    return true;
}
bool TryCallVoid(Services& s,uint32_t fn,std::initializer_list<uint32_t> args){
    size_t n;
    switch(fn){
    case 0x8001A478u:case 0x8001A4A4u:n=1;break;
    case 0x8003624Cu:case 0x8003954Cu:n=2;break;
    case 0x80039260u:n=0;break;
    default:return false;
    }
    if(args.size()!=n)throw std::invalid_argument("Movie setup void argument count");const auto a=args.begin();
    switch(fn){
    case 0x8001A478u:Volume8001A478(s,a[0]);break;
    case 0x8001A4A4u:Mute8001A4A4(s,a[0]);break;
    case 0x8003624Cu:SetRing8003624C(s,a[0],a[1]);break;
    case 0x80039260u:ClearRing80039260(s);break;
    case 0x8003954Cu:RingSlots8003954C(s,a[0],a[1]);break;
    }
    return true;
}
}
