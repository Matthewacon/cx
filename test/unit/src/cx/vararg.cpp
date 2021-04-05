#include <cx/test/common/common.h>

//Disable libc support for tests
#undef va_start
#undef va_arg
#undef va_end
#undef CX_LIBC_SUPPORT
#define CX_VARARG_INTRENSICS
#include <cx/vararg.h>

namespace CX {
}
