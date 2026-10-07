#pragma once
#include "pr_stage2_lifecycle_direct.h"
#include <array>

namespace PrStage2MovieForegroundDirect {
using Services=PrStage2LifecycleDirect::Services;
// Native producer policy, not another translated PSX function. Defer actual
// file data until the existing source ring can accept it without overrunning
// a frame still owned by its consumer. This query writes no source state.
bool CanAcceptHostVideoSector(Services&,const std::array<uint8_t,2352>&);
bool CanAcceptHostStreamSector(Services&,const std::array<uint8_t,2352>&);
// Bounded native SDK GetlocL operation. Its bytes come from a real stream
// producer; it does not own a drive, FIFO, MMIO register or hardware interrupt.
class LocationQuery final {
public:
    void Request(Services&,const std::array<uint8_t,8>& actualSectorHeader);
    bool Service(Services&,bool callbacksAllowed);
    bool Pending()const noexcept{return pending_;}
    uint64_t Requests()const noexcept{return requests_;}
    uint64_t Replies()const noexcept{return replies_;}
private:
    std::array<uint8_t,8> header_{};
    bool pending_=false,faulted_=false;
    uint64_t requests_=0,replies_=0;
};
// Source movie/text control flow. The decoder, file and input operations stay
// behind existing native services; no CPU, DMA or device model is owned here.
void SelectText80024C84(Services&,uint32_t row);
void EndText80024CF0(Services&,uint32_t ignoredWork);
int32_t UpdateText80024CF8(Services&,uint32_t work);
int32_t Ready8001A750(Services&);
int32_t RefreshLocation8001A3C8(Services&);
int32_t RequestLocation8001A280(Services&);
int32_t LastCommand800363A4(Services&);
int32_t MediaError8001A3B8(Services&);
int32_t Position8001A7A4(Services&,uint32_t start);
int32_t AtEnd8001A7F8(Services&,uint32_t record);
int32_t Prepare8001EC54(Services&,uint32_t work,uint32_t mode);
int32_t Submit8001ED3C(Services&,uint32_t ignoredWork);
int32_t Swap8001ED74(Services&);
int32_t DecodeIfReady80027528(Services&);
int32_t Blit8002756C(Services&);
int32_t Caption8001DB00(Services&,uint32_t text,uint32_t table);
int32_t ReadPad80035510(Services&,uint32_t ignoredPort);
bool TryCall(Services&,uint32_t,std::initializer_list<uint32_t>,int32_t&);
bool TryCallVoid(Services&,uint32_t,std::initializer_list<uint32_t>);
}
