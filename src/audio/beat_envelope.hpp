#pragma once
#include <cstdint>
#include <span>
#include <vector>
#include "config/visual_config.hpp"

namespace fantasynt {
class BeatEnvelope {
public:
    static BeatEnvelope analyze(std::span<const std::int16_t> pcm,int sample_rate,int channels,const VisualConfig& config=default_visual_config());
    float sample(double seconds) const;
    int pulse_group(double seconds,double start_seconds) const;
    double duration_seconds() const { return duration_seconds_; }
    std::size_t beat_count() const { return onsets_.size(); }
    const std::vector<double>& onsets() const { return onsets_; }
private:
    std::vector<float> envelope_;
    std::vector<double> onsets_;
    double duration_seconds_{};
    static constexpr double envelope_rate=100;
};
}
