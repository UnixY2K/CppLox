#include <cpplox/obj.hpp>

namespace lox {
std::string objToString(const Obj &obj) {
	std::string result;
	std::visit(
	    overloads{[&result](const std::string &value) { result = value; }},
	    obj);
	return result;
}
} // namespace lox
