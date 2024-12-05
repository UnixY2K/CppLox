#pragma once

#include <memory>
#include <string>
#include <variant>

namespace lox {

// helper type for the visitor
template <class... Ts> struct overloads : Ts... {
	using Ts::operator()...;
};

class Object {};
class Chunk;
struct ObjFunction {
	Object obj;
	std::unique_ptr<Chunk> chunk;
	size_t arity = 0;
	std::string name;

	ObjFunction();

	ObjFunction clone() const;

	std::string toString() const;
};

class Obj {
	using Obj_t = std::variant<std::string, ObjFunction>;
public:
	Obj() = default;
	Obj(std::string value);
	Obj(const ObjFunction &value);
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
