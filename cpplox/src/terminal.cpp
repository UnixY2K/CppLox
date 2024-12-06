#include <cpplox/terminal.hpp>

#include <format>
#include <iostream>
#include <string>
#include <string_view>

namespace lox::cli::terminal {

std::string red_colored(std::string_view message) {
	return std::format("\033[31m{}\033[0m", message);
}

std::string cyan_colored(std::string_view message) {
	return std::format("\033[96m{}\033[0m", message);
}

std::string gray_colored(std::string_view message) {
	return std::format("\033[90m{}\033[0m", message);
}

std::string yellow_colored(std::string_view message) {
	return std::format("\033[33m{}\033[0m", message);
}

std::string green_colored(std::string_view message) {
	return std::format("\033[32m{}\033[0m", message);
}

std::string orange_colored(std::string_view message) {
	return std::format("\033[33m\033[38;5;202m{}\033[0m", message);
}

void logError(std::string_view message) {
	std::cerr << std::format("{}\n", red_colored(message));
}

} // namespace lox::cli::terminal
