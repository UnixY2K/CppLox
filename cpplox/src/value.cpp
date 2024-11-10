#include <cpplox/value.hpp>

#include <format>
#include <string>
#include <variant>

namespace lox {

std::string valueToString(const Value &value) {
	std::string result;
	std::visit(
	    overloads{
	        [&result](double value) { result = std::format("{:g}", value); },
	        [&result](bool value) { result = std::format("{}", value); },
	        [&result](std::monostate) { result = std::format("nil"); },
	    },
	    value);

	return result;
}

bool isTruthy(const Value &value) {
	bool result = true;
	std::visit(overloads{
	               // same value as the boolean
	               [&result](bool b) { result = b; },
	               // nil is always false
	               [&result](std::monostate) { result = false; },
	               // other values are always true
	               [](auto) {},
	           },
	           value);
	return result;
}

bool valuesEqual(const Value &a, const Value &b) {
	bool result = false;
	std::visit(overloads{
	               [&result](double a, double b) { result = a == b; },
	               [&result](bool a, bool b) { result = a == b; },
	               [&result](std::monostate, std::monostate) { result = true; },
	               [](auto, auto) {},
	           },
	           a, b);
	return result;
}

} // namespace lox
