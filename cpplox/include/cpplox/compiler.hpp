#pragma once
#include <cpplox/chunk.hpp>
#include <cpplox/compiler.hpp>
#include <cpplox/obj.hpp>
#include <cpplox/scanner.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace lox {
enum class FunctionType { TYPE_FUNCTION, TYPE_SCRIPT };
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
		int depth;
		bool initialized = false;
	};

	struct CompilerScope {
		int depth = 0;
		std::vector<Local> locals;
	};

	Chunk &currentChunk();
	void errorAt(Token token, std::string_view message);
	void error(std::string_view message);
	void errorAtCurrent(std::string_view message);
	void advance();
	void consume(Token::TokenType type, std::string_view message);
	bool check(Token::TokenType type);
	bool match(Token::TokenType type);
	void emmitByte(std::byte byte);
	void emmitBytes(std::span<std::byte> bytes);
	void emmitLoop(size_t loopStart);
	size_t emmitJump(OpCode opCode);
	void emmitReturn();
	std::vector<std::byte> makeConstant(const Value &value);
	void emmitConstant(const Value &value);
	void patchJump(size_t offset);
	ObjFunction &endCompiler();
	void beginScope();
	void endScope();
	void binary(bool canAssign);
	void call(bool canAssign);
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
	int resolveLocal(const Token &name);
	void addLocal(Token name);
	void declareVariable();
	size_t parseVariable(std::string_view errorMessage);
	void markInitialized();
	void defineVariable(size_t global);
	uint8_t argumentList();
	void and_(bool canAssign);
	void or_(bool canAssign);
	ParseRule &getRule(Token::TokenType type);
	void expression();
	void block();
	void functionDefinition(FunctionType type);
	void funDeclaration();
	void varDeclaration();
	void expressionStatement();
	void forStatement();
	void ifStatement();
	void printStatement();
	void returnStatement();
	void whileStatement();
	void synchronize();
	void declaration();
	void statement();

  public:
	Compiler(Compiler *enclosing = nullptr, FunctionType type = FunctionType::TYPE_SCRIPT);
	auto compile(std::string_view source,
	             FunctionType type = FunctionType::TYPE_SCRIPT)
	    -> std::expected<std::reference_wrapper<ObjFunction>, std::string>;

	bool debug_print_code = false;

  private:
	Compiler *enclosing = nullptr;
	Parser parser;
	Scanner scanner;
	CompilerScope scope;
	ObjFunction function;
	FunctionType type = FunctionType::TYPE_FUNCTION;
};

} // namespace lox
