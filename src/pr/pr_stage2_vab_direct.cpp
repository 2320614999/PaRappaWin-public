#include "pr_stage2_vab_direct.h"
#include <array>
#include <stdexcept>

namespace PrStage2VabDirect {
namespace {
uint32_t U(int32_t v) { return static_cast<uint32_t>(v); }
int32_t S(uint32_t v) { return v<0x80000000u?int32_t(v):int32_t(int64_t(v)-0x100000000LL); }
uint32_t H(uint32_t v) { v&=65535u;return v<32768u?v:v|0xFFFF0000u; }
uint32_t ASR(uint32_t v,uint32_t n) { n&=31u;return !n?v:(v>>n)|((v&0x80000000u)?(~0u<<(32u-n)):0u); }
constexpr uint32_t Mask=0x0FFFFFFFu, Deleted=0x2FFFFFFFu, Tail=0x40000000u, FreeBit=0x80000000u;
constexpr uint32_t Capacity=0x80055DC4u, Last=0x80055DC8u, List=0x80055DCCu;
}

int32_t InitializeAllocator80035394(Services& s,uint32_t count,uint32_t records) {
    if(S(count)<=0)return 0;
    const uint32_t shift=s.Read32(0x800555ECu);
    s.Write32(records,0x40001010u);
    s.Write32(List,records);s.Write32(Last,0u);s.Write32(Capacity,count);
    s.Write32(records+4u,(0x10000u<<(shift&31u))-0x1010u);
    return S(count);
}

void Compact8002E0D8(Services& s) {
    // Five original passes: adjacent free coalescing, zero-sized tombstones,
    // address ordering, tombstone removal, and trailing free/tail coalescing.
    // All storage is the source-owned list in RAM, not a host allocator.
    uint32_t end=s.Read32(Last);
    if(S(end)>=0) {
        const uint32_t base=s.Read32(List);
        uint32_t i=0u,p=base;
        do {
            bool merged=false;
            if(s.Read32(p)&FreeBit) {
                uint32_t next=i+1u;
                while(s.Read32(base+8u*next)==Deleted)++next;
                const uint32_t q=base+8u*next,word=s.Read32(q);
                if((word&FreeBit)&&((word&Mask)==((s.Read32(p)&Mask)+s.Read32(p+4u)))) {
                    s.Write32(q,Deleted);
                    const uint32_t first=s.Read32(p+4u),second=s.Read32(q+4u);
                    s.Write32(p+4u,first+second);merged=true;
                }
            }
            if(!merged){p+=8u;++i;}
        }while(S(s.Read32(Last))>=S(i));
        end=s.Read32(Last);
    }
    if(S(end)>=0) {
        const uint32_t base=s.Read32(List),bound=end;
        for(uint32_t i=0u;S(i)<=S(bound);++i)
            if(s.Read32(base+8u*i+4u)==0u)s.Write32(base+8u*i,Deleted);
    }
    end=s.Read32(Last);
    if(S(end)>=0) {
        const uint32_t base=s.Read32(List);
        for(uint32_t i=0u;S(i)<=S(end);++i) {
            const uint32_t p=base+8u*i;
            if(s.Read32(p)&Tail)break;
            const uint32_t first=i+1u;
            if(S(first)<=S(end)) {
                const uint32_t bound=s.Read32(Last);
                for(uint32_t j=first;S(j)<=S(bound);++j) {
                    const uint32_t q=base+8u*j,next=s.Read32(q);
                    if(next&Tail)break;
                    const uint32_t here=s.Read32(p);
                    if((next&Mask)<(here&Mask)) {
                        s.Write32(p,next);
                        const uint32_t nextSize=s.Read32(q+4u),hereSize=s.Read32(p+4u);
                        s.Write32(p+4u,nextSize);s.Write32(q,here);s.Write32(q+4u,hereSize);
                    }
                }
            }
            end=s.Read32(Last);
        }
    }
    end=s.Read32(Last);
    if(S(end)>=0) {
        const uint32_t base=s.Read32(List);
        for(uint32_t i=0u;S(i)<=S(end);++i) {
            const uint32_t p=base+8u*i,word=s.Read32(p);
            if(word&Tail)break;
            if(word==Deleted) {
                const uint32_t q=base+8u*end;
                s.Write32(p,s.Read32(q));const uint32_t size=s.Read32(q+4u);
                s.Write32(Last,i);s.Write32(p+4u,size);break;
            }
            end=s.Read32(Last);
        }
    }
    uint32_t i=s.Read32(Last)-1u;
    if(S(i)<0)return;
    const uint32_t base=s.Read32(List);
    for(uint32_t p=base+8u*i;;p-=8u) {
        const uint32_t word=s.Read32(p);
        if(!(word&FreeBit))return;
        const uint32_t oldLast=s.Read32(Last);
        s.Write32(p,(word&Mask)|Tail);
        const uint32_t size=s.Read32(p+4u);
        s.Write32(Last,i);
        const uint32_t tailSize=s.Read32(base+8u*oldLast+4u);
        --i;s.Write32(p+4u,size+tailSize);
        if(S(i)<0)return;
    }
}

void Free8002E05C(Services& s,uint32_t address) {
    const uint32_t count=s.Read32(Capacity);
    if(S(count)>0) {
        uint32_t p=s.Read32(List);
        for(uint32_t i=0u;S(i)<S(count);++i,p+=8u) {
            const uint32_t word=s.Read32(p);
            if(word&Tail)break;
            if(word==address){s.Write32(p,address|FreeBit);break;}
        }
    }
    Compact8002E0D8(s);
}

int32_t Allocate8002E87C(Services& s,uint32_t bytes) {
    uint32_t reserve=0u;
    if(s.Read32(0x8005562Cu)!=0u) {
        const uint32_t value=s.Read32(0x80055630u),shift=s.Read32(0x800555ECu);
        reserve=(0x10000u-value)<<(shift&31u);
    }
    const uint32_t rounding=s.Read32(0x800555F4u);
    uint32_t rounded=bytes;
    if(bytes&~rounding)rounded+=rounding;
    const uint32_t shift=s.Read32(0x800555ECu);
    const uint32_t length=ASR(rounded,shift)<<(shift&31u);
    uint32_t selected=0xFFFFFFFFu;
    if(s.Read32(s.Read32(List))&Tail)selected=0u;
    else {
        Compact8002E0D8(s);
        const uint32_t bound=s.Read32(Capacity);
        if(S(bound)>0) {
            const uint32_t base=s.Read32(List);
            for(uint32_t i=0u;S(i)<S(bound);++i) {
                const uint32_t p=base+8u*i,word=s.Read32(p);
                if((word&Tail)||((word&FreeBit)&&s.Read32(p+4u)>=length)){selected=i;break;}
            }
        }
    }
    if(selected==0xFFFFFFFFu)return -1;
    const uint32_t offset=selected*8u,base=s.Read32(List),p=base+offset;
    const uint32_t word=s.Read32(p);
    if(word&Tail) {
        if(S(selected)>=S(s.Read32(Capacity)))return -1;
        if(s.Read32(p+4u)-reserve<length)return -1;
        const uint32_t next=selected+1u,q=base+8u*next;
        s.Write32(q,((s.Read32(p)&Mask)+length)|Tail);
        s.Write32(q+4u,s.Read32(p+4u)-length);
        const uint32_t first=s.Read32(p);
        s.Write32(Last,next);s.Write32(p+4u,length);s.Write32(p,first&Mask);
        Compact8002E0D8(s);
        return S(s.Read32(s.Read32(List)+offset));
    }
    const uint32_t available=s.Read32(p+4u);
    if(length<available) {
        const uint32_t end=s.Read32(Last),capacity=s.Read32(Capacity);
        if(S(end)<S(capacity)) {
            const uint32_t q=base+8u*end;
            const uint32_t old=s.Read32(q),oldSize=s.Read32(q+4u);
            s.Write32(q,(word+length)|FreeBit);s.Write32(q+4u,available-length);
            s.Write32(Last,end+1u);s.Write32(q+8u,old);s.Write32(q+12u,oldSize);
        }
    }
    const uint32_t current=s.Read32(List)+offset,value=s.Read32(current);
    s.Write32(current+4u,length);s.Write32(current,value&Mask);
    Compact8002E0D8(s);
    return S(s.Read32(s.Read32(List)+offset));
}

int32_t SetTransferBusy8002EB44(Services& s,uint32_t busy) {
    s.Write32(0x800555F8u,busy==1u?0u:1u);return 1;
}
int32_t TransferBusy8002EB70(Services& s){return s.Read32(0x800555F8u)==0u?1:0;}

int32_t Close8002DF80(Services& s,uint32_t bank) {
    if((bank&65535u)>=16u)return 0;
    const uint32_t id=H(bank),offset=id*4u;
    if(s.Read8(0x800928F8u+id)!=1u)return S(offset);
    Free8002E05C(s,s.Read32(0x801C35F8u+offset));
    s.Write8(0x800928F8u+id,0u);
    const uint32_t value=uint32_t(s.Read16(0x801C35F0u))-1u;
    s.Write16(0x801C35F0u,uint16_t(value));return S(value);
}
int32_t CloseCurrent80027120(Services& s) {
    const uint32_t bank=H(s.Read16(0x800943A8u));
    if(S(bank)>=0){(void)Close8002DF80(s,bank);s.Write16(0x800943A8u,65535u);}
    return 0;
}

int32_t Open8002E474(Services& s,uint32_t header,uint32_t bank,uint32_t suppliedAddress,uint32_t address) {
    if(TransferBusy8002EB70(s)==1)return -1;
    SetTransferBusy8002EB44(s,1u);
    uint32_t selected=16u,id=H(bank);
    if(S(id)<16) {
        if(id==0xFFFFFFFFu) {
            for(uint32_t i=0;i<16u;++i)if(s.Read8(0x800928F8u+i)==0u) {
                s.Write8(0x800928F8u+i,1u);
                const uint32_t count=s.Read16(0x801C35F0u);selected=i;
                s.Write16(0x801C35F0u,uint16_t(count+1u));break;
            }
        }else if(s.Read8(0x800928F8u+id)==0u) {
            s.Write8(0x800928F8u+id,1u);const uint32_t count=s.Read16(0x801C35F0u);
            selected=bank;s.Write16(0x801C35F0u,uint16_t(count+1u));
        }
    }
    id=H(selected);
    if(S(id)>=16){SetTransferBusy8002EB44(s,0u);return -1;}
    const auto fail=[&]() {
        s.Write8(0x800928F8u+id,0u);SetTransferBusy8002EB44(s,0u);
        s.Write16(0x801C35F0u,uint16_t(uint32_t(s.Read16(0x801C35F0u))-1u));return -1;
    };
    s.Write32(0x80091648u+4u*id,header);
    const uint32_t magic=s.Read32(header),programs=header+32u;
    if((magic>>8u)!=0x564142u)return fail();
    uint16_t capacity=64u;
    if((magic&255u)==0x70u&&S(s.Read32(header+4u))>=5)capacity=128u;
    s.Write16(0x800917A8u,capacity);
    const uint32_t count=s.Read16(header+18u),limit=H(s.Read16(0x800917A8u));
    if(S(limit)<int32_t(count))return fail();
    s.Write32(0x800901F8u+4u*id,programs);
    const uint32_t tones=programs+(limit<<4u);
    uint32_t compactIndex=0u;
    for(uint32_t i=0u;S(i)<S(limit);++i) {
        const uint32_t p=programs+16u*i;const uint8_t countHere=s.Read8(p);
        s.Write32(p+8u,compactIndex);if(countHere)++compactIndex;
    }
    s.Write32(0x80091690u+4u*id,tones);
    const uint32_t entries=s.Read16(header+18u),samples=s.Read8(header+22u);
    uint32_t sizes=tones+(entries<<9u),total=0u;
    // The original private table is consumed only through the inclusive sample
    // count. No native pointer or invented PSX stack location escapes here.
    std::array<uint32_t,256> lengths{};
    for(uint32_t i=0u;i<256u;++i,sizes+=2u)if(i<=samples) {
        const uint32_t version=s.Read32(header+4u),size=s.Read16(sizes);
        lengths[i]=size<<(S(version)>=5?3u:2u);total+=lengths[i];
    }
    s.Write32(0x801C3828u+4u*id,sizes);
    uint32_t base=address;
    if((suppliedAddress<<16u)==0u) {
        base=U(Allocate8002E87C(s,total));if(base==0xFFFFFFFFu)return fail();
    }
    if(base+total>0x80000u)return fail();
    s.Write32(0x801C35F8u+4u*id,base);
    uint32_t cumulative=0u;
    for(uint32_t i=0u;i<=samples;++i) {
        cumulative+=lengths[i];
        s.Write16(programs+16u*(i/2u)+((i&1u)?14u:12u),uint16_t((base+cumulative)>>3u));
    }
    s.Write32(0x801C35B0u+4u*id,cumulative);s.Write8(0x800928F8u+id,2u);
    return S(id);
}
int32_t OpenHeader8002E3D8(Services& s,uint32_t header,uint32_t bank) {
    return S(H(U(Open8002E474(s,header,H(bank),0u,0u))));
}
int32_t OpenCurrent80027078(Services& s,uint32_t header) {
    const uint32_t value=U(OpenHeader8002E3D8(s,header,0xFFFFFFFFu));
    s.Write16(0x800943A8u,uint16_t(value));return S(H(value))>=0?1:0;
}

int32_t AlignAddress8002A5CC(Services& s,uint32_t reg,uint32_t address) {
    if(s.Read32(0x800555E8u)!=0u) {
        const uint32_t modulus=s.Read32(0x800555F0u);
        if(!modulus)s.Break(0x8002A5F8u,7u);
        if(address%modulus)address=(address+modulus)&~s.Read32(0x800555F4u);
    }
    const uint32_t units=address>>(s.Read32(0x800555ECu)&31u);
    if(reg==0xFFFFFFFEu)return S(address);
    if(reg==0xFFFFFFFFu)return int32_t(units&65535u);
    const uint32_t base=s.Read32(0x800555C8u);
    s.Write16(base+2u*reg,uint16_t(units));return S(address);
}
int32_t WriteRegister8002A584(Services& s,uint32_t reg,uint32_t value,uint32_t shifted) {
    const uint32_t base=s.Read32(0x800555C8u),address=base+2u*reg;
    if(shifted)value>>=s.Read32(0x800555ECu)&31u;
    s.Write16(address,uint16_t(value));return S(address);
}
int32_t SetTransferMode8002ECDC(Services& s,uint32_t mode) {
    const uint32_t result=mode==1u?1u:0u;
    s.Write32(0x80055624u,mode);s.Write32(0x800555E0u,result);return int32_t(result);
}
int32_t SetTransferAddress8002ECA0(Services& s,uint32_t address) {
    const uint32_t units=U(AlignAddress8002A5CC(s,0xFFFFFFFFu,address));
    s.Write16(0x800555C4u,uint16_t(units));return S(address);
}
int32_t WriteTransfer8002EC40(Services& s,uint32_t source,uint32_t bytes) {
    const uint32_t count=bytes>0x7F000u?0x7F000u:bytes;
    (void)s.Call(0x8002A494u,{source,count});
    if(s.Read32(0x800555FCu)==0u)s.Write32(0x800555F8u,0u);
    return S(count);
}
int32_t Transfer8002EB80(Services& s,uint32_t source,uint32_t bank) {
    const uint32_t id=H(bank);
    // Preserve the source's <17 test (not a host-invented <16 correction).
    if((bank&65535u)>=17u||s.Read8(0x800928F8u+id)!=2u) {
        SetTransferBusy8002EB44(s,0u);return -1;
    }
    const uint32_t address=s.Read32(0x801C35F8u+4u*id);
    SetTransferMode8002ECDC(s,0u);SetTransferAddress8002ECA0(s,address);
    WriteTransfer8002EC40(s,source,s.Read32(0x801C35B0u+4u*id));
    s.Write8(0x800928F8u+id,1u);return S(id);
}
int32_t TransferCurrent800270D4(Services& s,uint32_t source) {
    return Transfer8002EB80(s,source,H(s.Read16(0x800943A8u)));
}
int32_t TransferCompleted8002EF28(Services& s,uint32_t wait) {
    if(s.Read32(0x80055624u)==1u||s.Read32(0x800555F8u)==1u)return 1;
    uint32_t result=U(s.Call(0x800489C0u,{s.Read32(0x80055678u)}));
    if(wait==1u) {
        while(result==0u) {
            // Pure TestEvent is still nonblocking. Only the source's explicit
            // blocking loop yields its retained activation to the device owner.
            s.AwaitDeviceProgress(0x1F8010C8u);
            result=U(s.Call(0x800489C0u,{s.Read32(0x80055678u)}));
        }
        result=1u;
    }
    if(result==1u)s.Write32(0x800555F8u,1u);
    return S(result);
}
int32_t TransferPending8002EEFC(Services& s,uint32_t wait) {
    // Despite its legacy label, this wrapper returns the signed completion
    // result unchanged. It does NOT invert TestEvent success.
    return S(H(U(TransferCompleted8002EF28(s,H(wait)))));
}
int32_t WaitCurrent800270FC(Services& s,uint32_t mode) {
    return TransferPending8002EEFC(s,mode==1u?1u:0u);
}

bool TryCall(Services& s,uint32_t fn,std::initializer_list<uint32_t> args,int32_t& result) {
    size_t n;
    switch(fn) {
    case 0x80027120u:case 0x8002EB70u:n=0;break;
    case 0x8002DF80u:case 0x8002E87Cu:case 0x8002EB44u:case 0x80027078u:
    case 0x800270D4u:case 0x8002ECDCu:case 0x8002ECA0u:case 0x8002EF28u:case 0x8002EEFCu:case 0x800270FCu:n=1;break;
    case 0x80035394u:case 0x8002E3D8u:case 0x8002EB80u:case 0x8002EC40u:case 0x8002A5CCu:n=2;break;
    case 0x8002A584u:n=3;break;
    case 0x8002E474u:n=4;break;
    case 0x8002E05Cu:case 0x8002E0D8u:throw std::invalid_argument("VAB allocator effect has no promised scalar return");
    default:return false;
    }
    if(args.size()!=n)throw std::invalid_argument("S2 VAB argument count mismatch");
    const auto a=args.begin();
    switch(fn) {
    case 0x80027120u:result=CloseCurrent80027120(s);break;
    case 0x8002EB70u:result=TransferBusy8002EB70(s);break;
    case 0x8002DF80u:result=Close8002DF80(s,a[0]);break;
    case 0x80035394u:result=InitializeAllocator80035394(s,a[0],a[1]);break;
    case 0x8002E87Cu:result=Allocate8002E87C(s,a[0]);break;
    case 0x8002EB44u:result=SetTransferBusy8002EB44(s,a[0]);break;
    case 0x8002E474u:result=Open8002E474(s,a[0],a[1],a[2],a[3]);break;
    case 0x8002E3D8u:result=OpenHeader8002E3D8(s,a[0],a[1]);break;
    case 0x80027078u:result=OpenCurrent80027078(s,a[0]);break;
    case 0x8002EB80u:result=Transfer8002EB80(s,a[0],a[1]);break;
    case 0x800270D4u:result=TransferCurrent800270D4(s,a[0]);break;
    case 0x8002ECDCu:result=SetTransferMode8002ECDC(s,a[0]);break;
    case 0x8002ECA0u:result=SetTransferAddress8002ECA0(s,a[0]);break;
    case 0x8002EC40u:result=WriteTransfer8002EC40(s,a[0],a[1]);break;
    case 0x8002A5CCu:result=AlignAddress8002A5CC(s,a[0],a[1]);break;
    case 0x8002A584u:result=WriteRegister8002A584(s,a[0],a[1],a[2]);break;
    case 0x8002EF28u:result=TransferCompleted8002EF28(s,a[0]);break;
    case 0x8002EEFCu:result=TransferPending8002EEFC(s,a[0]);break;
    case 0x800270FCu:result=WaitCurrent800270FC(s,a[0]);break;
    }
    return true;
}
bool TryCallVoid(Services& s,uint32_t fn,std::initializer_list<uint32_t> args) {
    if(fn==0x8002E05Cu) {
        if(args.size()!=1)throw std::invalid_argument("SpuFree argument count");
        Free8002E05C(s,*args.begin());return true;
    }
    if(fn==0x8002E0D8u) {
        if(args.size()!=0u)throw std::invalid_argument("SpuCompact argument count");
        Compact8002E0D8(s);return true;
    }
    int32_t ignored;return TryCall(s,fn,args,ignored);
}
}
