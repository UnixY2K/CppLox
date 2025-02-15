#pragma once

namespace lox::internals {
// helper type for the visitor
template <class... Ts> struct overloads : Ts... {
	using Ts::operator()...;
};
} // namespace lox::internal
