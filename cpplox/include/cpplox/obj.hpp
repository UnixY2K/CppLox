#pragma once

#include <string>
#include <variant>

namespace lox {

// helper type for the visitor
template <class... Ts> struct overloads : Ts... {
	using Ts::operator()...;
};

using Obj = std::variant<std::string>;

std::string objToString(const Obj &obj);

} // namespace lox
