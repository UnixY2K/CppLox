#define LIB_CppLoxLIBRARY_EXPORT
#include <cpplox/private/api.hpp>
#include <cstdlib>

namespace cpplox {

namespace API {
// C function calls
extern "C" {

LIB_CppLoxAPI void LIB_CPPLOX_free(void *ptr) { free(ptr); }
}
} // namespace API
} // namespace cpplox