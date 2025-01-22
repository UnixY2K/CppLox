#pragma once
#include <cpplox/obj.hpp>
#include <cpplox/object.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <variant>

namespace lox {

class Obj;
class ObjFunction;
class ObjFunction;
class ObjNative;

class Value {
	using Value_t = std::variant<bool, double, std::string, Obj,
	                             std::unique_ptr<Object>, std::monostate>;

  public:
	Value();
	Value(bool value);
	Value(double value);
	Value(const std::string_view value);
	Value(std::unique_ptr<Object> value);
	Value(const Object &value);
	Value(const NativeFn &function);
	// functions are owned by the object, so we need to move them
	Value(const ObjFunction &value);
	Value(const Value &other);
	Value(Value &&other) noexcept;

	Value &operator=(const Value &other);
	Value &operator=(Value &&other) noexcept;

	Value clone() const;

	std::string toString() const;
	bool isTruthy() const;
	bool equals(const Value &other) const;

	Value_t value;
};

} // namespace lox
