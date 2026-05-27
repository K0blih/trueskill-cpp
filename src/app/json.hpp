#pragma once

#include "trueskill/trueskill.hpp"

#include <nlohmann/json.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace trueskill::app {

using Json = nlohmann::json;

[[nodiscard]] Json parse_json(std::string_view input);

[[nodiscard]] Rating parse_rating(const Json& json);
[[nodiscard]] RatingGroups parse_rating_groups(const Json& json);
[[nodiscard]] std::vector<int> parse_ranks(const Json* json);
[[nodiscard]] Weights parse_weights(const Json* json);
[[nodiscard]] Environment parse_environment(const Json& request);

[[nodiscard]] Json rating_json(const Rating& rating);
[[nodiscard]] Json rating_groups_json(const RatingGroups& groups);
[[nodiscard]] std::string error_json(std::string_view message);

} // namespace trueskill::app
