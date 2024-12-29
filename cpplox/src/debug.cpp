#include <cpplox/chunk.hpp>
#include <cpplox/debug.hpp>
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

size_t getAddress(std::span<const std::byte>::iterator &ip) {
	auto instruction = static_cast<lox::OpCode>(*ip);
	size_t address = static_cast<uint8_t>(nextByte(ip));
	[[unlikely]]
	if (instruction == OpCode::OP_CONSTANT_LONG ||
	    instruction == OpCode::OP_GET_LOCAL_LONG ||
	    instruction == OpCode::OP_SET_LOCAL_LONG ||
	    instruction == OpCode::OP_GET_GLOBAL_LONG ||
	    instruction == OpCode::OP_DEFINE_GLOBAL_LONG ||
	    instruction == OpCode::OP_SET_GLOBAL_LONG ||
	    instruction == OpCode::OP_JUMP ||
	    instruction == OpCode::OP_JUMP_IF_FALSE ||
	    instruction == OpCode::OP_LOOP ||
	    instruction == OpCode::OP_CLOSURE_LONG) {
		address = address << 8 | static_cast<uint8_t>(nextByte(ip));
	}
	return address;
}

void ConstantInstruction(std::string_view name, const lox::Chunk &chunk,
                         std::span<const std::byte>::iterator &ip) {
	size_t address = getAddress(ip);

	std::string valueString = "?INVALID?";
	if (address < chunk.constants().size()) {
		auto &value = chunk.constants()[address];
		valueString = value.toString();
	}
	std::cout << std::format(
	    "{:<26} {}      '{}'\n", cli::terminal::cyan_colored(name),
	    cli::terminal::gray_colored(std::format("{:<4d}", address)),
	    cli::terminal::yellow_colored(std::format("{}", valueString)));
}

void SimpleInstruction(std::string_view name,
                       std::span<const std::byte>::iterator &ip) {
	std::cout << std::format("{}\n", cli::terminal::cyan_colored(name));
}

void ByteInstruction(std::string_view name, const lox::Chunk &chunk,
                     std::span<const std::byte>::iterator &ip) {
	size_t address = getAddress(ip);

	std::cout << std::format(
	    "{:<26} {}\n", cli::terminal::cyan_colored(name),
	    cli::terminal::gray_colored(std::format("{:<4d}", address)));
}

void JumpInstruction(std::string_view name, const lox::Chunk &chunk,
                     std::span<const std::byte>::iterator &ip, int sign) {
	size_t jump = getAddress(ip);
	size_t baseoffset = std::distance(chunk.code().begin(), ip);
	size_t offset = baseoffset + sign * jump + 1;
	std::cout << std::format(
	    "{:<26} {} {} {}\n", cli::terminal::cyan_colored(name),
	    cli::terminal::green_colored(std::format("0x{:04X}", baseoffset)),
	    cli::terminal::gray_colored("->"),
	    cli::terminal::green_colored(std::format("0x{:04X}", offset)));
}

void InstructionDisassembly(const lox::Chunk &chunk,
                            std::span<const std::byte>::iterator &ip) {
	auto address = std::distance(chunk.code().begin(), ip);
	size_t offset = address;
	// print offset
	std::cout << std::format(
	    "{}{}", cli::terminal::orange_colored("#"),
	    cli::terminal::green_colored(std::format("{:04X} ", offset)));
	auto instruction = static_cast<lox::OpCode>(peekByte(ip));
	// print line
	if (offset > 0 && chunk.getLine(offset) == chunk.getLine(offset - 1)) {
		std::cout << cli::terminal::gray_colored("   | ");
	} else {
		std::cout << std::format("{:4d} ", chunk.getLine(offset));
	}

	switch (instruction) {
	case OpCode::OP_CONSTANT:
		return ConstantInstruction("OP_CONSTANT", chunk, ip);
	case OpCode::OP_CONSTANT_LONG:
		return ConstantInstruction("OP_CONSTANT_LONG", chunk, ip);
	case OpCode::OP_NIL:
		return SimpleInstruction("OP_NIL", ip);
	case OpCode::OP_TRUE:
		return SimpleInstruction("OP_TRUE", ip);
	case OpCode::OP_FALSE:
		return SimpleInstruction("OP_FALSE", ip);
	case OpCode::OP_POP:
		return SimpleInstruction("OP_POP", ip);
	case OpCode::OP_GET_LOCAL:
		return ByteInstruction("OP_GET_LOCAL", chunk, ip);
	case OpCode::OP_GET_LOCAL_LONG:
		return ByteInstruction("OP_GET_LOCAL_LONG", chunk, ip);
	case OpCode::OP_SET_LOCAL:
		return ByteInstruction("OP_SET_LOCAL", chunk, ip);
	case OpCode::OP_SET_LOCAL_LONG:
		return ByteInstruction("OP_SET_LOCAL_LONG", chunk, ip);
	case OpCode::OP_GET_GLOBAL:
		return ConstantInstruction("OP_GET_GLOBAL", chunk, ip);
	case OpCode::OP_GET_GLOBAL_LONG:
		return ConstantInstruction("OP_GET_GLOBAL_LONG", chunk, ip);
	case OpCode::OP_DEFINE_GLOBAL:
		return ConstantInstruction("OP_DEFINE_GLOBAL", chunk, ip);
	case OpCode::OP_DEFINE_GLOBAL_LONG:
		return ConstantInstruction("OP_DEFINE_GLOBAL_LONG", chunk, ip);
	case OpCode::OP_SET_GLOBAL:
		return ConstantInstruction("OP_SET_GLOBAL", chunk, ip);
	case OpCode::OP_SET_GLOBAL_LONG:
		return ConstantInstruction("OP_SET_GLOBAL_LONG", chunk, ip);
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
	case OpCode::OP_JUMP:
		return JumpInstruction("OP_JUMP", chunk, ip, 1);
	case OpCode::OP_JUMP_IF_FALSE:
		return JumpInstruction("OP_JUMP_IF_FALSE", chunk, ip, 1);
	case OpCode::OP_LOOP:
		return JumpInstruction("OP_LOOP", chunk, ip, -1);
	case OpCode::OP_CALL:
		return ByteInstruction("OP_CALL", chunk, ip);
	case OpCode::OP_CLOSURE:
	case OpCode::OP_CLOSURE_LONG: {
		auto address = getAddress(ip);
		auto &value = chunk.constants()[address];
		std::cout << std::format(
		    "{:<26} {} {}\n",
		    cli::terminal::cyan_colored(address > UINT8_MAX //
		                                    ? "OP_CLOSURE_LONG"
		                                    : "OP_CLOSURE"),
		    cli::terminal::gray_colored(std::format("{:<4d}", address)),
		    cli::terminal::yellow_colored(value.toString()));
	}
	case OpCode::OP_RETURN:
		return SimpleInstruction("OP_RETURN", ip);
	}
	cli::terminal::logError(
	    std::format("OP_UNKWN ({:#04X})", static_cast<uint8_t>(instruction)));
}

void ChunkDisassembly(const lox::Chunk &chunk, std::string_view name) {
	std::cout << std::format("{:=^34}\n", std::format(" {} ", name));

	auto code = chunk.code();
	for (auto ip = code.begin(); ip != code.end(); ip++) {
		InstructionDisassembly(chunk, ip);
	}
}

} // namespace lox::debug
