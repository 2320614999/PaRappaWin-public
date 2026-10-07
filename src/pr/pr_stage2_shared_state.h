#pragma once
#include "pr_stage2_lifecycle_direct.h"
#include "pr_stage_payload_bank_direct.h"

namespace PrStage2SharedState {
// 关卡覆盖层继承主程序的共享状态；Windows 新建 S2 RAM 不等于 PSX 冷启动。
struct Entry {
    uint16_t mode=0,easy=0,language=0,subtitles=0,exitReason=0;
    uint16_t savePolicy=0;
    bool savePolicyKnown=false;
    PrStagePayloadBankDirect::MemoryState80092F10 payload;
};

// 第一次写入 RAM 前检查完整交接；回放必须携带真实回放数据与载入前备份。
void ImportEntry(PrStage2LifecycleDirect::Services& memory,const Entry& entry);
}
