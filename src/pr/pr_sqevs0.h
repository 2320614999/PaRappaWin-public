#pragma once

#include <cstdint>

struct PrGameContext;

namespace PrSqevs0 {
    void Init(PrGameContext& ctx);
    void Shutdown(PrGameContext& ctx);
    void Update(PrGameContext& ctx);

    void Emit(uint32_t id, uint32_t a0, uint32_t a1, uint32_t a2);
}
