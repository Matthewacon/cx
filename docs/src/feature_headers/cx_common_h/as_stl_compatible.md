# `CX::AsStlCompatible`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T>
using CX::AsStlCompatible = /*...*/;
```
A utility alias meta-function that yields a type that is compatible with STL
meta-functions. The result type will expose either a `type` member alias or
a `value` static member, depending on whether `T` is a type or value
meta-function, respectively.

## Example Usage
```c++
#include <cx/common.h>

//CX-style value meta-function conversion
using StlCompatibleTrue = CX::AsStlCompatible<CX::TrueType>;
static_assert(StlCompatibleTrue::value);

//CX-style type meta-function conversion
struct CxStyleTypeMetaFunction {
 using Type = void;
};

//`void`
using StlCompatibleTypeMetaFunction = CX::AsStlCompatible<CxStyleTypeMetaFunction>;
```

## See Also
 - [`<cx/common.h>`](../cx_common_h.md)
 - [`std::integral_constant`](https://en.cppreference.com/w/cpp/types/integral_constant)
