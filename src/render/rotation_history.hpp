#pragma once
#include <span>
namespace fantasynt {
struct AngularRateKey { double time,angle,rate; };
double rotation_integral(std::span<const AngularRateKey> keys,double time);
double rotation_angle(std::span<const AngularRateKey> keys,double birth,double time);
}
