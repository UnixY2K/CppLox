#include <cpplox/compiler.hpp>
#include <cpplox/scanner.hpp>

#include <cstddef>
#include <format>
#include <iostream>

namespace lox {
auto compile(std::string_view source)
    -> std::expected<std::vector<Chunk>, std::string> {
	size_t line = 1;
	Scanner scanner{source};
	for (Token token = scanner.scanToken();
	     token.type != Token::TokenType::TOKEN_EOF;
	     token = scanner.scanToken()) {
		if (token.line != line) {
			std::cout << std::format("{:4d} ", token.line);
			line = token.line;
		} else {
			std::cout << "   | ";
		}
		std::cout << std::format("{:16} '{}'\n", static_cast<int>(token.type),
		                         token.lexeme);
	}

	return std::vector<Chunk>{};
}
} // namespace lox
