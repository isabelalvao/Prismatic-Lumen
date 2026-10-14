#include "scene_controller.hpp"
#include <algorithm>
#include <cmath>
namespace fantasynt {
void SceneController::submit(const Command& command) {
    origin_ = current_;
    const auto& p = command.parameters;
    if (p.palette) target_.palette = *p.palette;
    if (p.light_intensity) target_.light_intensity = *p.light_intensity;
    if (p.motion_speed) target_.motion_speed = *p.motion_speed;
    if (p.pulse_strength) target_.pulse_strength = *p.pulse_strength;
    if (p.fog_density) target_.fog_density = *p.fog_density;
    if (p.geometry_density) target_.geometry_density = *p.geometry_density;
    if (p.bloom) target_.bloom = *p.bloom;
    elapsed_ = 0.0;
    duration_ = command.transition_seconds;
    ++revision_;
}
void SceneController::advance(double elapsed_seconds) {
    if (!std::isfinite(elapsed_seconds) || elapsed_seconds <= 0.0 || duration_ <= 0.0) return;
    elapsed_ = std::min(duration_, elapsed_ + elapsed_seconds);
    const auto t = static_cast<float>(elapsed_ / duration_);
    const float blend = t * t * (3.0f - 2.0f * t);
    auto mix = [blend](float a, float b) { return a + (b - a) * blend; };
    for (std::size_t i = 0; i < 2; ++i) {
        current_.palette[i] = {mix(origin_.palette[i].r, target_.palette[i].r),
            mix(origin_.palette[i].g, target_.palette[i].g),
            mix(origin_.palette[i].b, target_.palette[i].b)};
    }
    current_.light_intensity = mix(origin_.light_intensity, target_.light_intensity);
    current_.motion_speed = mix(origin_.motion_speed, target_.motion_speed);
    current_.pulse_strength = mix(origin_.pulse_strength, target_.pulse_strength);
    current_.fog_density = mix(origin_.fog_density, target_.fog_density);
    current_.geometry_density = mix(origin_.geometry_density, target_.geometry_density);
    current_.bloom = mix(origin_.bloom, target_.bloom);
}
}
