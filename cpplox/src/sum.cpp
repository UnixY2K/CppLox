#define LIB_CppLoxLIBRARY_EXPORT
#include <cpplox/private/api.hpp>

namespace cpplox {

namespace API {
extern "C" {
LIB_CppLoxAPI int LIB_CPPLOX_SUM(int a, int b) { return a + b; }
}
} // namespace API
} // namespace cpplox
