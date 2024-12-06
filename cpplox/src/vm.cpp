#include "cpplox/terminal.hpp"
#include <cpplox/private/constants.hpp>

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

VM::VM()
    : debug_trace_instruction(constants::debug_trace_instruction),
      debug_trace_stack(constants::debug_trace_stack) {}

void VM::runtimeError(std::string_view message) {
	std::cerr << std::format("{}\n", message);
	for (auto it = callFrames.rbegin(); it != callFrames.rend(); it++) {
		auto &frame = **it;
		auto &function = frame.function;
		auto &chunk = *function.chunk;
		size_t offset = frame.ip - chunk.code().begin();
		size_t line = chunk.getLine(offset);
		std::string name = function.toString();
		std::cerr << std::format(
		    "{} in {}\n",
		    cli::terminal::green_colored(std::format("[Line {}]", line)),
		    cli::terminal::yellow_colored(name));
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
	auto vb = (*stack.back()).clone();
	stack.pop_back();
	auto va = (*stack.back()).clone();
	stack.pop_back();
	auto onlyNumbers = [this](const auto &, const auto &) {
		runtimeError("Operands must be numbers.");
	};
	switch (instruction) {
	case OpCode::OP_EQUAL:
		stack.emplace_back(std::make_unique<Value>(va.equals(vb)));
		break;
	case OpCode::OP_NOT_EQUAL:
		stack.emplace_back(std::make_unique<Value>(!va.equals(vb)));
		break;
	case OpCode::OP_GREATER:
		std::visit(
		    overloads{
		        [this](double a, double b) {
			        stack.push_back(std::make_unique<Value>(a > b));
		        },
		        onlyNumbers,
		    },
		    va.value, vb.value);
		break;
	case OpCode::OP_GREATER_EQUAL:
		std::visit(
		    overloads{
		        [this](double a, double b) {
			        stack.push_back(std::make_unique<Value>(a >= b));
		        },
		        onlyNumbers,
		    },
		    va.value, vb.value);
		break;
	case OpCode::OP_LESS: {
		std::visit(
		    overloads{
		        [this](double a, double b) {
			        stack.push_back(std::make_unique<Value>(a < b));
		        },
		        onlyNumbers,
		    },
		    va.value, vb.value);
		break;
	}
	case OpCode::OP_LESS_EQUAL:
		std::visit(
		    overloads{
		        [this](double a, double b) {
			        stack.push_back(std::make_unique<Value>(a <= b));
		        },
		        onlyNumbers,
		    },
		    va.value, vb.value);
		break;
	case OpCode::OP_ADD:
		std::visit(
		    overloads{
		        [this](double a, double b) {
			        stack.push_back(std::make_unique<Value>(a + b));
		        },
		        [&](const Obj &a, const Obj &b) {
			        std::visit(
			            overloads{
			                [this](const std::string &a, const std::string &b) {
				                stack.emplace_back(
				                    std::make_unique<Value>(a + b));
			                },
			                [this](const auto &, const auto &) {
				                runtimeError("Operands must be two numbers or "
				                             "two strings.");
			                },
			            },
			            a.value, b.value);
		        },
		        [this](const auto &, const auto &) {
			        runtimeError("Operands must be two numbers or "
			                     "two strings.");
		        },
		    },
		    va.value, vb.value);
		break;
	case OpCode::OP_SUBTRACT:
		std::visit(
		    overloads{
		        [this](double a, double b) {
			        stack.push_back(std::make_unique<Value>(a - b));
		        },
		        onlyNumbers,
		    },
		    va.value, vb.value);
		break;
	case OpCode::OP_MULTIPLY:
		std::visit(
		    overloads{
		        [this](double a, double b) {
			        stack.push_back(std::make_unique<Value>(a * b));
		        },
		        onlyNumbers,
		    },
		    va.value, vb.value);
		break;
	case OpCode::OP_DIVIDE:
		std::visit(
		    overloads{
		        [this](double a, double b) {
			        if (b == 0) {
				        runtimeError("Division by zero.");
				        return;
			        }
			        stack.push_back(std::make_unique<Value>(a / b));
		        },
		        onlyNumbers,
		    },
		    va.value, vb.value);
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
		callFrames.emplace_back(std::make_unique<CallFrame>(function));
	} catch (const std::bad_alloc &) {
		runtimeError("could not allocate memory for call frame");
		return false;
	}

	return true;
}
bool VM::callValue(const Value &callee, size_t argCount) {

	if (auto *obj = std::get_if<Obj>(&callee.value); obj) {
		if (auto *function = std::get_if<ObjFunction>(&obj->value); function) {
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

auto VM::readConstant(std::span<const std::byte>::iterator &ip)
    -> std::optional<std::reference_wrapper<const Value>> {
	auto &chunk = *(*callFrames.back()).function.chunk;
	size_t address = readIndex(ip);
	if (address >= chunk.constants().size()) {
		runtimeError("Invalid constant address.");
		return std::nullopt;
	}
	auto &constant = chunk.constants()[address];
	return constant;
}

InterpretResult VM::run() {
	auto &callFrame = *callFrames.back();
	auto &currentChunk = *callFrame.function.chunk;
	auto code = currentChunk.code();
	for (auto ip = code.begin(); ip != code.end(); ip++) {
		auto instruction = static_cast<lox::OpCode>(peekByte(ip));
		if (debug_trace_instruction) {
			auto it = ip;
			if (debug_trace_stack) {
				std::cout << "		  ";
				for (auto &slot : stack) {
					std::cout << std::format("[ {} ]", (*slot).toString());
				}
				std::cout << "\n";
			}
			debug::InstructionDisassembly(currentChunk, it);
		}

		switch (instruction) {
		case OpCode::OP_CONSTANT:
		case OpCode::OP_CONSTANT_LONG: {
			auto constant = readConstant(ip);
			if (!constant.has_value()) {
				return InterpretResult::RUNTIME_ERROR;
			}
			stack.emplace_back(
			    std::make_unique<Value>(constant->get().clone()));
			break;
		}
		case OpCode::OP_NIL:
			stack.emplace_back(std::make_unique<Value>());
			break;
		case OpCode::OP_TRUE:
			stack.emplace_back(std::make_unique<Value>(true));
			break;
		case OpCode::OP_FALSE:
			stack.emplace_back(std::make_unique<Value>(false));
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
			if (callFrame.slots.size() <= index) {
				runtimeError("tried to access an non existing local");
				return InterpretResult::RUNTIME_ERROR;
			}
			stack.emplace_back(
			    std::make_unique<Value>(callFrame.slots[index].clone()));
			break;
		}
		case OpCode::OP_SET_LOCAL:
		case OpCode::OP_SET_LOCAL_LONG: {
			size_t index = readIndex(ip);
			if (stack.empty()) {
				runtimeError("Stack underflow");
				return InterpretResult::RUNTIME_ERROR;
			}
			if (callFrame.slots.size() <= index) {
				runtimeError("tried to set an non existing local");
				return InterpretResult::RUNTIME_ERROR;
			}
			callFrame.slots[index] = (*stack.back()).clone();
			break;
		}
		case OpCode::OP_GET_GLOBAL:
		case OpCode::OP_GET_GLOBAL_LONG: {
			auto value = readConstant(ip);
			if (!value.has_value()) {
				return InterpretResult::RUNTIME_ERROR;
			}
			std::string name = value->get().toString();
			if (auto it = globals.find(name); it != globals.end()) {
				stack.emplace_back(std::make_unique<Value>(it->second.clone()));
			} else {
				runtimeError(std::format("Undefined variable '{}'", name));
				return InterpretResult::RUNTIME_ERROR;
			}
			break;
		}
		case OpCode::OP_DEFINE_GLOBAL:
		case OpCode::OP_DEFINE_GLOBAL_LONG: {
			auto value = readConstant(ip);
			if (!value.has_value()) {
				return InterpretResult::RUNTIME_ERROR;
			}
			std::string name = value->get().toString();
			if (stack.empty()) {
				runtimeError("Stack underflow.");
				return InterpretResult::RUNTIME_ERROR;
			}
			globals[name] = (*stack.back()).clone();
			stack.pop_back();
			break;
		}
		case OpCode::OP_SET_GLOBAL:
		case OpCode::OP_SET_GLOBAL_LONG: {
			auto value = readConstant(ip);
			if (!value.has_value()) {
				return InterpretResult::RUNTIME_ERROR;
			}
			std::string name = value->get().toString();
			if (auto it = globals.find(name); it != globals.end()) {
				globals[name] = (*stack.back()).clone();
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
			auto &value = *stack.back();
			if (!std::holds_alternative<double>(value.value)) {
				runtimeError("Operand must be a number.");
				return InterpretResult::RUNTIME_ERROR;
			}
			value = -std::get<double>(value.value);
			break;
		}
		case OpCode::OP_NOT: {
			if (stack.empty()) {
				runtimeError("Stack underflow.");
				return InterpretResult::RUNTIME_ERROR;
			}
			auto &value = *stack.back();
			value = !value.isTruthy();
			break;
		}
		case OpCode::OP_PRINT: {
			if (stack.empty()) {
				runtimeError("Stack underflow.");
				return InterpretResult::RUNTIME_ERROR;
			}
			std::cout << std::format("{}\n", (*stack.back()).toString());
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
			if (!(*stack.back()).isTruthy()) {
				ip += offset;
			}
			break;
		}
		case OpCode::OP_LOOP: {
			size_t offset = readIndex(ip);
			ip -= offset;
			break;
		}
		case OpCode::OP_CALL: {
			size_t argCount = readIndex(ip);
			if (stack.empty()) {
				runtimeError("Stack underflow.");
				return InterpretResult::RUNTIME_ERROR;
			}
			if (!callValue((*stack.back()), argCount)) {
				return InterpretResult::RUNTIME_ERROR;
			}
			run();
			break;
		}
		case OpCode::OP_RETURN: {
			if (stack.empty()) {
				runtimeError("Stack underflow.");
				return InterpretResult::RUNTIME_ERROR;
			}
			Value result = (*stack.back()).clone();
			callFrames.pop_back();
			if (callFrames.empty()) {
				if (stack.empty()) {
					runtimeError("Stack underflow.");
					return InterpretResult::RUNTIME_ERROR;
				}
				stack.pop_back();
				return InterpretResult::OK;
			}
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
	call(function, 0);
	return run();
}

InterpretResult VM::interpret(std::string_view source) {
	auto compiler = Compiler{};
	if (const auto result = compiler.compile(source); result.has_value()) {
		auto &function = result->get();
		function.name = "<script>";
		return interpret(function);
	} else {
		return InterpretResult::COMPILE_ERROR;
	}
}

} // namespace lox
