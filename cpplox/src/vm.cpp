#include <cpplox/chunk.hpp>
#include <cpplox/compiler.hpp>
#include <cpplox/debug.hpp>
#include <cpplox/value.hpp>
#include <cpplox/vm.hpp>

#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>

namespace lox {

std::byte VM::readByte(std::span<const std::byte>::iterator &ip) {
	return *ip++;
}

std::byte VM::nextByte(std::span<const std::byte>::iterator &ip) {
	return *++ip;
}

std::byte VM::peekByte(std::span<const std::byte>::iterator &ip) { return *ip; }

void VM::binaryOp(std::span<const std::byte>::iterator &ip) {
	OpCode instruction = static_cast<OpCode>(peekByte(ip));
	Value b = stack.back();
	stack.pop_back();
	Value a = stack.back();
	stack.pop_back();
	switch (instruction) {
	case OpCode::OP_ADD:
		stack.push_back(a + b);
		break;
	case OpCode::OP_SUBTRACT:
		stack.push_back(a - b);
		break;
	case OpCode::OP_MULTIPLY:
		stack.push_back(a * b);
		break;
	case OpCode::OP_DIVIDE:
		stack.push_back(a / b);
		break;
	default:
		[[unlikely]] throw std::runtime_error("Unhandled OpCode in binaryOp");
	}
}

Value VM::readConstant(std::span<const std::byte>::iterator &ip) {
	auto instruction = static_cast<lox::OpCode>(*(ip));
	size_t address = static_cast<uint8_t>(nextByte(ip));
	[[unlikely]]
	if (instruction == OpCode::OP_CONSTANT_LONG) {
		address = address << 8 | static_cast<uint8_t>(nextByte(ip));
	}
	return chunk.constants()[address];
}

InterpretResult VM::run() {
	auto code = chunk.code();
	for (auto ip = code.begin(); ip != code.end(); ip++) {
		if (debug_trace_instruction) {
			auto it = ip;
			std::cout << "		  ";
			for (auto slot : stack) {
				std::cout << std::format("[ {} ]", slot);
			}
			std::cout << "\n";
			debug::InstructionDisassembly(chunk, it);
		}
		auto instruction = static_cast<lox::OpCode>(peekByte(ip));
		switch (instruction) {
		case OpCode::OP_CONSTANT_LONG:
		case OpCode::OP_CONSTANT: {
			Value constant = readConstant(ip);
			stack.push_back(constant);
			break;
		}
		case OpCode::OP_ADD:
		case OpCode::OP_SUBTRACT:
		case OpCode::OP_MULTIPLY:
		case OpCode::OP_DIVIDE:
			binaryOp(ip);
			break;
		case lox::OpCode::OP_NEGATE: {
			stack.back() = -stack.back();
			break;
		}
		case OpCode::OP_RETURN: {
			std::cout << std::format("{}\n", stack.back());
			stack.pop_back();
			return InterpretResult::OK;
		}
		}
	}
	return InterpretResult::OK;
}

InterpretResult VM::interpret(Chunk &chunk) {
	this->chunk = chunk;
	this->ip = this->chunk.code().begin();
	this->ip_end = this->chunk.code().end();
	return run();
}

InterpretResult VM::interpret(std::string_view source) {
	Chunk chunk;
	if (!lox::compile(source)) {
		return InterpretResult::COMPILE_ERROR;
	}
	return interpret(chunk);
}

} // namespace lox
