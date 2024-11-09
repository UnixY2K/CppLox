#pragma once
#include <cpplox/chunk.hpp>

#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace lox {
auto compile(std::string_view source)
    -> std::expected<std::vector<Chunk>, std::string>;

} // namespace lox
