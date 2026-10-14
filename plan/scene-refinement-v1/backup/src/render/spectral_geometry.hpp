#pragma once

#include "geometry.hpp"

namespace fantasynt {
inline constexpr int spectral_cone_piece_count = 9;
inline constexpr int crystal_variant_count = 6;
inline constexpr int crystal_piece_count = 9;

// Peças ordenadas da base larga para a ponta; altura 0..2, raio máximo 1,04.
SurfaceGeometry make_luminous_cone_piece_geometry(int piece);

// Aglomerados determinísticos: altura 0..2 e raio horizontal máximo 1,8.
SurfaceGeometry make_crystal_piece_geometry(int variant, int piece);
}
