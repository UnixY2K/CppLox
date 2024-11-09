#include <cpplox/chunk.hpp>
#include <cpplox/debug.hpp>
#include <cpplox/vm.hpp>
#include <repl.hpp>

#include <format>
#include <iostream>

int main(int argc, char *argv[]) {
	if (argc == 1) {
		lox::cli::repl();
	} else if (argc == 2) {
		return lox::cli::runFile(argv[1]);
	} else {
		std::cerr << std::format("Usage: {} [path]\n", argv[0]);
		exit(64);
	}
}
