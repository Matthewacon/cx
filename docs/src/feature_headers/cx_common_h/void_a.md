# `CX::VoidA`
## Description
<area id="no-interactive-code"></area>
```c++
template<auto...>
using CX::VoidA = void;
```
Utility meta-function that maps a pack of any values to the type
`void`. Useful for detecting ill-formed value expressions in
SFINAE contexts.

## Example Usage
### Virtual function identity idiom
```c++
#include <cx/common.h>

template<auto F, typename = void>
struct IsVirtualFunction : CX::TrueType {};

template<auto F>
struct IsVirtualFunction<F, CX::VoidA<F == F>> : CX::FalseType {};

struct VirtualExample {
 virtual void func() {}
};

struct NonVirtualExample {
 void func() {}
};

static_assert(IsVirtualFunction<&VirtualExample::func>::Value);
static_assert(!IsVirtualFunction<&NonVirtualExample::func>::Value);
```

## See Also
 - [`<cx/common.h>`](../cx_common_h.md)
 - [`CX::VoidT`](./void_t.md)
