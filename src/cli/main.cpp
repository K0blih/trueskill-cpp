#include "trueskill/trueskill.hpp"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace {

struct Json;
using JsonArray = std::vector<Json>;
using JsonObject = std::map<std::string, Json>;

struct Json {
    using Value = std::variant<std::nullptr_t, bool, double, std::string, JsonArray, JsonObject>;
    Value value;
};

class Parser {
public:
    explicit Parser(std::string_view input) : input_(input) {}

    Json parse() {
        Json json = parse_value();
        skip_ws();
        if (pos_ != input_.size()) {
            fail("unexpected trailing input");
        }
        return json;
    }

private:
    Json parse_value() {
        skip_ws();
        if (pos_ >= input_.size()) {
            fail("unexpected end of input");
        }
        const char ch = input_[pos_];
        if (ch == '{') {
            return Json{parse_object()};
        }
        if (ch == '[') {
            return Json{parse_array()};
        }
        if (ch == '"') {
            return Json{parse_string()};
        }
        if (ch == 't') {
            consume("true");
            return Json{true};
        }
        if (ch == 'f') {
            consume("false");
            return Json{false};
        }
        if (ch == 'n') {
            consume("null");
            return Json{nullptr};
        }
        return Json{parse_number()};
    }

    JsonObject parse_object() {
        expect('{');
        JsonObject object;
        skip_ws();
        if (match('}')) {
            return object;
        }
        while (true) {
            skip_ws();
            if (peek() != '"') {
                fail("expected object key");
            }
            std::string key = parse_string();
            skip_ws();
            expect(':');
            object.emplace(std::move(key), parse_value());
            skip_ws();
            if (match('}')) {
                return object;
            }
            expect(',');
        }
    }

    JsonArray parse_array() {
        expect('[');
        JsonArray array;
        skip_ws();
        if (match(']')) {
            return array;
        }
        while (true) {
            array.push_back(parse_value());
            skip_ws();
            if (match(']')) {
                return array;
            }
            expect(',');
        }
    }

    std::string parse_string() {
        expect('"');
        std::string result;
        while (pos_ < input_.size()) {
            const char ch = input_[pos_++];
            if (ch == '"') {
                return result;
            }
            if (ch == '\\') {
                if (pos_ >= input_.size()) {
                    fail("unterminated escape");
                }
                const char escaped = input_[pos_++];
                switch (escaped) {
                case '"':
                case '\\':
                case '/':
                    result.push_back(escaped);
                    break;
                case 'b':
                    result.push_back('\b');
                    break;
                case 'f':
                    result.push_back('\f');
                    break;
                case 'n':
                    result.push_back('\n');
                    break;
                case 'r':
                    result.push_back('\r');
                    break;
                case 't':
                    result.push_back('\t');
                    break;
                default:
                    fail("unsupported string escape");
                }
            } else {
                result.push_back(ch);
            }
        }
        fail("unterminated string");
    }

    double parse_number() {
        const std::size_t start = pos_;
        if (peek() == '-') {
            ++pos_;
        }
        consume_digits();
        if (peek() == '.') {
            ++pos_;
            consume_digits();
        }
        if (peek() == 'e' || peek() == 'E') {
            ++pos_;
            if (peek() == '+' || peek() == '-') {
                ++pos_;
            }
            consume_digits();
        }
        const std::string number(input_.substr(start, pos_ - start));
        char* end = nullptr;
        const double value = std::strtod(number.c_str(), &end);
        if (end == number.c_str() || *end != '\0') {
            fail("invalid number");
        }
        return value;
    }

    void consume_digits() {
        const std::size_t start = pos_;
        while (std::isdigit(static_cast<unsigned char>(peek()))) {
            ++pos_;
        }
        if (pos_ == start) {
            fail("expected digit");
        }
    }

    void consume(std::string_view literal) {
        if (input_.substr(pos_, literal.size()) != literal) {
            fail("unexpected token");
        }
        pos_ += literal.size();
    }

    void expect(char expected) {
        skip_ws();
        if (!match(expected)) {
            fail(std::string("expected '") + expected + "'");
        }
    }

