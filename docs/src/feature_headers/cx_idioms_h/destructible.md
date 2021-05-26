# `CX::Destructible`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T>
concept CX::Destructible = /*...*/;
```
Utility concept that checks whether or not a given type, `T`,
is destructible.

## Satisfaction Criterion
<table id="member-function-table">
 <tr><td>

  ```c++
  template<typename T>
  concept CX::Destructible = requires (T t) {
   { t.~T() } -> CX::SameType<void>;
  };
  ```
  ---
  The following constraints must be satisfied for this condition:
  1. For any given type, `T`, there must exist a destructor.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

struct A {
 virtual ~A() = default;
};

struct B {
 virtual ~B() = delete;
};

static_assert(CX::Destructible<int>);
static_assert(CX::Destructible<A>);
static_assert(!CX::Destructible<B>);
static_assert(!CX::Destructible<float[1234]>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::Constructible`](./constructible.md)
