# `CX::VoidT`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename...>
using CX::VoidT = void;
```
Utility meta-function that maps a pack of any types to the type
`void`. Useful for detecting ill-formed type expressions in
SFINAE contexts.

## Example Usage
### SFINAE example
```c++
#include <cx/common.h>

template<typename, typename = void>
struct IsTypeMetaFunction : CX::FalseType {};

template<typename T>
struct IsTypeMetaFunction<T, CX::VoidT<typename T::Type>> : CX::TrueType {};

struct ExampleTypeMetaFunction {
 using Type = float;
};

static_assert(IsTypeMetaFunction<ExampleTypeMetaFunction>::Value);
static_assert(!IsTypeMetaFunction<char>::Value);
```

## See Also
 - [`<cx/common.h>`](../cx_common_h.md)
 - [`CX::VoidA`](./void_a.md)
