#pragma once

#include <cpplox/value.hpp>

#include <cpplox/object/ObjFunction.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <variant>

namespace lox {

// helper type for the visitor
template <class... Ts> struct overloads : Ts... {
	using Ts::operator()...;
};

class Chunk;
class Object;
class Obj;
class ObjFunction;
class ObjNative;
class ObjClosure;
class Value;

using NativeFn = Value (*)(size_t argCount,
                           std::span<std::reference_wrapper<Value>> args);

class ObjNative {
  public:
	NativeFn function = nullptr;
	bool operator==(const ObjNative &other) const;
	std::string toString() const;
};

class ObjClosure {
  public:
	std::reference_wrapper<const ObjFunction> function;

	ObjClosure(const ObjFunction &function);

	bool operator==(const ObjClosure &other) const;

	size_t arity() const { return function.get().arity; }
	const std::unique_ptr<Chunk> &chunk() const { return function.get().chunk; }

	ObjClosure clone() const;
	std::string toString() const;
};

class Obj {
	using Obj_t = std::variant<std::string, ObjFunction, ObjNative, ObjClosure>;

  public:
	Obj() = default;
	Obj(std::string value);
	Obj(const ObjFunction &value);
	Obj(const ObjNative &value);
	Obj(const ObjClosure &value);
	Obj(const Obj &other) = delete;
	Obj(Obj &&other) noexcept;

	Obj &operator=(const Obj &other) = delete;
	Obj &operator=(Obj &&other) noexcept;

	bool operator==(const Obj &other) const;

	std::string toString() const;

	Obj clone() const;
	Obj_t value;
};

} // namespace lox
