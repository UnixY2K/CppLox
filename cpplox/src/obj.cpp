#include <cpplox/chunk.hpp>
#include <cpplox/obj.hpp>
#include <cpplox/value.hpp>

#include <format>

namespace lox {

bool ObjNative::operator==(const ObjNative &other) const {
	return function == other.function;
}

std::string ObjNative::toString() const { return std::format("<native fn>"); }

ObjFunction::ObjFunction() : chunk(std::make_unique<Chunk>()) {}

bool ObjFunction::operator==(const ObjFunction &other) const {
	if (name != other.name || arity != other.arity) {
		return false;
	}
	return *chunk == *other.chunk;
}

ObjFunction ObjFunction::clone() const {
	ObjFunction result;
	// copy the chunk data
	result.chunk = std::make_unique<Chunk>(*chunk);
	result.arity = arity;
	result.name = name;
	return result;
}

std::string ObjFunction::toString() const {
	if (name.empty()) {
		return "<lambda>";
	} else if (name == "<script>") {
		return "<script>";
	}
	return std::format("<fn {}>", name);
}

ObjClosure::ObjClosure(const ObjFunction &function) : function{function} {};

bool ObjClosure::operator==(const ObjClosure &other) const {
	return function.get() == other.function.get();
}

ObjClosure ObjClosure::clone() const { return ObjClosure{function}; }

std::string ObjClosure::toString() const {
	return std::format("<closure {}>", function.get().name);
}

Obj::Obj(std::string value) : value(value) {}

Obj::Obj(const ObjFunction &value) : value{value.clone()} {}

Obj::Obj(const ObjNative &value) : value(value) {}

Obj::Obj(const ObjClosure &value) : value(value) {}

Obj::Obj(Obj &&other) noexcept : value(std::move(other.value)) {}

Obj &Obj::operator=(Obj &&other) noexcept {
	value = std::move(other.value);
	return *this;
}

bool Obj::operator==(const Obj &other) const {
	bool result = false;
	std::visit(overloads{
	               [&result](const std::string &a, const std::string &b) {
		               result = a == b;
	               },
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
	        [&result](const std::string &value) { result = value; },
	        [&result](const ObjNative &value) { result = value.toString(); },
	        [&result](const ObjFunction &value) { result = value.toString(); },
	        [&result](const ObjClosure &value) { result = value.toString(); }},
	    value);
	return result;
}

Obj Obj::clone() const {
	Obj result;
	std::visit(
	    overloads{[&result](const std::string &value) { result = Obj{value}; },
	              [&result](const ObjNative &value) { result = Obj{value}; },
	              [&result](const ObjFunction &value) {
		              result = Obj{value.clone()};
	              },
	              [&result](const ObjClosure &value) {
		              result = Obj{value.clone()};
	              }},
	    value);
	return result;
}

} // namespace lox
