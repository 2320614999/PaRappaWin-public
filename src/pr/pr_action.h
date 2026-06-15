#pragma once

struct PrGameContext;

struct PrAction {
    static void Init(PrGameContext& ctx);
    static void Shutdown(PrGameContext& ctx);
    static void Update(PrGameContext& ctx);
};
