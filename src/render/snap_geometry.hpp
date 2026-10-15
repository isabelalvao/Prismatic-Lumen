#pragma once
#include "hand_geometry.hpp"
#include "interaction/hand_snap.hpp"

namespace fantasynt {
inline constexpr double snap_particle_lifetime=0.45;
inline constexpr float snap_spread_scale=1.25f,snap_point_scale=1.12f;
float snap_particle_fade(const interaction::SnapEvent& event,std::int64_t now);
HandGeometry make_snap_geometry(const interaction::SnapEvent& event,std::int64_t now,
                                float eye_height_cm=160,float eye_back_cm=450);
}
