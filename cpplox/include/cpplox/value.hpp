#pragma once
#include <variant>

#include <string>

namespace lox {

// helper type for the visitor
template <class... Ts> struct overloads : Ts... {
	using Ts::operator()...;
};

using Value = std::variant<double, bool, std::monostate>;
std::string valueToString(const Value &value);
bool isTruthy(const Value &value);
bool valuesEqual(const Value &a, const Value &b);

} // namespace lox
