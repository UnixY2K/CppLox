#pragma once

#include <string>
#include <string_view>

namespace lox::cli::terminal {

std::string red_colored(std::string_view message);

std::string cyan_colored(std::string_view message);

std::string yellow_colored(std::string_view message);

std::string green_colored(std::string_view message);

std::string orange_colored(std::string_view message);

void logError(std::string_view message);

} // namespace lox::cli::terminal
