# `CX::ImpossibleValueTemplateType`
## Description
<area id="no-interactive-code"></area>
```c++
template<auto...>
struct ImpossibleValueTemplateType;
```
An undefined (no linkage) type for compile-time meta-functions. Useful for
type meta-functions as an error type and optional default parameters for
concepts.

> ⚠️
> [ODR use](https://en.cppreference.com/w/cpp/language/definition#ODR-use) of
> any `CX::ImpossibleValueTemplateType` specialization will cause a compiler
> error, hence its usefullness as an error type for meta-functions.

## Example Usage
```c++
#include <cx/common.h>

TODO
```

> ℹ️
> See the [`CX::ImpossibleType` examples](./impossible_type.md#example-usage)
> for more complex examples.

## See Also
 - [`<cx/common.h>`](../cx_common_h.md)
 - [`CX::ImpossibleType`](./impossible_type.md)
 - [`CX::ImpossibleTemplateType`](./impossible_template_type.md)
