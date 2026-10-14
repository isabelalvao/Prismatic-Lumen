#pragma once
#include "control/parameters.hpp"
#include "geometry.hpp"
#include <array>
#include <span>

namespace fantasynt {
inline constexpr double sequence_duration=424.0;
inline constexpr double opening_duration=120.0;
struct CurveKey {
    float time, value, arrive_tangent, leave_tangent;
    std::uint8_t interpolation;
};
// Chaves finitas ordenadas; extrapolação constante para os canais selecionados.
float evaluate_curve(std::span<const CurveKey> keys,float time,float default_value=0);
double sample_camera_position(double seconds);
double sequence_camera_step(double from,double to,float motion_speed);
struct Composition {
    // Campo noturno, paisagem e túnel; o campo antigo é selecionado separadamente.
    std::array<float,3> weights{{0,0,1}};
    Color atmosphere{0.0f,0.008f,0.02f};
    float exposure{1};
    float opening_weight{};
    bool directed_extension{};
    float geometry_density_multiplier{1};
    float previous_exit_progress{};
    float tunnel_light_weight{1};
    float fog_multiplier{1};
};
Composition sample_sequence(double seconds);
Composition sample_opening(double seconds);
Composition static_composition(SceneLayout layout);
}
