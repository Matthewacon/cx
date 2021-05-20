# `CX::TrueType`
## Description
<area id="no-interactive-code"></area>
```c++
struct CX::TrueType {
 static constexpr auto const Value = true;
};
```
Base for all `bool` value meta-functions with the value `true`.

## Example Usage
### Int identity idiom
```c++
#include <cx/common.h>

template<typename T>
struct IsInt : CX::FalseType {};

template<>
struct IsInt<int> : CX::TrueType {};

static_assert(IsInt<int>::Value);
static_assert(!IsInt<float>::Value);
```

## See Also
 - [`<cx/common.h>`](../cx_common_h.md)
 - [`CX::FalseType`](./false_type.md)
