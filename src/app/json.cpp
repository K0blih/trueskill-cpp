#include "app/json.hpp"

#include <stdexcept>
#include <string>

namespace trueskill::app {
namespace {

const Json* find(const Json& object, std::string_view key) {
    if (!object.is_object()) {
        throw std::invalid_argument("request must be an object");
    }
    const auto iter = object.find(std::string(key));
    if (iter == object.end()) {
        return nullptr;
    }
    return &*iter;
}

const Json& require(const Json& object, std::string_view key) {
    const Json* value = find(object, key);
    if (value == nullptr) {
        throw std::invalid_argument("missing required key: " + std::string(key));
    }
    return *value;
}

double as_number(const Json& json, std::string_view label) {
    if (!json.is_number()) {
        throw std::invalid_argument(std::string(label) + " must be a number");
    }
    return json.get<double>();
}

bool as_bool(const Json& json, std::string_view label) {
    if (!json.is_boolean()) {
        throw std::invalid_argument(std::string(label) + " must be a boolean");
    }
    return json.get<bool>();
}

} // namespace

Json parse_json(std::string_view input) {
    try {
        return Json::parse(input);
    } catch (const Json::parse_error& error) {
        throw std::invalid_argument(error.what());
    }
}

Rating parse_rating(const Json& json) {
    if (!json.is_object()) {
        throw std::invalid_argument("rating must be an object");
    }
    return {as_number(require(json, "mu"), "mu"), as_number(require(json, "sigma"), "sigma")};
}

RatingGroups parse_rating_groups(const Json& json) {
    if (!json.is_array()) {
        throw std::invalid_argument("rating_groups must be an array");
    }

    RatingGroups groups;
    for (const Json& team_json : json) {
        if (!team_json.is_array()) {
            throw std::invalid_argument("each rating group must be an array");
        }
        Team team;
        for (const Json& rating_json : team_json) {
            team.push_back(parse_rating(rating_json));
        }
        groups.push_back(std::move(team));
    }
    return groups;
}

std::vector<int> parse_ranks(const Json* json) {
    std::vector<int> ranks;
    if (json == nullptr) {
        return ranks;
    }
    if (!json->is_array()) {
        throw std::invalid_argument("ranks must be an array");
    }
    for (const Json& rank_json : *json) {
        ranks.push_back(rank_json.get<int>());
    }
    return ranks;
}

Weights parse_weights(const Json* json) {
    Weights weights;
    if (json == nullptr) {
        return weights;
    }
    if (!json->is_array()) {
        throw std::invalid_argument("weights must be an array");
    }
    for (const Json& team_json : *json) {
        if (!team_json.is_array()) {
            throw std::invalid_argument("each weight group must be an array");
        }
        std::vector<double> team;
        for (const Json& weight_json : team_json) {
            team.push_back(as_number(weight_json, "weight"));
        }
        weights.push_back(std::move(team));
    }
    return weights;
}

Environment parse_environment(const Json& request) {
    const Json* env = find(request, "environment");
    if (env == nullptr) {
        return {};
    }
    if (!env->is_object()) {
        throw std::invalid_argument("environment must be an object");
    }
    return {
        as_number(require(*env, "mu"), "environment.mu"),
        as_number(require(*env, "sigma"), "environment.sigma"),
        as_number(require(*env, "beta"), "environment.beta"),
        as_number(require(*env, "tau"), "environment.tau"),
        as_number(require(*env, "draw_probability"), "environment.draw_probability"),
    };
}

Json rating_json(const Rating& rating) {
    return Json{{"mu", rating.mu}, {"sigma", rating.sigma}};
}

Json rating_groups_json(const RatingGroups& groups) {
    Json result = Json::array();
    for (const Team& team : groups) {
        Json team_json = Json::array();
        for (const Rating& rating : team) {
            team_json.push_back(rating_json(rating));
        }
        result.push_back(std::move(team_json));
    }
    return result;
}

std::string error_json(std::string_view message) {
    return Json{{"error", std::string(message)}}.dump();
}

} // namespace trueskill::app
