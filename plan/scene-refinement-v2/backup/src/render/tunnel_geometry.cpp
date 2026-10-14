#include "geometry.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>
#include <random>

namespace fantasynt {
std::vector<Instance> make_tunnel_instances(std::uint32_t seed,std::size_t count) {
    if(!count) return {};
    std::mt19937 random(seed);
    auto unit=[&]() { return static_cast<float>(random()>>8)/16777215.0f; };
    std::vector<Instance> items;
    items.reserve(4096);
    constexpr float tau=2*std::numbers::pi_v<float>;
    for(std::size_t i=0;i<4096;++i) {
        const float ring=static_cast<float>(i%64);
        const float angle=tau*(static_cast<float>(i/64)/24.0f+ring*0.017f)+unit()*0.12f;
        float radius=1060+unit()*380;
        float sx{},sy{},sz{},family{};
        if(i<768) { sx=110+unit()*110; sy=28+unit()*65; sz=180+unit()*110; family=3; }
        else if(i<1536) { sx=6+unit()*16; sy=15+unit()*30; sz=250+unit()*360; family=3; }
        else if(i<2496) { sx=35+unit()*100; sy=sx; sz=sx; family=4; }
        else if(i<2976) { sx=60+unit()*110; sy=sx; sz=60+unit()*100; family=5; }
        else if(i<3360) { sx=150+unit()*160; sy=sx; sz=100+unit()*130; family=6; radius+=160; }
        else { sx=2+unit()*6; sy=sx; sz=sx; family=7; radius=850+unit()*1300; }
        const float s=std::sin(angle),c=std::cos(angle);
        const float tilt=(unit()-0.5f)*0.8f,ct=std::cos(tilt),st=std::sin(tilt);
        Instance item;
        // Axes orthogonais: tangente, radial e profundidade, com inclinação local.
        item.transform={s*ct*sx,-c*ct*sx,st*sx,0,
            c*sy,s*sy,0,0, -s*st*sz,c*st*sz,ct*sz,0,
            c*radius,260+s*radius,ring*650+unit()*160,1};
        item.color_mix=unit(); item.phase=unit(); item.variation=family;
        items.push_back(item);
    }
    // A densidade conserva elementos em todo o volume em vez de esvaziar um lado.
    for(std::size_t i=items.size()-1;i>0;--i) std::swap(items[i],items[random()%(i+1)]);
    for(std::size_t i=0;i<items.size();++i)
        items[i].padding=(static_cast<float>(i)+0.5f)/4096*(1-1.0f/64);
    items.resize(std::min(count,std::size_t{4096}));
    std::stable_sort(items.begin(),items.end(),[](const auto& a,const auto& b) { return a.variation<b.variation; });
    return items;
}
std::vector<Instance> make_directed_tunnel_instances(std::uint32_t seed) {
    auto items=make_tunnel_instances(seed,4096);
    // Cada argola conserva uma posição periférica até à transformação no túnel.
    std::erase_if(items,[](const auto& item) { return item.variation==5; });
    for(int ring=0;ring<64;++ring) {
        Instance item;
        const float angle=ring*2.39996323f;
        const float radius=1200+(ring%5)*55.0f;
        const float size=90+(ring%7)*14.0f;
        item.transform={size,0,0,0, 0,size,0,0, 0,0,70,0,
            std::cos(angle)*radius,260+std::sin(angle)*radius,ring*800.0f,1};
        item.variation=5; item.phase=ring/63.0f; item.color_mix=ring/63.0f;
        item.padding=0; // O percurso mantém todos os anéis independentemente da densidade decorativa.
        items.push_back(item);
    }
    std::stable_sort(items.begin(),items.end(),[](const auto& a,const auto& b) { return a.variation<b.variation; });
    return items;
}
}
