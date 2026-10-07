#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2SoftFloatDirect {
using Memory = PrStage2LifecycleDirect::Services;
using Words64 = PrStage2LifecycleDirect::Words64;
using Arguments = std::initializer_list<uint32_t>;

// 保留原软件浮点位模式及 v0/v1 双字返回，不依赖宿主浮点环境。
Words64 Add8002864C(Memory& memory, Words64 lhs, Words64 rhs);
Words64 Divide80028B40(Memory& memory, Words64 lhs, Words64 rhs);
int32_t ToSigned80028F2C(Memory& memory, Words64 value);
Words64 FromSigned8002902C(uint32_t value);
int32_t ReportError800291A4(Memory& memory, uint32_t error, uint32_t operation);
bool TryCall64(Memory& memory, uint32_t function, Arguments args, Words64& result);
bool TryCall(Memory& memory, uint32_t function, Arguments args, int32_t& result);
}
