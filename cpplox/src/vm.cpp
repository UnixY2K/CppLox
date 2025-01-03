#include <cpplox/private/constants.hpp>

#include <cpplox/chunk.hpp>
#include <cpplox/compiler.hpp>
#include <cpplox/debug.hpp>
#include <cpplox/obj.hpp>
#include <cpplox/terminal.hpp>
#include <cpplox/value.hpp>
#include <cpplox/vm.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <ranges>
#include <span>
#include <string_view>
#include <variant>

namespace lox {

VM::VM()
    : debug_trace_instruction(constants::debug_trace_instruction),
      debug_trace_stack(constants::debug_trace_stack) {

	defineNative("clock",
	             [](size_t, std::span<std::reference_wrapper<Value>>) -> Value {
		             using namespace std::chrono;
		             auto now = system_clock::now().time_since_epoch();
		             auto ms = duration_cast<milliseconds>(now).count();
		             return Value(static_cast<double>(ms) / 1000.0);
	             });
}

void VM::defineNative(std::string_view name, NativeFn function) {
	// could also use heterogeneous lookup, but just for this is not worth it
	globals[std::string(name)] = function;
}

void VM::runtimeError(std::string_view message) {
	std::cerr << std::format("{}\n", message);
	for (auto it = callFrames.rbegin(); it != callFrames.rend(); it++) {
		auto &frame = **it;
		auto &function = frame.closure;
		auto &chunk = *function.chunk();
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

bool VM::call(const ObjClosure &closure, size_t argCount) {
	auto &function = closure.function.get();
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
		callFrames.emplace_back(
		    std::make_unique<CallFrame>(closure, stack.size()));
	} catch (const std::bad_alloc &) {
		runtimeError("could not allocate memory for call frame");
		return false;
	}

	return true;
}
bool VM::callValue(const Value &callee, size_t argCount) {
	if (stack.size() <= argCount && argCount > 0) {
		runtimeError("not enough values to call function");
		return false;
	}

	if (auto *obj = std::get_if<Obj>(&callee.value); obj) {
		if (auto *function = std::get_if<ObjFunction>(&obj->value); function) {
			return call(*function, argCount);
		} else if (auto *native = std::get_if<ObjNative>(&obj->value); native) {

			auto args_view =
			    stack | std::views::drop(stack.size() - argCount) |
			    std::views::take(argCount) |
			    std::views::transform(
			        [](const auto &ptr) -> std::reference_wrapper<Value> {
				        return std::ref(*ptr);
			        });
			std::vector<std::reference_wrapper<Value>> args_vec(
			    args_view.begin(), args_view.end());
			auto args = std::span<std::reference_wrapper<Value>>(args_vec);

			auto result = native->function(argCount, args);
			// remove the current args and the function in the stack
			stack.erase(stack.end() - (argCount + 1), stack.end());
			stack.emplace_back(std::make_unique<Value>(result));
			return true;
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
	    instruction == OpCode::OP_LOOP ||
	    instruction == OpCode::OP_CLOSURE_LONG) {
		address = address << 8 | static_cast<uint8_t>(nextByte(ip));
	}
	return address;
}

auto VM::readConstant(std::span<const std::byte>::iterator &ip)
    -> std::optional<std::reference_wrapper<const Value>> {
	auto &chunk = *(*callFrames.back()).closure.chunk();
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
	auto &currentChunk = *callFrame.closure.chunk();
	auto code = currentChunk.code();
	std::string_view line_glyph = debug_trace_instruction ? "|" : " ";
	for (auto ip = code.begin(); ip != code.end(); ip++) {
		auto instruction = static_cast<lox::OpCode>(peekByte(ip));
		if (debug_trace_stack) {
			std::cout << std::format("{}  {}	",
			                         cli::terminal::orange_colored("#STACK#"),
			                         cli::terminal::gray_colored(line_glyph));
			for (auto &slot : stack) {
				std::cout << std::format(
				    "[ {} ]",
				    cli::terminal::yellow_colored((*slot).toString()));
			}
			std::cout << "\n";
		}
		if (debug_trace_instruction) {
			auto it = ip;
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
			size_t relativeIndex = readIndex(ip);
			size_t index = callFrame.stackOffset - callFrame.closure.arity() +
			               relativeIndex;
			if (stack.size() <= index) {
				runtimeError("tried to access an non existing local");
				return InterpretResult::RUNTIME_ERROR;
			}
			stack.emplace_back(
			    std::make_unique<Value>((*stack[index]).clone()));
			break;
		}
		case OpCode::OP_SET_LOCAL:
		case OpCode::OP_SET_LOCAL_LONG: {
			size_t relativeIndex = readIndex(ip);
			size_t index = callFrame.stackOffset - callFrame.closure.arity() +
			               relativeIndex;
			if (stack.empty()) {
				runtimeError("Stack underflow");
				return InterpretResult::RUNTIME_ERROR;
			}
			if (stack.size() <= index) {
				runtimeError("tried to set an non existing local");
				return InterpretResult::RUNTIME_ERROR;
			}
			(*stack[index]) = (*stack.back()).clone();
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
			if (stack.size() < argCount + 1) {
				runtimeError("Stack underflow.");
				return InterpretResult::RUNTIME_ERROR;
			}
			size_t calleeIndex = stack.size() - argCount - 1;
			auto &callee = *stack[calleeIndex];
			bool isNative = std::holds_alternative<Obj>(callee.value) &&
			                std::holds_alternative<ObjNative>(
			                    std::get<Obj>(callee.value).value);
			if (!callValue(callee, argCount)) {
				return InterpretResult::RUNTIME_ERROR;
			}
			// only run if is not a native function
			if (!isNative) {
				auto result = run();
				if (result != InterpretResult::OK) {
					return result;
				}
			}
			break;
		}
		case OpCode::OP_CLOSURE:
		case OpCode::OP_CLOSURE_LONG: {
			auto constant = readConstant(ip);
			if (!constant.has_value()) {
				return InterpretResult::RUNTIME_ERROR;
			}

			if (!std::holds_alternative<Obj>(constant->get().value) ||
			    !std::holds_alternative<ObjFunction>(
			        std::get<Obj>(constant->get().value).value)) {
				runtimeError("Expected function for closure.");
				return InterpretResult::RUNTIME_ERROR;
			}

			auto &func = std::get<ObjFunction>(
			    std::get<Obj>(constant->get().value).value);
			stack.push_back(std::make_unique<Value>(func.clone()));
			break;
		}
		case OpCode::OP_RETURN: {
			// pop all the elements from the stack until the last frame
			size_t top = callFrame.stackOffset - callFrame.closure.arity() - 1;
			if (stack.size() < top) {
				runtimeError("Stack underflow.");
				return InterpretResult::RUNTIME_ERROR;
			}
			Value result = (*stack.back()).clone();
			stack.erase(stack.begin() + top, stack.end());
			stack.emplace_back(std::make_unique<Value>(result));
			if (callFrames.empty()) {
				runtimeError("CallFrames underflow.");
				return InterpretResult::RUNTIME_ERROR;
			}
			callFrames.pop_back();
			return InterpretResult::OK;
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
	stack.push_back(std::make_unique<Value>(function.clone()));
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
