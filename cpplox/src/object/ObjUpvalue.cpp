#include <cpplox/object/ObjUpvalue.hpp>

namespace lox {

ObjUpvalue::ObjUpvalue(Value &location) : location(location) {}

Value &ObjUpvalue::getValue() { return location.get(); }

std::unique_ptr<Object> ObjUpvalue::clone() const {
	return std::make_unique<ObjUpvalue>(location.get());
}

std::string ObjUpvalue::toString() const { return "upvalue"; }

bool ObjUpvalue::equals(const Object &other) const {
	if (!is<ObjUpvalue>(&other)) {
		return false;
	}
	const auto &otherUpvalue = static_cast<const ObjUpvalue &>(other);
	return &location.get() == &otherUpvalue.location.get();
}

} // namespace lox
