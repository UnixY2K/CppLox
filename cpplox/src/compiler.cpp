#include <cpplox/chunk.hpp>
#include <cpplox/compiler.hpp>
#include <cpplox/debug.hpp>
#include <cpplox/obj.hpp>
#include <cpplox/scanner.hpp>
#include <cpplox/value.hpp>

#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <ranges>
#include <vector>

#if defined(__APPLE__) && defined(__clang__)
#include <cstdio>
#else
#include <charconv>
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
	rules[static_cast<size_t>(Token::TokenType::TOKEN_AND)] = {
	    nullptr, &Compiler::and_, Precedence::PREC_AND};
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
	rules[static_cast<size_t>(Token::TokenType::TOKEN_OR)] = {
	    nullptr, &Compiler::or_, Precedence::PREC_OR};
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

Chunk &Compiler::currentChunk() { return *function.chunk; }

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
	currentChunk().write(byte, parser.previous.line);
}

void Compiler::emmitBytes(std::span<std::byte> bytes) {
	for (auto byte : bytes) {
		emmitByte(byte);
	}
}

void Compiler::emmitLoop(size_t loopStart) {
	emmitByte(static_cast<std::byte>(OpCode::OP_LOOP));

	Chunk &chunk = currentChunk();

	size_t offset = chunk.code().size() - loopStart + 2;
	if (offset > UINT16_MAX) {
		error("Loop body too large");
	}

	chunk.write(static_cast<std::byte>(offset >> 8), parser.previous.line);
	chunk.write(static_cast<std::byte>(offset & 0xff), parser.previous.line);
}

size_t Compiler::emmitJump(OpCode instruction) {
	emmitByte(static_cast<std::byte>(instruction));
	emmitByte(static_cast<std::byte>(0xff));
	emmitByte(static_cast<std::byte>(0xff));
	return currentChunk().code().size() - 2;
}

void Compiler::emmitReturn() {
	emmitByte(static_cast<std::byte>(OpCode::OP_RETURN));
}

