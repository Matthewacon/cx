# `CX::declval`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T>
T&& CX::declval() noexcept(true);
```
A compile-time meta-function that produces the value resulting from the type
expression `T&&`. Useful for inferences in unevaluated contexts.

> ⚠️
> [ODR use](https://en.cppreference.com/w/cpp/language/definition#ODR-use) of
> any `CX::declval` specialization will cause a compiler error.

## Example Usage
```c++
#include <cx/common.h>

template<typename T>
using FuncReturnType = decltype(CX::declval<T>().func());

struct Example_1 {
 void func() {}
};

struct Example_2 {
 int func() {
  return 0;
 }
};

//returns `void`
FuncReturnType<Example_1> doNothing1();

//returns `int`
FuncReturnType<Example_2> doNothing2();
```

## See Also
 - [`<cx/common.h>`](../cx_common_h.md)
