#pragma once
#include "geometry.hpp"
#include <filesystem>
#include <string>
#include <unordered_map>

namespace fantasynt {
enum class OpeningStage { Intro, Dome, Forest };
enum class ArtRole { Dark, Hex, Trace, Pulse, Pyramid, Point, Pole };
bool opening_visible(OpeningStage stage,double time);
struct OpeningGroup {
    std::string name,mesh;
    OpeningStage stage;
    int material{};
    float cull_distance{16000};
    double start_time{};
    ArtRole role{ArtRole::Dark};
    bool floor_anchored{};
    float shaft_height{};
    std::array<float,3> bounds_min{},bounds_max{},pivot{};
    std::vector<Instance> instances;
};
struct OpeningScene {
    std::vector<OpeningGroup> groups;
    std::unordered_map<std::string,SurfaceGeometry> meshes;
};
SurfaceGeometry load_reference_mesh(const std::filesystem::path& path);
OpeningScene load_opening_scene(const std::filesystem::path& directory);
}
