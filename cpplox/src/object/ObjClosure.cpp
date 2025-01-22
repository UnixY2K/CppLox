#include <cpplox/chunk.hpp>
#include <cpplox/obj.hpp>
#include <cpplox/value.hpp>

#include <format>

namespace lox {

ObjClosure::ObjClosure(const ObjFunction &function) : function{function} {};

bool ObjClosure::operator==(const ObjClosure &other) const {
	return function.get() == other.function.get();
}

ObjClosure ObjClosure::copy() const { return ObjClosure{function}; }

std::unique_ptr<Object> ObjClosure::clone() const {
	return std::make_unique<ObjClosure>(*this);
}

std::string ObjClosure::toString() const {
	return std::format("<closure {}>", function.get().name);
}

bool ObjClosure::equals(const Object &other) const {
	if (auto otherClosure = dynamic_cast<const ObjClosure *>(&other)) {
		return *this == *otherClosure;
	}
	return false;
}

} // namespace lox
