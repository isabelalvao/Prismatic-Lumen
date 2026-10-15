#pragma once
#include "observation.hpp"
#include <array>
#include <optional>
#include <vector>

namespace fantasynt::interaction {
inline constexpr const char* snap_detector_version="thumb-middle-release-1";
struct SnapEvent {
    std::size_t hand{};
    std::int64_t xr_time_ns{};
    std::uint64_t origin_epoch{};
    vr::Vector3 position_m;
    float release_m{},closing_m_s{};
};
// Aproximação cinemática; não escuta nem confirma um estalo sonoro.
class SnapDetector {
public:
    std::vector<SnapEvent> update(const ObservationFrame& frame);
    void reset();
private:
    struct HandState {
        std::int64_t contact_since{},last_contact{},cooldown_until{};
        float tip_distance{},distal_distance{};
        bool armed{};
    };
    std::array<HandState,2> hands_;
    std::optional<std::int64_t> previous_time_;
    std::uint64_t epoch_{};
};
bool snap_hand_usable(const HandSample& hand);
}
