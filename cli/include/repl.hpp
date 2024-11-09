#pragma once

#include <string_view>

namespace lox::cli {
void repl();
int runFile(std::string_view path);
} // namespace lox::cli
