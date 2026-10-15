#pragma once
#include <array>
#include <cstddef>
#include <string_view>

namespace fantasynt::behavior {
inline constexpr std::string_view repertoire_version="p1-scheduled-long-dip-2";
inline constexpr double neutral_deadline=392.0;
inline constexpr double light_depth=0.06, gain_depth=0.12, ramp_seconds=2.0;
struct MotifDose {
    double light_attenuation{light_depth},master_attenuation{gain_depth};
};
MotifDose validated_dose(MotifDose dose);
struct Opportunity {
    std::string_view id,origin,ancestor;
    double start,duration;
};
// Uma só família no ensaio P1: dose/pico iguais para oportunidades de igual duração.
inline constexpr std::array<Opportunity,8> agenda{{
    {"M1","autonomous","",50,12}, {"M1.1","autonomous_return","M1",66,8},
    {"M1.2","autonomous_return","M1.1",152,10}, {"M2","autonomous","M1.2",188,12},
    {"M2.1","autonomous_return","M2",202,6}, {"M3","autonomous","M2.1",316,12},
    {"M3.1","autonomous_return","M3",334,10}, {"M1.3","autonomous_return","M1.2",370,12}
}};
bool protected_time(double seconds);
double envelope(const Opportunity& plan,double seconds);
double scheduled_envelope(double seconds);
double master_gain(double seconds,double attenuation=gain_depth);
}
