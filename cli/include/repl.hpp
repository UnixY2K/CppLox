#pragma once

#include "cpplox/vm.hpp"
#include <string_view>

namespace lox::cli {
void repl();
int runFile(std::string_view path);
} // namespace lox::cli
