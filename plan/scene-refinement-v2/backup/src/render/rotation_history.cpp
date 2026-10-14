#include "rotation_history.hpp"
#include <algorithm>
#include <cmath>
namespace fantasynt {
double rotation_integral(std::span<const AngularRateKey> keys,double time) {
    if(keys.empty()||!std::isfinite(time)) return 0;
    const auto next=std::upper_bound(keys.begin(),keys.end(),time,[](double t,const auto& key) { return t<key.time; });
    const auto& key=next==keys.begin()?keys.front():*(next-1);
    return key.angle+(time-key.time)*key.rate;
}
double rotation_angle(std::span<const AngularRateKey> keys,double birth,double time) {
    if(time<=birth||!std::isfinite(birth)||!std::isfinite(time)) return 0;
    // Integrar só depois de nascer: segmentos futuros começam sempre no ângulo zero.
    return std::max(0.0,rotation_integral(keys,time)-rotation_integral(keys,birth));
}
}
