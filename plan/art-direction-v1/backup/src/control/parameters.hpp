#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace fantasynt {
struct Color {
    float r{}, g{}, b{};
};
struct Parameters {
    std::array<Color, 2> palette{{{0.723055f, 0.822786f, 1.0f}, {0.327778f, 0.47932f, 0.871367f}}};
    float light_intensity{0.6f};
    float motion_speed{0.3f};
    float pulse_strength{0.4f};
    float fog_density{0.2f};
    float geometry_density{0.5f};
    float bloom{0.35f};
};
struct ParameterPatch {
    std::optional<std::array<Color, 2>> palette;
    std::optional<float> light_intensity, motion_speed, pulse_strength;
    std::optional<float> fog_density, geometry_density, bloom;
};
struct Command {
    std::string id;
    double transition_seconds{};
    ParameterPatch parameters;
};
struct ValidationError {
    std::string code;
    std::vector<std::string> fields;
    std::string id;
};
using ParseResult = std::variant<Command, ValidationError>;
inline constexpr std::size_t max_message_bytes = 16 * 1024;
}
