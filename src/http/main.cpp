#include "app/commands.hpp"
#include "app/json.hpp"

#include <httplib.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

struct Config {
    std::string host = "127.0.0.1";
    int port = 8080;
};

Config parse_args(int argc, char** argv) {
    Config config;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if ((arg == "--host" || arg == "-h") && i + 1 < argc) {
            config.host = argv[++i];
        } else if ((arg == "--port" || arg == "-p") && i + 1 < argc) {
            config.port = std::stoi(argv[++i]);
        } else if (arg == "--help") {
            std::cout << "Usage: trueskill_http [--host 127.0.0.1] [--port 8080]\n";
            std::exit(0);
        } else {
            throw std::invalid_argument("unknown argument: " + arg);
        }
    }
    if (config.port <= 0 || config.port > 65535) {
        throw std::invalid_argument("port must be in 1..65535");
    }
    return config;
}

void set_json(httplib::Response& response, int status, std::string body) {
    response.status = status;
    response.set_content(std::move(body), "application/json");
}

void register_command(httplib::Server& server, const char* route, const char* command) {
    server.Post(route, [command](const httplib::Request& request, httplib::Response& response) {
        try {
            set_json(response, 200, trueskill::app::run_command(command, request.body));
        } catch (const std::invalid_argument& error) {
            set_json(response, 400, trueskill::app::error_json(error.what()));
        } catch (const std::exception& error) {
            set_json(response, 500, trueskill::app::error_json(error.what()));
        }
    });
}

int run(int argc, char** argv) {
    const Config config = parse_args(argc, argv);
    httplib::Server server;

    server.Get("/health", [](const httplib::Request&, httplib::Response& response) {
        set_json(response, 200, "{\"status\":\"ok\"}");
    });
    register_command(server, "/rate", "rate");
    register_command(server, "/rate-1vs1", "rate-1vs1");
    register_command(server, "/quality", "quality");
    register_command(server, "/quality-1vs1", "quality-1vs1");
    register_command(server, "/expose", "expose");

    server.set_error_handler([](const httplib::Request&, httplib::Response& response) {
        if (response.status == 404) {
            set_json(response, 404, trueskill::app::error_json("unknown route"));
        }
    });

    std::cout << "trueskill_http listening on http://" << config.host << ':' << config.port << '\n';
    if (!server.listen(config.host, config.port)) {
        throw std::runtime_error("failed to listen on " + config.host + ":" + std::to_string(config.port));
    }
    return 0;
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
