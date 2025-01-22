#include <cpplox/chunk.hpp>
#include <cpplox/obj.hpp>
#include <cpplox/value.hpp>

#include <format>

namespace lox {

bool ObjNative::operator==(const ObjNative &other) const {
	return function == other.function;
}

std::unique_ptr<Object> ObjNative::clone() const {
	return std::make_unique<ObjNative>(*this);
}
std::string ObjNative::toString() const { return std::format("<native fn>"); }
bool ObjNative::equals(const Object &other) const {
	if (auto *otherNative = dynamic_cast<const ObjNative *>(&other)) {
		return *this == *otherNative;
	}
	return false;
}

} // namespace lox
