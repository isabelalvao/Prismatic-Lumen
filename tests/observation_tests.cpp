#include "interaction/observation.hpp"
#include "test_support.hpp"
#include <limits>

using namespace fantasynt::interaction;
namespace {
ObservationFrame frame(std::int64_t time) {
    ObservationFrame result;
    result.xr_time_ns=time;
    result.frame_status="located";
    result.head.position_valid=result.head.orientation_valid=true;
    result.head.position_tracked=result.head.orientation_tracked=true;
    result.hands[0].status=result.hands[1].status="located";
    for(auto& hand:result.hands) {
        hand.active=true;
        hand.joints[0]=result.head;
    }
    return result;
}
}
int main() {
    ObservationHistory history;
    auto first=frame(100'000'000);
    CHECK(!history.advance(first).head_linear_m_s);
    auto second=frame(200'000'000);
    auto stationary=history.advance(second);
    CHECK(stationary.head_linear_m_s.has_value());
    CHECK_NEAR(*stationary.head_linear_m_s,0.0,1e-9);
    CHECK(stationary.hand_linear_m_s[0].has_value());

    auto moved=frame(300'000'000);
    moved.head.pose.position.x=0.02f;
    auto motion=history.advance(moved);
    CHECK_NEAR(*motion.head_linear_m_s,0.2,1e-6);
    CHECK_NEAR(*motion.head_angular_rad_s,0.0,1e-9);
    // A origem e os intervalos sem seguimento interrompem todas as derivadas.
    moved.origin_epoch=1; moved.xr_time_ns+=100'000'000;
    CHECK(!history.advance(moved).head_linear_m_s);
    moved.hands[0].status="inactive"; moved.hands[0].active=false;
    moved.xr_time_ns+=100'000'000;
    CHECK(!history.advance(moved).hand_linear_m_s[0]);
    moved.hands[0].status="located"; moved.hands[0].active=true;
    moved.xr_time_ns+=100'000'000;
    CHECK(!history.advance(moved).hand_linear_m_s[0]);

    moved.head.position_valid=false; moved.xr_time_ns+=100'000'000;
    CHECK(!history.advance(moved).head_linear_m_s);
    auto json=observation_to_json(moved);
    CHECK(json["head"]["position_m"].is_null());
    CHECK(json["head"]["orientation_xyzw"].is_array());
    moved.head.position_valid=true; moved.xr_time_ns+=100'000'000;
    CHECK(!history.advance(moved).head_linear_m_s);
    moved.head.position_tracked=false; moved.xr_time_ns+=100'000'000;
    CHECK(!history.advance(moved).head_linear_m_s);
    moved.head.position_tracked=true; moved.xr_time_ns+=100'000'000;
    CHECK(!history.advance(moved).head_linear_m_s);

    moved.frame_status="not_rendering"; moved.xr_time_ns+=100'000'000;
    CHECK(!history.advance(moved).head_linear_m_s);
    moved.frame_status="located"; moved.xr_time_ns+=100'000'000;
    CHECK(!history.advance(moved).head_linear_m_s);
    CHECK(!history.advance(moved).head_linear_m_s); // Tempo repetido.
    moved.head.pose.position.x=std::numeric_limits<float>::quiet_NaN();
    moved.xr_time_ns+=100'000'000;
    CHECK(!history.advance(moved).head_linear_m_s);
    CHECK(observation_to_json(moved)["head"]["position_m"].is_null());

    auto a=frame(100'000'000),b=frame(200'000'000);
    b.head.pose.orientation={0,0,1,0};
    ObservationHistory rotation;
    rotation.advance(a);
    CHECK_NEAR(*rotation.advance(b).head_angular_rad_s,31.41592653589793,1e-6);
    // Relógio de receção diferente não muda a velocidade física.
    ObservationHistory replay;
    a.received_monotonic_ns=50; b.received_monotonic_ns=900000000000LL;
    replay.advance(a);
    CHECK_NEAR(*replay.advance(b).head_angular_rad_s,31.41592653589793,1e-6);
    b.xr_time_ns+=1'000'000'000;
    CHECK(!replay.advance(b).head_linear_m_s); // Suspensão longa.
    return test_finish();
}
