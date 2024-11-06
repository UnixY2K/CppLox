#include <cpplox/chunk.hpp>
#include <span>

namespace lox {
void Chunk::write(std::byte byte, int line) {
	m_code.push_back(static_cast<std::byte>(byte));
	m_lines.push_back(line);
}
size_t Chunk::addConstant(Value value) {
	m_constants.push_back(value);
	return m_constants.size() - 1;
}

std::span<const std::byte> Chunk::code() const { return m_code; }
std::span<const int> Chunk::lines() const { return m_lines; }
std::span<const Value> Chunk::constants() const { return m_constants; }

} // namespace lox
