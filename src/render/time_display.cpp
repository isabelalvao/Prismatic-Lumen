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
std::array<float,4> transform(const std::array<float,16>& matrix,std::array<float,4> point) {
    std::array<float,4> result{};
    for(int row=0;row<4;++row)
        for(int column=0;column<4;++column) result[row]+=matrix[column*4+row]*point[column];
    return result;
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
    TimeDisplayUniforms result{{{static_cast<float>(timeline_tenths(display.seconds)),display.visible?1.0f:0.0f,aspect,display.motif_active?1.0f:0.0f},
        {bounded(display.x,0.5f,0.0f,1.0f),bounded(display.y,0.65f,0.0f,1.0f),
         bounded(display.scale,1.5f,0.25f,4.0f),bounded(display.opacity,0.95f,0.0f,1.0f)}}};
    const float glyph_height=0.052f*result[1][2];
    result[2]={aspect/glyph_height,0,1.82f-result[1][0]*aspect/glyph_height,0};
    result[3]={0,1/glyph_height,0.5f-result[1][1]/glyph_height,0};
    result[4]={0,0,1,0};
    return result;
}
TimeDisplayUniforms time_display_uniforms_vr(const TimeDisplay& display,
    const std::array<float,16>& projection,const std::array<float,16>& head_to_eye,float depth) {
    auto result=time_display_uniforms(display,1,1);
    const auto finite=[](const auto& matrix) {
        return std::all_of(matrix.begin(),matrix.end(),[](float value) { return std::isfinite(value); });
    };
    if(!std::isfinite(depth)||depth<=0||!finite(projection)||!finite(head_to_eye)) {
        result[0][1]=0; return result;
    }
    // Coordenadas normalizadas referem-se a uma única placa frontal, não a cada lente.
    const float size=0.052f*result[1][2]*2*depth;
    const float center_x=(result[1][0]-0.5f)*2*depth;
    const float center_y=(0.5f-result[1][1])*2*depth;
    const auto clip=[&](std::array<float,4> point) {
        return transform(projection,transform(head_to_eye,point));
    };
    if(clip({center_x,center_y,depth,1})[3]<=0) { result[0][1]=0; return result; }
    const std::array<std::array<float,4>,3> basis{{{size,0,0,0},{0,-size,0,0},
        {center_x-1.82f*size,center_y+0.5f*size,depth,1}}};
    std::array<double,9> h{};
    for(int column=0;column<3;++column) {
        const auto projected=clip(basis[column]);
        h[column]=0.5*(projected[0]+projected[3]);
        h[3+column]=0.5*(projected[3]-projected[1]);
        h[6+column]=projected[3];
    }
    // Inverte a homografia placa→UV; preserva perspetiva e inclinação de cada olho.
    const std::array<double,9> inverse{
        h[4]*h[8]-h[5]*h[7],h[2]*h[7]-h[1]*h[8],h[1]*h[5]-h[2]*h[4],
        h[5]*h[6]-h[3]*h[8],h[0]*h[8]-h[2]*h[6],h[2]*h[3]-h[0]*h[5],
        h[3]*h[7]-h[4]*h[6],h[1]*h[6]-h[0]*h[7],h[0]*h[4]-h[1]*h[3]};
    const double determinant=h[0]*inverse[0]+h[1]*inverse[3]+h[2]*inverse[6];
    if(!std::isfinite(determinant)||std::abs(determinant)<1e-12) { result[0][1]=0; return result; }
    for(int row=0;row<3;++row) {
        for(int column=0;column<3;++column)
            result[2+row][column]=static_cast<float>(inverse[row*3+column]*depth/determinant);
        if(!finite(result[2+row])) { result[0][1]=0; return result; }
    }
    return result;
}
}
