#pragma once
#include <cstddef>
#include <string_view>

#include <cpplox/chunk.hpp>

namespace lox::cli::debug {

int ConstantInstruction(std::string_view name, const lox::Chunk &chunk,
                        size_t offset);

int SimpleInstruction(std::string_view name, int offset);

int InstructionDisassembly(const lox::Chunk &chunk, size_t offset);

void ChunkDisassembly(const lox::Chunk &chunk, std::string_view name);

} // namespace lox::cli::debug
