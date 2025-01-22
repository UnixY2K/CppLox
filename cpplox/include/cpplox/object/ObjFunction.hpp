#pragma once
#include <cpplox/object.hpp>

#include <cpplox/chunk.hpp>

#include <memory>
#include <string>

namespace lox {

class Chunk;

class ObjFunction : public Object {
  public:
	std::string name;
	size_t arity = 0;
	size_t upvalueCount = 0;
	std::shared_ptr<Chunk> chunk;

	ObjFunction();
	bool operator==(const ObjFunction &other) const;
	ObjFunction copy() const;

	std::unique_ptr<Object> clone() const override;
	std::string toString() const override;
	bool equals(const Object &other) const override;

	~ObjFunction() = default;
};
} // namespace lox
