#include "surface_choreography.hpp"
#include <algorithm>
#include <cmath>

namespace fantasynt {
namespace {
double reveal(double elapsed, double duration) {
    const double progress = std::clamp(elapsed / std::max(duration, 0.0001), 0.0, 1.0);
    return progress * progress * (3 - 2 * progress);
}
double cone_delay(int segment, double seed, const SurfaceChoreographySettings& settings) {
    return std::clamp(segment, 0, 6) * settings.segment_interval
        + std::clamp(seed, 0.0, 1.0) * settings.per_cone_delay;
}
}

int cone_segment_index(double original_local_y) {
    return static_cast<int>(std::floor(std::clamp(original_local_y * 3.5 + 0.0001, 0.0, 6.0)));
}

double cone_segment_visibility(int segment, double seed, double scene_time,
    const SurfaceChoreographySettings& settings) {
    return reveal(scene_time - settings.cones_start - cone_delay(segment, seed, settings),
        settings.segment_reveal_seconds);
}

double cone_segment_angle(int segment, double seed, double animation_clock,
    const SurfaceChoreographySettings& settings) {
    const double elapsed = std::max(0.0,
        animation_clock - settings.cone_rotation_start_clock - settings.segment_reveal_seconds
        - cone_delay(segment, seed, settings));
    return elapsed * settings.cone_rotation_speed * (std::clamp(segment, 0, 6) % 2 == 0 ? 1 : -1);
}

double previous_piece_visibility(double seed, double exit_progress) {
    return 1 - reveal(std::clamp(exit_progress, 0.0, 1.0) - std::clamp(seed, 0.0, 1.0) * 0.65, 0.35);
}

double tube_piece_visibility(double rank, double scene_time, double previous_exit_progress,
    const SurfaceChoreographySettings& settings) {
    if (previous_exit_progress < 1) return 0;
    const double reveal_seconds = std::max(settings.tube_piece_reveal_seconds, 0.0001);
    const double delay = std::clamp(rank, 0.0, 1.0)
        * std::max(0.0, settings.tube_build_seconds - reveal_seconds);
    return reveal(scene_time - settings.tube_start - delay, reveal_seconds);
}

double generic_piece_visibility(double seed, double scene_time, double family_start,
    const SurfaceChoreographySettings& settings) {
    const double reveal_seconds = std::max(settings.segment_reveal_seconds, 0.0001);
    const double delay = std::clamp(seed, 0.0, 1.0)
        * std::max(0.0, settings.generic_forms_reveal_seconds - reveal_seconds);
    return reveal(scene_time - family_start - delay, reveal_seconds);
}

}
