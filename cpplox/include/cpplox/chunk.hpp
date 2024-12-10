#pragma once
#include <cpplox/value.hpp>

#include <cstddef>
#include <span>
#include <tuple>
#include <vector>

namespace lox {

enum class OpCode {
	OP_CONSTANT,
	OP_CONSTANT_LONG,
	OP_NIL,
	OP_TRUE,
	OP_FALSE,
	OP_POP,
	OP_GET_LOCAL,
	OP_GET_LOCAL_LONG,
	OP_SET_LOCAL,
	OP_SET_LOCAL_LONG,
	OP_GET_GLOBAL,
	OP_GET_GLOBAL_LONG,
	OP_DEFINE_GLOBAL,
	OP_DEFINE_GLOBAL_LONG,
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
	OP_JUMP,
	OP_JUMP_IF_FALSE,
	OP_LOOP,
	OP_CALL,
	OP_RETURN,
};

class Chunk {
  public:
	void write(std::byte byte, size_t line);
	void writeConstant(const Value &value, size_t line);
	size_t addConstant(const Value &value);
	bool patchByte(size_t offset, std::byte byte);

	std::span<const std::byte> code() const;
	std::size_t getLine(std::size_t offset) const;
	std::span<const Value> constants() const;

	bool operator==(const Chunk &other) const;

  private:
	std::vector<std::byte> m_code;
	// RLE encoding of line numbers
	std::vector<std::tuple<size_t, size_t>> m_lines;
	std::vector<Value> m_constants;
};

} // namespace lox
