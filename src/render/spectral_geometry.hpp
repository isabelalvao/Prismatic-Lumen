#pragma once

#include "geometry.hpp"

namespace fantasynt {
inline constexpr int spectral_cone_rib_count = 8;
inline constexpr int spectral_cone_ring_count = 9;
inline constexpr int spectral_cone_piece_count = spectral_cone_rib_count + spectral_cone_ring_count;
inline constexpr int crystal_variant_count = 6;
inline constexpr int crystal_piece_count = 9;

// Oito nervuras completas, depois nove círculos da base à ponta; altura 0..2.
SurfaceGeometry make_luminous_cone_piece_geometry(int piece);

// Rochas facetadas determinísticas: altura 0..0,82, bases assentes em y=0.
SurfaceGeometry make_crystal_piece_geometry(int variant, int piece);
}
