#pragma once
#include <cpplox/object.hpp>

#include <cpplox/object/ObjFunction.hpp>

namespace lox {
class ObjClosure {
  public:
	std::reference_wrapper<const ObjFunction> function;

	ObjClosure(const ObjFunction &function);

	bool operator==(const ObjClosure &other) const;

	size_t arity() const { return function.get().arity; }
	const std::shared_ptr<Chunk> &chunk() const { return function.get().chunk; }

	ObjClosure clone() const;
	std::string toString() const;
};
} // namespace lox
