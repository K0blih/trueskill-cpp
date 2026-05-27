#pragma once

#include <string>
#include <string_view>

namespace skill_rating::app {

[[nodiscard]] std::string run_command(std::string_view command, std::string_view request_body);

} // namespace skill_rating::app
