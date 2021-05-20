# `CX::ImpossibleValueTemplateType`
## Description
<area id="no-interactive-code"></area>
```c++
template<auto...>
struct CX::ImpossibleValueTemplateType;
```
An undefined (no linkage) type for compile-time meta-functions. Useful for
type meta-functions as an error type and optional default parameters for
concepts.

> ⚠️
> [ODR use](https://en.cppreference.com/w/cpp/language/definition#ODR-use) of
> any `CX::ImpossibleValueTemplateType` specialization will cause a compiler
> error, hence its usefullness as an error type for meta-functions.

## Example Usage
### Nested non-template type parameter deduction
```c++
#include <cx/common.h>

//Non-template type parameter deduction meta-function,
//with extra restrictions
template<typename>
struct Deducer {
 template<template<auto...> typename = CX::ImpossibleValueTemplateType>
 using Type = CX::ImpossibleValueTemplateType<>;
};

template<template<auto...> typename T, auto... Values>
requires (T<Values...>::Value)
struct Deducer<T<Values...>> {
 template<template<auto...> typename Receiver = CX::DummyValueTemplate>
 using Type = Receiver<Values...>;
};

template<auto... Values>
struct ExampleValueTemplate {
 static constexpr auto const Value = sizeof...(Values) > 3;
};

//`CX::ImpossibleValueTemplateType<>`
using ArgumentTypes1 = typename Deducer<ExampleValueTemplate<0, 3>>::template Type<>;

//`CX::DummyValueTemplate<3, 4, 5, 6, 7>`
using ArgumentTypes2 = typename Deducer<ExampleValueTemplate<3, 4, 5, 6, 7>>::Type<>;
```

> ℹ️
> See the [`CX::ImpossibleType` examples](./impossible_type.md#example-usage)
> for more complex examples.

## See Also
 - [`<cx/common.h>`](../cx_common_h.md)
 - [`CX::ImpossibleType`](./impossible_type.md)
 - [`CX::ImpossibleTemplateType`](./impossible_template_type.md)
 - [`CX::DummyValueTemplate`](./dummy_value_template.md)
