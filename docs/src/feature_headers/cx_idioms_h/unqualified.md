# `CX::Unqualified`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T>
using CX::Unqualified = /*...*/;
```
Utility type meta-function that removes all type qualifiers
from the type `T`.

## Example Usage
```c++
#include <cx/idioms.h>

//`float`
using TypeA = CX::Unqualified<int ** const&>;
static_assert(CX::SameType<TypeA, int>);
//`char`
using TypeB = CX::Unqualified<char *>;
static_assert(CX::SameType<TypeB, char>);
//`void`
using TypeC = CX::Unqualified<void *[1234]>;
static_assert(CX::SameType<TypeC, void>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
