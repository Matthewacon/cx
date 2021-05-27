# `CX::Volatile`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T>
concept CX::Volatile = /*...*/;
```
Volatile identity concept. Checks whether or not a given type,
`T`, is `volatile` qualified.

## Satisfaction Criterion
<table id="member-function-table">
 <tr><td>

  ```c++
  template<typename T>
  concept CX::Volatile = CX::SameType<T, T volatile>;
  ```
  ---
  The following constraints must be satisfied for this condition:
  1. For a given type, `T`, the resulting type of `T volatile`
     must be the same as `T`.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

static_assert(CX::Volatile<int volatile>);
static_assert(CX::Volatile<float volatile&>);
static_assert(CX::Volatile<char volatile(&&)[]>);
static_assert(!CX::Volatile<short>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::VolatileDecayed`](./volatile_decayed.md)
 - [`CX::ConstVolatileDecayed`](./const_volatile_decayed.md)
 - [`CX::ConstVolatilePropagated`](./const_volatile_propagated.md)
