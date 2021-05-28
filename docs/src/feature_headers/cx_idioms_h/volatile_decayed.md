# `CX::VolatileDecayed`
## Description
<area id="no-interactive-code"></area>
```c++
template<typename T>
using CX::VolatileDecayed = /*...*/;
```
Utility alias to remove immediate `volatile` qualifiers from a
given type, `T`. If `T` is not `volatile` qualified, it will be
unmodified.

## Ambiguous Volatile Qualifications
<table id="member-function-table">
 <tr><td>

  ```c++
  //Unmodified
  int volatile *              -> int volatile *
  float volatile * volatile * -> float volatile * volatile *

  //Modified
  char volatile * volatile -> char volatile *
  short * volatile         -> short *
  ```
  ---
  `volatile` qualifiers that are not applied to the right-most
  non-`volatile` qualifier, will be unmodified.

 </td></tr>
 <tr><td>

  ```c++
  T volatile&  -> T&
  T volatile&& -> T&&
  ```
  ---
  Since references cannot be `volatile` qualified, references to
  `volatile` qualified types will be converted to references to the
  underlying type.

 </td></tr>
 <tr><td>

  ```c++
  T volatile(&)[]  -> T(&)[]
  T volatile(&&)[] -> T(&&)[]
  ```
  ---
  Since refernces to arrays cannot be `volatile` qualified, references
  to `volatile` qualified array element types will be converted to
  references to an array with the underlying array element type.

 </td></tr>
</table>

## Example Usage
```c++
#include <cx/idioms.h>

//`char`
using TypeA = CX::VolatileDecayed<char volatile>;
static_assert(CX::SameType<TypeA, char>);
//`void *(&)[]`
using TypeB = CX::VolatileDecayed<void * volatile(&)[]>;
static_assert(CX::SameType<TypeB, void *(&)[]>);
//`float&&`
using TypeC = CX::VolatileDecayed<float volatile&&>;
static_assert(CX::SameType<TypeC, float&&>);
```

## See Also
 - [`<cx/idioms.h>`](../cx_idioms_h.md)
 - [`CX::Volatile`](./volatile.md)
