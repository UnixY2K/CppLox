#pragma once
#include <cpplox/value.hpp>

#include <cstddef>
#include <span>
#include <tuple>
#include <vector>

namespace lox {

enum class OpCode {
	OP_CONSTANT_LONG,
	OP_CONSTANT,
	OP_NIL,
	OP_TRUE,
	OP_FALSE,
	OP_POP,
	OP_GET_GLOBAL_LONG,
	OP_GET_GLOBAL,
	OP_DEFINE_GLOBAL_LONG,
	OP_DEFINE_GLOBAL,
	OP_SET_GLOBAL_LONG,
	OP_SET_GLOBAL,
	OP_EQUAL,
	OP_NOT_EQUAL,
	OP_GREATER,
	OP_GREATER_EQUAL,
	OP_LESS,
	OP_LESS_EQUAL,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_NOT,
	OP_NEGATE,
	OP_PRINT,
	OP_RETURN,
};

class Chunk {
  public:
	void write(std::byte byte, size_t line);
	void writeConstant(Value value, size_t line);
	size_t addConstant(Value value);

	std::span<const std::byte> code() const;
	std::size_t getLine(std::size_t offset) const;
	std::span<const Value> constants() const;

  private:
	std::vector<std::byte> m_code;
	// RLE encoding of line numbers
	std::vector<std::tuple<size_t, size_t>> m_lines;
	std::vector<Value> m_constants;
};

} // namespace lox
