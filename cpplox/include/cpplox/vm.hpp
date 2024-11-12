#pragma once
#include <cpplox/chunk.hpp>
#include <cpplox/value.hpp>

#include <cstddef>
#include <string_view>
#include <unordered_map>

namespace lox {
enum class InterpretResult { OK, COMPILE_ERROR, RUNTIME_ERROR };

class VM {
	void runtimeError(std::string_view message);

	// gets the byte and increments the instruction pointer
	std::byte readByte(std::span<const std::byte>::iterator &ip);
	// increments the instruction pointer and then returns the current byte
	std::byte nextByte(std::span<const std::byte>::iterator &ip);
	// returns the current byte
	std::byte peekByte(std::span<const std::byte>::iterator &ip);

	void binaryOp(std::span<const std::byte>::iterator &ip);

	Value readConstant(std::span<const std::byte>::iterator &ip);
	InterpretResult run();

  public:
	InterpretResult interpret(const Chunk &chunk);
	InterpretResult interpret(std::string_view source);
	bool debug_trace_instruction = false;
	bool debug_stack = false;

  private:
	bool had_error = false;
	Chunk chunk;
	std::vector<Value> stack;
	std::unordered_map<std::string, Value> globals;
	std::span<const std::byte>::iterator ip;
	std::span<const std::byte>::iterator ip_end;
};
} // namespace lox
