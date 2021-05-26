# `CX::CopyAssignable`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T>
concept CX::CopyAssignable = /*...*/;
```
Utility concept that checks whether or not a given type, `T`,
is copy-assignable.

## Satisfaction Criterion
<table id="member-function-table">
 <tr><td>

  ```c++
  template<typename T>
  concept CX::CopyAssignable =
   requires (T t1, T t2) {
    { t1.operator=((T const&)t2) } -> CX::SameType<T&>;
   }
   || requires (T t1, T t2) {
    { t1 = (T&&)t2 } -> CX::SameType<T&>;
   };
  ```
  ---
  Any of the following constraints must be satisfied for this
  condition:
  1. For any given type, `T`, there must exist a member assignment
     operator, <br> `T& T::operator=(T const&)`, that accepts a
     parameter with the type `T const&` and returns a reference to
     the assigned value with the type `T&`.
  2. For any given type, `T`, there must exist an assignment
     operator, <br> `T& operator=(T const&)`, which accepts a
     parameter with the type `T const&` and returns a reference to
     the assgined value with the type `T&`.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

struct A {};

struct B {
 B& operator=(B const&) = delete;
};

static_assert(CX::CopyAssignable<int>);
static_assert(CX::CopyAssignable<A>);
static_assert(!CX::CopyAssignable<B>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::CopyConstructible`](./copy_constructible.md)
 - [`CX::MoveConstructible`](./move_constructible.md)
 - [`CX::MoveAssignable`](./move_assignable.md)
