#pragma once

struct PrGameContext;

struct PrCMenu {
    static void Init(PrGameContext& ctx);
    static void Shutdown(PrGameContext& ctx);
    static void Update(PrGameContext& ctx);
    static void Render(PrGameContext& ctx);
};
