#pragma once

struct PrGameContext;

struct PrApp {
    static bool Init(PrGameContext& ctx);
    static void Shutdown(PrGameContext& ctx);
    static void Tick(PrGameContext& ctx);
};
