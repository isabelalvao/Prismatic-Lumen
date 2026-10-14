#pragma once
#include "opening_scene.hpp"
#include <string_view>

namespace fantasynt {
double directed_camera_position(double seconds);
double directed_camera_step(double from,double to,float motion_speed);
float opening_art_presence(ArtRole role,double time,double start_time);
float falling_point_height(float source_height,double time);
ArtRole classify_art_role(std::string_view name,int material);
void prepare_art_bounds(OpeningGroup& group,const SurfaceGeometry& mesh);
Instance directed_instance(const Instance& source,const OpeningGroup& group,double time);
bool intersects_directed_route(const Instance& source,const OpeningGroup& group,const SurfaceGeometry& mesh);
std::size_t clear_directed_route(OpeningScene& scene);
std::size_t extend_opening_intro(OpeningScene& scene);
}
