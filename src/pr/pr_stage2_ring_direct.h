#pragma once
#include "pr_stage2_source_word.h"
#include <array>

namespace PrStage2RingDirect {
using Services=PrStage2LifecycleDirect::Services;
struct Bus {
    virtual ~Bus()=default;
    // On-die SB must transport the full source register before lane shifting.
    // A uint8_t Services write would irreversibly discard bus payload bits.
    virtual void StoreOnDieByte(uint32_t address,uint32_t sourceRegister)=0;
    virtual uint8_t LoadOnDieByte(uint32_t address)=0;
};
struct Reply {
    int32_t result=0;
    std::array<uint8_t,8> bytes{};
    bool initialized=false;
};
Reply ReadyPrivate800372F0(Services&,uint32_t nonblocking);
int32_t Ready800372F0(Services&,uint32_t nonblocking,uint32_t output);
int32_t Ready800364F0(Services&,uint32_t nonblocking,uint32_t output);
// The optional initial word represents explicitly supplied ORIGINAL stack
// provenance in differential tests. Production dispatch always leaves it
// indeterminate; it is never a made-up emulated stack address or zero seed.
int32_t Receive80039670(Services&,PrStage2SourceWord::Word originalLocal={});
int32_t StartDma8003A014(Services&,uint32_t channel,uint32_t destination,uint32_t blocks,
                       uint32_t words,uint32_t control,uint32_t interrupt);
void CopyWords80039FE0(Services&,uint32_t destination,uint32_t source,uint32_t words);
void ClearSlots8003954C(Services&,uint32_t first,uint32_t count);
bool TryCall(Services&,uint32_t,std::initializer_list<uint32_t>,int32_t&);
bool TryCallVoid(Services&,uint32_t,std::initializer_list<uint32_t>);
}
