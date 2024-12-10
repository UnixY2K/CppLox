#pragma once

#include <cpplox/value.hpp>

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

class Object {};
class Chunk;
class Object;
class Obj;
struct ObjFunction;
struct ObjNative;
class Value;

using NativeFn = Value (*)(size_t argCount,
                           std::span<std::reference_wrapper<Value>> args);

struct ObjNative {
	NativeFn function = nullptr;
	std::string name;
	bool operator==(const ObjNative &other) const;
	std::string toString() const;
};

struct ObjFunction {
	std::string name;
	size_t arity = 0;
	std::unique_ptr<Chunk> chunk;
	Object obj;

	ObjFunction();
	bool operator==(const ObjFunction &other) const;
	ObjFunction clone() const;
	std::string toString() const;
};

class Obj {
	using Obj_t = std::variant<std::string, ObjFunction, ObjNative>;

  public:
	Obj() = default;
	Obj(std::string value);
	Obj(const ObjFunction &value);
	Obj(const ObjNative &value);
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
