# `CX::expect`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T>
void CX::expect(T) noexcept(true);
```
A compile-time meta-function that consumes a value of the type
`T`. Useful for inferences in unevaluated contexts.

> ⚠️
> [ODR use](https://en.cppreference.com/w/cpp/language/definition#ODR-use) of
> any `CX::expect` specialization will cause a compiler error.

## Example Usage
```c++
#include <cx/common.h>

template<typename T>
concept HasMemberHWithTypeInt = requires (T t) {
 CX::expect<int>(t.H());
};

struct PositiveExample {
 int H() {
  return 0;
 }
};

struct NegativeExample {
 void H() {}
};

static_assert(HasMemberHWithTypeInt<PositiveExample>);
static_assert(!HasMemberHWithTypeInt<NegativeExample>);
```

## See Also
 - [`<cx/common.h>`](../cx_common_h.md)
