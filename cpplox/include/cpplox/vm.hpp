#pragma once
#include <cpplox/chunk.hpp>
#include <cpplox/value.hpp>

#include <cstddef>

namespace lox {
enum class InterpretResult { OK, COMPILE_ERROR, RUNTIME_ERROR };

class VM {
	std::byte readByte(std::span<const std::byte>::iterator &ip);
	Value readConstant(std::span<const std::byte>::iterator &ip);
	InterpretResult run();

  public:
	VM(bool debug_trace = false) : debug_trace_instruction(debug_trace) {}
	InterpretResult interpret(Chunk &chunk);

  private:
	bool debug_trace_instruction;
	Chunk chunk;
	std::span<const std::byte>::iterator ip;
	std::span<const std::byte>::iterator ip_end;
};
} // namespace lox
