#include "observation.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>

namespace fantasynt::interaction {
namespace {
using Json=nlohmann::json;
bool finite_position(const PoseSample& sample) {
    const auto& p=sample.pose.position;
    return sample.position_valid&&std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z);
}
double orientation_norm(const PoseSample& sample) {
    const auto& q=sample.pose.orientation;
    return std::sqrt(static_cast<double>(q.x)*q.x+static_cast<double>(q.y)*q.y+
        static_cast<double>(q.z)*q.z+static_cast<double>(q.w)*q.w);
}
bool finite_orientation(const PoseSample& sample) {
    const double norm=orientation_norm(sample);
    return sample.orientation_valid&&std::isfinite(norm)&&norm>1e-6;
}
Json pose_json(const PoseSample& sample) {
    const auto& p=sample.pose.position;
    const auto& q=sample.pose.orientation;
    return {{"position_m",finite_position(sample)?Json::array({p.x,p.y,p.z}):Json(nullptr)},
        {"orientation_xyzw",finite_orientation(sample)?Json::array({q.x,q.y,q.z,q.w}):Json(nullptr)},
        {"position_valid",finite_position(sample)},{"orientation_valid",finite_orientation(sample)},
        {"position_tracked",sample.position_tracked},{"orientation_tracked",sample.orientation_tracked},
        {"radius_m",sample.radius_m>0&&std::isfinite(sample.radius_m)?Json(sample.radius_m):Json(nullptr)}};
}
std::optional<double> linear_speed(const PoseSample& a,const PoseSample& b,double dt) {
    if(!finite_position(a)||!finite_position(b)||!a.position_tracked||!b.position_tracked) return {};
    const auto& x=a.pose.position; const auto& y=b.pose.position;
    return std::hypot(static_cast<double>(y.x)-x.x,static_cast<double>(y.y)-x.y,
        static_cast<double>(y.z)-x.z)/dt;
}
std::optional<double> angular_speed(const PoseSample& a,const PoseSample& b,double dt) {
    if(!finite_orientation(a)||!finite_orientation(b)||!a.orientation_tracked||!b.orientation_tracked) return {};
    const auto& x=a.pose.orientation; const auto& y=b.pose.orientation;
    const double dot=static_cast<double>(x.x)*y.x+static_cast<double>(x.y)*y.y+
        static_cast<double>(x.z)*y.z+static_cast<double>(x.w)*y.w;
    return 2*std::acos(std::clamp(std::abs(dot)/(orientation_norm(a)*orientation_norm(b)),0.0,1.0))/dt;
}
Json optional_number(const std::optional<double>& value) { return value?Json(*value):Json(nullptr); }
}
std::int64_t monotonic_now_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
MotionDescriptors ObservationHistory::advance(const ObservationFrame& frame) {
    MotionDescriptors result;
    // Não derivar movimento através de pausa, perda, recentragem ou salto temporal.
    if(previous_&&previous_->frame_status=="located"&&frame.frame_status=="located"&&
       previous_->origin_epoch==frame.origin_epoch&&frame.xr_time_ns>previous_->xr_time_ns) {
        const double dt=static_cast<double>(frame.xr_time_ns-previous_->xr_time_ns)/1e9;
        if(dt<=0.25) {
            result.head_linear_m_s=linear_speed(previous_->head,frame.head,dt);
            result.head_angular_rad_s=angular_speed(previous_->head,frame.head,dt);
            for(std::size_t i=0;i<2;++i)
                if(previous_->hands[i].active&&frame.hands[i].active&&
                   previous_->hands[i].status=="located"&&frame.hands[i].status=="located")
                    result.hand_linear_m_s[i]=linear_speed(previous_->hands[i].joints[0],frame.hands[i].joints[0],dt);
        }
    }
    previous_=frame;
    return result;
}
Json observation_to_json(const ObservationFrame& frame) {
    Json hands=Json::array();
    for(const auto& hand:frame.hands) {
        Json joints=Json::array();
        for(const auto& joint:hand.joints) joints.push_back(pose_json(joint));
        hands.push_back({{"status",hand.status},{"active",hand.active},
            {"locate_result",hand.locate_result},{"joints",std::move(joints)}});
    }
    return {{"sequence",frame.sequence},{"origin_epoch",frame.origin_epoch},{"reference_space","LOCAL"},
        {"xr_time_ns",frame.xr_time_ns>0?Json(frame.xr_time_ns):Json(nullptr)},
        {"pose_time_kind","predicted_display"},{"received_monotonic_ns",frame.received_monotonic_ns},
        {"received_qpc",frame.received_qpc},{"qpc_frequency",frame.qpc_frequency},
        {"predicted_qpc",frame.predicted_qpc?Json(*frame.predicted_qpc):Json(nullptr)},
        {"clock_mapping",frame.predicted_qpc?"xr_to_qpc":"paired_receipt_only"},
        {"query_duration_ns",frame.query_duration_ns},{"frame_status",frame.frame_status},
        {"session_state",frame.session_state},{"view_state_flags",frame.view_state_flags},
        {"reference_change_time_xr",frame.reference_change_time_xr?Json(*frame.reference_change_time_xr):Json(nullptr)},
        {"head_source","VIEW_space_in_LOCAL"},{"head_locate_result",frame.head_locate_result},
        {"head",pose_json(frame.head)},{"hands",std::move(hands)}};
}
Json motion_to_json(const MotionDescriptors& motion) {
    return {{"head_linear_m_s",optional_number(motion.head_linear_m_s)},
        {"head_angular_rad_s",optional_number(motion.head_angular_rad_s)},
        {"hand_linear_m_s",Json::array({optional_number(motion.hand_linear_m_s[0]),optional_number(motion.hand_linear_m_s[1])})}};
}
bool tracking_quality_changed(const ObservationFrame& a,const ObservationFrame& b) {
    const auto changed=[](const PoseSample& x,const PoseSample& y) {
        return finite_position(x)!=finite_position(y)||finite_orientation(x)!=finite_orientation(y)||
            x.position_tracked!=y.position_tracked||x.orientation_tracked!=y.orientation_tracked;
    };
    if(a.frame_status!=b.frame_status||a.origin_epoch!=b.origin_epoch||a.session_state!=b.session_state||
       a.head_locate_result!=b.head_locate_result||changed(a.head,b.head)||
       a.predicted_qpc.has_value()!=b.predicted_qpc.has_value()) return true;
    for(std::size_t i=0;i<2;++i) {
        const auto& x=a.hands[i]; const auto& y=b.hands[i];
        if(x.active!=y.active||x.status!=y.status||x.locate_result!=y.locate_result) return true;
        for(std::size_t j=0;j<hand_joint_count;++j) if(changed(x.joints[j],y.joints[j])) return true;
    }
    return false;
}
}
