#pragma once

#include <string>
#include <variant>

namespace lox {

// helper type for the visitor
template <class... Ts> struct overloads : Ts... {
	using Ts::operator()...;
};

class Object {};
class Chunk;

struct Function {
	Object obj;
	Chunk *chunk;
	size_t arity;
	std::string name;
};

using Obj = std::variant<std::string, Function>;

std::string objToString(const Obj &obj);

} // namespace lox
