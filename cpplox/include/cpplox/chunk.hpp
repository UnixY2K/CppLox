#pragma once
#include <cpplox/value.hpp>

#include <cstddef>
#include <span>
#include <tuple>
#include <vector>

namespace lox {

enum class OpCode {
	OP_CONSTANT,
	OP_RETURN,
};

class Chunk {
  public:
	void write(std::byte byte, size_t line);
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
