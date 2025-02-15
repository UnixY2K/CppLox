#pragma once
#include <cpplox/object.hpp>
#include <cpplox/value.hpp>

namespace lox {

class ObjUpvalue : public Object {
	std::reference_wrapper<Value> location;

  public:
	ObjUpvalue(Value &location);

	Value &getValue();

	virtual std::unique_ptr<Object> clone() const override;
	virtual std::string toString() const override;
	virtual bool equals(const Object &other) const override;

	virtual ~ObjUpvalue() = default;
};
} // namespace lox

