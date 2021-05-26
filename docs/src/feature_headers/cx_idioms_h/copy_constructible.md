# `CX::CopyConstructible`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T>
concept CX::CopyConstructible = /*...*/;
```
Utility concept that checks whether or not a given type, `T`,
is copy-constructible.

## Satisfaction Criterion
<table id="member-function-table">
 <tr><td>

  ```c++
  template<typename T>
  concept CX::CopyConstructible = __is_constructible(T, T const&)
   && requires (T * buf, T const t) {
    { new (buf) T{(T const&)t} } -> CX::SameType<T *>;
   };
  ```
  ---
  The following constraints must be satisfied for this condition:
  1. For a given type, `T`, there must exist a constructor defined
     for `T` that accepts a prameter with the type `T const&`.
  2. For a given type, `T`, there must exist an `operator new`
     defined that allows for construction of `T` with a parameter
     with the type `T const &`.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

struct A {};

struct B {
 B(B const&) = delete;
};

static_assert(CX::CopyConstructible<int>);
static_assert(CX::CopyConstructible<A>);
static_assert(!CX::CopyConstructible<B>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::CopyAssignable`](./copy_assignable.md)
 - [`CX::MoveConstructible`](./move_constructible.md)
 - [`CX::MoveAssignable`](./move_assignable.md)
