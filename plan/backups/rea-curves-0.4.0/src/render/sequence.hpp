#pragma once
#include "control/parameters.hpp"
#include "geometry.hpp"
#include <array>

namespace fantasynt {
inline constexpr double sequence_duration=424.0;
struct Composition {
    // Campo noturno, paisagem e túnel; o campo antigo é selecionado separadamente.
    std::array<float,3> weights{{0,0,1}};
    Color atmosphere{0.0f,0.008f,0.02f};
    float exposure{1};
};
Composition sample_sequence(double seconds);
Composition static_composition(SceneLayout layout);
}
