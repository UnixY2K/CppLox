#pragma once
#include <cpplox/obj.hpp>

#include <string>
#include <string_view>
#include <variant>

namespace lox {

using Value = std::variant<bool, double, Obj, std::monostate>;
std::string valueToString(const Value &value);
Value stringToValue(std::string_view str);
bool isTruthy(const Value &value);
bool valuesEqual(const Value &a, const Value &b);

} // namespace lox
