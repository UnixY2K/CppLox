#include <cpplox/obj.hpp>
#include <cpplox/value.hpp>

#include <format>
#include <string>
#include <variant>

namespace lox {

std::string valueToString(const Value &value) {
	std::string result;
	std::visit(
	    overloads{
	        [&result](bool value) { result = std::format("{}", value); },
	        [&result](double value) { result = std::format("{}", value); },
	        [&result](const Obj &value) {
		        result = std::format("{}", objToString(value));
	        },
	        [&result](std::monostate) { result = std::format("nil"); },
	    },
	    value);

	return result;
}

Value stringToValue(std::string_view str) { return Obj{std::string{str}}; }

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
	               [&result](const Obj &a, const Obj &b) {
		               std::visit(overloads{
		                              [&result](const std::string &a,
		                                        const std::string &b) {
			                              result = a == b;
		                              },
		                              [&result](auto, auto) { result = true; },
		                          },
		                          a, b);
	               },
	               [](auto, auto) {},
	           },
	           a, b);
	return result;
}

} // namespace lox
