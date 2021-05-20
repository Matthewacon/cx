# `CX::DummyTemplate`
## Description
<area id="no-interactive-code"></area>
```c++
template<template<typename...> typename...>
struct CX::DummyTemplate {};
```
A defined (has linkage) type for deductions in compile-time meta-functions
and conversions in runtime contexts.

## Example Usage
### Template-template parameter deduction
```c++
#include <cx/common.h>
//Note: These headers are not required to use `CX::DummyTemplate`,
//they are a part of the example
#include <vector>
#include <tuple>

TODO
```

> ℹ️
> See the [`CX::Dummy` examples](./dummy.md#example-usage) for more 
> complex examples.

## See Also
 - [`<cx/common.h>`](../cx_common_h.md)
 - [`CX::Dummy`](./dummy.md)
 - [`CX::DummyValueTemplate`](./dummy_value_template.md)
 - [`CX::ImpossibleTemplateType`](./impossible_template_type.md)
