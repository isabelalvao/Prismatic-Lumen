#include "motif_curve.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace fantasynt::behavior {
namespace {
double smooth(double t) { t=std::clamp(t,0.0,1.0); return t*t*(3.0-2.0*t); }
}
bool protected_time(double t) {
    return !std::isfinite(t)||t<40||(t>=76&&t<124)||(t>=164&&t<184)||
        (t>=208&&t<306)||t>=neutral_deadline;
}
double envelope(const Opportunity& plan,double t) {
    if(protected_time(t)||t<=plan.start||t>=plan.start+plan.duration) return 0;
    return std::min(smooth((t-plan.start)/ramp_seconds),
                    smooth((plan.start+plan.duration-t)/ramp_seconds));
}
double scheduled_envelope(double t) {
    for(const auto& plan:agenda) if(t>=plan.start&&t<plan.start+plan.duration) return envelope(plan,t);
    return 0;
}
MotifDose validated_dose(MotifDose dose) {
    if(!std::isfinite(dose.light_attenuation)||dose.light_attenuation<0||dose.light_attenuation>0.6||
       !std::isfinite(dose.master_attenuation)||dose.master_attenuation<0||dose.master_attenuation>1)
        throw std::invalid_argument("Invalid P1 dose: light attenuation 0..0.6, MASTER attenuation 0..1");
    return dose;
}
double master_gain(double t,double attenuation) { return 1.0-attenuation*scheduled_envelope(t); }
}
