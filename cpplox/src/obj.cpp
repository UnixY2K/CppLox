#include <cpplox/chunk.hpp>
#include <cpplox/obj.hpp>

#include <format>

namespace lox {

ObjFunction::ObjFunction() : chunk(std::make_unique<Chunk>()) {}

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
		return "<script>";
	}
	return std::format("<fn {}>", name);
}

Obj::Obj(std::string value) : value(value) {}

Obj::Obj(const ObjFunction &value) : value{value.clone()} {}

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
	               // TODO: how to compare functions?
	               [&result](const ObjFunction &a, const ObjFunction &b) {
		               result = true;
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
	        [&result](const ObjFunction &value) { result = value.toString(); }},
	    value);
	return result;
}

Obj Obj::clone() const {
	Obj result;
	std::visit(
	    overloads{[&result](const std::string &value) { result = Obj{value}; },
	              [&result](const ObjFunction &value) {
		              result = Obj{value.clone()};
	              }},
	    value);
	return result;
}

} // namespace lox
