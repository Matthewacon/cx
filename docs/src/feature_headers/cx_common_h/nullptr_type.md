# `CX::NullptrType`
## Description
<area id="no-interactive-code"></area>
```c++
using CX::NullptrType = decltype(nullptr);
```
`CX::NullptrType` is the type of the null pointer literal, `nullptr`.
The type is not useful for most situations, however, it can be useful as
a default parameter type for base specializations of recursive templates
or SFINAE failures.

> ℹ️
> `CX::NullptrType` is equivalent to [`std::nullptr_t`](https://en.cppreference.com/w/cpp/types/nullptr_t).

## Example Usage
```c++
#include <cx/common.h>

void uselessFunc(CX::NullptrType ptr) {
 //`ptr` will always be `nullptr`
}

int main() {
 uselessFunc(nullptr);
}
```

## See Also
 - [`<cx/common.h>`](../cx_common_h.md)
