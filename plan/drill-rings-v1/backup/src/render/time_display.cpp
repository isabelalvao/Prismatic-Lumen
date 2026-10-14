#include "time_display.hpp"
#include <algorithm>
#include <cmath>

namespace fantasynt {
namespace {
int timeline_tenths(double seconds) {
    if(!std::isfinite(seconds)||seconds<=0) return 0;
    return static_cast<int>(std::floor(std::min(seconds,5999.9)*10.0));
}
float bounded(float value,float fallback,float minimum,float maximum) {
    return std::isfinite(value)?std::clamp(value,minimum,maximum):fallback;
}
}
std::string format_time_display(double seconds) {
    const int tenths=timeline_tenths(seconds);
    const int minutes=tenths/600,whole_seconds=tenths/10%60;
    std::string text="00:00.0";
    text[0]=static_cast<char>('0'+minutes/10);
    text[1]=static_cast<char>('0'+minutes%10);
    text[3]=static_cast<char>('0'+whole_seconds/10);
    text[4]=static_cast<char>('0'+whole_seconds%10);
    text[6]=static_cast<char>('0'+tenths%10);
    return text;
}
TimeDisplayUniforms time_display_uniforms(const TimeDisplay& display,
    std::uint16_t width,std::uint16_t height) {
    const float aspect=width&&height?static_cast<float>(width)/height:1.0f;
    return {{{static_cast<float>(timeline_tenths(display.seconds)),display.visible?1.0f:0.0f,aspect,0.0f},
        {bounded(display.x,0.5f,0.0f,1.0f),bounded(display.y,0.65f,0.0f,1.0f),
         bounded(display.scale,1.5f,0.25f,4.0f),bounded(display.opacity,0.95f,0.0f,1.0f)}}};
}
}
