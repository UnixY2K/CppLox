#include <cpplox/object/ObjUpvalue.hpp>

namespace lox {

ObjUpvalue::ObjUpvalue(std::shared_ptr<Value> location) : location{location} {}

std::shared_ptr<Value> ObjUpvalue::getLocation() const { return location; }

Value &ObjUpvalue::getValue() { return *location.get(); }

std::unique_ptr<Object> ObjUpvalue::clone() const {
	return std::make_unique<ObjUpvalue>(*this);
}

std::string ObjUpvalue::toString() const { return "upvalue"; }

bool ObjUpvalue::equals(const Object &other) const {
	if (!is<ObjUpvalue>(&other)) {
		return false;
	}
	const auto &otherUpvalue = static_cast<const ObjUpvalue &>(other);
	return location == otherUpvalue.location;
}

} // namespace lox
