#include <cpplox/chunk.hpp>
#include <cpplox/obj.hpp>
#include <cpplox/value.hpp>

#include <format>
#include <memory>
#include <string>
#include <variant>

namespace lox {

Value::Value() : value(std::monostate{}) {}

Value::Value(bool value) : value(value) {}

Value::Value(double value) : value(value) {}

Value::Value(const std::string_view value) : value(std::string(value)) {}

Value::Value(std::unique_ptr<Object> value) : value(std::move(value)) {}

Value::Value(const Object &value) : value(value.clone()) {}

Value::Value(const NativeFn &function) : value(Obj{ObjNative{function}}) {}

Value::Value(const ObjFunction &value) : value(value.clone()) {}

Value::Value(const Value &other) : value(other.clone().value) {}

Value::Value(Value &&other) noexcept : value(std::move(other.value)) {}

Value &Value::operator=(const Value &other) {
	value = other.clone().value;
	return *this;
}

Value &Value::operator=(Value &&other) noexcept {
	value = std::move(other.value);
	return *this;
}

Value Value::clone() const {
	Value result;
	std::visit(
	    overloads{
	        [&result](const std::string &value) { result = Value{value}; },
	        [&result](bool value) { result = Value{value}; },
	        [&result](double value) { result = Value{value}; },
	        [&result](const std::unique_ptr<Object> &value) {
		        result.value = value->clone();
	        },
	        [&result](const Obj &value) { result.value = value.clone(); },
	        [&result](std::monostate) { result = Value{}; },
	    },
	    value);
	return result;
}

std::string Value::toString() const {
	std::string result;
	std::visit(
	    overloads{
	        [&result](const std::string &value) { result = value; },
	        [&result](bool value) { result = std::format("{}", value); },
	        [&result](double value) { result = std::format("{}", value); },
	        [&result](const std::unique_ptr<Object> &value) {
		        result = std::format("{}", value->toString());
	        },
	        [&result](const Obj &value) {
		        result = std::format("{}", value.toString());
	        },
	        [&result](std::monostate) { result = std::format("nil"); },
	    },
	    value);

	return result;
}

bool Value::isTruthy() const {
	bool result = true;

	std::visit(overloads{
	               // same value as the boolean
	               [&result](bool value) { result = value; },
	               // nil is always false
	               [&result](std::monostate) { result = false; },
	               // everything else is true
	               [](const auto &) {},
	           },
	           value);
	return result;
}

bool Value::equals(const Value &other) const {
	bool result = false;
	std::visit(overloads{
	               [&result](const std::string &a, const std::string &b) {
		               result = a == b;
	               },
	               [&result](double a, double b) { result = a == b; },
	               [&result](bool a, bool b) { result = a == b; },
	               [&result](std::monostate, std::monostate) { result = true; },
	               [&result](const std::unique_ptr<Object> &a,
	                         const std::unique_ptr<Object> &b) {
		               result = a->equals(*b);
	               },
	               [&result](const Obj &a, const Obj &b) { result = a == b; },
	               // dont bother comparing different types
	               [](const auto &, const auto &) {},
	           },
	           value, other.value);
	return result;
}

} // namespace lox
