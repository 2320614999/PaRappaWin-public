#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2VabDirect {
using Services = PrStage2LifecycleDirect::Services;
// Source-owned VAB directory, original SPU allocation list, and transfer calls.
// No decoded-bank cache, synthetic PSX stack address, or transfer receipt.
int32_t CloseCurrent80027120(Services&);
int32_t Close8002DF80(Services&, uint32_t bank);
void Free8002E05C(Services&, uint32_t address);
void Compact8002E0D8(Services&);
int32_t InitializeAllocator80035394(Services&, uint32_t count, uint32_t records);
int32_t Allocate8002E87C(Services&, uint32_t bytes);
int32_t SetTransferBusy8002EB44(Services&, uint32_t busy);
int32_t TransferBusy8002EB70(Services&);
int32_t Open8002E474(Services&, uint32_t header, uint32_t bank,
                   uint32_t suppliedAddress, uint32_t address);
int32_t OpenHeader8002E3D8(Services&, uint32_t header, uint32_t bank);
int32_t OpenCurrent80027078(Services&, uint32_t header);
int32_t Transfer8002EB80(Services&, uint32_t source, uint32_t bank);
int32_t TransferCurrent800270D4(Services&, uint32_t source);
int32_t SetTransferMode8002ECDC(Services&, uint32_t mode);
int32_t SetTransferAddress8002ECA0(Services&, uint32_t address);
int32_t WriteTransfer8002EC40(Services&, uint32_t source, uint32_t bytes);
int32_t AlignAddress8002A5CC(Services&, uint32_t reg, uint32_t address);
int32_t WriteRegister8002A584(Services&, uint32_t reg, uint32_t value, uint32_t shifted);
int32_t TransferCompleted8002EF28(Services&, uint32_t wait);
int32_t TransferPending8002EEFC(Services&, uint32_t wait);
int32_t WaitCurrent800270FC(Services&, uint32_t mode);

bool TryCall(Services&, uint32_t function, std::initializer_list<uint32_t>, int32_t&);
bool TryCallVoid(Services&, uint32_t function, std::initializer_list<uint32_t>);
}
