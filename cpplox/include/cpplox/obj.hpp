#pragma once

#include <cpplox/value.hpp>

#include <cpplox/object/ObjClosure.hpp>
#include <cpplox/object/ObjFunction.hpp>
#include <cpplox/object/ObjNative.hpp>

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

class Obj {
	using Obj_t = std::variant<ObjNative>;

  public:
	Obj() = default;
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
