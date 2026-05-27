#pragma once

#include <string>
#include <string_view>

namespace trueskill::app {

[[nodiscard]] std::string run_command(std::string_view command, std::string_view request_body);

} // namespace trueskill::app
