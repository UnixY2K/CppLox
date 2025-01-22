#include <cpplox/chunk.hpp>
#include <cpplox/obj.hpp>
#include <cpplox/value.hpp>

#include <format>

namespace lox {

ObjClosure::ObjClosure(const ObjFunction &function) : function{function} {};

bool ObjClosure::operator==(const ObjClosure &other) const {
	return function.get() == other.function.get();
}

ObjClosure ObjClosure::clone() const { return ObjClosure{function}; }

std::string ObjClosure::toString() const {
	return std::format("<closure {}>", function.get().name);
}

} // namespace lox
