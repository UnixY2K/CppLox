#pragma once
#include <cpplox/chunk.hpp>
#include <cpplox/scanner.hpp>

#include <array>
#include <cstddef>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace lox {

struct Parser;
class Compiler {

	enum class Precedence {
		PREC_NONE,
		PREC_ASSIGNMENT, // =
		PREC_OR,         // or
		PREC_AND,        // and
		PREC_EQUALITY,   // == !=
		PREC_COMPARISON, // < > <= >=
		PREC_TERM,       // + -
		PREC_FACTOR,     // * /
		PREC_UNARY,      // ! -
		PREC_CALL,       // . ()
		PREC_PRIMARY
	};

	// virtual class for parsing rules
	using ParseFn = void (Compiler::*)(bool canAssign);
	struct ParseRule {
		ParseFn prefix = nullptr;
		ParseFn infix = nullptr;
		Precedence precedence = Precedence::PREC_NONE;
	};

	// list of parsing rules
	using parseRuleArray =
	    std::array<ParseRule,
	               static_cast<size_t>(Token::TokenType::TOKEN_EOF) + 1>;

	static parseRuleArray &getRules();
	void initializeRules();

	struct Parser {
		Token current;
		Token previous;
		bool hadError = false;
		bool panicMode = false;
	};

	struct Local {
		Token name;
		size_t depth;
		bool initialized = false;
	};

	struct CompilerScope {
		size_t depth = 0;
		std::vector<Local> locals;
	};

	void errorAt(Token token, std::string_view message);
	void error(std::string_view message);
	void errorAtCurrent(std::string_view message);
	void advance();
	void consume(Token::TokenType type, std::string_view message);
	bool check(Token::TokenType type);
	bool match(Token::TokenType type);
	void emmitByte(std::byte byte);
	void emmitBytes(std::span<std::byte> bytes);
	void emmitReturn();
	std::vector<std::byte> makeConstant(Value value);
	void emmitConstant(Value value);
	void endCompiler();
	void beginScope();
	void endScope();
	void binary(bool canAssign);
	void literal(bool canAssign);
	void grouping(bool canAssign);
	void number(bool canAssign);
	void namedVariable(Token name, bool canAssign);
	void variable(bool canAssign);
	void string(bool canAssign);
	void unary(bool canAssign);
	void parsePrecedence(Precedence precedence);
	size_t identifierConstant(Token name);
	bool identifiersEqual(const Token &a, const Token &b);
	int resolveLocal(const Token& name);
	void addLocal(Token name);
	void declareVariable();
	size_t parseVariable(std::string_view errorMessage);
	void markInitialized();
	void defineVariable(size_t global);
	ParseRule &getRule(Token::TokenType type);
	void expression();
	void block();
	void varDeclaration();
	void expressionStatement();
	void printStatement();
	void synchronize();
	void declaration();
	void statement();

  public:
	Compiler();
	auto compile(std::string_view source) -> std::expected<Chunk, std::string>;

	bool debug_print_code = false;

  private:
	Parser parser;
	Scanner scanner;
	Chunk chunk;
	CompilerScope scope;
};

} // namespace lox
