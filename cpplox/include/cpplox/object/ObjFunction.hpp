#pragma once
#include <cpplox/chunk.hpp>
#include <cpplox/object.hpp>


#include <memory>
#include <string>

namespace lox {

class Chunk;

class ObjFunction {
  public:
	std::string name;
	size_t arity = 0;
	size_t upvalueCount = 0;
	std::unique_ptr<Chunk> chunk;

	ObjFunction();
	bool operator==(const ObjFunction &other) const;
	ObjFunction clone() const;
	std::string toString() const;
};
} // namespace lox
