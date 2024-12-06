#pragma once
#include <cpplox/chunk.hpp>
#include <cpplox/obj.hpp>
#include <cpplox/value.hpp>

#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>

namespace lox {
enum class InterpretResult { OK, COMPILE_ERROR, RUNTIME_ERROR };

struct CallFrame {

	CallFrame(const ObjFunction &function)
	    : function(function), ip(function.chunk->code().begin()),
	      slots(function.arity) {}

	// move constructor
	CallFrame(CallFrame &&other) noexcept
	    : function(other.function), ip(other.ip),
	      slots(std::move(other.slots)) {
		other.ip = other.function.chunk->code().begin();
	}

	// callframes are not copyable
	CallFrame(const CallFrame &) = delete;

	const ObjFunction &function;
	std::span<const std::byte>::iterator ip;
	std::vector<Value> slots;
};

class VM {
	void runtimeError(std::string_view message);

	// gets the byte and increments the instruction pointer
	std::byte readByte(std::span<const std::byte>::iterator &ip);
	// increments the instruction pointer and then returns the current byte
	std::byte nextByte(std::span<const std::byte>::iterator &ip);
	// returns the current byte
	std::byte peekByte(std::span<const std::byte>::iterator &ip);

	void binaryOp(std::span<const std::byte>::iterator &ip);

	bool call(const ObjFunction &function, size_t argCount);
	bool callValue(const Value &callee, size_t argCount);

	size_t readIndex(std::span<const std::byte>::iterator &ip);
	auto readConstant(std::span<const std::byte>::iterator &ip)
	    -> std::optional<std::reference_wrapper<const Value>>;
	InterpretResult run();

  public:
	InterpretResult interpret(const ObjFunction &function);
	InterpretResult interpret(std::string_view source);
	bool debug_trace_instruction = true;
	bool debug_stack = false;
	size_t max_callframes_size = 1024;

  private:
	bool had_error = false;
	std::vector<std::unique_ptr<CallFrame>> callFrames;
	std::vector<std::unique_ptr<Value>> stack;
	std::unordered_map<std::string, Value> globals;
	std::span<const std::byte>::iterator ip;
};
} // namespace lox
