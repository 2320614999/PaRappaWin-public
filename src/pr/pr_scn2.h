#pragma once

struct PrGameContext;

namespace PrScn2 {
    int Fn0(PrGameContext& ctx);
    void Fn1(PrGameContext& ctx);
    int Fn2(PrGameContext& ctx);
    void Pump(PrGameContext& ctx);
    bool OwnsNativePresentation(const PrGameContext& ctx);
    bool BeginResidentDirectory(PrGameContext& ctx, int previousScene);
    void Main(PrGameContext& ctx);
    void Render(PrGameContext& ctx);
}
