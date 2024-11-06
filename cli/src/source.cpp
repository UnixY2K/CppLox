#include <cpplox/chunk.hpp>
#include <cstddef>
#include <debug.hpp>

int main(int argc, char *argv[]) {
	lox::Chunk chunk;
	size_t constant = chunk.addConstant(1.2);
	chunk.write(static_cast<std::byte>(lox::OpCode::OP_CONSTANT), 123);
	chunk.write(static_cast<std::byte>(constant), 123);
	chunk.write(static_cast<std::byte>(lox::OpCode::OP_RETURN), 123);

	lox::cli::debug::ChunkDisassembly(chunk, "test chunk");
}
