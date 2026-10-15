#pragma once
#include "vr/view_math.hpp"
#include <nlohmann/json.hpp>
#include <array>
#include <cstdint>
#include <optional>
#include <string>

namespace fantasynt::interaction {
inline constexpr std::size_t hand_joint_count=26;
struct PoseSample {
    vr::Pose pose;
    bool position_valid{},orientation_valid{},position_tracked{},orientation_tracked{};
    float radius_m{};
};
struct HandSample {
    std::string status{"unavailable"};
    bool active{};
    std::int32_t locate_result{};
    std::array<PoseSample,hand_joint_count> joints;
};
struct ObservationFrame {
    std::uint64_t sequence{},origin_epoch{};
    std::int64_t xr_time_ns{},received_monotonic_ns{},received_qpc{},qpc_frequency{};
    std::optional<std::int64_t> predicted_qpc;
    std::int64_t query_duration_ns{};
    std::string frame_status{"not_running"};
    std::int32_t session_state{},head_locate_result{};
    std::uint64_t view_state_flags{};
    std::optional<std::int64_t> reference_change_time_xr;
    PoseSample head;
    std::array<HandSample,2> hands;
};
struct MotionDescriptors {
    std::optional<double> head_linear_m_s,head_angular_rad_s;
    std::array<std::optional<double>,2> hand_linear_m_s;
};
class ObservationHistory {
public:
    MotionDescriptors advance(const ObservationFrame& frame);
private:
    std::optional<ObservationFrame> previous_;
};
std::int64_t monotonic_now_ns();
nlohmann::json observation_to_json(const ObservationFrame& frame);
nlohmann::json motion_to_json(const MotionDescriptors& motion);
bool tracking_quality_changed(const ObservationFrame& a,const ObservationFrame& b);
}
