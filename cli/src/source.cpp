#include <cpplox/compiler.hpp>
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
		// if the first argument is "-c" then only compile the file and print
		// the bytecode
		if (argc == 3 && std::string_view(argv[1]) == "-c") {
			return lox::cli::compileFile(argv[2]);
		} else {
			std::cerr << std::format("Usage: {} [path]\n", argv[0]);
			exit(64);
		}
	}
}
