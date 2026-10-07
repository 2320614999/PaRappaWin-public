#pragma once
#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2SaveUiDirect {
using Services = PrStage2LifecycleDirect::Services;
int32_t InitializeEvent80017E58(Services&, uint32_t context);
int32_t Feedback80017E6C(Services&, uint32_t event, uint32_t context,
                        int32_t choice, uint32_t busy);
uint32_t SelectEvent800180D8(Services&, uint32_t state, uint32_t eventAddress,
                            uint32_t context);
int32_t BeginCardCommunication80017524(Services&);
int32_t EndCardCommunication80017574(Services&);
int32_t PollCard80017594(Services&);
int32_t SnapshotCardDirectory80017B18(Services&, uint32_t channel, uint32_t count);
int32_t SnapshotDirectory80018F70(Services&);
int32_t RunDispatcher80018FB0(Services&, uint32_t payload, uint32_t input,
                             uint32_t card, uint32_t state, uint32_t event);
int32_t RunSave80019148(Services&, uint32_t payload);
// Build the original 512-byte card header in the S2 save buffer.  The source
// keeps the icon/title inputs in PSX RAM; no host card image is consulted here.
int32_t BuildSaveBlockHeader80017C08(Services&, uint32_t destination,
                                     uint32_t encodedTitle);
bool TryCall(Services&, uint32_t function, std::initializer_list<uint32_t> args,
             int32_t& result);
}