std::vector<std::byte> Compiler::makeConstant(Value value) {
	auto index = currentChunk().addConstant(value);
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

void Compiler::patchJump(size_t offset) {
	Chunk &chunk = currentChunk();
	// -2 to adjust for the bytecode for the jump offset itself
	size_t jump = chunk.code().size() - offset - 2;
	if (jump > UINT16_MAX) {
		error("Too much code to jump over");
	}
	chunk.patchByte(offset, static_cast<std::byte>(jump >> 8));
	chunk.patchByte(offset + 1, static_cast<std::byte>(jump & 0xff));
}

ObjFunction &Compiler::endCompiler() {
	ObjFunction &function = this->function;
	if (debug_print_code && !parser.hadError) {
		debug::ChunkDisassembly(
		    currentChunk(), function.name.empty() ? "<script>" : function.name);
	}
	emmitReturn();
	return function;
}

void Compiler::beginScope() { scope.depth++; }

void Compiler::endScope() {
	scope.depth--;

	while (!scope.locals.empty() && scope.locals.back().depth > scope.depth) {
		emmitByte(static_cast<std::byte>(OpCode::OP_POP));
		scope.locals.pop_back();
	}
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
	std::vector<std::byte> bytes;
	OpCode getOp, setOp;
	int arg = resolveLocal(name);
	// depending on the size we use OP_GET_GLOBAL or OP_GET_GLOBAL_LONG
	if (arg > UINT16_MAX) {
		error("Too many variables in one chunk");
		return;
	}

	if (arg != -1) {
		getOp =
		    arg > UINT8_MAX ? OpCode::OP_GET_LOCAL_LONG : OpCode::OP_GET_LOCAL;
		setOp =
		    arg > UINT8_MAX ? OpCode::OP_SET_LOCAL_LONG : OpCode::OP_SET_LOCAL;
	} else {
		arg = identifierConstant(name);
		getOp = arg > UINT8_MAX ? OpCode::OP_GET_GLOBAL_LONG
		                        : OpCode::OP_GET_GLOBAL;
		setOp = arg > UINT8_MAX ? OpCode::OP_SET_GLOBAL_LONG
		                        : OpCode::OP_SET_GLOBAL;
	}

	OpCode opcode;
	if (canAssign && match(Token::TokenType::TOKEN_EQUAL)) {
		expression();
		opcode = setOp;
	} else {
		opcode = getOp;
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

bool Compiler::identifiersEqual(const Token &a, const Token &b) {
	return a.lexeme == b.lexeme;
}

int Compiler::resolveLocal(const Token &token) {

	size_t i = 0;
	for (const auto local : scope.locals | std::views::reverse) {
		if (identifiersEqual(local.name, token)) {
			if (!local.initialized) {
				error("Can't read local variable in its own initializer.");
			}
			return scope.locals.size() - ++i;
		}
		i++;
	}
	return -1;
}

void Compiler::addLocal(Token name) {
	if (scope.locals.size() == UINT16_MAX) {
		error("Too many local variables in function");
		return;
	}

	scope.locals.emplace_back(Local{.name = name, .depth = scope.depth});
}

void Compiler::declareVariable() {
	if (scope.depth == 0) {
		return;
	}
	Token &name = parser.previous;
	for (const auto &local : scope.locals) {
		if (local.depth != -1 && local.depth < scope.depth) {
			break;
		}
		if (identifiersEqual(name, local.name)) {
			error("Already a variable with this name in this scope.");
		}
	}
	addLocal(name);
}

size_t Compiler::parseVariable(std::string_view errorMessage) {
	consume(Token::TokenType::TOKEN_IDENTIFIER, errorMessage);
	declareVariable();
	if (scope.depth > 0) {
		return 0;
	}
	return identifierConstant(parser.previous);
}

void Compiler::markInitialized() { scope.locals.back().initialized = true; }

void Compiler::defineVariable(size_t global) {
	// check if we are in the global scope
	if (scope.depth > 0) {
		markInitialized();
		return;
	}

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

void Compiler::and_(bool canAssign) {
	size_t endJump = emmitJump(OpCode::OP_JUMP_IF_FALSE);

	emmitByte(static_cast<std::byte>(OpCode::OP_POP));
	parsePrecedence(Precedence::PREC_AND);

	patchJump(endJump);
}

void Compiler::or_(bool canAssign) {
	size_t elseJump = emmitJump(OpCode::OP_JUMP_IF_FALSE);
	size_t endJump = emmitJump(OpCode::OP_JUMP);

	patchJump(elseJump);
	emmitByte(static_cast<std::byte>(OpCode::OP_POP));

	parsePrecedence(Precedence::PREC_OR);
	patchJump(endJump);
}

Compiler::ParseRule &Compiler::getRule(Token::TokenType type) {
	return getRules()[static_cast<size_t>(type)];
}

void Compiler::expression() { parsePrecedence(Precedence::PREC_ASSIGNMENT); }

void Compiler::block() {
	while (!check(Token::TokenType::TOKEN_RIGHT_BRACE) &&
	       !check(Token::TokenType::TOKEN_EOF)) {
		declaration();
	}
	consume(Token::TokenType::TOKEN_RIGHT_BRACE, "Expect '}' after block");
}

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

void Compiler::forStatement() {
	Chunk &chunk = currentChunk();
	beginScope();
	consume(Token::TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'for'");
	if (match(Token::TokenType::TOKEN_SEMICOLON)) {
		// no initializer
	} else if (match(Token::TokenType::TOKEN_VAR)) {
		varDeclaration();
	} else {
		expressionStatement();
	}

	size_t loopStart = chunk.code().size();
	int exitJump = -1;

	if (!match(Token::TokenType::TOKEN_SEMICOLON)) {
		expression();
		consume(Token::TokenType::TOKEN_SEMICOLON,
		        "Expect ';' after loop condition");

		exitJump = emmitJump(OpCode::OP_JUMP_IF_FALSE);
		emmitByte(static_cast<std::byte>(OpCode::OP_POP));
	}

	if (!match(Token::TokenType::TOKEN_RIGHT_PAREN)) {
		size_t bodyJump = emmitJump(OpCode::OP_JUMP);
		size_t incrementStart = chunk.code().size();
		expression();
		emmitByte(static_cast<std::byte>(OpCode::OP_POP));
		consume(Token::TokenType::TOKEN_RIGHT_PAREN,
		        "Expect ')' after for clauses");

		emmitLoop(loopStart);
		loopStart = incrementStart;
		patchJump(bodyJump);
	}

	statement();
	emmitLoop(loopStart);

	if (exitJump != -1) {
		patchJump(exitJump);
		emmitByte(static_cast<std::byte>(OpCode::OP_POP));
	}

	endScope();
}

void Compiler::ifStatement() {
	consume(Token::TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'if'");
	expression();
	consume(Token::TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after condition");

	size_t thenJump = emmitJump(OpCode::OP_JUMP_IF_FALSE);
	emmitByte(static_cast<std::byte>(OpCode::OP_POP));
	statement();

	size_t elseJump = emmitJump(OpCode::OP_JUMP);

	patchJump(thenJump);
	emmitByte(static_cast<std::byte>(OpCode::OP_POP));

	if (match(Token::TokenType::TOKEN_ELSE)) {
		statement();
	}

	patchJump(elseJump);
}

void Compiler::printStatement() {
	expression();
	consume(Token::TokenType::TOKEN_SEMICOLON, "Expect ';' after value");
	emmitByte(static_cast<std::byte>(OpCode::OP_PRINT));
}

void Compiler::whileStatement() {
	Chunk &chunk = currentChunk();
	size_t loopStart = chunk.code().size();
	consume(Token::TokenType::TOKEN_LEFT_PAREN, "Expect '(' after 'while'");
	expression();
	consume(Token::TokenType::TOKEN_RIGHT_PAREN, "Expect ')' after condition");

	size_t exitJump = emmitJump(OpCode::OP_JUMP_IF_FALSE);
	emmitByte(static_cast<std::byte>(OpCode::OP_POP));
	statement();
	emmitLoop(loopStart);

	patchJump(exitJump);
	emmitByte(static_cast<std::byte>(OpCode::OP_POP));
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
	} else if (match(Token::TokenType::TOKEN_FOR)) {
		forStatement();
	} else if (match(Token::TokenType::TOKEN_IF)) {
		ifStatement();
	} else if (match(Token::TokenType::TOKEN_WHILE)) {
		whileStatement();
	} else if (match(Token::TokenType::TOKEN_LEFT_BRACE)) {
		beginScope();
		block();
		endScope();
	} else {
		expressionStatement();
	}
}

auto Compiler::compile(std::string_view source, FunctionType type)
    -> std::expected<std::reference_wrapper<ObjFunction>, std::string> {
	// reset the compiler state
	parser = Parser{};
	scanner = Scanner{source};
	scope = CompilerScope{};
	function = ObjFunction{};
	this->type = type;
	advance();

	while (!match(Token::TokenType::TOKEN_EOF)) {
		declaration();
	}

	ObjFunction &function = endCompiler();
	return parser.hadError ? std::unexpected("Compilation error")
	                       : std::expected<std::reference_wrapper<ObjFunction>, std::string>{function};
}

} // namespace lox
