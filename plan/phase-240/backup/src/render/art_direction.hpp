#pragma once
#include "opening_scene.hpp"
#include "config/visual_config.hpp"
#include "sequence.hpp"
#include <string_view>

namespace fantasynt {
struct ArtMotion { double points{},traces{},pyramids{},sky{}; };
ArtMotion sample_art_motion(double time,const VisualConfig& config);
Composition directed_composition(double time,bool opening_only,const VisualConfig& config);
double directed_camera_position(double seconds,const VisualConfig& config=default_visual_config());
double directed_camera_step(double from,double to,float motion_speed,const VisualConfig& config=default_visual_config());
float opening_art_presence(ArtRole role,double time,double start_time,const VisualConfig& config=default_visual_config());
float falling_point_height(float source_height,double time,const VisualConfig& config=default_visual_config());
ArtRole classify_art_role(std::string_view name,int material);
void prepare_art_bounds(OpeningGroup& group,const SurfaceGeometry& mesh);
Instance directed_instance(const Instance& source,const OpeningGroup& group,double time,const VisualConfig& config=default_visual_config());
Instance directed_instance_with_motion(const Instance& source,const OpeningGroup& group,const ArtMotion& motion,const VisualConfig& config);
double directed_group_start(const OpeningGroup& group,const VisualConfig& config);
bool directed_group_visible(const OpeningGroup& group,double time,const VisualConfig& config);
bool intersects_directed_route(const Instance& source,const OpeningGroup& group,const SurfaceGeometry& mesh,const VisualConfig& config=default_visual_config());
std::size_t clear_directed_route(OpeningScene& scene,const VisualConfig& config=default_visual_config());
std::size_t extend_opening_intro(OpeningScene& scene,const VisualConfig& config=default_visual_config());
}
