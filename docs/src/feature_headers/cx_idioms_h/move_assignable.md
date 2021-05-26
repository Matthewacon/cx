# `CX::MoveAssignable`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T>
concept CX::MoveAssignable = /*...*/;
```
Utility concept that checeks whether or not a given type, `T`,
is move-assignable.

## Satisfaction Criterion
<table id="member-function-table">
 <tr><td>

  ```c++
  template<typename T>
  concept CX::MoveAssignable =
   requires (T t1, T t2) {
    { t1.operator=((T&&)t2) } -> CX::SameType<T&>;
   }
   || requires (T t1, T t2) {
    { t1 = (T&&)t2 } -> CX::SameType<T&>;
   };
  ```
  ---
  Any of the following constraints must be satisfied for this
  condition:
  1. For any given type, `T`, there must exist a member assignment
     operator, <br> `T& T::operator=(T&&)`, that accepts a parameter
     with the type `T&&` and returns a reference to the assigned
     value with the type `T&`.
  2. For any given type, `T`, there must exist an assignment
     operator, `T& operator=(T&&)`, which accepts a parameter with
     the type `T&&` and returns a reference to the assigned value
     with the type `T&`.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

struct A {};

struct B {
 B& operator=(B&&) = delete;
};

static_assert(CX::MoveAssignable<int>);
static_assert(CX::MoveAssignable<A>);
static_assert(!CX::MoveAssignable<B>);
```
## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::MoveConstructible`](./move_constructible.md)
 - [`CX::CopyConstructible`](./copy_constructible.md)
 - [`CX::CopyAssignable`](./copy_assignable.md)
