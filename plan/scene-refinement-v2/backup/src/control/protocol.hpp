#pragma once
#include "parameters.hpp"
#include <string_view>
#include <nlohmann/json_fwd.hpp>

namespace fantasynt {
ParseResult parse_command(std::string_view payload);
nlohmann::json parameters_to_json(const Parameters& state);
}
