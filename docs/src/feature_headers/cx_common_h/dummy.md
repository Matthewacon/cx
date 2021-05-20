# `CX::Dummy`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename...>
struct CX::Dummy {};
```
A defined (has linkage) type for deductions in compile-time meta-functions
and conversions in runtime contexts.

## Example Usage
### Template parameter type deduction
```c++
#include <cx/common.h>
//Note: This header is not required to use `CX::Dummy`,
//it is part of the example
#include <tuple>

template<typename>
struct ArgumentDeducer;

template<template<typename...> typename T, typename... Args>
struct ArgumentDeducer<T<Args...>> {
 using Type = CX::Dummy<Args...>;
};

template<typename T>
using DeduceArguments = typename ArgumentDeducer<T>::Type;

int main() {
 std::tuple myTuple{1234, 3.14, 'c'};
 //`CX::Dummy<int, double, char>`
 using Arguments = DeduceArguments<decltype(myTuple)>;
}
```

### Member pointer type erasure
```c++
#include <cx/common.h>
//Note: This header is not required to use `CX::Dummy`,
//it is part of the example
#include <iostream>

struct Eraser {
 void (CX::Dummy<>::*erased)();

 template<typename T>
 Eraser(void (T::*memberFunc)()) :
  erased((decltype(erased))memberFunc)
 {}

 void invoke(void * inst) {
  auto dummyInst = (CX::Dummy<>*)inst;
  return (dummyInst->*erased)();
 }
};

struct ExampleType {
 void eraseMe() {
  std::cout << "ExampleType::eraseMe()" << std::endl;
 }
};

int main() {
 Eraser erased{&ExampleType::eraseMe};
 ExampleType example;
 //Prints: `ExampleType::eraseMe()`
 erased.invoke(&example);
}
```

## See Also
 - [`<cx/common.h>`](../cx_common_h.md)