    bool match(char expected) {
        if (peek() == expected) {
            ++pos_;
            return true;
        }
        return false;
    }

    char peek() const {
        if (pos_ >= input_.size()) {
            return '\0';
        }
        return input_[pos_];
    }

    void skip_ws() {
        while (pos_ < input_.size() && std::isspace(static_cast<unsigned char>(input_[pos_]))) {
            ++pos_;
        }
    }

    [[noreturn]] void fail(const std::string& message) const {
        throw std::runtime_error("JSON parse error at byte " + std::to_string(pos_) + ": " + message);
    }

    std::string_view input_;
    std::size_t pos_ = 0;
};

const JsonObject& as_object(const Json& json, std::string_view label) {
    if (const auto* object = std::get_if<JsonObject>(&json.value)) {
        return *object;
    }
    throw std::invalid_argument(std::string(label) + " must be an object");
}

const JsonArray& as_array(const Json& json, std::string_view label) {
    if (const auto* array = std::get_if<JsonArray>(&json.value)) {
        return *array;
    }
    throw std::invalid_argument(std::string(label) + " must be an array");
}

double as_number(const Json& json, std::string_view label) {
    if (const auto* value = std::get_if<double>(&json.value)) {
        return *value;
    }
    throw std::invalid_argument(std::string(label) + " must be a number");
}

bool as_bool(const Json& json, std::string_view label) {
    if (const auto* value = std::get_if<bool>(&json.value)) {
        return *value;
    }
    throw std::invalid_argument(std::string(label) + " must be a boolean");
}

const Json* find(const JsonObject& object, std::string_view key) {
    const auto iter = object.find(std::string(key));
    if (iter == object.end()) {
        return nullptr;
    }
    return &iter->second;
}

const Json& require(const JsonObject& object, std::string_view key) {
    const Json* value = find(object, key);
    if (value == nullptr) {
        throw std::invalid_argument("missing required key: " + std::string(key));
    }
    return *value;
}

trueskill::Rating parse_rating(const Json& json) {
    const JsonObject& object = as_object(json, "rating");
    return {as_number(require(object, "mu"), "mu"), as_number(require(object, "sigma"), "sigma")};
}

