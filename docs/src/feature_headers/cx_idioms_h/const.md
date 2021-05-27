# `CX::Const`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T>
concept CX::Const = /*...*/;
```
Const identity concept. Checks whether or not a given type, `T`,
is `const` qualified.

## Satisfaction Criterion
<table id="member-function-table">
 <tr><td>

  ```c++
  template<typename T>
  concept CX::Const = CX::SameType<T, T const>;
  ```
  ---
  The following constraints must be satisfied for this condition:
  1. For a given type, `T`, the resulting type of `T const` must
     be the same as `T`.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

static_assert(CX::Const<int const>);
static_assert(CX::Const<char * const>);
static_assert(CX::Const<float const&>);
static_assert(CX::Const<double const(&&)[]>);
static_assert(!CX::Const<unsigned long>);
```

## See Also
 - [`<cx/common.h>`](../cx_common_h.md)
 - [`CX::ConstDecayed`](./const_decayed.md)
 - [`CX::ConstVolatileDecayed`](./const_volatile_decayed.md)
 - [`CX::ConstVolatilePropagated`](const_volatile_propagated.md)
