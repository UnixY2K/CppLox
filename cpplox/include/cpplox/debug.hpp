#pragma once
#include <cstddef>
#include <string_view>

#include <cpplox/chunk.hpp>

namespace lox::debug {

void ConstantInstruction(std::string_view name, const lox::Chunk &chunk,
                         std::span<const std::byte>::iterator &ip);

void SimpleInstruction(std::string_view name,
                       std::span<const std::byte>::iterator &ip);

void InstructionDisassembly(const lox::Chunk &chunk,
                            std::span<const std::byte>::iterator &ip);

void ChunkDisassembly(const lox::Chunk &chunk, std::string_view name);

} // namespace lox::cli::debug