#include "sequence.hpp"
#include "sequence_curves.hpp"
#include <algorithm>
#include <cmath>

namespace fantasynt {
namespace {
float smooth(double start,double end,double time) {
    const auto t=static_cast<float>(std::clamp((time-start)/(end-start),0.0,1.0));
    return t*t*(3-2*t);
}
Color atmosphere(double time) {
    const auto t=static_cast<float>(time);
    return {evaluate_curve(sequence_curves::fog_red,t),
            evaluate_curve(sequence_curves::fog_green,t),
            evaluate_curve(sequence_curves::fog_blue,t)};
}
}
float evaluate_curve(std::span<const CurveKey> keys,float time,float default_value) {
    if(keys.empty()) return default_value;
    if(!std::isfinite(time)) time=keys.front().time;
    if(time<=keys.front().time) return keys.front().value;
    if(time>=keys.back().time) return keys.back().value;
    const auto right=std::upper_bound(keys.begin(),keys.end(),time,
        [](float t,const CurveKey& key) { return t<key.time; });
    const auto& a=*(right-1);
    const auto& b=*right;
    const float duration=b.time-a.time;
    if(duration<=0 || a.interpolation==1) return a.value;
    const float alpha=(time-a.time)/duration;
    if(a.interpolation==0) return a.value+(b.value-a.value)*alpha;
    // Bézier cúbico observado no executável: tangentes da chave esquerda/direita.
    const float p1=a.value+duration*a.leave_tangent*0.33333334f;
    const float p2=b.value-duration*b.arrive_tangent*0.33333334f;
    const auto lerp=[alpha](float x,float y) { return x+(y-x)*alpha; };
    const float q0=lerp(a.value,p1),q1=lerp(p1,p2),q2=lerp(p2,b.value);
    return lerp(lerp(q0,q1),lerp(q1,q2));
}
double sample_camera_position(double seconds) {
    if(!std::isfinite(seconds)) seconds=0;
    return evaluate_curve(sequence_curves::camera_y,
        static_cast<float>(std::clamp(seconds,0.0,sequence_duration)));
}
double sequence_camera_step(double from,double to,float motion_speed) {
    // O padrão .3 conserva o percurso; alterações JSON modulam só o deslocamento.
    return (sample_camera_position(to)-sample_camera_position(from))*(motion_speed/0.3f);
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
