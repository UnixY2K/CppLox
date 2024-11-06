#include <cpplox/chunk.hpp>
#include <cstddef>
#include <span>

namespace lox {
void Chunk::write(std::byte byte, size_t line) {
	m_code.push_back(static_cast<std::byte>(byte));
	if (m_lines.empty() || std::get<0>(m_lines.back()) != line) {
		m_lines.push_back({line, 1});
	} else {
		++std::get<1>(m_lines.back());
	}
}
size_t Chunk::addConstant(Value value) {
	m_constants.push_back(value);
	return m_constants.size() - 1;
}

std::span<const std::byte> Chunk::code() const { return m_code; }
std::size_t Chunk::getLine(std::size_t offset) const {
	size_t line = 1;
	for (const auto &[lineNumber, count] : m_lines) {
		size_t end = offset + count - 1;
		if(line <= end) {
			return lineNumber;
		}
		line += count;
	}
	return line;
}
std::span<const Value> Chunk::constants() const { return m_constants; }

} // namespace lox
