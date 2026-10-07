#include "pr/pr_modern_rail_animation.h"
#include <cmath>
#include <stdexcept>
#include <iostream>

int main() {
    using PrModernRailAnimation::Sample;
    const auto require=[](bool ok){if(!ok)throw std::runtime_error("Modern note animation contract");};
    unsigned checks=0;
    for(int pop:{0,1,5,12})for(int flip:{0,1,8,14})for(int fade:{0,2,16}) {
        const float impact=float((std::max)(1,pop));
        const float turn=float((std::max)(1,flip));
        require(Sample(0,pop,2,flip,fade).x==2);
        require(Sample(0,pop,2,flip,fade).glow==1);
        float previousSize=2,previousGlow=1;
        for(int i=0;i<=100;++i) {
            const auto p=Sample(impact*i/100,pop,2,flip,fade);
            require(p.x<=previousSize && p.x>=1 && p.x==p.y);
            require(p.glow<=previousGlow && p.glow>=0);
            previousSize=p.x;previousGlow=p.glow;++checks;
        }
        // Most of the shrink happens early, before the single card turn.
        require(Sample(impact/2,pop,2,flip,fade).x<1.25f);
        require(std::abs(Sample(impact+turn/4,pop,2,flip,fade).x)<0.00001f);
        require(std::abs(Sample(impact+turn/2,pop,2,flip,fade).x+1)<0.00001f);
        require(std::abs(Sample(impact+3*turn/4,pop,2,flip,fade).x)<0.00001f);
        for(float age:{impact,impact+turn/2,impact+turn,1000.0f}) {
            const auto p=Sample(age,pop,2,flip,fade);require(p.y==1 && p.glow==0);++checks;
        }
        require(Sample(impact+turn,pop,2,flip,fade).x==1);
    }
    std::cout<<"PASS "<<checks<<" shared impact/glow/single-turn samples\n";
}
