#pragma once
#include <cpplox/object.hpp>
#include <cpplox/value.hpp>

#include <memory>

namespace lox {

class ObjUpvalue : public Object {
	std::shared_ptr<Value> location;

  public:
	ObjUpvalue(std::shared_ptr<Value> location);

	Value &getValue();
	std::shared_ptr<Value> getLocation() const;

	virtual std::unique_ptr<Object> clone() const override;
	virtual std::string toString() const override;
	virtual bool equals(const Object &other) const override;

	virtual ~ObjUpvalue() = default;
};
} // namespace lox
