#include <cpplox/chunk.hpp>
#include <cpplox/compiler.hpp>
#include <cpplox/debug.hpp>
#include <cpplox/obj.hpp>
#include <cpplox/value.hpp>
#include <cpplox/vm.hpp>

#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <string_view>
#include <variant>

namespace lox {

void VM::runtimeError(std::string_view message) {
	std::cerr << std::format("{}\n", message);
	std::cerr << std::format("[line {}] in script\n", 0);
	had_error = true;
	stack.clear();
}

std::byte VM::readByte(std::span<const std::byte>::iterator &ip) {
	return *ip++;
}

std::byte VM::nextByte(std::span<const std::byte>::iterator &ip) {
	return *++ip;
}

std::byte VM::peekByte(std::span<const std::byte>::iterator &ip) { return *ip; }

void VM::binaryOp(std::span<const std::byte>::iterator &ip) {
	OpCode instruction = static_cast<OpCode>(peekByte(ip));
	Value vb = stack.back();
	stack.pop_back();
	Value va = stack.back();
	stack.pop_back();
	auto onlyNumbers = [this](auto, auto) {
		runtimeError("Operands must be numbers.");
	};
	switch (instruction) {
	case OpCode::OP_EQUAL:
		stack.push_back(valuesEqual(va, vb));
		break;
	case OpCode::OP_NOT_EQUAL:
		stack.push_back(!valuesEqual(va, vb));
		break;
	case OpCode::OP_GREATER:
		std::visit(
		    overloads{
		        [this](double a, double b) { stack.push_back(a > b); },
		        onlyNumbers,
		    },
		    va, vb);
		break;
	case OpCode::OP_GREATER_EQUAL:
		std::visit(
		    overloads{
		        [this](double a, double b) { stack.push_back(a >= b); },
		        onlyNumbers,
		    },
		    va, vb);
		break;
	case OpCode::OP_LESS: {
		std::visit(
		    overloads{
		        [this](double a, double b) { stack.push_back(a < b); },
		        onlyNumbers,
		    },
		    va, vb);
		break;
	}
	case OpCode::OP_LESS_EQUAL:
		std::visit(
		    overloads{
		        [this](double a, double b) { stack.push_back(a <= b); },
		        onlyNumbers,
		    },
		    va, vb);
		break;
	case OpCode::OP_ADD:
		std::visit(
		    overloads{
		        [this](double a, double b) { stack.push_back(a + b); },
		        [&](const Obj &a, const Obj &b) {
			        std::visit(
			            overloads{
			                [this](const std::string &a, const std::string &b) {
				                stack.push_back(a + b);
			                },
			                [this](auto, auto) {
				                runtimeError("Operands must be two numbers or "
				                             "two strings.");
			                },
			            },
			            a, b);
		        },
		        [this](auto, auto) {
			        runtimeError("Operands must be two numbers or "
			                     "two strings.");
		        },
		    },
		    va, vb);
		break;
	case OpCode::OP_SUBTRACT:
		std::visit(
		    overloads{
		        [this](double a, double b) { stack.push_back(a - b); },
		        onlyNumbers,
		    },
		    va, vb);
		break;
	case OpCode::OP_MULTIPLY:
		std::visit(
		    overloads{
		        [this](double a, double b) { stack.push_back(a * b); },
		        onlyNumbers,
		    },
		    va, vb);
		break;
	case OpCode::OP_DIVIDE:
		std::visit(
		    overloads{
		        [this](double a, double b) {
			        if (b == 0) {
				        runtimeError("Division by zero.");
				        return;
			        }
			        stack.push_back(a / b);
		        },
		        onlyNumbers,
		    },
		    va, vb);
		break;
	default:
		[[unlikely]] throw std::runtime_error("Unhandled OpCode in binaryOp");
	}
}

size_t VM::readIndex(std::span<const std::byte>::iterator &ip) {
	auto instruction = static_cast<lox::OpCode>(*(ip));
	size_t address = static_cast<uint8_t>(nextByte(ip));
	// 16 bit addresses
	[[unlikely]]
	if (instruction == OpCode::OP_CONSTANT_LONG ||
	    instruction == OpCode::OP_GET_LOCAL_LONG ||
	    instruction == OpCode::OP_SET_LOCAL_LONG ||
	    instruction == OpCode::OP_GET_GLOBAL_LONG ||
	    instruction == OpCode::OP_DEFINE_GLOBAL_LONG ||
	    instruction == OpCode::OP_SET_GLOBAL_LONG ||
	    instruction == OpCode::OP_JUMP ||
	    instruction == OpCode::OP_JUMP_IF_FALSE) {
		address = address << 8 | static_cast<uint8_t>(nextByte(ip));
	}
	return address;
}

Value VM::readConstant(std::span<const std::byte>::iterator &ip) {
	return chunk.constants()[readIndex(ip)];
}

InterpretResult VM::run() {
	auto code = chunk.code();
	for (auto ip = code.begin(); ip != code.end(); ip++) {
		if (debug_trace_instruction) {
			auto it = ip;
			if (debug_stack) {
				std::cout << "		  ";
				for (auto slot : stack) {
					std::cout << std::format("[ {} ]", valueToString(slot));
				}
				std::cout << "\n";
			}
			debug::InstructionDisassembly(chunk, it);
		}
		auto instruction = static_cast<lox::OpCode>(peekByte(ip));
		switch (instruction) {
		case OpCode::OP_CONSTANT:
		case OpCode::OP_CONSTANT_LONG: {
			Value constant = readConstant(ip);
			stack.push_back(constant);
			break;
		}
		case OpCode::OP_NIL:
			stack.push_back(std::monostate{});
			break;
		case OpCode::OP_TRUE:
			stack.push_back(true);
			break;
		case OpCode::OP_FALSE:
			stack.push_back(false);
			break;
		case OpCode::OP_POP: {
			if (stack.empty()) {
				runtimeError("Stack underflow.");
				return InterpretResult::RUNTIME_ERROR;
			}
			stack.pop_back();
			break;
		}
		case OpCode::OP_GET_LOCAL:
		case OpCode::OP_GET_LOCAL_LONG: {
			size_t index = readIndex(ip);
			stack.push_back(stack[index]);
			break;
		}
		case OpCode::OP_SET_LOCAL:
		case OpCode::OP_SET_LOCAL_LONG: {
			size_t index = readIndex(ip);
			if (stack.empty()) {
				runtimeError("Stack underflow");
				return InterpretResult::RUNTIME_ERROR;
			}
			stack[index] = stack.back();
			break;
		}
		case OpCode::OP_GET_GLOBAL:
		case OpCode::OP_GET_GLOBAL_LONG: {
			Value value = readConstant(ip);
			std::string name = valueToString(value);
			if (auto it = globals.find(name); it != globals.end()) {
				stack.push_back(it->second);
			} else {
				runtimeError(std::format("Undefined variable '{}'", name));
				return InterpretResult::RUNTIME_ERROR;
			}
			break;
		}
		case OpCode::OP_DEFINE_GLOBAL:
		case OpCode::OP_DEFINE_GLOBAL_LONG: {
			Value value = readConstant(ip);
			std::string name = valueToString(value);
			if (stack.empty()) {
				runtimeError("Stack underflow.");
				return InterpretResult::RUNTIME_ERROR;
			}
			globals[name] = stack.back();
			stack.pop_back();
			break;
		}
		case OpCode::OP_SET_GLOBAL:
		case OpCode::OP_SET_GLOBAL_LONG: {
			Value value = readConstant(ip);
			std::string name = valueToString(value);
			if (auto it = globals.find(name); it != globals.end()) {
				globals[name] = stack.back();
			} else {
				runtimeError(std::format("Undefined variable '{}'", name));
				return InterpretResult::RUNTIME_ERROR;
			}
			break;
		}
		case OpCode::OP_EQUAL:
		case OpCode::OP_NOT_EQUAL:
		case OpCode::OP_GREATER:
		case OpCode::OP_GREATER_EQUAL:
		case OpCode::OP_LESS:
		case OpCode::OP_LESS_EQUAL:
		case OpCode::OP_ADD:
		case OpCode::OP_SUBTRACT:
		case OpCode::OP_MULTIPLY:
		case OpCode::OP_DIVIDE: {
			// check that there is at least 2 elements in the stack
			if (stack.size() < 2) {
				runtimeError("Stack underflow.");
				return InterpretResult::RUNTIME_ERROR;
			}
			binaryOp(ip);
			break;
		}
		case lox::OpCode::OP_NEGATE: {
			if (stack.empty()) {
				runtimeError("Stack underflow.");
				return InterpretResult::RUNTIME_ERROR;
			}
			auto &value = stack.back();
			if (!std::holds_alternative<double>(value)) {
				runtimeError("Operand must be a number.");
				return InterpretResult::RUNTIME_ERROR;
			}
			value = -std::get<double>(value);
			break;
		}
		case OpCode::OP_NOT: {
			if (stack.empty()) {
				runtimeError("Stack underflow.");
				return InterpretResult::RUNTIME_ERROR;
			}
			auto &value = stack.back();
			value = !isTruthy(value);
			break;
		}
		case OpCode::OP_PRINT: {
			if (stack.empty()) {
				runtimeError("Stack underflow.");
				return InterpretResult::RUNTIME_ERROR;
			}
			std::cout << std::format("{}\n", valueToString(stack.back()));
			stack.pop_back();
			break;
		}
		case OpCode::OP_JUMP: {
			size_t offset = readIndex(ip);
			ip += offset;
			break;
		}
		case OpCode::OP_JUMP_IF_FALSE: {
			size_t offset = readIndex(ip);
			if (!isTruthy(stack.back())) {
				ip += offset;
			}
			break;
		}
		case OpCode::OP_RETURN: {
		}
		}
		if (had_error) {
			return InterpretResult::RUNTIME_ERROR;
		}
	}
	return InterpretResult::OK;
}

InterpretResult VM::interpret(const Chunk &chunk) {
	had_error = false;
	this->chunk = chunk;
	this->ip = this->chunk.code().begin();
	this->ip_end = this->chunk.code().end();
	return run();
}

InterpretResult VM::interpret(std::string_view source) {
	auto compiler = Compiler{};
	if (const auto result = compiler.compile(source); result.has_value()) {
		return interpret(result.value());
	} else {
		return InterpretResult::COMPILE_ERROR;
	}
}

} // namespace lox
