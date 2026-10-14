#include "geometry.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>
#include <random>

namespace fantasynt {
std::vector<Instance> make_landscape_instances(std::uint32_t seed,std::size_t count,bool colorful) {
    if(!count) return {};
    std::mt19937 random(seed);
    auto unit=[&] { return static_cast<float>(random()>>8)/16777215.0f; };
    std::vector<Instance> items;
    items.reserve(4096);
    constexpr float tau=2*std::numbers::pi_v<float>;
    for(std::size_t i=0;i<4096;++i) {
        float x=(unit()-0.5f)*32000,z=unit()*41600,y=0;
        float sx{},sy{},sz{},family{},angle=unit()*tau;
        if(i<160) {
            // Uma avenida de estruturas monumentais; afastamento seguro do eixo central.
            x=(static_cast<float>(i%8)-3.5f)*3200+(unit()-0.5f)*1200;
            z=static_cast<float>(i/8)*2050+650+unit()*500;
            sx=650+unit()*550; sy=800+unit()*1200; sz=colorful?sx:250+unit()*150;
            family=colorful?9.0f:8.0f;
            angle=colorful?angle:(unit()-0.5f)*0.45f;
        } else if(i<1456) {
            sx=35+unit()*220; sy=80+unit()*380; sz=sx; family=10;
            y=sy;
        } else if(i<2352) {
            sx=8+unit()*12; sy=200+unit()*1500; sz=sx; family=11;
            y=sy;
        } else if(i<2736) {
            sx=65+unit()*190; sy=90+unit()*260; sz=sx; family=12;
            y=1100+unit()*4500;
        } else if(i<3248) {
            sx=110+unit()*260; sy=40+unit()*80; sz=sx; family=13;
            y=sy;
        } else {
            sx=2+unit()*9; sy=sx; sz=sx; family=12;
            y=80+unit()*6000;
        }
        const float c=std::cos(angle),s=std::sin(angle);
        Instance item;
        item.transform={c*sx,0,-s*sx,0, 0,sy,0,0, s*sz,0,c*sz,0, x,y,z,1};
        item.color_mix=unit(); item.phase=unit(); item.variation=family;
        items.push_back(item);
    }
    for(std::size_t i=items.size()-1;i>0;--i) std::swap(items[i],items[random()%(i+1)]);
    for(std::size_t i=0;i<items.size();++i)
        items[i].padding=(static_cast<float>(i)+0.5f)/4096*(1-1.0f/64);
    items.resize(std::min(count,std::size_t{4096}));
    std::stable_sort(items.begin(),items.end(),[](const auto& a,const auto& b) { return a.variation<b.variation; });
    return items;
}
}
