#pragma once
#include <cpplox/obj.hpp>

#include <string>
#include <string_view>
#include <variant>

namespace lox {

class Value {
	using Value_t = std::variant<bool, double, Obj, std::monostate>;

  public:
	Value() = default;
	Value(bool value);
	Value(double value);
	Value(const std::string_view value);
	// functions are owned by the object, so we need to move them
	Value(ObjFunction &&value);
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
