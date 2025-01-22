#pragma once

#include <cpplox/value.hpp>

#include <cpplox/object/ObjClosure.hpp>
#include <cpplox/object/ObjFunction.hpp>
#include <cpplox/object/ObjNative.hpp>

namespace lox {

// helper type for the visitor
template <class... Ts> struct overloads : Ts... {
	using Ts::operator()...;
};

} // namespace lox
