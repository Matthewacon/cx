# `CX::DummyValueTemplate`
## Description
<area id="no-interactive-code"></area>
```c++
template<auto...>
struct CX::DummyValueTemplate {};
```
A defined (has linkage) type for deductions in compile-time meta-functions
and conversions in runtime contexts.

## Example Usage
### Non-type template parameter deduction
```c++
#include <cx/common.h>

template<typename>
struct NonTypeTemplateArgumentDeducer;

template<template<auto...> typename T, auto... Values>
struct NonTypeTemplateArgumentDeducer<T<Values...>> {
 template<template<auto...> typename Receiver = CX::DummyValueTemplate>
 using Type = Receiver<Values...>;
};

template<typename T>
using DeduceArguments = typename NonTypeTemplateArgumentDeducer<T>
 ::template Type<>;

template<auto... Values>
struct ExampleNonTypeTemplate {};

int main() {
 ExampleNonTypeTemplate<1, 2, 3, 4, 5, 6> example;

 //`CX::DummyValueTemplate<1, 2, 3, 4, 5, 6>`
 using Arguments = DeduceArguments<decltype(example)>;
}
```

> ℹ️
> See the [`CX::Dummy` examples](./dummy.md#example-usage) for more 
> complex examples.

## See Also
 - [`<cx/common.h>`](../cx_common_h.md)
 - [`CX::Dummy`](./dummy.md)
 - [`CX::DummyTemplate`](./dummy_template.md)
