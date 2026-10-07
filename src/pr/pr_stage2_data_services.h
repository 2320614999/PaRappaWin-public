#pragma once
#include "pr_stage2_memory_services.h"
#include <filesystem>

namespace PrStage2DataServices {
// The first SCUS startup loop, not the remainder of start()/BIOS/heap setup.
// It preserves the original ordered word writes over [8006ECB8, 801C3870).
void ClearScusBss80028590(PrStage2LifecycleDirect::Services& services);

// Cold native data initialization from the shipped S2 native data pack. A
// matching SCUS_941.83 is accepted as an offline-validation source, but is
// not required by the Windows runtime. Original code bytes are data only:
// calls still use the compiled native dispatcher, never this image.
// Construct once per fresh RAM owner; no live reloading or full-RAM zeroing.
// Overlay loading, BIOS initialization and concrete devices are not implied.
class Services : public PrStage2MemoryServices::Services {
public:
    explicit Services(const std::filesystem::path& scusPath);
};
}
