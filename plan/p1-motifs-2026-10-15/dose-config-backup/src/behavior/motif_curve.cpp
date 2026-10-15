#include "motif_curve.hpp"
#include <algorithm>
#include <cmath>

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
double master_gain(double t) { return 1.0-gain_depth*scheduled_envelope(t); }
}
