#include <cpplox/terminal.hpp>

#include <format>
#include <iostream>
#include <string>
#include <string_view>

namespace lox::cli::terminal {

std::string red_colored(std::string_view message) {
	return std::format("\x1B[31m{}\x1B[0m", message);
}

std::string cyan_colored(std::string_view message) {
	return std::format("\x1B[96m{}\x1B[0m", message);
}

std::string gray_colored(std::string_view message) {
	return std::format("\x1B[90m{}\x1B[0m", message);
}

std::string yellow_colored(std::string_view message) {
	return std::format("\x1B[33m{}\x1B[0m", message);
}

std::string green_colored(std::string_view message) {
	return std::format("\x1B[32m{}\x1B[0m", message);
}

std::string orange_colored(std::string_view message) {
	return std::format("\x1B[38;5;202m{}\x1B[0m", message);
}

std::string lime_colored(std::string_view message) {
	return std::format("\x1B[38;5;112m{}\x1B[0m", message);
}

void logError(std::string_view message) {
	std::cerr << std::format("{}\n", red_colored(message));
}

} // namespace lox::cli::terminal
