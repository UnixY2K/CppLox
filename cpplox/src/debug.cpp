#include <cpplox/chunk.hpp>
#include <cpplox/terminal.hpp>
#include <cpplox/value.hpp>

#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <iterator>
#include <span>
#include <string_view>

namespace lox::debug {

std::byte readByte(std::span<const std::byte>::iterator &ip) { return *ip++; }

std::byte nextByte(std::span<const std::byte>::iterator &ip) { return *++ip; }

std::byte peekByte(std::span<const std::byte>::iterator &ip) { return *ip; }

void ConstantInstruction(std::string_view name, const lox::Chunk &chunk,
                         std::span<const std::byte>::iterator &ip) {
	auto instruction = static_cast<lox::OpCode>(*ip);
	size_t address = static_cast<uint8_t>(nextByte(ip));
	[[unlikely]]
	if (instruction == OpCode::OP_CONSTANT_LONG) {
		address = address << 8 | static_cast<uint8_t>(nextByte(ip));
	}
	Value value = chunk.constants()[address];
	std::cout << std::format(
	    "{:<4} {} '", cli::terminal::cyan_colored(name),
	    cli::terminal::yellow_colored(std::format("{:4d}", address)));
	std::cout << cli::terminal::green_colored(
	    std::format("{}", valueToString(value)));
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
	std::cout << std::format("{}{}", cli::terminal::orange_colored("#"),
	                         std::format("{:04d} ", offset));
	auto instruction = static_cast<lox::OpCode>(peekByte(ip));
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
	case OpCode::OP_NIL:
		return SimpleInstruction("OP_NIL", ip);
	case OpCode::OP_TRUE:
		return SimpleInstruction("OP_TRUE", ip);
	case OpCode::OP_FALSE:
		return SimpleInstruction("OP_FALSE", ip);
	case OpCode::OP_EQUAL:
		return SimpleInstruction("OP_EQUAL", ip);
	case OpCode::OP_NOT_EQUAL:
		return SimpleInstruction("OP_NOT_EQUAL", ip);
	case OpCode::OP_GREATER:
		return SimpleInstruction("OP_GREATER", ip);
	case OpCode::OP_GREATER_EQUAL:
		return SimpleInstruction("OP_GREATER_EQUAL", ip);
	case OpCode::OP_LESS:
		return SimpleInstruction("OP_LESS", ip);
	case OpCode::OP_LESS_EQUAL:
		return SimpleInstruction("OP_LESS_EQUAL", ip);
	case OpCode::OP_ADD:
		return SimpleInstruction("OP_ADD", ip);
	case OpCode::OP_SUBTRACT:
		return SimpleInstruction("OP_SUBTRACT", ip);
	case OpCode::OP_MULTIPLY:
		return SimpleInstruction("OP_MULTIPLY", ip);
	case OpCode::OP_DIVIDE:
		return SimpleInstruction("OP_DIVIDE", ip);
	case OpCode::OP_NOT:
		return SimpleInstruction("OP_NOT", ip);
	case OpCode::OP_NEGATE:
		return SimpleInstruction("OP_NEGATE", ip);
	case OpCode::OP_PRINT:
		return SimpleInstruction("OP_PRINT", ip);
	case OpCode::OP_RETURN:
		return SimpleInstruction("OP_RETURN", ip);
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

} // namespace lox::debug
