#pragma once
#include <cpplox/object.hpp>

#include <span>

namespace lox {
class Value;

using NativeFn = Value (*)(size_t argCount,
                           std::span<std::reference_wrapper<Value>> args);

class ObjNative : public Object {
  public:
	ObjNative(NativeFn function) : function(function) {}
	bool operator==(const ObjNative &other) const;

	std::unique_ptr<Object> clone() const override;
	std::string toString() const override;
	bool equals(const Object &other) const override;

	NativeFn function = nullptr;

	~ObjNative() = default;
};
} // namespace lox
