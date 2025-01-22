#include <cpplox/chunk.hpp>
#include <cpplox/obj.hpp>
#include <cpplox/value.hpp>

namespace lox {

Obj::Obj(const ObjNative &value) : value(value) {}

Obj::Obj(Obj &&other) noexcept : value(std::move(other.value)) {}

Obj &Obj::operator=(Obj &&other) noexcept {
	value = std::move(other.value);
	return *this;
}

bool Obj::operator==(const Obj &other) const {
	bool result = false;
	std::visit(overloads{
	               [&result](const ObjFunction &a, const ObjFunction &b) {
		               result = a == b;
	               },
	               [&result](const ObjNative &a, const ObjNative &b) {
		               result = a == b;
	               },
	               // dont bother comparing different types
	               [](const auto &, const auto &) {},
	           },
	           value, other.value);
	return result;
}

std::string Obj::toString() const {
	std::string result;
	std::visit(
	    overloads{
	        [&result](const ObjNative &value) { result = value.toString(); },
	    },
	    value);
	return result;
}

Obj Obj::clone() const {
	Obj result;
	std::visit(
	    overloads{
	        [&result](const ObjNative &value) { result = Obj{value}; },
	    },
	    value);
	return result;
}

} // namespace lox
