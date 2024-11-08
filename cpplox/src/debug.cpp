#include <cpplox/chunk.hpp>
#include <cpplox/value.hpp>
#include <cpplox/terminal.hpp>

#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <iterator>
#include <span>
#include <string_view>


namespace lox::debug {

std::byte readByte(std::span<const std::byte>::iterator &ip) { return *(ip++); }

void ConstantInstruction(std::string_view name, const lox::Chunk &chunk,
                         std::span<const std::byte>::iterator &ip) {
	auto instruction = static_cast<lox::OpCode>(*ip);
	size_t address = static_cast<uint8_t>(readByte(ip));
	[[unlikely]]
	if (instruction == OpCode::OP_CONSTANT_LONG) {
		address = address << 8 | static_cast<uint8_t>(readByte(ip));
	}
	Value value = chunk.constants()[address];
	std::cout << std::format(
	    "{:<4} {} '", cli::terminal::cyan_colored(name),
	    cli::terminal::yellow_colored(std::format("{:4d}", address)));
	std::cout << cli::terminal::green_colored(std::format("{:g}", value));
	std::cout << "'\n";
}

void SimpleInstruction(std::string_view name,
                       std::span<const std::byte>::iterator &ip) {
	std::cout << std::format("{}\n", cli::terminal::cyan_colored(name));
}

void InstructionDisassembly(const lox::Chunk &chunk,
                            std::span<const std::byte>::iterator &ip) {
	auto address = std::distance(chunk.code().begin(), ip);
	size_t offset = address;
	std::cout << std::format("{:#04d} ", offset);
	auto instruction = static_cast<lox::OpCode>(chunk.code()[offset]);
	if (offset > 0 && chunk.getLine(offset) == chunk.getLine(offset - 1)) {
		std::cout << std::format("   | ");
	} else {
		std::cout << std::format("{:4d} ", chunk.getLine(offset));
	}

	switch (instruction) {
	case OpCode::OP_CONSTANT_LONG:
	case OpCode::OP_CONSTANT:
		ConstantInstruction("OP_CONSTANT", chunk, ip);
		break;
	case OpCode::OP_RETURN:
		SimpleInstruction("OP_RETURN", ip);
		break;
	default:
		cli::terminal::logError(std::format("OP_UNKWN ({:#04x})",
		                               static_cast<uint8_t>(instruction)));
		return;
	}
}

void ChunkDisassembly(const lox::Chunk &chunk, std::string_view name) {
	std::cout << std::format("{:=^34}\n", std::format(" {} ", name));

	auto code = chunk.code();
	for (auto ip = code.begin(); ip != code.end(); ip++) {
		InstructionDisassembly(chunk, ip);
	}
}

} // namespace lox::cli::debug
