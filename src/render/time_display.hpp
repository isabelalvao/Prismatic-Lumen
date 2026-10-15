#pragma once
#include <array>
#include <cstdint>
#include <string>

namespace fantasynt {
struct TimeDisplay {
    bool visible{};
    double seconds{};
    float scale=1.5f;
    float x=0.5f;
    float y=0.65f;
    float opacity=0.95f;
    bool motif_active{};
};
using TimeDisplayUniforms=std::array<std::array<float,4>,5>;
std::string format_time_display(double seconds);
TimeDisplayUniforms time_display_uniforms(const TimeDisplay& display,
    std::uint16_t width,std::uint16_t height);
// Matrizes column-major; cabeça em centímetros, +Z frontal e +Y vertical.
// A placa é comum aos dois olhos: cada olho recebe apenas a sua projeção.
TimeDisplayUniforms time_display_uniforms_vr(const TimeDisplay& display,
    const std::array<float,16>& projection,const std::array<float,16>& head_to_eye,
    float depth=200.0f);
}
