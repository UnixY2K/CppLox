#pragma once

#include <cpplox/private/api.hpp>

namespace cpplox {

inline int sum(int a, int b) { return API::LIB_CPPLOX_SUM(a, b); }

} // namespace cpplox