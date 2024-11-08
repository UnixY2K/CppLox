#include <cpplox/chunk.hpp>
#include <cpplox/value.hpp>
#include <cpplox/vm.hpp>
#include <cpplox/debug.hpp>

#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>

namespace lox {

std::byte VM::readByte(std::span<const std::byte>::iterator& ip){
	return *(ip++);
}

Value VM::readConstant(std::span<const std::byte>::iterator &ip) {
	auto instruction = static_cast<lox::OpCode>(*ip);
	size_t address = static_cast<uint8_t>(readByte(ip));
	[[unlikely]]
	if (instruction == OpCode::OP_CONSTANT_LONG) {
		address = address << 8 | static_cast<uint8_t>(readByte(ip));
	}
	return chunk.constants()[address];
}

InterpretResult VM::run() {
	auto code = chunk.code();
	for (auto ip = code.begin(); ip != code.end(); ip++) {
		if(debug_trace_instruction){
			auto it = ip;
			debug::InstructionDisassembly(chunk, it);
		}
		auto instruction = static_cast<lox::OpCode>(*ip);
		switch (instruction) {
		case OpCode::OP_CONSTANT_LONG:
		case OpCode::OP_CONSTANT: {
			Value constant = readConstant(ip);
			std::cout << std::format("{}\n", constant);
			break;
		}

		case OpCode::OP_RETURN: {
			return InterpretResult::OK;
		}
		}
	}

}

InterpretResult VM::interpret(Chunk &chunk) {
	this->chunk = chunk;
	this->ip = this->chunk.code().begin();
	this->ip_end = this->chunk.code().end();
	return run();
}
} // namespace lox
