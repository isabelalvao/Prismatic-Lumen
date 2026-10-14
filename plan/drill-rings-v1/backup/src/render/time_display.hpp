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
};
using TimeDisplayUniforms=std::array<std::array<float,4>,2>;
std::string format_time_display(double seconds);
TimeDisplayUniforms time_display_uniforms(const TimeDisplay& display,
    std::uint16_t width,std::uint16_t height);
}
