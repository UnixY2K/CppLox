#include "cpplox/vm.hpp"
#include <cstddef>
#include <repl.hpp>

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>

namespace lox::cli {

bool is_whitespace(char p) {
	auto ne = [p](auto q) { return p != q; };
	return !!(" \t\n\v\r\f" | std::views::drop_while(ne));
}

std::string_view trim(std::string_view sv) {
	sv.remove_prefix(
	    std::ranges::find_if(sv, [](char c) { return !is_whitespace(c); }) -
	    sv.begin());
	sv.remove_suffix(sv.end() -
	                 std::ranges::find_if(sv | std::views::reverse, [](char c) {
		                 return !is_whitespace(c);
	                 }).base());
	return sv;
}

void repl() {
	VM vm;
	std::string line;
	std::cout << "Lox REPL\n";
	std::cout << "Type '#exit' to quit\n";
	std::cout << "> ";
	while (std::getline(std::cin, line)) {
		line = trim(line);
		if (!line.empty()) {

			// check if starts with #
			if (line[0] == '#') {
				if (line == "#exit") {
					return;
				} else if (line == "#help") {
					std::cout << "Lox REPL\n";
					std::cout << "Type '#exit' to quit\n";
				} else if (line == "#clear") {
					std::cout << "\033[2J\033[1;1H";
				} else if (line == "#debug_trace") {
					vm.debug_trace_instruction = !vm.debug_trace_instruction;
					std::cout << std::format(
					    "Debug trace is {}\n",
					    vm.debug_trace_instruction ? "on" : "off");
				} else {
					std::cout << "Unknown command\n";
				}
			} else {
				vm.interpret(line);
			}
		}
		std::cout << "> ";
	}
}

int runFile(std::string_view path) {
	// check if the file exists
	if (!std::filesystem::exists(path)) {
		std::cerr << std::format("File '{}' does not exist\n", path);
		return 1;
	}
	std::ifstream file(path.data());
	if (!file.is_open()) {
		std::cerr << std::format("Could not open file '{}'\n", path);
		return 1;
	}
	{
		size_t size = std::filesystem::file_size(path);
		std::string source;
		source.reserve(size);
		std::copy(std::istreambuf_iterator<char>(file),
		          std::istreambuf_iterator<char>(), std::back_inserter(source));
		VM vm;
		InterpretResult result = vm.interpret(source);
		if (result == InterpretResult::COMPILE_ERROR) {
			return 65;
		}
		if (result == InterpretResult::RUNTIME_ERROR) {
			return 70;
		}
	}

	return 0;
}
} // namespace lox::cli