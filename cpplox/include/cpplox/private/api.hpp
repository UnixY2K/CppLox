#pragma once
#include <cpplox/private/exportAPI.hpp>
// C function calls
namespace cpplox {
namespace API {
extern "C" {
LIB_CppLoxAPI void LIB_CPPLOX_free(void *ptr);
LIB_CppLoxAPI int LIB_CPPLOX_SUM(int a, int b);
} // extern "C"
} // namespace API
} // namespace cpplox