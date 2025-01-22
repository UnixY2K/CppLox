#pragma once
#include <cpplox/object.hpp>

#include <span>

namespace lox {
class Value;

using NativeFn = Value (*)(size_t argCount,
                           std::span<std::reference_wrapper<Value>> args);

class ObjNative {
  public:
	NativeFn function = nullptr;
	bool operator==(const ObjNative &other) const;
	std::string toString() const;
};
} // namespace lox
