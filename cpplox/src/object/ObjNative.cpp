#include <cpplox/chunk.hpp>
#include <cpplox/obj.hpp>
#include <cpplox/value.hpp>

#include <format>

namespace lox {

bool ObjNative::operator==(const ObjNative &other) const {
	return function == other.function;
}

std::string ObjNative::toString() const { return std::format("<native fn>"); }

} // namespace lox
