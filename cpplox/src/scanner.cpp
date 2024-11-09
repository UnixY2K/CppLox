#include <cctype>
#include <cpplox/scanner.hpp>
#include <string_view>
#include <unordered_map>

namespace lox {

Scanner::Scanner(std::string_view source) {
	this->source = source;
	this->start = source.begin();
	this->current = source.begin();
	this->line = 1;
}

Token Scanner::scanToken() {
	skipWhitespace();
	start = current;
	if (isAtEnd()) {
		return makeToken(Token::TokenType::TOKEN_EOF);
	}
	char c = advance();

	if (std::isalpha(c) || c == '_') {
		return identifier();
	}
	if (std::isdigit(c)) {
		return number();
	}

	switch (c) {
	case '(':
		return makeToken(Token::TokenType::TOKEN_LEFT_PAREN);
	case ')':
		return makeToken(Token::TokenType::TOKEN_RIGHT_PAREN);
	case '{':
		return makeToken(Token::TokenType::TOKEN_LEFT_BRACE);
	case '}':
		return makeToken(Token::TokenType::TOKEN_RIGHT_BRACE);
	case ';':
		return makeToken(Token::TokenType::TOKEN_SEMICOLON);
	case ',':
		return makeToken(Token::TokenType::TOKEN_COMMA);
	case '.':
		return makeToken(Token::TokenType::TOKEN_DOT);
	case '-':
		return makeToken(Token::TokenType::TOKEN_MINUS);
	case '+':
		return makeToken(Token::TokenType::TOKEN_PLUS);
	case '/':
		return makeToken(Token::TokenType::TOKEN_SLASH);
	case '*':
		return makeToken(Token::TokenType::TOKEN_STAR);
	case '!':
		return makeToken(match('=') ? Token::TokenType::TOKEN_BANG_EQUAL
		                            : Token::TokenType::TOKEN_BANG);
	case '=':
		return makeToken(match('=') ? Token::TokenType::TOKEN_EQUAL_EQUAL
		                            : Token::TokenType::TOKEN_EQUAL);
	case '<':
		return makeToken(match('=') ? Token::TokenType::TOKEN_LESS_EQUAL
		                            : Token::TokenType::TOKEN_LESS);
	case '>':
		return makeToken(match('=') ? Token::TokenType::TOKEN_GREATER_EQUAL
		                            : Token::TokenType::TOKEN_GREATER);
	case '?':
		return makeToken(Token::TokenType::TOKEN_QUESTION);
	case ':':
		return makeToken(Token::TokenType::TOKEN_COLON);
	case '"':
		return string();
	}

	return errorToken("Unexpected character.");
}

bool Scanner::isAtEnd() const { return current == source.end(); }
Token Scanner::makeToken(Token::TokenType type) const {
	Token token;
	token.type = type;
	token.lexeme = std::string_view{start, current};
	token.line = line;
	return token;
}
char Scanner::advance() { return *current++; }
char Scanner::peek() const { return isAtEnd() ? '\0' : *current; }
char Scanner::peekNext() const {
	if (!isAtEnd()) {
		return *(current + 1);
	}
	return '\0';
}
bool Scanner::match(char expected) {
	if (!isAtEnd() && *current == expected) {
		++current;
		return true;
	}
	return false;
}

Token Scanner::errorToken(std::string_view message) const {
	Token token;
	token.type = Token::TokenType::TOKEN_ERROR;
	token.lexeme = message;
	token.line = line;
	return token;
}

void Scanner::skipWhitespace() {
	for (;;) {
		char c = peek();
		switch (c) {
		case '\n':
			++line;
		case ' ':
		case '\r':
		case '\t':
			advance();
			break;
		case '/':
			// single line comment, read until end of line or EOF
			if (peekNext() == '/') {
				while (peek() != '\n' && !isAtEnd()) {
					advance();
				}
			}
			// multiline comments
			else if (peekNext() == '*') {
				while (!(peek() == '*' && peekNext() == '/') && !isAtEnd()) {
					if (peek() == '\n') {
						++line;
					}
					advance();
				}
				// advance past the closing */
				if (!isAtEnd()) {
					advance();
					advance();
				}
			} else {
				return;
			}
			break;
		default:
			return;
		}
	}
}

Token::TokenType Scanner::identifierType() {
	auto iter = keywords.find(std::string_view{start, current});
	if (iter != keywords.end()) {
		return iter->second;
	}
	return Token::TokenType::TOKEN_IDENTIFIER;
}

Token Scanner::identifier() {
	while (std::isalnum(peek()) || peek() == '_') {
		advance();
	}

	return makeToken(identifierType());
}

Token Scanner::number() {
	while (std::isdigit(peek())) {
		advance();
	}

	if (peek() == '.' && std::isdigit(peekNext())) {
		advance(); // consume the '.'
		while (std::isdigit(peek())) {
			advance();
		}
	}

	return makeToken(Token::TokenType::TOKEN_NUMBER);
}

Token Scanner::string() {
	while (peek() != '"' && !isAtEnd()) {
		if (peek() == '\n') {
			++line;
		}
		advance();
	}

	if (isAtEnd()) {
		return errorToken("Unterminated string.");
	}

	advance(); // closing quote
	return makeToken(Token::TokenType::TOKEN_STRING);
}

const std::unordered_map<std::string_view, Token::TokenType> Scanner::keywords{
    {"and", Token::TokenType::TOKEN_AND},
    {"class", Token::TokenType::TOKEN_CLASS},
    {"else", Token::TokenType::TOKEN_ELSE},
    {"false", Token::TokenType::TOKEN_FALSE},
    {"for", Token::TokenType::TOKEN_FOR},
    {"fun", Token::TokenType::TOKEN_FUN},
    {"if", Token::TokenType::TOKEN_IF},
    {"nil", Token::TokenType::TOKEN_NIL},
    {"or", Token::TokenType::TOKEN_OR},
    {"print", Token::TokenType::TOKEN_PRINT},
    {"return", Token::TokenType::TOKEN_RETURN},
    {"super", Token::TokenType::TOKEN_SUPER},
    {"this", Token::TokenType::TOKEN_THIS},
    {"true", Token::TokenType::TOKEN_TRUE},
    {"var", Token::TokenType::TOKEN_VAR},
    {"while", Token::TokenType::TOKEN_WHILE},
};

} // namespace lox
