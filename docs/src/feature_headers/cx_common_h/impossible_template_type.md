# `CX::ImpossibleTemplateType`
## Description
<area id="no-interactive-code"></area>
```c++
template<template<typename...> typename>
struct ImpossibleTemplateType;
```
An undefined (no linkage) type for compile-time meta-functions. Useful for
type meta-functions as an error type and optional default parameters for
concepts.

> ⚠️
> [ODR use](https://en.cppreference.com/w/cpp/language/definition#ODR-use) of
> any `CX::ImpossibleTemplateType` specialization will cause a compiler error,
> hence its usefullness as an error type for meta-functions.

## Example Usage
```c++
#include <cx/common.h>

template<typename>
struct TemplateForwarder {
 template<template<typename...> typename Arg>
 using Type = CX::ImpossibleTemplateType<Arg>;
};

template<typename T>
requires requires {
 typename T::template Type<>;
}
struct TemplateForwarder<T> {
 template<template<typename...> typename... Args>
 using Type = CX::DummyTemplate<Args...>;
};

struct ExampleTemplateMetaFunction {
 template<template<typename...> typename... Args>
 struct Template {};
};

//`ExampleTempalteMetaFunction::Template<CX::Dummy>`
using ForwardedType1 = typename TemplateForwarder<ExampleTemplateMetaFunction>
 ::template Type<CX::Dummy>;

//`CX::ImpossibleTemplateType<CX::Dummy>`
using ForwardedType2 = typename TemplateForwarder<char>
 ::template Type<CX::Dummy>;

int main() {
 ForwardedType1 f1;
 //error: implicit instantiation of undefined template `CX::ImpossibleTemplateType<CX::Dummy>`
 ForwardedType2 f2;
}
```

> ℹ️
> See the [`CX::ImpossibleType` examples](./impossible_type.md#example-usage)
> for more complex examples.

## See Also
 - [`<cx/common.h>`](../cx_common_h.md)
 - [`CX::ImpossibleType`](./impossible_type.md)
 - [`CX::ImpossibleValueTemplateType`](./impossible_value_template_type.md)
