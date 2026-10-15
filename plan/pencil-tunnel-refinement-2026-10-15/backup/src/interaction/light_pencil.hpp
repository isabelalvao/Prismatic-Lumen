#pragma once
#include "observation.hpp"
#include <array>
#include <optional>
#include <string_view>
#include <vector>

namespace fantasynt::interaction {
inline constexpr const char* light_pencil_version="index-only-trail-1";
inline constexpr double pencil_lifetime=0.65;
inline constexpr std::size_t pencil_max_points=128;
struct PencilPoint {
    vr::Vector3 position_m;
    std::int64_t xr_time_ns{};
    std::uint64_t stroke{};
};
struct PencilEvent {
    std::size_t hand{};
    std::int64_t xr_time_ns{};
    std::uint64_t origin_epoch{};
    std::string_view event,reason;
};
bool pencil_hand_usable(const HandSample& hand);
bool index_only_pose(const HandSample& hand);
class LightPencil {
public:
    std::vector<PencilEvent> update(const ObservationFrame& frame);
    const std::vector<PencilPoint>& points(std::size_t hand) const { return hands_[hand].points; }
    bool writing(std::size_t hand) const { return hands_[hand].writing; }
private:
    struct State {
        std::vector<PencilPoint> points;
        bool writing{};
        std::int64_t candidate_since{};
        std::uint64_t stroke{};
        std::optional<vr::Vector3> raw,filtered;
    };
    std::array<State,2> hands_;
    std::optional<std::int64_t> previous_time_;
    std::uint64_t epoch_{};
};
}
