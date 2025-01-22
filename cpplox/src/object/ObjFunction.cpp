#include <cpplox/chunk.hpp>
#include <cpplox/obj.hpp>
#include <cpplox/value.hpp>

#include <format>

namespace lox {

ObjFunction::ObjFunction() : chunk(std::make_unique<Chunk>()) {}

bool ObjFunction::operator==(const ObjFunction &other) const {
	if (name != other.name || arity != other.arity ||
	    upvalueCount != other.upvalueCount) {
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
	result.upvalueCount = upvalueCount;
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

} // namespace lox
