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
	for (auto it = callFrames.rbegin(); it != callFrames.rend(); it++) {
		auto &frame = *it;
		auto &function = frame.function;
		auto &chunk = function.chunk.get();
		size_t offset = frame.ip - chunk.code().begin();
		size_t line = chunk.getLine(offset);
		std::cerr << std::format("[line {}] in ", line);
		if (function.name.empty()) {
			std::cerr << "script\n";
		} else {
			std::cerr << std::format("function {}\n", function.name);
		}
	}
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

bool VM::call(const ObjFunction &function, size_t argCount) {
	if (function.arity != argCount) {
		runtimeError(std::format("Expected {} arguments but got {}.",
		                         function.arity, argCount));
		return false;
	}

	if (callFrames.size() == max_callframes_size) {
		runtimeError("Stack overflow.");
		return false;
	}

	try {
		callFrames.push_back(CallFrame{const_cast<ObjFunction &>(function)});
	} catch (const std::bad_alloc &) {
		runtimeError("could not allocate memory for call frame");
		return false;
	}

	return true;
}
bool VM::callValue(const Value &callee, size_t argCount) {
	if (auto *obj = std::get_if<Obj>(&callee); obj) {
		if (auto *function = std::get_if<ObjFunction>(obj); function) {
			return call(*function, argCount);
		}
	}
	runtimeError("Can only call functions and classes.");
	return false;
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
	    instruction == OpCode::OP_JUMP_IF_FALSE ||
	    instruction == OpCode::OP_LOOP) {
		address = address << 8 | static_cast<uint8_t>(nextByte(ip));
	}
	return address;
}

Value VM::readConstant(std::span<const std::byte>::iterator &ip) {
	auto &chunk = callFrames.back().function.chunk.get();
	return chunk.constants()[readIndex(ip)];
}

InterpretResult VM::run() {
	auto frame = std::ref(callFrames.back());
	auto &currentChunk = frame.get().function.chunk.get();
	auto code = currentChunk.code();
	for (auto ip = code.begin(); ip != code.end(); ip++) {
		auto instruction = static_cast<lox::OpCode>(peekByte(ip));
		if (debug_trace_instruction) {
			auto it = ip;
			if (debug_stack) {
				std::cout << "		  ";
				for (auto slot : stack) {
					std::cout << std::format("[ {} ]", valueToString(slot));
				}
				std::cout << "\n";
			}
			debug::InstructionDisassembly(currentChunk, it);
		}

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
			if (frame.get().slots.size() <= index) {
				runtimeError("tried to access an non existing local");
				return InterpretResult::RUNTIME_ERROR;
			}
			stack.push_back(frame.get().slots[index]);
			break;
		}
		case OpCode::OP_SET_LOCAL:
		case OpCode::OP_SET_LOCAL_LONG: {
			size_t index = readIndex(ip);
			if (stack.empty()) {
				runtimeError("Stack underflow");
				return InterpretResult::RUNTIME_ERROR;
			}
			if (frame.get().slots.size() <= index) {
				runtimeError("tried to set an non existing local");
				return InterpretResult::RUNTIME_ERROR;
			}
			frame.get().slots[index] = stack.back();
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
			frame.get().ip += offset;
			break;
		}
		case OpCode::OP_JUMP_IF_FALSE: {
			size_t offset = readIndex(ip);
			if (!isTruthy(stack.back())) {
				frame.get().ip += offset;
			}
			break;
		}
		case OpCode::OP_LOOP: {
			size_t offset = readIndex(ip);
			frame.get().ip -= offset;
			break;
		}
		case OpCode::OP_CALL: {
			size_t argCount = readIndex(ip);
			if (stack.empty()) {
				runtimeError("Stack underflow.");
				return InterpretResult::RUNTIME_ERROR;
			}
			if (!callValue(stack.back(), argCount)) {
				return InterpretResult::RUNTIME_ERROR;
			}
			frame = std::ref(callFrames.back());
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

InterpretResult VM::interpret(const ObjFunction &function) {
	had_error = false;
	callFrames.clear();
	callFrames.push_back(CallFrame{const_cast<ObjFunction &>(function)});
	call(callFrames.back().function, 0);
	return run();
}

InterpretResult VM::interpret(std::string_view source) {
	auto compiler = Compiler{};
	if (const auto result = compiler.compile(source); result.has_value()) {
		auto &function = result->get();
		return interpret(function);
	} else {
		return InterpretResult::COMPILE_ERROR;
	}
}

} // namespace lox
