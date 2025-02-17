#pragma once
#include <cpplox/object.hpp>
#include <cpplox/object/ObjFunction.hpp>

#include <vector>

namespace lox {
class ObjClosure : public Object {
  public:
	std::reference_wrapper<const ObjFunction> function;
	std::vector<std::shared_ptr<Value>> upvalues;

	ObjClosure(const ObjFunction &function);

	bool operator==(const ObjClosure &other) const;

	size_t arity() const { return function.get().arity; }
	const std::shared_ptr<Chunk> &chunk() const { return function.get().chunk; }

	ObjClosure copy() const;

	std::unique_ptr<Object> clone() const override;
	std::string toString() const override;
	bool equals(const Object &other) const override;

	~ObjClosure() = default;
};
} // namespace lox
