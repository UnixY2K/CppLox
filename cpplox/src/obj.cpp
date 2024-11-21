#include <cpplox/obj.hpp>

#include <format>

namespace lox {
std::string objToString(const Obj &obj) {
	std::string result;
	std::visit(
	    overloads{[&result](const std::string &value) { result = value; },
	              [&result](const Function &value) {
		              result = std::format("<fn {}>", value.name);
	              }},
	    obj);
	return result;
}
} // namespace lox
