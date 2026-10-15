#pragma once
#include "geometry.hpp"
#include "config/visual_config.hpp"
#include <span>

namespace fantasynt {
Instance surface_fade_instance(const Instance& source,double opacity);
double spectral_piece_visibility(int piece,double seed,double time,double start,double interval,double reveal_seconds,double instance_delay);
double beat_aligned_time(double nominal_time,std::span<const double> onsets);
double rear_piece_visibility(double depth_cm,double radius_cm,const VisualConfig& config);
double retained_wrapped_depth(double depth_cm,double radius_cm,const VisualConfig& config);
double required_ring_lead(double maximum_speed_m_s,const VisualConfig& config);
Instance directed_tunnel_instance(const Instance& source,double time,double camera,double alignment_camera,
    const VisualConfig& config,double speed_multiplier=1,double alignment_lead_m=-1,double animation_clock=-1);

struct SurfaceChoreographySettings {
    double cones_start = 136;
    double segment_interval = 2;
    double segment_reveal_seconds = 0.8;
    double cone_rotation_speed = 0.034906585;
    double per_cone_delay = 2;
    double cone_rotation_start_clock = 136;
    double tube_start = 224;
    double tube_build_seconds = 48;
    double tube_piece_reveal_seconds = 0.8;
    double generic_forms_reveal_seconds = 8;
    double cone_emerge_seconds = 14;
    double cone_separate_seconds = 3;
};

struct DrillConeMotion { double rise{},split{},surface_time{},rotation_time{}; };
DrillConeMotion drill_cone_motion(double seed,double scene_time,const SurfaceChoreographySettings& settings);
double cone_module_local_offset(int segment,const DrillConeMotion& motion);
double tunnel_departure_progress(double seed,double scene_time,double start,double duration);

int cone_segment_index(double original_local_y);
double cone_segment_visibility(int segment, double seed, double scene_time,
    const SurfaceChoreographySettings& settings);
double cone_segment_angle(int segment, double seed, double animation_clock,
    const SurfaceChoreographySettings& settings);
double previous_piece_visibility(double seed, double exit_progress);
double tube_piece_visibility(double rank, double scene_time, double previous_exit_progress,
    const SurfaceChoreographySettings& settings);
double generic_piece_visibility(double seed, double scene_time, double family_start,
    const SurfaceChoreographySettings& settings);

}
