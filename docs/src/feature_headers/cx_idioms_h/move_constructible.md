# `CX::MoveConstructible`
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
  concept CX::MoveConstructible = __is_constructible(T, T&&)
   && requires (T * t1, T t2) {
    { new (t1) T {(T&&)t2} } -> SameType<T *>;
   };
  ```
  ---
  The following constraints must be satisfied for this condition:
  1. For any given type, `T`, there must exist a constructor defined
     for `T` that accepts a parameter with the type `T&&`.
  2. For a given type, `T`, there must exist an `operator new`
     defined that allows for construction of `T` with a parameter
     with the type `T&&`.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

struct A {};

struct B {
 B(B&&) = delete;
};

static_assert(CX::MoveConstructible<int>);
static_assert(CX::MoveConstructible<A>);
static_assert(!CX::MoveConstructible<B>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::MoveAssignable`](./move_assignable.md)
 - [`CX::CopyConstructible`](./copy_constructible.md)
 - [`CX::CopyAssignable`](./copy_assignable.md)
