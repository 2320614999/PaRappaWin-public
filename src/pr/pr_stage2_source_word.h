#pragma once
#include "pr_stage2_lifecycle_direct.h"
#include <stdexcept>

namespace PrStage2SourceWord {
// Value-preserving copy of original data whose byte provenance may be absent.
// Unknown lanes have NO numeric value. They may be transported, never consumed
// as a number, rendered, or silently initialized to zero by the native owner.
struct Word { uint32_t value=0; uint8_t known=0; };
struct Memory {
    virtual ~Memory()=default;
    virtual Word ReadSourceWord32(uint32_t address)=0;
    virtual void WriteSourceWord32(uint32_t address,Word word)=0;
};
inline Word Read(PrStage2LifecycleDirect::Services& s,uint32_t a){
    if(auto* m=dynamic_cast<Memory*>(&s))return m->ReadSourceWord32(a);
    return {s.Read32(a),15u};
}
inline void Write(PrStage2LifecycleDirect::Services& s,uint32_t a,Word word){
    if(word.known==15u){s.Write32(a,word.value);return;}
    if(auto* m=dynamic_cast<Memory*>(&s)){m->WriteSourceWord32(a,word);return;}
    throw std::logic_error("Indeterminate original word transport is not bound");
}
inline Word LoadUnaligned(PrStage2LifecycleDirect::Services& s,uint32_t a){
    // Original little-endian LWL(a+3), LWR(a), including duplicate aligned reads.
    const Word upper=Read(s,(a+3u)&~3u),lower=Read(s,a&~3u);
    const unsigned n=a&3u;
    if(!n)return lower;
    return {(lower.value>>(8u*n))|(upper.value<<(32u-8u*n)),
        uint8_t(((lower.known>>n)|(upper.known<<(4u-n)))&15u)};
}
inline void StoreUnaligned(PrStage2LifecycleDirect::Services& s,uint32_t a,Word word){
    // Original USW is SWL(a+3), SWR(a); preserve two full aligned stores too.
    const unsigned n=a&3u;
    if(!n){Write(s,a,word);Write(s,a,word);return;}
    Word upper=Read(s,(a+3u)&~3u);
    const uint32_t hiMask=(1u<<(8u*n))-1u;
    upper.value=(upper.value&~hiMask)|(word.value>>(32u-8u*n));
    upper.known=uint8_t((upper.known&~((1u<<n)-1u))|(word.known>>(4u-n)));
    Write(s,(a+3u)&~3u,upper);
    Word lower=Read(s,a&~3u);
    lower.value=(lower.value&hiMask)|(word.value<<(8u*n));
    lower.known=uint8_t(((lower.known&((1u<<n)-1u))|(word.known<<n))&15u);
    Write(s,a&~3u,lower);
}
}
