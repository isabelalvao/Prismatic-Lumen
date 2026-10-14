#include "test_support.hpp"
#include "render/rotation_history.hpp"
#include <array>
using namespace fantasynt;
int main() {
    constexpr double old_rate=0.034906585,new_rate=0.2617993878;
    const std::array<AngularRateKey,2> keys{{{0,0,old_rate},{146,146*old_rate,new_rate}}};
    CHECK_NEAR(rotation_angle(keys,148.8,148.799),0,0.000001);
    CHECK_NEAR(rotation_angle(keys,148.8,148.8),0,0.000001);
    CHECK_NEAR(rotation_angle(keys,148.8,148.81),0.01*new_rate,0.000001);
    CHECK_NEAR(rotation_angle(keys,136.8,146),(146-136.8)*old_rate,0.000001);
    CHECK_NEAR(rotation_angle(keys,136.8,147),(146-136.8)*old_rate+new_rate,0.000001);
    const std::array<AngularRateKey,3> twice{{keys[0],keys[1],{149,146*old_rate+3*new_rate,0}}};
    CHECK_NEAR(rotation_angle(twice,148.8,150),0.2*new_rate,0.000001);
    return test_finish();
}
