#pragma once
#include <array>
#include <cstdint>
#include <vector>
#include "config/visual_config.hpp"

namespace fantasynt {
struct Instance {
    std::array<float, 16> transform{};
    float color_mix{}, phase{}, variation{}, padding{};
};
std::vector<Instance> make_instances(std::uint32_t seed, std::size_t count);
enum class SceneLayout { Tunnel, Field, Night, Landscape, Opening };
struct SurfaceVertex { float x,y,z,nx,ny,nz,u{},v{}; };
struct SurfaceGeometry {
    std::vector<SurfaceVertex> vertices;
    std::vector<std::uint16_t> indices;
};
std::vector<Instance> make_tunnel_instances(std::uint32_t seed, std::size_t count);
std::vector<Instance> make_directed_tunnel_instances(std::uint32_t seed);
SurfaceGeometry make_sphere_geometry();
SurfaceGeometry make_torus_geometry();
SurfaceGeometry make_ribbon_geometry();
SurfaceGeometry make_panel_geometry();
std::vector<Instance> make_landscape_instances(std::uint32_t seed, std::size_t count, bool colorful,
    const VisualConfig& config=default_visual_config());
SurfaceGeometry make_fan_geometry();
SurfaceGeometry make_segmented_cone_geometry();
SurfaceGeometry make_diamond_geometry();
SurfaceGeometry make_plate_geometry();
}
