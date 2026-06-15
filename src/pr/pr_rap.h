#pragma once

#include <cstdint>

struct PrGameContext;

struct PrRap {
    static void Init(PrGameContext& ctx);
    static void Shutdown(PrGameContext& ctx);
    static void Update(PrGameContext& ctx);
    static void Render(PrGameContext& ctx);

    static void SubmitTap(uint16_t buttons);
};
