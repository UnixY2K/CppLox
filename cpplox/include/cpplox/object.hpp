#pragma once

#include <memory>
#include <string>

namespace lox {

class Object {
  public:
	virtual std::unique_ptr<Object> clone() const = 0;
	virtual std::string toString() const = 0;
	virtual bool equals(const Object &other) const = 0;
	virtual ~Object() = default;
};

// template helper method to check if a pointer is of a certain type
template <typename T> bool is(const Object *object) {
	return dynamic_cast<const T *>(object) != nullptr;
}
template <typename T, typename F> void match(const Object *object, F &&f) {
	if (is<T>(object)) {
		f(static_cast<const T &>(object));
	}
}
} // namespace lox
