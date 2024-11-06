#pragma once
#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <span>

#include <terminal.hpp>

#include <cpplox/chunk.hpp>

namespace lox::cli::debug {

int ConstantInstruction(std::string_view name, const lox::Chunk &chunk,
                        size_t offset) {
	uint8_t constant = static_cast<uint8_t>(chunk.code()[offset + 1]);
	std::cout << std::format("{:<4} {} '", terminal::cyan_colored(name),
	                         terminal::yellow_colored(std::format("{:4d}", constant)));
	std::cout << terminal::green_colored(std::format("{:g}", chunk.constants()[constant]));
	std::cout << "'\n";
	return offset + 2;
}

int SimpleInstruction(std::string_view name, int offset) {
	std::cout << std::format("{}\n", terminal::cyan_colored(name));
	return offset + 1;
}

int InstructionDisassembly(const lox::Chunk &chunk, size_t offset) {
	std::cout << std::format("{:#04d} ", offset);
	auto instruction = static_cast<lox::OpCode>(chunk.code()[offset]);
	if (offset > 0 && chunk.lines()[offset] == chunk.lines()[offset - 1]) {
		std::cout << std::format("   | ");
	} else {
		std::cout << std::format("{:4d} ", chunk.lines()[offset]);
	}

	switch (instruction) {
	case OpCode::OP_CONSTANT:
		return ConstantInstruction("OP_CONSTANT", chunk, offset);
	case OpCode::OP_RETURN:
		return SimpleInstruction("OP_RETURN", offset);
	default:
		terminal::logError(std::format("OP_UNKWN ({:#04x})",
		                               static_cast<uint8_t>(instruction)));
		return offset + 1;
	}
}

void ChunkDisassembly(const lox::Chunk &chunk, std::string_view name) {
	std::cout << std::format("{:=^34}\n", std::format(" {} ", name));

	for (size_t offset = 0; offset < chunk.code().size();) {
		offset = InstructionDisassembly(chunk, offset);
	}
}

} // namespace lox::cli::debug