trueskill::RatingGroups parse_rating_groups(const Json& json) {
    trueskill::RatingGroups groups;
    for (const Json& team_json : as_array(json, "rating_groups")) {
        trueskill::Team team;
        for (const Json& rating_json : as_array(team_json, "team")) {
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
    for (const Json& rank_json : as_array(*json, "ranks")) {
        ranks.push_back(static_cast<int>(as_number(rank_json, "rank")));
    }
    return ranks;
}

trueskill::Weights parse_weights(const Json* json) {
    trueskill::Weights weights;
    if (json == nullptr) {
        return weights;
    }
    for (const Json& team_json : as_array(*json, "weights")) {
        std::vector<double> team;
        for (const Json& weight_json : as_array(team_json, "weight team")) {
            team.push_back(as_number(weight_json, "weight"));
        }
        weights.push_back(std::move(team));
    }
    return weights;
}

trueskill::Environment parse_environment(const JsonObject& request) {
    const Json* env_json = find(request, "environment");
    if (env_json == nullptr) {
        return {};
    }
    const JsonObject& env = as_object(*env_json, "environment");
    return {
        as_number(require(env, "mu"), "environment.mu"),
        as_number(require(env, "sigma"), "environment.sigma"),
        as_number(require(env, "beta"), "environment.beta"),
        as_number(require(env, "tau"), "environment.tau"),
        as_number(require(env, "draw_probability"), "environment.draw_probability"),
    };
}

std::string rating_json(const trueskill::Rating& rating) {
    std::ostringstream out;
    out << std::setprecision(12) << "{\"mu\":" << rating.mu << ",\"sigma\":" << rating.sigma << '}';
    return out.str();
}

std::string rating_groups_json(const trueskill::RatingGroups& groups) {
    std::ostringstream out;
    out << '[';
    for (std::size_t team = 0; team < groups.size(); ++team) {
        if (team != 0) {
            out << ',';
        }
        out << '[';
        for (std::size_t player = 0; player < groups[team].size(); ++player) {
            if (player != 0) {
                out << ',';
            }
            out << rating_json(groups[team][player]);
        }
        out << ']';
    }
    out << ']';
    return out.str();
}

std::string read_input(int argc, char** argv) {
    if (argc >= 3 && std::string_view(argv[2]) != "-") {
        const std::string arg = argv[2];
        if (!arg.empty() && (arg[0] == '{' || arg[0] == '[')) {
            return arg;
        }
        std::ifstream file(arg);
        if (!file) {
            throw std::runtime_error("failed to open input file: " + arg);
        }
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    std::ostringstream buffer;
    buffer << std::cin.rdbuf();
    return buffer.str();
}

void print_usage() {
    std::cerr
        << "Usage: trueskill_cli <command> [input.json|-|json]\n"
        << "\nCommands:\n"
        << "  rate          Update arbitrary teams. Input keys: rating_groups, optional ranks, weights, environment, min_delta.\n"
        << "  rate-1vs1     Update two players. Input keys: first_player, second_player, optional drawn, environment, min_delta.\n"
        << "  quality       Compute arbitrary-team match quality. Input keys: rating_groups, optional weights, environment.\n"
        << "  quality-1vs1  Compute two-player match quality. Input keys: first_player, second_player, optional environment.\n"
        << "  expose        Compute conservative exposure. Input keys: rating, optional environment.\n";
}

int run(int argc, char** argv) {
    if (argc < 2 || std::string_view(argv[1]) == "--help" || std::string_view(argv[1]) == "-h") {
        print_usage();
        return argc < 2 ? 1 : 0;
    }

    const std::string command = argv[1];
    const Json request_json = Parser(read_input(argc, argv)).parse();
    const JsonObject& request = as_object(request_json, "request");
    const trueskill::Environment env = parse_environment(request);

    if (command == "rate") {
        const trueskill::RatingGroups groups = parse_rating_groups(require(request, "rating_groups"));
        const std::vector<int> ranks = parse_ranks(find(request, "ranks"));
        const trueskill::Weights weights = parse_weights(find(request, "weights"));
        const double min_delta = find(request, "min_delta") == nullptr ? 0.0001 : as_number(*find(request, "min_delta"), "min_delta");
        std::cout << "{\"rating_groups\":" << rating_groups_json(env.rate(groups, ranks, weights, min_delta)) << "}\n";
        return 0;
    }
    if (command == "rate-1vs1") {
        const trueskill::Rating first = parse_rating(require(request, "first_player"));
        const trueskill::Rating second = parse_rating(require(request, "second_player"));
        const bool drawn = find(request, "drawn") == nullptr ? false : as_bool(*find(request, "drawn"), "drawn");
        const double min_delta = find(request, "min_delta") == nullptr ? 0.0001 : as_number(*find(request, "min_delta"), "min_delta");
        const auto [updated_first, updated_second] = env.rate_1vs1(first, second, drawn, min_delta);
        std::cout << "{\"first_player\":" << rating_json(updated_first) << ",\"second_player\":" << rating_json(updated_second) << "}\n";
        return 0;
    }
    if (command == "quality") {
        const trueskill::RatingGroups groups = parse_rating_groups(require(request, "rating_groups"));
        const trueskill::Weights weights = parse_weights(find(request, "weights"));
        std::cout << std::setprecision(12) << "{\"quality\":" << env.quality(groups, weights) << "}\n";
        return 0;
    }
    if (command == "quality-1vs1") {
        const trueskill::Rating first = parse_rating(require(request, "first_player"));
        const trueskill::Rating second = parse_rating(require(request, "second_player"));
        std::cout << std::setprecision(12) << "{\"quality\":" << env.quality_1vs1(first, second) << "}\n";
        return 0;
    }
    if (command == "expose") {
        std::cout << std::setprecision(12) << "{\"exposure\":" << env.expose(parse_rating(require(request, "rating"))) << "}\n";
        return 0;
    }

    print_usage();
    throw std::invalid_argument("unknown command: " + command);
}

} // namespace

int main(int argc, char** argv) {
    try {
        return run(argc, argv);
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
