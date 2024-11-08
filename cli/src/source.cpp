#include <cpplox/vm.hpp>
#include <cpplox/chunk.hpp>

#include <cpplox/debug.hpp>

#include <cstddef>

int main(int argc, char *argv[]) {
	lox::VM vm;
	lox::Chunk chunk;
	size_t constant = chunk.addConstant(1.2);
	chunk.write(static_cast<std::byte>(lox::OpCode::OP_CONSTANT), 123);
	chunk.write(static_cast<std::byte>(constant), 123);
	chunk.write(static_cast<std::byte>(lox::OpCode::OP_RETURN), 123);

	lox::debug::ChunkDisassembly(chunk, "test chunk");
	vm.interpret(chunk);
}
