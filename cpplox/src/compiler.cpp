#include <cpplox/chunk.hpp>
#include <cpplox/compiler.hpp>
#include <cpplox/debug.hpp>
#include <cpplox/obj.hpp>
#include <cpplox/scanner.hpp>
#include <cpplox/value.hpp>

#include <charconv>
#include <cstddef>
#include <format>
#include <iostream>
#include <vector>

#if defined(__APPLE__) && defined(__clang__)
#include <cstdio>
#endif

namespace lox {

Compiler::Compiler() {
	static bool rulesInitialized = false;
	if (!rulesInitialized) {
		rulesInitialized = true;
		initializeRules();
	}
}

Compiler::parseRuleArray &Compiler::getRules() {
	static parseRuleArray rules;
	return rules;
}

void Compiler::initializeRules() {
	auto &rules = getRules();

	struct Rule : ParseRule {
		void (Compiler::*prefix)();
		void (Compiler::*infix)(Compiler &);
		Precedence precedence;
	};

	// Single-character tokens.

	// TOKEN_LEFT_PAREN
	rules[static_cast<size_t>(Token::TokenType::TOKEN_LEFT_PAREN)] = {
	    &Compiler::grouping, nullptr, Precedence::PREC_NONE};
	// TOKEN_RIGHT_PAREN
	// TOKEN_LEFT_BRACE
	// TOKEN_RIGHT_BRACE
	// TOKEN_COMMA
	// TOKEN_DOT
	// TOKEN_MINUS
	rules[static_cast<size_t>(Token::TokenType::TOKEN_MINUS)] = {
	    &Compiler::unary, &Compiler::binary, Precedence::PREC_TERM};
	// TOKEN_PLUS
	rules[static_cast<size_t>(Token::TokenType::TOKEN_PLUS)] = {
	    nullptr, &Compiler::binary, Precedence::PREC_TERM};
	// TOKEN_SEMICOLON
	// TOKEN_SLASH
	rules[static_cast<size_t>(Token::TokenType::TOKEN_SLASH)] = {
	    nullptr, &Compiler::binary, Precedence::PREC_FACTOR};
	// TOKEN_STAR
	rules[static_cast<size_t>(Token::TokenType::TOKEN_STAR)] = {
	    nullptr, &Compiler::binary, Precedence::PREC_FACTOR};

	// One or two character tokens.

	// TOKEN_BANG
	rules[static_cast<size_t>(Token::TokenType::TOKEN_BANG)] = {
	    &Compiler::unary, nullptr, Precedence::PREC_NONE};
	// TOKEN_BANG_EQUAL
	rules[static_cast<size_t>(Token::TokenType::TOKEN_BANG_EQUAL)] = {
	    nullptr, &Compiler::binary, Precedence::PREC_EQUALITY};
	// TOKEN_EQUAL
	// TOKEN_EQUAL_EQUAL
	rules[static_cast<size_t>(Token::TokenType::TOKEN_EQUAL_EQUAL)] = {
	    nullptr, &Compiler::binary, Precedence::PREC_EQUALITY};
	// TOKEN_GREATER
	rules[static_cast<size_t>(Token::TokenType::TOKEN_GREATER)] = {
	    nullptr, &Compiler::binary, Precedence::PREC_COMPARISON};
	// TOKEN_GREATER_EQUAL
	rules[static_cast<size_t>(Token::TokenType::TOKEN_GREATER_EQUAL)] = {
	    nullptr, &Compiler::binary, Precedence::PREC_COMPARISON};
	// TOKEN_LESS
	rules[static_cast<size_t>(Token::TokenType::TOKEN_LESS)] = {
	    nullptr, &Compiler::binary, Precedence::PREC_COMPARISON};
	// TOKEN_LESS_EQUAL
	rules[static_cast<size_t>(Token::TokenType::TOKEN_LESS_EQUAL)] = {
	    nullptr, &Compiler::binary, Precedence::PREC_COMPARISON};

	// misc '?', ':'

	// TOKEN_QUESTION
	// TOKEN_COLON

	// Literals.

	// TOKEN_IDENTIFIER
	rules[static_cast<size_t>(Token::TokenType::TOKEN_IDENTIFIER)] = {
	    &Compiler::variable, nullptr, Precedence::PREC_NONE};
	// TOKEN_STRING
	rules[static_cast<size_t>(Token::TokenType::TOKEN_STRING)] = {
	    &Compiler::string, nullptr, Precedence::PREC_NONE};
	// TOKEN_NUMBER
	rules[static_cast<size_t>(Token::TokenType::TOKEN_NUMBER)] = {
	    &Compiler::number, nullptr, Precedence::PREC_NONE};

	// Literals.

	// TOKEN_AND
	// TOKEN_CLASS
	// TOKEN_ELSE
	// TOKEN_FALSE
	rules[static_cast<size_t>(Token::TokenType::TOKEN_FALSE)] = {
	    &Compiler::literal, nullptr, Precedence::PREC_NONE};
	// TOKEN_FOR
	// TOKEN_FUN
	// TOKEN_IF
	// TOKEN_NIL
	rules[static_cast<size_t>(Token::TokenType::TOKEN_NIL)] = {
	    &Compiler::literal, nullptr, Precedence::PREC_NONE};
	// TOKEN_OR
	// TOKEN_PRINT
	// TOKEN_RETURN
	// TOKEN_SUPER
	// TOKEN_THIS
	// TOKEN_TRUE
	rules[static_cast<size_t>(Token::TokenType::TOKEN_TRUE)] = {
	    &Compiler::literal, nullptr, Precedence::PREC_NONE};
	// TOKEN_VAR
	// TOKEN_WHILE
	// TOKEN_CONTINUE
	// TOKEN_BREAK

	// TOKEN_ERROR
	// TOKEN_EOF
}

void Compiler::errorAt(Token token, std::string_view message) {
	if (!parser.panicMode) {
		parser.panicMode = true;
		std::cerr << std::format("[line {}] Error", token.line);

		if (token.type == Token::TokenType::TOKEN_EOF) {
			std::cerr << " at end";
		} else if (token.type == Token::TokenType::TOKEN_ERROR) {
			// Nothing.
		} else {
			std::cerr << std::format(" at '{}'", token.lexeme);
		}

		std::cerr << std::format(": {}\n", message);
		parser.hadError = true;
	}
}

void Compiler::error(std::string_view message) {
	errorAt(parser.previous, message);
}

void Compiler::errorAtCurrent(std::string_view message) {
	errorAt(parser.current, message);
}

void Compiler::advance() {
	parser.previous = parser.current;
	while (true) {
		parser.current = scanner.scanToken();
		if (!check(Token::TokenType::TOKEN_ERROR)) {
			break;
		}
		errorAtCurrent(parser.current.lexeme);
	}
}

void Compiler::consume(Token::TokenType type, std::string_view message) {
	if (check(type)) {
		advance();
		return;
	}
	errorAtCurrent(message);
}

bool Compiler::check(Token::TokenType type) {
	return parser.current.type == type;
}

bool Compiler::match(Token::TokenType type) {
	if (!check(type)) {
		return false;
	}
	advance();
	return true;
}

void Compiler::emmitByte(std::byte byte) {
	chunk.write(byte, parser.previous.line);
}

void Compiler::emmitBytes(std::span<std::byte> bytes) {
	for (auto byte : bytes) {
		emmitByte(byte);
	}
}

void Compiler::emmitReturn() {
	emmitByte(static_cast<std::byte>(OpCode::OP_RETURN));
}

std::vector<std::byte> Compiler::makeConstant(Value value) {
	auto index = chunk.addConstant(value);
	// depending on the index size we use OP_CONSTANT or OP_CONSTANT_LONG
	auto opcode =
	    index > UINT8_MAX ? OpCode::OP_CONSTANT_LONG : OpCode::OP_CONSTANT;
	// check that we did not exeed the 2byte limit
	if (index > UINT16_MAX) {
		error("Too many constants in one chunk");
		return {static_cast<std::byte>(OpCode::OP_RETURN)};
	}
	// then push the opcode along with the byte(s) of the constant
	std::vector<std::byte> bytes;
	bytes.push_back(static_cast<std::byte>(opcode));
	// for OP_CONSTANT_LONG we need to push 2 bytes and for OP_CONSTANT 1 byte
	if (opcode == OpCode::OP_CONSTANT_LONG) {
		bytes.push_back(static_cast<std::byte>(index >> 8));
	}
	bytes.push_back(static_cast<std::byte>(index & 0xff));
	return bytes;
}

void Compiler::emmitConstant(Value value) {
	auto bytes = makeConstant(value);
	emmitBytes(bytes);
}

void Compiler::endCompiler() {
	if (debug_print_code && !parser.hadError) {
		debug::ChunkDisassembly(chunk, "code");
	}
	emmitReturn();
}

void Compiler::literal(bool canAssign) {
	switch (parser.previous.type) {
	case Token::TokenType::TOKEN_FALSE:
		emmitByte(static_cast<std::byte>(OpCode::OP_FALSE));
		break;
	case Token::TokenType::TOKEN_NIL:
		emmitByte(static_cast<std::byte>(OpCode::OP_NIL));
		break;
	case Token::TokenType::TOKEN_TRUE:
		emmitByte(static_cast<std::byte>(OpCode::OP_TRUE));
		break;
	default:
		return; // unreachable
	}
}

void Compiler::binary(bool canAssign) {
	Token::TokenType operatorType = parser.previous.type;
	ParseRule &rule = getRule(operatorType);
	parsePrecedence(
	    static_cast<Precedence>(static_cast<size_t>(rule.precedence) + 1));
	switch (operatorType) {
	case Token::TokenType::TOKEN_BANG_EQUAL:
		emmitByte(static_cast<std::byte>(OpCode::OP_NOT_EQUAL));
		break;
	case Token::TokenType::TOKEN_EQUAL_EQUAL:
		emmitByte(static_cast<std::byte>(OpCode::OP_EQUAL));
		break;
	case Token::TokenType::TOKEN_GREATER:
		emmitByte(static_cast<std::byte>(OpCode::OP_GREATER));
		break;
	case Token::TokenType::TOKEN_GREATER_EQUAL:
		emmitByte(static_cast<std::byte>(OpCode::OP_GREATER_EQUAL));
		break;
	case Token::TokenType::TOKEN_LESS:
		emmitByte(static_cast<std::byte>(OpCode::OP_LESS));
		break;
	case Token::TokenType::TOKEN_LESS_EQUAL:
		emmitByte(static_cast<std::byte>(OpCode::OP_LESS_EQUAL));
		break;
	case Token::TokenType::TOKEN_PLUS:
		emmitByte(static_cast<std::byte>(OpCode::OP_ADD));
		break;
	case Token::TokenType::TOKEN_MINUS:
		emmitByte(static_cast<std::byte>(OpCode::OP_SUBTRACT));
		break;
	case Token::TokenType::TOKEN_STAR:
		emmitByte(static_cast<std::byte>(OpCode::OP_MULTIPLY));
		break;
	case Token::TokenType::TOKEN_SLASH:
		emmitByte(static_cast<std::byte>(OpCode::OP_DIVIDE));
		break;
	default:
		return; // unreachable
	}
}

void Compiler::grouping(bool canAssign) {
	expression();
	consume(Token::TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after expression");
}

void Compiler::number(bool canAssign) {
	double value;
// apparently for apple clang 16, there is not support for std::from_chars
// for float types, so we use sscanf as a fallback instead
#if defined(__APPLE__) && defined(__clang__)
	// use sscanf instead
	int lenght = parser.previous.lexeme.length();
	std::string format = std::format("%{}lf%n", lenght);
	int read;
	bool result = sscanf(parser.previous.lexeme.data(), format.c_str(), &value,
	                     &read) == 1;
	if (!result || read != lenght) {
		error(std::format("Invalid number '{}'", parser.previous.lexeme));
		return;
	}
#else
	auto result = std::from_chars(
	    parser.previous.lexeme.data(),
	    parser.previous.lexeme.data() + parser.previous.lexeme.size(), value);
	if (result.ec == std::errc::invalid_argument) {
		error(std::format("Invalid number '{}'", parser.previous.lexeme));
		return;
	}
#endif
	emmitConstant(value);
}

void Compiler::namedVariable(Token name, bool canAssign) {
	size_t arg = identifierConstant(name);
	std::vector<std::byte> bytes;
	// depending on the size we use OP_GET_GLOBAL or OP_GET_GLOBAL_LONG

	if (arg > UINT16_MAX) {
		error("Too many variables in one chunk");
		return;
	}

	OpCode opcode;
	if (canAssign && match(Token::TokenType::TOKEN_EQUAL)) {
		expression();
		opcode = arg > UINT8_MAX ? OpCode::OP_SET_GLOBAL_LONG
		                         : OpCode::OP_SET_GLOBAL;
	} else {
		opcode = arg > UINT8_MAX ? OpCode::OP_GET_GLOBAL_LONG
		                         : OpCode::OP_GET_GLOBAL;
	}

	bytes.push_back(static_cast<std::byte>(opcode));
	if (opcode == OpCode::OP_GET_GLOBAL_LONG) {
		bytes.push_back(static_cast<std::byte>(arg >> 8));
	}
	bytes.push_back(static_cast<std::byte>(arg & 0xff));
	emmitBytes(bytes);
}

void Compiler::variable(bool canAssign) {
	namedVariable(parser.previous, canAssign);
}

void Compiler::string(bool canAssign) {
	// remove the quotes from the string
	auto str =
	    parser.previous.lexeme.substr(1, parser.previous.lexeme.size() - 2);
	emmitConstant(stringToValue(str));
}

void Compiler::unary(bool canAssign) {
	Token::TokenType operatorType = parser.previous.type;
	// compile the operand
	parsePrecedence(Precedence::PREC_UNARY);
	// emit the operator instruction
	switch (operatorType) {
	case Token::TokenType::TOKEN_MINUS:
		emmitByte(static_cast<std::byte>(OpCode::OP_NEGATE));
		break;
	case Token::TokenType::TOKEN_BANG:
		emmitByte(static_cast<std::byte>(OpCode::OP_NOT));
		break;
	default:
		return; // unreachable
	}
}

void Compiler::parsePrecedence(Precedence precedence) {
	advance();
	auto prefixRule = getRule(parser.previous.type).prefix;
	if (prefixRule == nullptr) {
		error("Expect expression");
		return;
	}

	bool canAssign = precedence <= Precedence::PREC_ASSIGNMENT;
	(this->*prefixRule)(canAssign);

	while (precedence <= getRule(parser.current.type).precedence) {
		advance();
		auto infixRule = getRule(parser.previous.type).infix;
		(this->*infixRule)(canAssign);
	}

	if (canAssign && match(Token::TokenType::TOKEN_EQUAL)) {
		error("Invalid assignment target");
	}
}

size_t Compiler::identifierConstant(Token name) {
	std::vector<std::byte> value = makeConstant(stringToValue(name.lexeme));
	// we reconstruct the bytes to an size_t
	// ignore the first bythe which is the opcode
	value.erase(value.begin());
	// then check that we did not exeed the size_t limit
	if (value.size() > sizeof(size_t)) {
		error("Too many constants in one chunk");
		return 0;
	}
	size_t index = 0;
	for (auto byte : value) {
		index = (index << 8) | static_cast<size_t>(byte);
	}
	return index;
}

size_t Compiler::parseVariable(std::string_view errorMessage) {
	consume(Token::TokenType::TOKEN_IDENTIFIER, errorMessage);
	return identifierConstant(parser.previous);
}

void Compiler::defineVariable(size_t global) {
	// depending on the size we use OP_DEFINE_GLOBAL or OP_DEFINE_GLOBAL_LONG
	auto opcode = global > UINT8_MAX ? OpCode::OP_DEFINE_GLOBAL_LONG
	                                 : OpCode::OP_DEFINE_GLOBAL;
	if (global > UINT16_MAX) {
		error("Too many variables in one chunk");
		return;
	}
	std::vector<std::byte> bytes;
	bytes.push_back(static_cast<std::byte>(opcode));
	if (opcode == OpCode::OP_DEFINE_GLOBAL_LONG) {
		bytes.push_back(static_cast<std::byte>(global >> 8));
	}
	bytes.push_back(static_cast<std::byte>(global & 0xff));
	emmitBytes(bytes);
}

Compiler::ParseRule &Compiler::getRule(Token::TokenType type) {
	return getRules()[static_cast<size_t>(type)];
}

void Compiler::expression() { parsePrecedence(Precedence::PREC_ASSIGNMENT); }

void Compiler::varDeclaration() {
	size_t global = parseVariable("Expect variable name");
	if (match(Token::TokenType::TOKEN_EQUAL)) {
		expression();
	} else {
		emmitByte(static_cast<std::byte>(OpCode::OP_NIL));
	}
	consume(Token::TokenType::TOKEN_SEMICOLON,
	        "Expect ';' after variable declaration");
	defineVariable(global);
}

void Compiler::expressionStatement() {
	expression();
	consume(Token::TokenType::TOKEN_SEMICOLON, "Expect ';' after expression");
	emmitByte(static_cast<std::byte>(OpCode::OP_POP));
}

void Compiler::printStatement() {
	expression();
	consume(Token::TokenType::TOKEN_SEMICOLON, "Expect ';' after value");
	emmitByte(static_cast<std::byte>(OpCode::OP_PRINT));
}

void Compiler::synchronize() {
	parser.panicMode = false;

	while (!check(Token::TokenType::TOKEN_EOF)) {
		if (parser.previous.type == Token::TokenType::TOKEN_SEMICOLON) {
			return;
		}
		switch (parser.current.type) {
		case Token::TokenType::TOKEN_CLASS:
		case Token::TokenType::TOKEN_FUN:
		case Token::TokenType::TOKEN_VAR:
		case Token::TokenType::TOKEN_FOR:
		case Token::TokenType::TOKEN_IF:
		case Token::TokenType::TOKEN_WHILE:
		case Token::TokenType::TOKEN_PRINT:
		case Token::TokenType::TOKEN_RETURN:
			return;

		default:; // Do nothing.
		}
		advance();
	}
}

void Compiler::declaration() {
	if (match(Token::TokenType::TOKEN_VAR)) {
		varDeclaration();
	} else {
		statement();
	}

	if (parser.panicMode) {
		synchronize();
	}
}

void Compiler::statement() {
	if (match(Token::TokenType::TOKEN_PRINT)) {
		printStatement();
	} else {
		expressionStatement();
	}
}

auto Compiler::compile(std::string_view source)
    -> std::expected<Chunk, std::string> {
	// reset the compiler state
	parser = Parser{};
	chunk = Chunk{};
	scanner = Scanner{source};
	advance();

	while (!match(Token::TokenType::TOKEN_EOF)) {
		declaration();
	}

	endCompiler();
	return parser.hadError ? std::unexpected("Compilation error")
	                       : std::expected<Chunk, std::string>{chunk};
}

} // namespace lox
