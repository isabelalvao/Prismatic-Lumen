#include "protocol.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

namespace fantasynt {
namespace {
using Json = nlohmann::json;
bool identifier_valid(const std::string& id) {
    return !id.empty() && id.size() <= 64 && std::all_of(id.begin(), id.end(), [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9') || c == '-' || c == '_';
    });
}
float to_linear(float c) {
    return c <= 0.04045f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
}
int hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
std::optional<Color> parse_color(const Json& value) {
    if (!value.is_string()) return {};
    const auto& text = value.get_ref<const std::string&>();
    if (text.size() != 7 || text[0] != '#') return {};
    std::array<float, 3> rgb{};
    for (std::size_t i = 0; i < 3; ++i) {
        const int a = hex_digit(text[1 + 2 * i]), b = hex_digit(text[2 + 2 * i]);
        if (a < 0 || b < 0) return {};
        rgb[i] = to_linear(static_cast<float>(16 * a + b) / 255.0f);
    }
    return Color{rgb[0], rgb[1], rgb[2]};
}
std::string color_hex(const Color& c) {
    std::ostringstream result;
    result << '#' << std::hex << std::uppercase << std::setfill('0');
    for (auto value : {c.r, c.g, c.b}) {
        value = std::clamp(value, 0.0f, 1.0f);
        const float srgb = value <= 0.0031308f ? value * 12.92f :
            1.055f * std::pow(value, 1.0f / 2.4f) - 0.055f;
        result << std::setw(2) << static_cast<int>(std::round(srgb * 255.0f));
    }
    return result.str();
}
struct Field {
    const char* name;
    std::optional<float> ParameterPatch::* member;
};
constexpr std::array<Field, 6> fields{{
    {"light_intensity", &ParameterPatch::light_intensity},
    {"motion_speed", &ParameterPatch::motion_speed},
    {"pulse_strength", &ParameterPatch::pulse_strength},
    {"fog_density", &ParameterPatch::fog_density},
    {"geometry_density", &ParameterPatch::geometry_density},
    {"bloom", &ParameterPatch::bloom}
}};
}

ParseResult parse_command(std::string_view payload) {
    if (payload.size() > max_message_bytes) return ValidationError{"message_too_large", {}, {}};
    try {
        bool duplicate = false;
        std::vector<std::unordered_set<std::string>> objects;
        auto callback = [&](int depth, Json::parse_event_t event, Json& value) {
            if (depth > 32) throw std::runtime_error("JSON nesting limit");
            if (event == Json::parse_event_t::object_start) objects.emplace_back();
            else if (event == Json::parse_event_t::key && !objects.empty())
                duplicate |= !objects.back().insert(value.get<std::string>()).second;
            else if (event == Json::parse_event_t::object_end && !objects.empty()) objects.pop_back();
            return true;
        };
        const auto root = Json::parse(payload.begin(), payload.end(), callback);
        if (!root.is_object()) return ValidationError{"invalid_envelope", {"root"}, {}};
        std::string id;
        if (root.contains("id") && root["id"].is_string()) {
            auto candidate = root["id"].get<std::string>();
            if (identifier_valid(candidate)) id = candidate;
        }
        auto reject = [&](const char* code, std::string field) -> ParseResult {
            return ValidationError{code, {std::move(field)}, id};
        };
        if (duplicate) return reject("duplicate_key", "root");
        const std::unordered_set<std::string> allowed{"schema_version", "id", "command", "transition_seconds", "parameters"};
        for (auto it = root.begin(); it != root.end(); ++it)
            if (!allowed.contains(it.key())) return reject("unknown_field", it.key());
        for (const auto& key : allowed)
            if (!root.contains(key)) return reject("missing_field", key);
        if (!root["schema_version"].is_number_integer() || root["schema_version"] != 1)
            return reject("unsupported_version", "schema_version");
        if (id.empty()) return reject("invalid_identifier", "id");
        if (!root["command"].is_string() || root["command"] != "apply_parameters")
            return reject("unknown_command", "command");
        if (!root["transition_seconds"].is_number()) return reject("invalid_type", "transition_seconds");
        const double seconds = root["transition_seconds"].get<double>();
        if (!std::isfinite(seconds) || seconds < 0.1 || seconds > 15.0)
            return reject("out_of_range", "transition_seconds");
        const auto& params = root["parameters"];
        if (!params.is_object() || params.empty()) return reject("invalid_parameters", "parameters");
        Command command{id, seconds, {}};
        for (auto it = params.begin(); it != params.end(); ++it) {
            if (it.key() == "palette") {
                if (!it.value().is_array() || it.value().size() != 2) return reject("invalid_palette", "palette");
                auto a = parse_color(it.value()[0]), b = parse_color(it.value()[1]);
                if (!a || !b) return reject("invalid_palette", "palette");
                command.parameters.palette = std::array<Color, 2>{*a, *b};
                continue;
            }
            auto field = std::find_if(fields.begin(), fields.end(), [&](const auto& f) { return it.key() == f.name; });
            if (field == fields.end()) return reject("unknown_field", it.key());
            if (!it.value().is_number()) return reject("invalid_type", it.key());
            const double number = it.value().get<double>();
            if (!std::isfinite(number) || number < 0.0 || number > 1.0) return reject("out_of_range", it.key());
            command.parameters.*(field->member) = static_cast<float>(number);
        }
        return command;
    } catch (const std::exception&) {
        return ValidationError{"invalid_json", {"root"}, {}};
    }
}

nlohmann::json parameters_to_json(const Parameters& state) {
    return {{"palette", {color_hex(state.palette[0]), color_hex(state.palette[1])}},
        {"light_intensity", state.light_intensity}, {"motion_speed", state.motion_speed},
        {"pulse_strength", state.pulse_strength}, {"fog_density", state.fog_density},
        {"geometry_density", state.geometry_density}, {"bloom", state.bloom}};
}
}
