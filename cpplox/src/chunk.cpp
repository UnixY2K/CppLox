#include <cpplox/chunk.hpp>

#include <cstddef>
#include <cstdint>
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
void Chunk::writeConstant(const Value &value, size_t line) {
	size_t index = addConstant(value);
	if (index <= UINT8_MAX) {
		write(static_cast<std::byte>(OpCode::OP_CONSTANT), line);
		write(static_cast<std::byte>(index), line);
	} else {
		write(static_cast<std::byte>(OpCode::OP_CONSTANT_LONG), line);
		write(static_cast<std::byte>(index >> 8), line);
		write(static_cast<std::byte>(index & 0x00ff), line);
	}
}

size_t Chunk::addConstant(const Value &value) {
	m_constants.emplace_back(value.clone());
	return m_constants.size() - 1;
}

bool Chunk::patchByte(size_t offset, std::byte byte) {
	if (offset >= m_code.size()) {
		return false;
	}
	m_code[offset] = byte;
	return true;
}

std::span<const std::byte> Chunk::code() const { return m_code; }
std::size_t Chunk::getLine(std::size_t offset) const {
	size_t line = 1;
	for (const auto &[lineNumber, count] : m_lines) {
		if (offset < count) {
			return lineNumber;
		}
		offset -= count;
	}
	return line;
}
std::span<const Value> Chunk::constants() const { return m_constants; }

} // namespace lox
