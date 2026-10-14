#pragma once

namespace fantasynt {

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
};

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
