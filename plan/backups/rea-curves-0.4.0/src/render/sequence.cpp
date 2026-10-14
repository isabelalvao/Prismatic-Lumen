#include "sequence.hpp"
#include <algorithm>
#include <cmath>

namespace fantasynt {
namespace {
float smooth(double start,double end,double time) {
    const auto t=static_cast<float>(std::clamp((time-start)/(end-start),0.0,1.0));
    return t*t*(3-2*t);
}
struct FogKey { double time; Color value; };
// Valores de FogInscatteringColor recuperados de ChezNousSequence; interpolação própria.
constexpr std::array<FogKey,18> fog{{
    {0,{0,0,0}}, {80,{0,0,0}}, {112,{0.024f,0.038f,0.06f}},
    {144,{0,0.122f,0.247f}}, {175,{0,0.122f,0.247f}},
    {177,{0.319f,0.001f,0.003f}}, {207,{0.991f,0.033f,0.015f}},
    {208,{0,0,0}}, {224,{0,0,0}}, {256,{0,0.008f,0.02f}},
    {288,{0,0.008f,0.02f}}, {290,{0.0307f,0.016f,0.0497f}},
    {320,{0.0307f,0.016f,0.0497f}}, {322,{0.1749f,0.0085f,0.0264f}},
    {351,{0.991f,0.033f,0.015f}}, {352,{0,0.008f,0.02f}},
    {416,{0,0.008f,0.02f}}, {424,{0,0,0}}
}};
Color atmosphere(double time) {
    for(std::size_t i=1;i<fog.size();++i) if(time<=fog[i].time) {
        const float t=smooth(fog[i-1].time,fog[i].time,time);
        const auto a=fog[i-1].value,b=fog[i].value;
        return {a.r+(b.r-a.r)*t,a.g+(b.g-a.g)*t,a.b+(b.b-a.b)*t};
    }
    return fog.back().value;
}
}
Composition sample_sequence(double seconds) {
    if(!std::isfinite(seconds)) seconds=0;
    seconds=std::clamp(seconds,0.0,sequence_duration);
    const float forest=smooth(80,112,seconds),tunnel=smooth(224,232,seconds);
    Composition result;
    result.weights={1-forest,forest*(1-tunnel),tunnel};
    result.atmosphere=atmosphere(seconds);
    result.exposure=1-smooth(416,424,seconds);
    return result;
}
Composition static_composition(SceneLayout layout) {
    if(layout==SceneLayout::Night || layout==SceneLayout::Field) return {{1,0,0},{0.004f,0.012f,0.03f},1};
    if(layout==SceneLayout::Landscape) return {{0,1,0},{0.32f,0.009f,0.035f},1};
    return {};
}
}
