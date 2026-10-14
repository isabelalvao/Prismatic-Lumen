#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace fantasynt {
struct Instance {
    std::array<float, 16> transform{};
    float color_mix{}, phase{}, variation{}, padding{};
};
std::vector<Instance> make_instances(std::uint32_t seed, std::size_t count);
}
