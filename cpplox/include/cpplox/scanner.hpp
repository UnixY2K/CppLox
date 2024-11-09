#pragma once

#include <cstddef>
#include <string_view>
#include <unordered_map>
namespace lox {

struct Token {
	enum class TokenType {
		// Single-character tokens.
		TOKEN_LEFT_PAREN,
		TOKEN_RIGHT_PAREN,
		TOKEN_LEFT_BRACE,
		TOKEN_RIGHT_BRACE,
		TOKEN_COMMA,
		TOKEN_DOT,
		TOKEN_MINUS,
		TOKEN_PLUS,
		TOKEN_SEMICOLON,
		TOKEN_SLASH,
		TOKEN_STAR,
		// One or two character tokens.
		TOKEN_BANG,
		TOKEN_BANG_EQUAL,
		TOKEN_EQUAL,
		TOKEN_EQUAL_EQUAL,
		TOKEN_GREATER,
		TOKEN_GREATER_EQUAL,
		TOKEN_LESS,
		TOKEN_LESS_EQUAL,
		// Literals.
		TOKEN_IDENTIFIER,
		TOKEN_STRING,
		TOKEN_NUMBER,
		// Keywords.
		TOKEN_AND,
		TOKEN_CLASS,
		TOKEN_ELSE,
		TOKEN_FALSE,
		TOKEN_FOR,
		TOKEN_FUN,
		TOKEN_IF,
		TOKEN_NIL,
		TOKEN_OR,
		TOKEN_PRINT,
		TOKEN_RETURN,
		TOKEN_SUPER,
		TOKEN_THIS,
		TOKEN_TRUE,
		TOKEN_VAR,
		TOKEN_WHILE,

		TOKEN_ERROR,
		TOKEN_EOF
	};

	TokenType type;
	std::string_view lexeme;
	size_t line;
};

class Scanner {

	bool isAtEnd() const;
	char advance();
	char peek() const;
	char peekNext() const;
	bool match(char expected);
	Token makeToken(Token::TokenType type) const;
	Token errorToken(std::string_view message) const;
	void skipWhitespace();
	Token::TokenType identifierType();
	Token identifier();
	Token number();
	Token string();

  public:
	Scanner(std::string_view source);
	Token scanToken();

  private:
	std::string_view source;
	// iterator for start and current character
	std::string_view::iterator start;
	std::string_view::iterator current;
	size_t line = 1;

	// set of keywords
	static const std::unordered_map<std::string_view, Token::TokenType>
	    keywords;
};
} // namespace lox
