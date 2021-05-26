# `CX::Constructible`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T, typename... Args>
concept CX::Constructible = /*...*/;
```
Utility concept that checks whether or not a given type, `T`,
is constructible given a set of argument types, `Args...`.

## Satisfaction Criterion
<table id="member-function-table">
 <tr><td>

  ```c++
  template<typename T, typename... Args>
  concept CX::Constructible = __is_constructible(T, Args...);
  ```
  ---
  The following constraints must be satisfied for this condition:
  1. For a given type, `T`, there must exist a valid constructor
     that accepts the exact ordered parameter list with the types
     `Args...`.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

struct A {
 A(int, float, char, void *) {}
};

static_assert(CX::Constructible<int>);
static_assert(CX::Constructible<A, float, char, void *>);
static_assert(!CX::Constructible<A>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::Destructible`](./destructible.md)
