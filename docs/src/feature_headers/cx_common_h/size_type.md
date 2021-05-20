# `CX::SizeType`
## Description
<area id="no-interactive-code"></area>
```c++
using CX::SizeType = decltype(sizeof(0));
```
`CX::SizeType` is the unsigned integer result of the expression above.
The actual underlying type is implementation defined.

> ℹ️
> Equivalent to [`std::size_t`](https://en.cppreference.com/w/cpp/types/size_t).

## Example Usage
```c++
#include <cx/common.h>

CX::SizeType sizeOfInt = sizeof(int);
```

## See Also
 - [`<cx/common.h>`](../cx_common_h.md)
 - [`CX::AlignType`](./align_type.md)
