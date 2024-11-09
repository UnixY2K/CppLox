#pragma once
#include <cpplox/chunk.hpp>
#include <cpplox/value.hpp>

#include <cstddef>
#include <string_view>

namespace lox {
enum class InterpretResult { OK, COMPILE_ERROR, RUNTIME_ERROR };

class VM {
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
	VM(bool debug_trace = false) : debug_trace_instruction(debug_trace) {}
	InterpretResult interpret(Chunk &chunk);
	InterpretResult interpret(std::string_view source);
	bool debug_trace_instruction;

  private:
	Chunk chunk;
	std::vector<Value> stack;
	std::span<const std::byte>::iterator ip;
	std::span<const std::byte>::iterator ip_end;
};
} // namespace lox
