#pragma once

#include <cstdint>

struct PrGameContext;

struct PrEval {
    static void Init();
    static void Shutdown();
    static void Update(PrGameContext& ctx);
};
