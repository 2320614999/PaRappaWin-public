#pragma once

#include "pr_stage2_lifecycle_direct.h"

namespace PrStage2IntLoaderDirect {
using Services = PrStage2LifecycleDirect::Services;

int32_t LocationToLba80036A78(Services& s, uint32_t location);
int32_t LbaToLocation80036974(Services& s, uint32_t lba, uint32_t location);
int32_t SeekFile8001A89C(Services& s, uint32_t descriptor, uint32_t offset);
int32_t FindFile8001A2B0(Services& s, uint32_t descriptor, uint32_t path);
int32_t OpenFile8001A324(Services& s, uint32_t record);
int32_t LoadInt8001AC18(Services& s, uint32_t record, uint32_t loadingSound);
int32_t FindHeapEntry80025BFC(Services& s, uint32_t address);
int32_t SplitHeapEntry80025BBC(Services& s, uint32_t address, uint32_t index, uint32_t bytes);
int32_t ReadSectors8001A818(Services& s, uint32_t destination, uint32_t sectors, uint32_t mode);
// The descriptor is a genuine RAM CdlFILE, not an invented address for the
// private copy in 8001AC18. Its seek helper mutates the descriptor's position.
int32_t ParseInt8001A8F0(Services& s, uint32_t descriptor, uint32_t readMode, uint32_t loadingSound);
}
