#pragma once
#include <cpplox/value.hpp>

#include <cstddef>
#include <span>
#include <vector>

namespace lox {

enum class OpCode {
	OP_CONSTANT,
	OP_RETURN,
};

class Chunk {
  public:
	void write(std::byte byte, int line);
	size_t addConstant(Value value);

	std::span<const std::byte> code() const;
	std::span<const int> lines() const;
	std::span<const Value> constants() const;

  private:
	std::vector<std::byte> m_code;
	std::vector<int> m_lines;
	std::vector<Value> m_constants;
};

} // namespace lox
