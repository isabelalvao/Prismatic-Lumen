#pragma once
#include "interaction/light_pencil.hpp"
#include <span>
#include <vector>

namespace fantasynt {
struct PencilVertex { float x,y,z,nx,ny,nz,fade,padding; };
struct PencilGeometry { std::vector<PencilVertex> vertices; std::vector<std::uint16_t> indices; };
PencilGeometry make_pencil_geometry(std::span<const interaction::PencilPoint> points,std::int64_t now,
                                    float eye_height_cm=160,float eye_back_cm=450);
}
