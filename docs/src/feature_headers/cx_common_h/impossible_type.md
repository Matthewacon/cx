# `CX::ImpossibleType`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename...>
struct ImpossibleType;
```
An undefined (no linkage) type for compile-time meta-functions. Useful for
type meta-functions as an error type and optional default parameters for
concepts.

> ⚠️
> [ODR use](https://en.cppreference.com/w/cpp/language/definition#ODR-use) of
> any `CX::ImpossibleType` specialization will cause a compiler error, hence
> its usefullness as an error type for meta-functions.

## Example Usage
### Error type meta-function
```c++
#include <cx/common.h>

template<typename>
struct TypeForwarder {
 using Type = CX::ImpossibleType<>;
};

template<typename T>
requires requires {
 typename T::Type;
}
struct TypeForwarder<T> {
 using Type = typename T::Type;
};

struct ExampleTypeMetaFunction {
 using Type = int;
};

//`int`
using ForwardedType1 = typename TypeForwarder<ExampleTypeMetaFunction>::Type;

//`CX::ImpossibleType<>`
using ForwardedType2 = typename TypeForwarder<float>::Type;

int main() {
 ForwardedType1 f1;
 //error: implicit instantiation of undefined template 'CX::ImpossibleType<>'
 ForwardedType2 f2;
}
```

### Optional default parameter
```c++
#include <cx/common.h>
//Note: This header is not required to use `CX::ImpossibleType<>`,
//it is part of the example
#include <cx/idioms.h>

namespace Meta {
 //Utility concept to test for default parameters
 template<typename T>
 concept IsDefaultParameter = CX::SameType<T, CX::ImpossibleType<>>;

 //Base specialization for `FunctionWithPrototype` identity meta-function
 template<typename...>
 struct FunctionWithPrototype {
  template<typename...>
  struct Test : CX::FalseType {};
 };

 //Test specialization for `FunctionWithPrototype` identity meta-function
 template<typename RExpected, typename... ArgsExpected>
 struct FunctionWithPrototype<RExpected, ArgsExpected...> {
  template<typename>
  struct Test : CX::FalseType {};

  template<typename RTest, typename... ArgsTest>
  struct Test<RTest (ArgsTest...)> {
   static constexpr auto const Value =
    //Test `RTest` against `RExpected`, only if `RExpected` is not
    //`CX::ImpossibleType`
    (IsDefaultParameter<RExpected> || (!IsDefaultParameter<RExpected> && CX::SameType<RTest, RExpected>))
    //Test `ArgsTest` against `ArgsExpected`, only if `RExpected` is
    //not `CX::ImpossibleType`
    && (IsDefaultParameter<RExpected> || (!IsDefaultParameter<RExpected> && CX::SameType<CX::Dummy<ArgsTest...>, CX::Dummy<ArgsExpected...>>));
  };
 };
}

//Concept forward definition to simplify identity meta-function usage
template<
 typename FunctionToTest,
 typename ExpectedReturnType = CX::ImpossibleType<>,
 typename... ExpectedArgumentTypes
>
concept FunctionWithPrototype = Meta
 ::FunctionWithPrototype<ExpectedReturnType, ExpectedArgumentTypes...>
 ::template Test<FunctionToTest>
 ::Value;

//Example positive case
static_assert(FunctionWithPrototype<
 void () // - Function prototype to test
>);

//Example positive case
static_assert(FunctionWithPrototype<
 int (float, char *), // - Function prototype to test
 int,                 // - Return type to expect,
 float,               // - Ordered list of arguments
 char *               //   to expect
>);

//Example negative case
static_assert(!FunctionWithPrototype<
 char (float *), // - Function prototype to test
 void            // - Return type to expect
>);
```

## See Also
 - [`<cx/common.h>`](../cx_common_h.md)
 - [`CX::ImpossibleTemplateType`](./impossible_template_type.md)
 - [`CX::ImpossibleValueType`](./impossible_value_type.md)
