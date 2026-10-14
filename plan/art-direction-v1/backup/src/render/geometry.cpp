#include "geometry.hpp"
#include <algorithm>
#include <cmath>
#include <random>
namespace fantasynt {
std::vector<Instance> make_instances(std::uint32_t seed, std::size_t count) {
    if (!count) return {};
    std::mt19937 random(seed);
    auto unit = [&]() { return static_cast<float>(random() >> 8) / 16777215.0f; };
    std::vector<Instance> instances;
    instances.reserve(4096);
    for (std::size_t i = 0; i < 4096; ++i) {
        const float column = static_cast<float>(i % 64) - 31.5f;
        const float row = static_cast<float>(i / 64);
        const float x = column * 750.0f + (column < 0 ? -500.0f : 500.0f);
        const float shape = unit();
        const float height = shape < 0.55f ? 850.0f + unit() * 1000.0f : 180.0f + unit() * 650.0f;
        const float width = shape < 0.55f ? 250.0f + unit() * 280.0f : 50.0f + unit() * 85.0f;
        Instance instance;
        instance.transform = {width,0,0,0, 0,height*0.5f,0,0, 0,0,width,0,
            x,height*0.5f,(row+1)*650.0f,1};
        instance.color_mix = unit();
        instance.phase = unit();
        instance.variation = shape;
        instances.push_back(instance);
    }
    // O prefixo mantém uma distribuição espacial estável em todas as densidades.
    for (std::size_t i = instances.size() - 1; i > 0; --i)
        std::swap(instances[i], instances[random() % (i + 1)]);
    for (std::size_t i = 0; i < instances.size(); ++i)
        instances[i].padding = (static_cast<float>(i) + 0.5f) / 4096.0f * (1.0f-1.0f/64.0f);
    instances.resize(std::min(count, std::size_t{4096}));
    return instances;
}
}
