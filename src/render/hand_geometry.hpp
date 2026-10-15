#pragma once
#include "interaction/observation.hpp"
#include <vector>

namespace fantasynt {
struct HandVertex { float x,y,z,nx,ny,nz; };
struct HandGeometry {
    std::vector<HandVertex> vertices;
    std::vector<std::uint16_t> indices;
    std::size_t joint_count{},bone_count{};
};
// Geometria em centímetros no mesmo espaço físico usado pelos olhos, sem avanço virtual.
HandGeometry make_hand_geometry(const interaction::HandSample& hand,float eye_height_cm=160,float eye_back_cm=450);
}
